#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#include "remotedev_global.h"

#include <extensionsystem/iplugin.h>

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
    ShutdownFlag aboutToShutdown();

private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

