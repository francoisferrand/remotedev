#include "sftpconnection.h"

#include <functional>

#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

#include "sftpchannelexecutor.h"

namespace RemoteDev {

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
    auto *queue = new RemoteJobQueue;
    queue->append(createSubDirsActions(remote, file));

    const auto localFile = local.appendPath(file.toString()).toString();
    const auto remoteFile = remote.appendPath(file.toString()).toString();
    queue->enqueue([localFile, remoteFile, mode] (QSsh::SftpChannel *channel) {
        auto id = channel->uploadFile(localFile, remoteFile,
                                      static_cast<QSsh::SftpOverwriteMode>(mode));
        qDebug() << "SFTP" << "*" << id << ": upload file:"
                 << localFile << " -> " << remoteFile;
        return id;
    });

    return createJob(queue);
}

RemoteJobId SftpConnection::downloadFile(Utils::FileName localDir,
                                         Utils::FileName remoteDir,
                                         const Utils::FileName &file,
                                         OverwriteMode mode)
{
    const auto remoteFile = remoteDir.appendPath(file.toString()).toString();
    const auto localFile = localDir.appendPath(file.toString()).toString();

    // NOTE: .appendPath changes the FileName object
    if (!QDir().mkpath(localDir.parentDir().toString()))
        return REMOTE_INVALID_JOB;

    auto *actions = new RemoteJobQueue;
    actions->enqueue([remoteFile, localFile, mode](QSsh::SftpChannel *channel) {
        auto id = channel->downloadFile(remoteFile, localFile,
                                        static_cast<QSsh::SftpOverwriteMode>(mode));
        qDebug() << "SFTP" << "*" << id << ": download file:"
                 << remoteFile << "->" << localFile;

        return id;
    });

    return createJob(actions);
}

RemoteJobId SftpConnection::uploadDirectory(Utils::FileName local,
                                            Utils::FileName remote,
                                            const Utils::FileName &directory,
                                            OverwriteMode mode)
{
    auto actions = new RemoteJobQueue;
    if (! directory.isEmpty()) {
        actions->append(createSubDirsActions(remote, directory));

        const auto localDir = local.appendPath(directory.toString()).toString();
        const auto remoteDir = remote.appendPath(directory.toString()).parentDir().toString();
        actions->enqueue([localDir, remoteDir] (QSsh::SftpChannel *channel) {
            auto id = channel->uploadDir(localDir, remoteDir);
            qDebug() << "SFTP" << "*" << id << ": upload directory:"
                     << localDir << " -> " << remoteDir;
            return id;
        });
    } else {
        // relative path is empty -> have to upload contents of local to remote
        // have to work around stupid directoty upload policy of SftpChannel
        actions->append(uploadDirContentsActions(local, remote, mode));
    }

    return createJob(actions);
}

void SftpConnection::startJobs() {
    auto channel = m_ssh.createSftpChannel();
    if (! channel) {
        qDebug() << "Failed to create SFTP channel" << m_ssh.errorString();
        emit error(QStringLiteral("Failed to create SFTP channel: %1").arg(m_ssh.errorString()));
        return;
    }

    // helper becomes child of the channel, will be destroyed together with the channel
    auto helper = new Internal::SftpChannelExecutor(channel.data(), std::move(m_actions));
    m_actions.clear();

    connect(helper, &Internal::SftpChannelExecutor::jobFinished,
            this, &SftpConnection::onUploadFinished);
    connect(helper, &Internal::SftpChannelExecutor::jobError,
            this, &SftpConnection::onUploadError);
    connect(helper, &Internal::SftpChannelExecutor::error,
            this, &Connection::error);

    helper->startJobs();
}

void SftpConnection::onSshError(QSsh::SshError errorState)
{
    Q_UNUSED(errorState);
    emit error(m_ssh.errorString());
}

void SftpConnection::onUploadFinished(RemoteJobId job, SftpConnection::RemoteJobQueue *leftovers)
{
    delete leftovers;
    emit Connection::uploadFinished(job);
}

void SftpConnection::onUploadError(RemoteJobId job, const QString &error, SftpConnection::RemoteJobQueue *leftovers)
{
    delete leftovers;
    emit Connection::uploadError(job, error);
}

SftpConnection::RemoteJobQueue SftpConnection::createSubDirsActions(Utils::FileName remoteBase,
                                                                    const Utils::FileName &target)
{
    SftpConnection::RemoteJobQueue actions;

    auto parts = target.toString().split(QLatin1Char('/'));
    parts.removeLast();

    for (const auto &dir : parts) {
        const auto remoteDir = remoteBase.appendPath(dir).toString();
        qDebug() << "Queue: create directory:" << remoteDir;

        actions.enqueue([remoteDir](QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->createDirectory(remoteDir);
            qDebug() << "SFTP"
                     << "*" << id << ": create directory:" << remoteDir;
            return id;
        });
    }

    return actions;
}

SftpConnection::RemoteJobQueue SftpConnection::uploadDirContentsActions(Utils::FileName local,
                                                                        Utils::FileName remote,
                                                                        OverwriteMode mode)
{
    RemoteJobQueue actions;

    // TODO: use QDir with filters when "ignored patterns" feature
    // is implemented
    QDir localParentDir(local.toString());
    auto entries = localParentDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (auto entry : entries) {
        if (entry.isFile()) {
            auto localFile = entry.filePath();
            auto remoteFile = Utils::FileName(remote).appendPath(entry.fileName()).toString();

            qDebug() << "Queue: upload file:" << localFile << "->" << remoteFile;
            actions.enqueue([localFile, remoteFile, mode] (QSsh::SftpChannel *channel) {
                auto action = channel->uploadFile(localFile, remoteFile,
                                                  static_cast<QSsh::SftpOverwriteMode>(mode));
                qDebug() << "SFTP" << "*" << action << ": upload file: " << localFile << "->" << remoteFile;
                return action;
            });
        } else if (entry.isDir()) {
            auto localDir = entry.filePath();

            qDebug() << "Queue: upload directory:" << localDir << "->" << remote.toString();
            actions.enqueue([localDir, remote] (QSsh::SftpChannel *channel) {
                auto action = channel->uploadDir(localDir, remote.toString());
                qDebug() << "SFTP" << "*" << action << ": upload directory" << localDir << "->" << remote.toString();
                return action;
            });
        } else {
            qDebug() << "Queue: unsupported file type for: " << entry.fileName();
        }
    }

    return actions;
}

RemoteJobId SftpConnection::createJob(RemoteJobQueue *actions)
{
    if (!actions) return REMOTE_INVALID_JOB;

    auto jobId = createJobId();
    while (m_actions.contains(jobId))
        jobId = createJobId();

    m_actions.insert(jobId, actions);

    if (m_ssh.state() == QSsh::SshConnection::Connected) {
        startJobs();
    } else {
        m_ssh.connectToHost();
    }

    return jobId;
}

QString SftpConnection::errorString() const
{
    return m_ssh.errorString();
}

} // namespace RemoteDev
