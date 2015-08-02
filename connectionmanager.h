#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "remotedev_global.h"

#include <QMutex>
#include <QSharedPointer>

#include "remoteconnection.h"

namespace RemoteDev {
namespace Internal {

class RemoteDevPlugin;

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    static ConnectionManager *instance();

    /**
     * @brief connectionForAlias - create/get connection to host by alias
     * The function tries to look up a connection in the connection pool,
     * if none found, creates new by a configuration
     * TODO: configuration wizard?
     * @param alias Host alias for lookup
     * @return a connection instance
     */
    static RemoteConnection::SharedPointer connectionForAlias(const QString &alias);

signals:
    void disconnected(RemoteConnection::SharedPointer connection);
    void connectionError(RemoteConnection::SharedPointer connection);

private:
    explicit ConnectionManager(QObject *parent = 0);
    virtual ~ConnectionManager();

private slots:
    void onDisconnected();
    void onConnectionError();

private:
    static ConnectionManager *m_instance;

    QHash<QString, RemoteConnection::SharedPointer> m_connectionPool;
    QMutex m_connectionPoolMutex;

    friend class RemoteDev::Internal::RemoteDevPlugin;
};

} // namespace Internal
} // namespace RemoteDev

#endif // CONNECTIONMANAGER_H
