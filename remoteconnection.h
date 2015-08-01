#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include "remotedev_global.h"

#include <ssh/sftpdefs.h>
#include <ssh/sshconnection.h>

namespace Utils {
    class FileName;
}

namespace RemoteDev {
namespace Internal {

typedef QSsh::SftpJobId RemoteJobId;

enum OverwriteMode {
    OverwriteExisting   = QSsh::SftpOverwriteExisting,
    AppendToExisting    = QSsh::SftpAppendToExisting,
    SkipExisting        = QSsh::SftpSkipExisting
};

/**
 * @brief RemoteInvalidJob - value for indicating an invalid job ID
 */
extern REMOTEDEVSHARED_EXPORT const RemoteJobId REMOTE_INVALID_JOB;

/**
 * @brief The RemoteConnection class
 * This class is intended to be a polymorphic wrapper around
 * different connection types: SSH, FTP etc...
 * For the moment there is only SSH connection, refactor as needed
 */
class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<RemoteConnection> SharedPointer;

    explicit RemoteConnection(const QString &alias,
                              const QSsh::SshConnectionParameters &serverInfo,
                              QObject *parent = 0);

    virtual ~RemoteConnection();

    /**
     * @brief alias - get a short reckognizable name for this connection
     * @return the short name
     */
    inline const QString &alias() const;

    /**
     * @brief uploadFile - upload a file using this connection
     * @param local     Path for local file
     * @param remote    Path for remote file
     * @param mode      OverwriteMode
     * @return Reference job ID
     */
    RemoteJobId uploadFile(const Utils::FileName &local,
                           const Utils::FileName &remote,
                           OverwriteMode mode);

    /**
     * @brief errorString - get a description for the last error
     * @return the description
     */
    QString errorString() const;

signals:
    void connected();

    void connectError();
    void uploadError(RemoteJobId job, const QString &reason);
    void uploadFinished(RemoteJobId job);

    void disconnected();

private slots:
    void triggerSftpFileUpload(RemoteJobId job,
                               const QString &localFile,
                               const QString &remoteFile,
                               QSsh::SftpOverwriteMode mode);

    void onSftpUploadFinished(QSsh::SftpJobId job, const QString &error);

private:
    const QString m_alias;

    QSsh::SshConnection m_ssh;

    RemoteJobId m_jobIdCounter;
    QHash<RemoteJobId, QSharedPointer<QSsh::SftpChannel>> m_openedChannels;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTECONNECTION_H
