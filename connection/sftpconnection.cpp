#include "sftpconnection.h"

#include <functional>

#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

#include "sftpchannelhelper.h"

using namespace RemoteDev;

SftpConnection::SftpConnection(const QString &alias,
                               const QSsh::SshConnectionParameters &serverInfo,
                               QObject *parent) :
    Connection(alias, parent),
    m_ssh(serverInfo, parent)
{
    connect(&m_ssh, &QSsh::SshConnection::error,
            this, &SftpConnection::onSshError);
    connect(&m_ssh, &QSsh::SshConnection::connected,
            this, &Connection::connected);
    // FIXME: handle this internally also
    connect(&m_ssh, &QSsh::SshConnection::disconnected,
            this, &Connection::disconnected);

    // local methods
    connect(this, &Connection::connected,
            this, &SftpConnection::startJobs);
}


SftpConnection::~SftpConnection()
{
    disconnect(this, SIGNAL(error()));
    disconnect(this, SIGNAL(disconnected()));
}

RemoteJobId SftpConnection::uploadFile(Utils::FileName local,
                                       Utils::FileName remote,
                                       const Utils::FileName &file,
                                       OverwriteMode mode)
{
    RemoteJobId jobId = ++m_jobIdCounter;

    auto queue = new RemoteJobQueue;
    enqueueCreatePath(*queue, remote, file);

    const auto localFile = local.appendPath(file.toString()).toString();
    const auto remoteFile = remote.appendPath(file.toString()).toString();
    queue->enqueue([localFile, remoteFile, mode] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
        auto id = channel->uploadFile(localFile, remoteFile, (QSsh::SftpOverwriteMode) mode);
        qDebug() << "SFTP" << "*" << id << ": upload file:"
                 << localFile << " -> " << remoteFile;
        return id;
    });

    m_actions.insert(jobId, queue);

    if (m_ssh.state() == QSsh::SshConnection::Connected) {
        // we are already connected -> do the job
        emit connected();
    } else {
        m_ssh.connectToHost();
    }

    return jobId;
}

RemoteJobId SftpConnection::uploadDirectory(Utils::FileName local,
                                            Utils::FileName remote,
                                            const Utils::FileName &directory,
                                            OverwriteMode mode)
{
    RemoteJobId jobId = ++m_jobIdCounter;

    auto actions = new RemoteJobQueue;
    if (! directory.isEmpty()) {
        enqueueCreatePath(*actions, remote, directory);

        const auto localDir = local.appendPath(directory.toString()).toString();
        const auto remoteDir = remote.appendPath(directory.toString()).parentDir().toString();
        actions->enqueue([localDir, remoteDir] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->uploadDir(localDir, remoteDir);
            qDebug() << "SFTP" << "*" << id << ": upload directory:"
                     << localDir << " -> " << remoteDir;
            return id;
        });
    } else {
        // relative path is empty -> have to upload contents of local to remote
        // have to work around stupid directoty upload policy of SftpChannel
        enqueueUploadContents(*actions, local, remote, mode);
    }

    m_actions.insert(jobId, actions);

    if (m_ssh.state() == QSsh::SshConnection::Connected) {
        // we are already connected -> do the job
        emit connected();
    } else {
        m_ssh.connectToHost();
    }

    return jobId;
}

void SftpConnection::startJobs() {
    auto channel = m_ssh.createSftpChannel();
    if (! channel) {
        qDebug() << "Failed to create SFTP channel" << m_ssh.errorString();
        emit error(QStringLiteral("Failed to create SFTP channel: %1").arg(m_ssh.errorString()));
        return;
    }

    // helper becomes of the channel, will be destroyed together with the channel
    auto helper = new Internal::SftpChannelHelper(channel.data(), m_actions);

    connect(helper, &Internal::SftpChannelHelper::jobFinished,
            this, &SftpConnection::uploadFinished);
    connect(helper, &Internal::SftpChannelHelper::jobError,
            this, &Connection::uploadError);
    connect(helper, &Internal::SftpChannelHelper::error,
            this, &Connection::error);

    helper->startJobs();
}

void SftpConnection::onSshError(QSsh::SshError errorState)
{
    Q_UNUSED(errorState);
    emit error(m_ssh.errorString());
}

void SftpConnection::enqueueCreatePath(SftpConnection::RemoteJobQueue &actions,
                                       Utils::FileName remoteBase,
                                       const Utils::FileName &target)
{

    auto parts = target.toString().split(QLatin1Char('/'));
    parts.takeLast();

    for (const auto &dir : parts) {
        const auto remoteDir = remoteBase.appendPath(dir).toString();
        qDebug() << "Queue: create directory:" << remoteDir;

        actions.enqueue([remoteDir] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->createDirectory(remoteDir);
            qDebug() << "SFTP" << "*" << id << ": create directory:" << remoteDir;
            return id;
        });
    }
}

void SftpConnection::enqueueUploadContents(RemoteJobQueue &actions,
                                           Utils::FileName local,
                                           Utils::FileName remote,
                                           OverwriteMode mode)
{
    // TODO: use QDir with filters when "ignored patterns" feature
    // is implemented
    QDir localParentDir(local.toString());
    auto entries = localParentDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (auto entry : entries) {
        if (entry.isFile()) {
            auto localFile = entry.filePath();
            auto remoteFile = Utils::FileName(remote).appendPath(entry.fileName()).toString();

            qDebug() << "Queue: upload file:" << localFile << "->" << remoteFile;
            actions.enqueue([localFile, remoteFile, mode] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
                auto action = channel->uploadFile(localFile, remoteFile, (QSsh::SftpOverwriteMode) mode);
                qDebug() << "SFTP" << "*" << action << ": upload file: " << localFile << "->" << remoteFile;
                return action;
            });
        } else if (entry.isDir()) {
            auto localDir = entry.filePath();

            qDebug() << "Queue: upload directory:" << localDir << "->" << remote.toString();
            actions.enqueue([localDir, remote] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
                auto action = channel->uploadDir(localDir, remote.toString());
                qDebug() << "SFTP" << "*" << action << ": upload directory" << localDir << "->" << remote.toString();
                return action;
            });
        } else {
            qDebug() << "Queue: unsupported file type for: " << entry.fileName();
        }
    }
}

QString SftpConnection::errorString() const
{
    return m_ssh.errorString();
}
