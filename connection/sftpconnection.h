#ifndef SFTPCONNECTION_H
#define SFTPCONNECTION_H

#include <functional>

#include <QHash>
#include <QObject>
#include <QQueue>

#include <ssh/sshconnection.h>

#include "../connection.h"

namespace RemoteDev {

namespace Internal { class SftpChannelExecutor; }

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
                           OverwriteMode mode) override;

    RemoteJobId uploadDirectory(Utils::FileName local,
                                Utils::FileName remote,
                                const Utils::FileName &directory,
                                OverwriteMode mode) override;

    RemoteJobId downloadFile(Utils::FileName localDir,
                             Utils::FileName remoteDir,
                             const Utils::FileName &file,
                             OverwriteMode mode) override;

    QString errorString() const override;

private:
    friend class RemoteDev::Internal::SftpChannelExecutor;
    using RemoteJobQueue = QQueue<std::function<QSsh::SftpJobId (QSsh::SftpChannel *)>>;

    static RemoteJobQueue createSubDirsActions(Utils::FileName remoteBase,
                                            const Utils::FileName &target);

    static RemoteJobQueue uploadDirContentsActions(Utils::FileName local,
                                                   Utils::FileName remote,
                                                   OverwriteMode mode);

    //! Create and run job for provided actions
    //!
    //! \note actions have to be allocated
    RemoteJobId createJob(RemoteJobQueue *actions);

private slots:
    void startJobs();
    void onSshError(QSsh::SshError errorState);

    void onUploadFinished(RemoteJobId job, RemoteJobQueue *leftovers);
    void onUploadError(RemoteJobId job, const QString& error, RemoteJobQueue *leftovers);

private:
    QSsh::SshConnection m_ssh;
    QHash<RemoteJobId, RemoteJobQueue *> m_actions;
};

} // namespace RemoteDev

#endif // SFTPCONNECTION_H
