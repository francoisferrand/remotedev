#ifndef SFTPCHANNELHELPER_H
#define SFTPCHANNELHELPER_H

#pragma once

#include <functional>

#include <QObject>
#include <QHash>

#include "sftpconnection.h"

namespace QSsh { class SftpChannel; }

namespace RemoteDev {
namespace Internal {

class SftpChannelHelper : public QObject
{
    Q_OBJECT
public:
    using RemoteJobQueue = SftpConnection::RemoteJobQueue;
    SftpChannelHelper(QSsh::SftpChannel *parent,
                      QHash<RemoteJobId, RemoteJobQueue *> &actions);

signals:
    void error(const QString &reason = QString());
    void jobFinished(RemoteJobId job);
    void jobError(RemoteJobId job, const QString &error);

    void actionFinished(RemoteJobId job);

protected:
    friend class RemoteDev::SftpConnection;

    void startJobs();

private:
    void finishJob(RemoteJobId job, const QString &error = QString());

private slots:
    void onChannelInitialized();
    void onChannelError(const QString &reason);

    void startNextAction(RemoteJobId job);
    void onActionFinished(QSsh::SftpJobId action, const QString &error);

private:
    QHash<RemoteJobId, RemoteJobQueue *> m_actions;
    QHash<QSsh::SftpJobId, RemoteJobId>  m_jobs;
};

} // namespace Internal
} // namespace RemoteDev

#endif // SFTPCHANNELHELPER_H
