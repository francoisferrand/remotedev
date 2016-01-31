#include "connectionhelper.h"

#include <QTime>

using namespace RemoteDev::Internal;

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
    m_timers.insert(job, timer);
}
