#include "connectionmanager.h"

#include <QMutexLocker>

#include <ssh/sshconnection.h>

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

RemoteConnection::SharedPointer ConnectionManager::connectionForAlias(const QString &alias)
{
    QMutexLocker locker(&m_instance->m_connectionPoolMutex);
    if (! m_instance->m_connectionPool.contains(alias)) {
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

        RemoteConnection::SharedPointer connection(new RemoteConnection(alias, params, m_instance));

        connect(connection.data(), SIGNAL(disconnected()), m_instance, SLOT(onDisconnected()));
        connect(connection.data(), SIGNAL(connectError()), m_instance, SLOT(onConnectionError()));

        m_instance->m_connectionPool.insert(alias, connection);
    }
    locker.unlock();

    return m_instance->m_connectionPool.value(alias);
}

void ConnectionManager::onDisconnected()
{
    RemoteConnection::SharedPointer connection(qobject_cast<RemoteConnection *>(sender()));
    emit disconnected(connection);
}

void ConnectionManager::onConnectionError()
{
    RemoteConnection::SharedPointer connection(qobject_cast<RemoteConnection *>(sender()));
    emit connectionError(connection);
}