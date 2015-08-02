#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <QHash>

#include <extensionsystem/iplugin.h>

#include "remoteconnection.h"

QT_BEGIN_NAMESPACE
class QAction;
class QTime;
QT_END_NAMESPACE

namespace Core { class IEditor; }

namespace RemoteDev {
namespace Internal {

class RemoteDevPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "RemoteDev.json")

public:
    RemoteDevPlugin();
    ~RemoteDevPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    bool delayedInitialize();
    ShutdownFlag aboutToShutdown();

public slots:
    void uploadCurrentDocument();

private slots:
    void triggerAction();

    // ConnectionManager
    void onConnectionError(RemoteConnection::SharedPointer connection);

    // EditorManager
    void onEditorOpened(Core::IEditor *);

    // ActionManager
    void onSaveAction();

private:
    void showDebug(const QString &string) const;

private:
    QHash<RemoteJobId, QSharedPointer<QTime>> m_timers;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

