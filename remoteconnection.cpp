#include "remoteconnection.h"

#include <utils/fileutils.h>

namespace RemoteDev {
namespace Internal {
    const RemoteJobId REMOTE_INVALID_JOB = QSsh::SftpInvalidJob;
} // namespace RemoteDev
} // namespace Internal

using namespace RemoteDev::Internal;

RemoteConnection::RemoteConnection(const QString &alias, QObject *parent) :
    QObject(parent),
    m_jobIdCounter(REMOTE_INVALID_JOB),
    m_alias(alias)
{}

RemoteConnection::~RemoteConnection()
{}

const QString &RemoteConnection::alias() const
{
    return m_alias;
}
