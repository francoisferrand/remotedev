#include "sftpchannelexecutor.h"

#include <QDir>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

namespace RemoteDev {
namespace Internal {

SftpChannelExecutor::SftpChannelExecutor(QSsh::SftpChannel *parent,
                                     QHash<RemoteJobId, RemoteJobQueue *> &&actions) :
    QObject(parent),
    m_actions(std::move(actions))
{
    connect(parent, &QSsh::SftpChannel::initialized,
            this, &SftpChannelExecutor::onChannelInitialized);

    connect(parent, &QSsh::SftpChannel::finished,
            this, &SftpChannelExecutor::onChannelFinished);

    connect(this, &SftpChannelExecutor::actionFinished,
            this, &SftpChannelExecutor::startNextAction);
}

void SftpChannelExecutor::startJobs()
{
    // this forwards to onChannelInitialized() once the channel is initialized
    qobject_cast<QSsh::SftpChannel *>(parent())->initialize();
}

void SftpChannelExecutor::finishJob(RemoteJobId job, const QString &error)
{
    auto actions = m_actions.take(job);

    qDebug() << "SFTP" << job << "*" << ": job finished:" << error;
    if (error.isEmpty()) {
        emit jobFinished(job, actions);
    } else {
        emit jobError(job, error, actions);
    }

    if (m_actions.isEmpty()) {
        qobject_cast<QSsh::SftpChannel *>(parent())->closeChannel();
    }
}

void SftpChannelExecutor::onChannelInitialized()
{
    for (auto id : m_actions.keys()) {
        startNextAction(id);
    }
}

void SftpChannelExecutor::onChannelError(const QString &reason)
{
    m_jobs.clear();
    m_actions.clear();
    qDebug() << "SFTP Channel Error: " << reason;
    emit error(reason);
}

void SftpChannelExecutor::startNextAction(RemoteJobId job)
{
    auto queue = m_actions.value(job);
    if (! queue || queue->isEmpty()) {
        finishJob(job);
        return;
    }

    auto channel = qobject_cast<QSsh::SftpChannel *>(parent());
    auto actionId = queue->dequeue()(channel);
    if (actionId != QSsh::SftpInvalidJob) {
        m_jobs.insert(actionId, job);
    } else {
        qDebug() << "SFTP" << job << actionId
                 << ": action failed: internal error";

        // ignore any errors if this is not the last action
        if (!queue->isEmpty()) {
            startNextAction(job);
        } else {
            finishJob(job, tr("internal error"));
        }
    }
}

void SftpChannelExecutor::onChannelFinished(QSsh::SftpJobId action, const QString &error)
{
    auto job = m_jobs.take(action);
    auto actions = m_actions.value(job);
    if (! actions->isEmpty()) {
        qDebug() << "SFTP" << job << action << ": action finished:" << error;
        startNextAction(job);
    } else {
        finishJob(job, error);
    }
}

} // namespace Internal
} // namespace RemoteDev
