#include "connectionmanager.h"

#include <QMutexLocker>

#include <ssh/sshconnection.h>
#include <projectexplorer/devicesupport/idevice.h>

#include "connection/sftpconnection.h"

using namespace RemoteDev;
using namespace RemoteDev::Internal;

ConnectionManager *ConnectionManager::m_instance;

ConnectionManager::ConnectionManager(QObject *parent) :
    QObject(parent),
    m_connectionPoolMutex(QMutex::NonRecursive)
{
    m_instance = this;
}

ConnectionManager::~ConnectionManager()
{
    disconnect(this, SLOT(onConnectionError()));
    disconnect(this, SLOT(onDisconnected()));
}

ConnectionManager *ConnectionManager::instance()
{
    return m_instance;
}

Connection::Ptr ConnectionManager::connectionForDevice(const ProjectExplorer::IDevice *device)
{
    Connection::Ptr connection(nullptr);

    if (device) {
        QMutexLocker locker(&m_instance->m_connectionPoolMutex);
        if (! m_instance->m_connectionPool.contains(device->id())) {
            QSsh::SshConnectionParameters params = device->sshParameters();

            // Defaul localhost device has invalid parameters!
            // TODO: test if Localhost device works
            if (!params.host.isEmpty() && params.port != 0) {
                connection = Connection::Ptr(new SftpConnection(device->displayName(), params));
                m_instance->m_connectionPool.insert(device->id(), connection);
            }
        } else {
            connection = m_instance->m_connectionPool.value(device->id());
        }
        locker.unlock();
    }

    return connection;
}

void ConnectionManager::onDisconnected()
{
    Connection::Ptr connection(qobject_cast<Connection *>(sender()));
    emit disconnected(connection);
}

void ConnectionManager::onConnectionError()
{
    Connection::Ptr connection(qobject_cast<Connection *>(sender()));
    emit connectionError(connection);
}
