#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include "remotedev_global.h"

#include <ssh/sftpdefs.h>

namespace Utils {
    class FileName;
}

namespace RemoteDev {

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
 */
class Connection : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<Connection> Ptr;

    virtual ~Connection();

    /**
     * @brief alias - get a short recognizable name for this connection
     * @return the short name
     */
    const QString &alias() const;

    /**
     * @brief uploadFile - upload a file using this connection
     * @param local     Path for local file
     * @param remote    Path for remote file
     * @param mode      OverwriteMode
     * @return Reference job ID
     */
    virtual RemoteJobId uploadFile(const Utils::FileName &local,
                                   const Utils::FileName &remote,
                                   OverwriteMode mode) = 0;

    /**
     * @brief errorString - get a description for the last error
     * @return the description
     */
    virtual QString errorString() const = 0;

signals:
    void connected();

    /**
     * @brief error - is emitted when connection error occurs
     * TODO this signal should be emitted together with error information
     */
    void error();
    void uploadError(RemoteJobId job, const QString &reason);
    void uploadFinished(RemoteJobId job);

    void disconnected();

protected:
    explicit Connection(const QString &alias, QObject *parent = nullptr);

protected:
    RemoteJobId m_jobIdCounter;

private:
    const QString m_alias;
};

} // namespace RemoteDev

#endif // REMOTECONNECTION_H
