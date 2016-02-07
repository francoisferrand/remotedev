#include "sftpchannelhelper.h"

#include <QDir>

#include <utils/fileutils.h>
#include <ssh/sftpchannel.h>

using namespace RemoteDev::Internal;

SftpChannelHelper::SftpChannelHelper(QSsh::SftpChannel *parent,
                                     QHash<RemoteJobId, RemoteJobQueue *> &actions) :
    QObject(parent),
    m_actions(actions)
{
    // helper is now responsible for peforming all these actions
    actions.clear();

    connect(parent, &QSsh::SftpChannel::initialized,
            this, &SftpChannelHelper::onChannelInitialized);

    connect(parent, &QSsh::SftpChannel::finished,
            this, &SftpChannelHelper::onActionFinished);

    connect(this, &SftpChannelHelper::actionFinished,
            this, &SftpChannelHelper::startNextAction);
}

void SftpChannelHelper::startJobs()
{
    qobject_cast<QSsh::SftpChannel *>(parent())->initialize();
}

void SftpChannelHelper::finishJob(RemoteDev::RemoteJobId job, const QString &error)
{
    auto actions = m_actions.take(job);
    delete actions;

    qDebug() << "SFTP" << job << "*" << ": job finished:" << error;
    if (error.isEmpty()) {
        emit jobFinished(job);
    } else {
        emit jobError(job, error);
    }

    if (m_actions.isEmpty()) {
        qobject_cast<QSsh::SftpChannel *>(parent())->closeChannel();
    }
}

void SftpChannelHelper::onChannelInitialized()
{
    for (auto id : m_actions.keys()) {
        startNextAction(id);
    }
}

void SftpChannelHelper::onChannelError(const QString &reason)
{
    m_jobs.clear();
    m_actions.clear();
    qDebug() << "SFTP Channel Error: " << reason;
    emit error(reason);
}

void SftpChannelHelper::startNextAction(RemoteDev::RemoteJobId job)
{
    auto channel = qobject_cast<QSsh::SftpChannel *>(parent());

    auto queue = m_actions.value(job);
    if (! queue || queue->isEmpty()) {
        finishJob(job);
        return;
    }

    auto actionId = queue->dequeue()(channel);
    if (actionId != QSsh::SftpInvalidJob) {
        m_jobs.insert(actionId, job);
    } else {
        qDebug() << "SFTP" << job << actionId
                 << ": action failed: internal error";

        // ignore any errors if this is not the last action
        if (! queue->isEmpty()) {
            emit actionFinished(job);
        } else {
            finishJob(job, tr("internal error"));
        }
    }
}

void SftpChannelHelper::onActionFinished(QSsh::SftpJobId action, const QString &error)
{
    auto job = m_jobs.take(action);
    auto actions = m_actions.value(job);
    if (! actions->isEmpty()) {
        qDebug() << "SFTP" << job << action << ": action finished:" << error;
        emit actionFinished(job);
    } else {
        finishJob(job, error);
    }
}
