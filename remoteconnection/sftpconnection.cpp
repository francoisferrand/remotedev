#include "sftpconnection.h"

#include <functional>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

using namespace RemoteDev::Internal;

SftpConnection::SftpConnection(const QString &alias,
                               const QSsh::SshConnectionParameters &serverInfo,
                               QObject *parent) :
    RemoteConnection(alias, parent),
    m_ssh(serverInfo, parent)
{
    connect(&m_ssh, &QSsh::SshConnection::error, this, &RemoteConnection::connectionError);
    connect(&m_ssh, &QSsh::SshConnection::disconnected, this, &RemoteConnection::disconnected);
}


SftpConnection::~SftpConnection()
{
    disconnect(this, SIGNAL(connectionError()));
    disconnect(this, SIGNAL(disconnected()));
}


void SftpConnection::triggerSftpFileUpload(RemoteJobId job,
                                           const QString &localFile,
                                           const QString &remoteFile,
                                           QSsh::SftpOverwriteMode mode)
{
    auto channel = this->m_ssh.createSftpChannel();
    if (! channel) {
        this->onSftpUploadFinished(job, tr("Failed to create channel")
                                        + QString::fromLatin1(": ")
                                        + m_ssh.errorString());
    }

    this->m_openedChannels.insert(job, channel);

    // start upload when channel is initialized
    connect(channel.data(), &QSsh::SftpChannel::initialized,
            [channel, job, localFile, remoteFile, mode, this] () -> void {
                QSsh::SftpJobId job = channel->uploadFile(localFile, remoteFile, mode);
                if (job == QSsh::SftpInvalidJob) {
                    this->onSftpUploadFinished(job, tr("Failed to start upload job")
                                                    + QString::fromLatin1(": ")
                                                    + m_ssh.errorString());
                }
            });

     // handle channel errors (if any)
     connect(channel.data(), &QSsh::SftpChannel::channelError,
             [job, this] (const QString &reason) -> void {
                this->onSftpUploadFinished(job, tr("Channel error")
                                                + QString::fromLatin1(": ")
                                                + reason);
             });

     // handle successful upload
     connect(channel.data(), &QSsh::SftpChannel::finished,
             [job, this] (QSsh::SftpJobId, const QString &error) -> void {
                 this->onSftpUploadFinished(job, error);
             });

     channel->initialize();
}


RemoteJobId SftpConnection::uploadFile(const Utils::FileName &local,
                                       const Utils::FileName &remote,
                                       OverwriteMode mode)
{
    RemoteJobId result = ++m_jobIdCounter;

    auto doUpload = std::bind(&SftpConnection::triggerSftpFileUpload,
                              this, result, local.toString(), remote.toString(),
                              static_cast<QSsh::SftpOverwriteMode>(mode));

    if (m_ssh.state() == QSsh::SshConnection::Connected) {
        doUpload();
    } else {
        // FIXME: dirty workaround for connecting the handler only once
        static bool uploadAttached = false;
        if (! uploadAttached) {
            uploadAttached = true;
            connect(&m_ssh, &QSsh::SshConnection::connected, doUpload);
        }

        if (m_ssh.state() == QSsh::SshConnection::Unconnected) {
            // In case of state == Connecting no re-connection is required
            m_ssh.connectToHost();
        }
    }

    return result;
}

QString SftpConnection::errorString() const
{
    return m_ssh.errorString();
}


void SftpConnection::onSftpUploadFinished(QSsh::SftpJobId job, const QString &error)
{
    if (error.isEmpty()) {
        emit uploadFinished(job);
    } else {
        emit uploadError(job, error);
    }

    auto channel = m_openedChannels.value(job);
    if (channel) {
        channel->closeChannel();
        m_openedChannels.remove(job);
    }
}
