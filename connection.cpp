#include "connection.h"

#include <utils/fileutils.h>

namespace RemoteDev {
    const RemoteJobId REMOTE_INVALID_JOB = QSsh::SftpInvalidJob;
} // namespace Internal

using namespace RemoteDev;

Connection::Connection(const QString &alias, QObject *parent) :
    QObject(parent),
    m_jobIdCounter(REMOTE_INVALID_JOB),
    m_alias(alias)
{}

Connection::~Connection()
{}

const QString &Connection::alias() const
{
    return m_alias;
}
