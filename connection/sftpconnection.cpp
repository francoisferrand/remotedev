#include "sftpconnection.h"

#include <functional>

#include <QDir>
#include <QFileInfo>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

using namespace RemoteDev;

SftpConnection::SftpConnection(const QString &alias,
                               const QSsh::SshConnectionParameters &serverInfo,
                               QObject *parent) :
    Connection(alias, parent),
    m_ssh(serverInfo, parent)
{
    connect(&m_ssh, &QSsh::SshConnection::error, this, &Connection::error);
    connect(&m_ssh, &QSsh::SshConnection::disconnected, this, &Connection::disconnected);
}


SftpConnection::~SftpConnection()
{
    disconnect(this, SIGNAL(error()));
    disconnect(this, SIGNAL(disconnected()));
}


// FIXME: remote paths may have different fileseparator than native!
void SftpConnection::triggerSftpFileUpload(RemoteJobId job,
                                           const Utils::FileName &local,
                                           const Utils::FileName &remote,
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
        [channel, job, local, remote, mode, this] () -> void {
            QVector<QString> commonPart;

            auto remoteBase = remote.parentDir();
            auto localBase = local.parentDir();

            while (remoteBase.fileName() == localBase.fileName()) {
                commonPart.append(remoteBase.fileName());

                remoteBase = remoteBase.parentDir();
                localBase = localBase.parentDir();
            }

            for (auto &part : commonPart) {
                remoteBase.appendPath(part);
                auto sftpJob = channel->createDirectory(remoteBase.toString());

                // FIXME: next directory should be created only after the previous job finishes
                if (sftpJob == QSsh::SftpInvalidJob) {
                    this->onSftpUploadFinished(job,
                                               tr("Failed to create remote directory: ")
                                               + m_ssh.errorString());
                    return;
                }
            }


            QSsh::SftpJobId sftpJob = channel->uploadFile(local.toString(),
                                                          remote.toString(), mode);
            if (sftpJob != QSsh::SftpInvalidJob) {
                // and now, handle the last upload success
                connect(channel.data(), &QSsh::SftpChannel::finished,
                    [job, sftpJob, this] (QSsh::SftpJobId doneJob, const QString &error) -> void {
                        if (doneJob == sftpJob) {
                            this->onSftpUploadFinished(job, error);
                        }
                    }
                );
            } else {
                this->onSftpUploadFinished(job, tr("Failed to start upload job")
                                                + QString::fromLatin1(": ")
                                                + m_ssh.errorString());
            }
        }
    );

     // handle channel errors (if any)
     connect(channel.data(), &QSsh::SftpChannel::channelError,
         [job, this] (const QString &reason) -> void {
            this->onSftpUploadFinished(job, tr("Channel error")
                                            + QString::fromLatin1(": ")
                                            + reason);
         }
     );

//     // handle successful upload
//     connect(channel.data(), &QSsh::SftpChannel::finished,
//         [job, this] (QSsh::SftpJobId, const QString &error) -> void {
//             this->onSftpUploadFinished(job, error);
//         }
//     );

     channel->initialize();
}


RemoteJobId SftpConnection::uploadFile(const Utils::FileName &local,
                                       const Utils::FileName &remote,
                                       OverwriteMode mode)
{
    RemoteJobId result = ++m_jobIdCounter;

    auto doUpload = std::bind(&SftpConnection::triggerSftpFileUpload,
                              this, result, local, remote,
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
