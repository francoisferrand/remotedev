#include "connectionmanager.h"

#include <QMutexLocker>

#include <ssh/sshconnection.h>

#include "remoteconnection/sftpconnection.h"

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

        // FIXME: connection factory?
        RemoteConnection::SharedPointer connection(new SftpConnection(alias, params, m_instance));

        connect(connection.data(), &RemoteConnection::disconnected,
                m_instance, &ConnectionManager::onDisconnected);
        connect(connection.data(), &RemoteConnection::connectionError,
                m_instance, &ConnectionManager::onConnectionError);

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
