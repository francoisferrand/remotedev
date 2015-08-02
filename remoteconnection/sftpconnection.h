#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include "../remoteconnection.h"

#include <ssh/sshconnection.h>

namespace RemoteDev {
namespace Internal {

class SftpConnection : public RemoteConnection
{
public:
    explicit SftpConnection(const QString &alias,
                            const QSsh::SshConnectionParameters &serverInfo,
                            QObject *parent = 0);
    ~SftpConnection();

    RemoteJobId uploadFile(const Utils::FileName &local,
                           const Utils::FileName &remote,
                           OverwriteMode mode);

    QString errorString() const;

private slots:
    void triggerSftpFileUpload(RemoteJobId job,
                               const QString &localFile,
                               const QString &remoteFile,
                               QSsh::SftpOverwriteMode mode);

    void onSftpUploadFinished(QSsh::SftpJobId job, const QString &error);

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, QSharedPointer<QSsh::SftpChannel>> m_openedChannels;
};

} // namespace Internal
} // namespace RemoteDev


#endif // SFTPCONNECTION_H
