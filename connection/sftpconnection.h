#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include <functional>

#include <QObject>
#include <QQueue>
#include <QHash>

#include <ssh/sshconnection.h>

#include "../connection.h"

namespace RemoteDev {

namespace Internal { class SftpChannelHelper; }

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

private:
    friend class RemoteDev::Internal::SftpChannelHelper;
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
    void onSshError(QSsh::SshError errorState);

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, RemoteJobQueue *> m_actions;
};

} // namespace RemoteDev

#endif // SFTPCONNECTION_H
