#include "connection.h"

#include <utils/fileutils.h>

namespace RemoteDev {

const RemoteJobId REMOTE_INVALID_JOB = QSsh::SftpInvalidJob;

Connection::Connection(const QString &alias, QObject *parent) :
    QObject(parent),
    m_alias(alias)
{}

RemoteJobId Connection::createJobId() const
{
    return ++m_jobIdCounter;
}

Connection::~Connection()
{}

const QString &Connection::alias() const
{
    return m_alias;
}

} // namespace RemoteDev
