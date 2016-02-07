#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include <functional>

#include <QObject>
#include <QQueue>
#include <QHash>

#include <ssh/sshconnection.h>

#include "../connection.h"

namespace RemoteDev {

// TERMINOLOGY
// Job      a queue of actions to achieve some result, e.g.
//          upload file and create parent directories (if necessary)
// Action   a single step on the way to complete a job, e.g.
//          create directory

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

    static void enqueueCreatePath(RemoteJobQueue &actions,
                                  Utils::FileName remoteBase,
                                  const Utils::FileName &target);

    static void enqueueUploadContents(RemoteJobQueue &actions,
                                      Utils::FileName local,
                                      Utils::FileName remote,
                                      OverwriteMode mode);

private slots:
    void startJobs();

    void startNextAction(QSsh::SftpChannel *channel, RemoteJobId job);
    void onActionFinished(QSsh::SftpChannel *channel,
                          QSsh::SftpJobId action, const QString &reason);

    void onJobFinished(QSsh::SftpChannel *channel,
                       RemoteJobId job, const QString &reason = QString());

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, RemoteJobQueue *> m_actions;
    QHash<QSsh::SftpJobId, RemoteJobId> m_jobs;
};

} // namespace RemoteDev

#endif // SFTPCONNECTION_H
