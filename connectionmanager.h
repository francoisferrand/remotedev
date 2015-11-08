#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "remotedev_global.h"

#include <QMutex>
#include <QSharedPointer>

// FIXME: declare these locally: Core!
//#include <coreplugin/id.h>
//#include <projectexplorer/devicesupport/idevice.h>

namespace Core { class Id; }
namespace ProjectExplorer { class IDevice; }

#include "connection.h"

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
    static Connection::Ptr connectionForAlias(const QString &alias);

    static Connection::Ptr connectionForDevice(const ProjectExplorer::IDevice *device);

signals:
    void disconnected(Connection::Ptr connection);
    void connectionError(Connection::Ptr connection);

private:
    explicit ConnectionManager(QObject *parent = 0);
    virtual ~ConnectionManager();

private slots:
    void onDisconnected();
    void onConnectionError();

private:
    static ConnectionManager *m_instance;

    QHash<QString, Connection::Ptr> m_connectionPool_;

    QHash<Core::Id, Connection::Ptr> m_connectionPool;
    QMutex m_connectionPoolMutex;

    friend class RemoteDev::Internal::RemoteDevPlugin;
};

} // namespace Internal
} // namespace RemoteDev

#endif // CONNECTIONMANAGER_H
