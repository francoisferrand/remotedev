#ifndef REMOTEDEV_H
#define REMOTEDEV_H

#pragma once

#include "remotedev_global.h"

#include <extensionsystem/iplugin.h>

#include "connection.h"

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

namespace Core { class IEditor; }
namespace ProjectExplorer { class Project; }

namespace RemoteDev {
namespace Internal {

class OptionsPage;
class ConnectionManager;
class MappingsManager;
class DeviceManager;

class RemoteDevPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "RemoteDev.json")

public:
    RemoteDevPlugin();
    ~RemoteDevPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    bool delayedInitialize() override;
    ShutdownFlag aboutToShutdown() override;

private:
    using UploadMethod = RemoteJobId (Connection::*)(Utils::FileName,
                                                     Utils::FileName,
                                                     const Utils::FileName&,
                                                     OverwriteMode);

public slots:
    void uploadCurrentDocument();
    void uploadCurrentNode();

private slots:
    void triggerAction();

    // ConnectionManager
    void onConnectionError(Connection::Ptr connection);

    void upload(const Utils::FileName &file,
                ProjectExplorer::Project *project,
                UploadMethod uploadMethod);

private:
    void createOptionsPage();
    void createProjectSettingsPage();
    void createFileMenus();

    void showDebug(const QString &string) const;

private:
    OptionsPage *m_optionsPage;

    QAction *m_uploadFile;
    QAction *m_uploadDirectory;

    ConnectionManager *m_connManager;
    MappingsManager   *m_mapManager;
    DeviceManager     *m_devManager;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEDEV_H

