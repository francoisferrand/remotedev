#include "sftpconnection.h"

#include <functional>

#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

using namespace RemoteDev;

SftpConnection::SftpConnection(const QString &alias,
                               const QSsh::SshConnectionParameters &serverInfo,
                               QObject *parent) :
    Connection(alias, parent),
    m_ssh(serverInfo, parent)
{
    connect(&m_ssh, &QSsh::SshConnection::error,
            this, &Connection::error);
    connect(&m_ssh, &QSsh::SshConnection::disconnected,
            this, &Connection::disconnected);
    connect(&m_ssh, &QSsh::SshConnection::connected,
            this, &Connection::connected);

    // local methods
    connect(this, &Connection::connected,
            this, &SftpConnection::startJobs);

    connect(this, &SftpConnection::actionFinished,
            this, &SftpConnection::takeJobAction);
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
    queue->enqueue([this, localFile, remoteFile, mode] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
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
        actions->enqueue([this, localDir, remoteDir/*, mode*/] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->uploadDir(localDir, remoteDir);
            qDebug() << "SFTP" << "*" << id << ": upload directory:"
                     << localDir << " -> " << remoteDir;
            return id;
        });
    } else {
        // relative path is empty -> have to upload contents of local to remote
        // have to work around stupid directoty upload policy of SftpChannel
        enqueueUploadContents(actions, local, remote, mode);
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
        emit error();
        return;
    }

    connect(channel.data(), &QSsh::SftpChannel::channelError,
        [this] (const QString &reason) {
            qDebug() << "Channel error:" << reason;
            emit error();
        }
    );

    connect(channel.data(), &QSsh::SftpChannel::initialized,
        [this, channel] () {
            // FIXME: handle case when actions are empty at the start
            for (auto id : m_actions.keys()) {
                takeJobAction(channel.data(), id);
            }
        }
    );

    connect(channel.data(), &QSsh::SftpChannel::finished,
        [this, channel] (QSsh::SftpJobId action, const QString &reason) {
            this->onActionFinished(channel.data(), action, reason);
        }
    );

    channel->initialize();
}

void SftpConnection::takeJobAction(QSsh::SftpChannel *channel, RemoteJobId job)
{
    auto queue = m_actions.value(job);
    if (! queue || queue->isEmpty()) return;

    auto actionId = queue->dequeue()(channel);
    if (actionId != QSsh::SftpInvalidJob) {
        m_jobs.insert(actionId, job);
    } else {
        qDebug() << "SFTP" << job << actionId
                 << ": action failed" <<  m_ssh.errorString();

        if (! queue->isEmpty()) {
            emit actionFinished(channel, job);
        } else {
            const QString error = m_ssh.errorString().isEmpty()
                    ? QStringLiteral("internal error")
                    : m_ssh.errorString();

            onJobFinished(channel, job, error);
        }
    }
}

void SftpConnection::onActionFinished(QSsh::SftpChannel *channel,
                                      QSsh::SftpJobId action, const QString &reason)
{
    auto job = m_jobs.take(action);
    auto actions = m_actions.value(job);
    if (! actions->isEmpty()) {
        qDebug() << "SFTP" << job << action << ": action finished:" << reason;
        emit actionFinished(channel, job);
    } else {
        onJobFinished(channel, job, reason);
    }
}

void SftpConnection::onJobFinished(QSsh::SftpChannel *channel,
                                   RemoteJobId job, const QString &reason)
{
    auto actions = m_actions.take(job);
    delete actions;

    // when there is no more jobs running around,
    // the channel can be closed
    if (m_actions.isEmpty()) {
        channel->closeChannel();
    }

    qDebug() << "SFTP" << job << "*" << ": job finished:" << reason;
    if (reason.isEmpty()) {
        emit uploadFinished(job);
    } else {
        emit uploadError(job, reason);
    }
}

void SftpConnection::enqueueCreatePath(SftpConnection::RemoteJobQueue &queue,
                                       Utils::FileName remoteBase,
                                       const Utils::FileName &target)
{

    auto parts = target.toString().split(QLatin1Char('/'));
    parts.takeLast();

    for (const auto &dir : parts) {
        const auto remoteDir = remoteBase.appendPath(dir).toString();
        qDebug() << "Queue: create directory:" << remoteDir;

        queue.enqueue([this, remoteDir] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->createDirectory(remoteDir);
            qDebug() << "SFTP" << "*" << id << ": create directory:" << remoteDir;
            return id;
        });
    }
}

void SftpConnection::enqueueUploadContents(RemoteJobQueue *actions,
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
            actions->enqueue([localFile, remoteFile, mode] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
                auto action = channel->uploadFile(localFile, remoteFile, (QSsh::SftpOverwriteMode) mode);
                qDebug() << "SFTP" << "*" << action << ": upload file: " << localFile << "->" << remoteFile;
                return action;
            });
        } else if (entry.isDir()) {
            auto localDir = entry.filePath();

            qDebug() << "Queue: upload directory:" << localDir << "->" << remote.toString();
            actions->enqueue([localDir, remote] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
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
