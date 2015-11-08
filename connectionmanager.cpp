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

Connection::Ptr ConnectionManager::connectionForAlias(const QString &alias)
{
    // DeviceManager?

    QMutexLocker locker(&m_instance->m_connectionPoolMutex);
    if (! m_instance->m_connectionPool_.contains(alias)) {
        QSsh::SshConnectionParameters params;
        params.host = QString::fromLatin1("localhost");
        params.userName = QString::fromLatin1("elvenfighter");
        params.privateKeyFile = QString::fromLatin1("/home/elvenfighter/Projects/keys/localhost.rsa");
        params.timeout = 30;
        params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
        params.port = 22;
//        params.options &= ~QSsh::SshEnableStrictConformanceChecks;
//        params.hostKeyCheckingMode = QSsh::SshHostKeyCheckingStrict;
//        params.hostKeyDatabase = QSsh::SshHostKeyDatabasePtr::create();

        // FIXME: connection factory?
        Connection::Ptr connection(new SftpConnection(alias, params, m_instance));

        connect(connection.data(), &Connection::disconnected,
                m_instance, &ConnectionManager::onDisconnected);
        connect(connection.data(), &Connection::error,
                m_instance, &ConnectionManager::onConnectionError);

        m_instance->m_connectionPool_.insert(alias, connection);
    }
    locker.unlock();

    return m_instance->m_connectionPool_.value(alias);
}

Connection::Ptr ConnectionManager::connectionForDevice(const ProjectExplorer::IDevice *device)
{
    Connection::Ptr connection(nullptr);

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

    return connection;
}

//Connection::Ptr ConnectionManager::connectionForAlias()

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
