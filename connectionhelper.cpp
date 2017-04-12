#include "connectionhelper.h"

#include "connection.h"

#include <QTime>

namespace RemoteDev {
namespace Internal {

ConnectionHelper::ConnectionHelper(RemoteDev::Connection *parent) :
    QObject(parent)
{}

int ConnectionHelper::endJob(RemoteDev::RemoteJobId job)
{
    auto timer = m_timers.take(job);
    return timer.isNull() ? -1 : timer->elapsed();
}

void ConnectionHelper::startJob(std::function<RemoteDev::RemoteJobId ()> code)
{
    auto timer = QSharedPointer<QTime>(new QTime);
    timer->start();

    auto job = code();
    if (job != REMOTE_INVALID_JOB) {
        m_timers.insert(job, timer);
    }
}

} // namespace Internal
} // namespace RemoteDev
