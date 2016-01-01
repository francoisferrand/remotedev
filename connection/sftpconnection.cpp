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

RemoteJobId SftpConnection::uploadFile(const Utils::FileName &local,
                                       const Utils::FileName &remote,
                                       const Utils::FileName &file,
                                       OverwriteMode mode)
{
    RemoteJobId jobId = ++m_jobIdCounter;

    m_jobActions.insert(jobId, createJobQueue(local, remote, file, mode));

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
            for (auto id : m_jobActions.keys()) {
                takeJobAction(channel.data(), id);
            }
        }
    );

    connect(channel.data(), &QSsh::SftpChannel::finished,
        [this, channel] (QSsh::SftpJobId action, const QString &reason) {
            auto job = m_actionJobs.take(action);

            auto queue = m_jobActions.value(job);
            if (! queue->isEmpty()) {
                qDebug() << "SFTP" << job << action << ": action finished:" << reason;
                emit actionFinished(channel.data(), job);
            } else {
                m_jobActions.remove(job);
                delete queue;

                // FIXME: cannot rely on this in multi-threaded environment
                if (m_jobActions.isEmpty()) {
                    channel->closeChannel();
                }

                qDebug() << "SFTP" << job << action << ": job finished:" << reason;
                if (reason.isEmpty()) {
                    emit uploadFinished(job);
                } else {
                    emit uploadError(job, reason);
                }
            }
        }
    );

    channel->initialize();
}

void SftpConnection::takeJobAction(QSsh::SftpChannel *channel, RemoteJobId job)
{
    auto queue = m_jobActions.value(job);
    if (! queue || queue->isEmpty()) return;

    auto actionId = queue->dequeue()(channel);
    if (actionId != QSsh::SftpInvalidJob) {
        m_actionJobs.insert(actionId, job);
    } else {
        qDebug() << "SFTP" << job << actionId
                 << ": action failed" <<  m_ssh.errorString();

        if (! queue->isEmpty()) {
            emit actionFinished(channel, job);
        } else {
            // FIXME: cleanup queue code duplication
            m_jobActions.remove(job);
            delete queue;

            if (m_jobActions.isEmpty()) {
                channel->closeChannel();
            }

            emit uploadError(job, m_ssh.errorString());
        }
    }
}

SftpConnection::RemoteJobQueue *SftpConnection::createJobQueue(const Utils::FileName &local,
                                                               const Utils::FileName &remote,
                                                               const Utils::FileName &file,
                                                               OverwriteMode mode)
{
    auto result = new RemoteJobQueue;

    Utils::FileName remotePath = remote;
    auto parts = file.toString().split(QLatin1Char('/'));
    parts.takeLast();
    for (auto dir : parts) {
        const auto remoteDir = remotePath.appendPath(dir).toString();
        qDebug() << "Queue: create directory:" << remoteDir;

        result->enqueue([this, remoteDir] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->createDirectory(remoteDir);
            qDebug() << "SFTP" << "*" << id << ": create directory:" << remoteDir;
            return id;
        });
    }

    const auto localFile = Utils::FileName(local).appendPath(file.toString()).toString();
    const auto remoteFile = Utils::FileName(remote).appendPath(file.toString()).toString();
    qDebug() << "Queue: upload file:" << localFile << " -> " << remoteFile;

    result->enqueue(
        [this, localFile, remoteFile, mode] (QSsh::SftpChannel *channel) -> QSsh::SftpJobId {
            auto id = channel->uploadFile(localFile, remoteFile, (QSsh::SftpOverwriteMode) mode);
            qDebug() << "SFTP" << "*" << id << ": upload file:"
                     << localFile << " -> " << remoteFile;
            return id;
        }
    );

    return result;
}

QString SftpConnection::errorString() const
{
    return m_ssh.errorString();
}
