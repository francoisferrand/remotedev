#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include "../connection.h"

#include <ssh/sshconnection.h>

namespace RemoteDev {

class SftpConnection : public Connection
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

    enum AuthenticationType {
        AuthenticationTypePassword = QSsh::SshConnectionParameters::AuthenticationTypePassword,
        AuthenticationTypePublicKey = QSsh::SshConnectionParameters::AuthenticationTypePublicKey,
        AuthenticationTypeKeyboardInteractive = QSsh::SshConnectionParameters::AuthenticationTypeKeyboardInteractive,

        // this one is not provided by QSsh
        AuthenticationTypeHostBased = QSsh::SshConnectionParameters::AuthenticationTypeTryAllPasswordBasedMethods + 1,

        // Some servers disable "password", others disable "keyboard-interactive".
        AuthenticationTypeTryAllPasswordBasedMethods = QSsh::SshConnectionParameters::AuthenticationTypeTryAllPasswordBasedMethods
    };
    Q_ENUM(AuthenticationType)

private slots:
    void triggerSftpFileUpload(RemoteJobId job,
                               const Utils::FileName &local,
                               const Utils::FileName &remote,
                               QSsh::SftpOverwriteMode mode);

    void onSftpUploadFinished(QSsh::SftpJobId job, const QString &error);

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, QSharedPointer<QSsh::SftpChannel>> m_openedChannels;
};

} // namespace RemoteDev


#endif // SFTPCONNECTION_H
