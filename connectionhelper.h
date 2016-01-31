#ifndef CONNECTIONHELPER_H
#define CONNECTIONHELPER_H

#pragma once

#include <functional>

#include <QObject>
#include <QHash>
#include <QSharedPointer>

#include "connection.h"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

class ConnectionHelper : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHelper(Connection *parent);

    int endJob(RemoteJobId job);

public slots:
    void startJob(std::function<RemoteJobId ()> code);

private:
    QHash<RemoteJobId, QSharedPointer<QTime>> m_timers;
};

} // namespace Internal
} // namespace RemoteDev

#endif // CONNECTIONHELPER_H
