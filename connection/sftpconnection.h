#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include <functional>

#include <QObject>
#include <QQueue>
#include <QHash>

#include <ssh/sshconnection.h>

#include "../connection.h"

namespace RemoteDev {

class SftpConnection : public Connection
{
    Q_OBJECT

public:
    explicit SftpConnection(const QString &alias,
                            const QSsh::SshConnectionParameters &serverInfo,
                            QObject *parent = 0);

    ~SftpConnection();

    RemoteJobId uploadFile(Utils::FileName local,
                           Utils::FileName remote,
                           const Utils::FileName &file,
                           OverwriteMode mode);

    RemoteJobId uploadDirectory(Utils::FileName local,
                                Utils::FileName remote,
                                const Utils::FileName &directory,
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

signals:
    void actionFinished(QSsh::SftpChannel *channel, RemoteJobId id);

private:
    typedef QQueue<std::function<QSsh::SftpJobId (QSsh::SftpChannel *)>> RemoteJobQueue;

    void enqueueCreatePath(RemoteJobQueue &queue,
                           Utils::FileName remoteBase,
                           const Utils::FileName &target);

private slots:
    void startJobs();

    void takeJobAction(QSsh::SftpChannel *channel, RemoteJobId id);

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, RemoteJobQueue *> m_jobActions;
    QHash<QSsh::SftpJobId, RemoteJobId> m_actionJobs;
};

} // namespace RemoteDev

#endif // SFTPCONNECTION_H
