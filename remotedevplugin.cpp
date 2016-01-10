#include "remotedevplugin.h"
#include "remotedevconstants.h"

#include <QAction>
#include <QMenu>
#include <QTime>
#include <QStandardItemModel>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <projectexplorer/devicesupport/devicemanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectpanelfactory.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>

#include "connectionmanager.h"
#include "connectionspage.h"
#include "projectsettingswidget.h"
#include "devicemanager.h"
#include "mappingsmanager.h"

#include <QDebug>

using namespace RemoteDev::Internal;

RemoteDevPlugin::RemoteDevPlugin() :
    m_uploadFile(nullptr),
    m_uploadDirectory(nullptr),
    m_connManager(new ConnectionManager),
    m_mapManager(new MappingsManager),
    m_devManager(new DeviceManager)
{}

RemoteDevPlugin::~RemoteDevPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    delete m_connManager;
    delete m_mapManager;
    delete m_devManager;
}

bool RemoteDevPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    QAction *action = new QAction(tr("RemoteDev action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(
                action, Constants::ACTION_ID,
                //Core::Context(Core::Constants::C_NAVIGATION_PANE)
                Core::Context(Core::Constants::C_GLOBAL)
    );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("RemoteDev"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    auto *editorManager = Core::EditorManager::instance();
    // NOTE: currentEditorChanged is also triggered upon editorOpened
    //connect(editorManager, &Core::EditorManager::editorOpened,
    //        this, &RemoteDevPlugin::onEditorOpened);
    connect(editorManager, &Core::EditorManager::currentEditorChanged,
            this, &RemoteDevPlugin::onEditorOpened);

    QAction *saveAction = Core::ActionManager::command(Core::Constants::SAVE)->action();
    connect(saveAction, &QAction::triggered,
            this, &RemoteDevPlugin::onSaveAction);

    ConnectionManager *connectionManager = ConnectionManager::instance();
    connect(connectionManager, &ConnectionManager::connectionError,
            this, &RemoteDevPlugin::onConnectionError);

    createOptionsPage();
    createProjectSettingsPage();
    createFileMenus();

    return true;
}

void RemoteDevPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

bool RemoteDevPlugin::delayedInitialize()
{
    // Perforn non-trivial startup sequence after application startup
    // Return true, if implemented

    m_devManager->startDeviceSync();

    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag RemoteDevPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    // If required to wait, until external process finishes
    // return AsynchronousShutdown;
    // emit asynchronousShutdownFinished();

    // TODO: disconnect all existing connections?
    // Should it be done asynchronously?

    disconnect(this, SLOT(onConnectionError(Connection::Ptr)));
    disconnect(this, SLOT(onEditorOpened(Core::IEditor*)));
    disconnect(this, SLOT(onSaveAction());

    disconnect(m_uploadFile, SIGNAL(triggered(bool)));
    disconnect(m_uploadDirectory, SIGNAL(triggered(bool)));

    return SynchronousShutdown;
}

void RemoteDevPlugin::uploadCurrentDocument()
{
    auto document = Core::EditorManager::currentDocument();
    if (document) {
        uploadFile(document->filePath());
    }
}

void RemoteDevPlugin::uploadCurrentNode()
{
    auto node = ProjectExplorer::ProjectTree::currentNode();
    if (! node) return;

    switch (node->nodeType()) {
//    FileNodeType = 1,
//    FolderNodeType,
//    VirtualFolderNodeType,
//    ProjectNodeType,
//    SessionNodeType
    case ProjectExplorer::FileNodeType:
        uploadFile(node->path());
        break;
    case ProjectExplorer::FolderNodeType:
        showDebug(QStringLiteral("TODO: Upload directory: %1").arg(node->path().toString()));
        break;
    case ProjectExplorer::ProjectNodeType: {
        auto project = ProjectExplorer::SessionManager::projectForNode(node);
        showDebug(QStringLiteral("TODO: Upload project directory: %1")
                  .arg(project->projectDirectory().toString()));
        break;
    }
    default:
        showDebug(QStringLiteral("FIXME: unsupported node type ProjectExplorer::%1")
                  .arg(node->nodeType()));
    }
}

void RemoteDevPlugin::triggerAction()
{
    this->showDebug(tr("Action triggered"));
}

void RemoteDevPlugin::onConnectionError(Connection::Ptr connection)
{
    if (connection) {
        this->showDebug(tr("Remote connection error")
                        + QString::fromLatin1(": ")
                        + connection->errorString());
    } else {
        this->showDebug(tr("Remote connection error"));
    }
}

void RemoteDevPlugin::onEditorOpened(Core::IEditor *)
{
    uploadCurrentDocument();
}

void RemoteDevPlugin::onSaveAction()
{
    uploadCurrentDocument();
}

void RemoteDevPlugin::uploadFile(const Utils::FileName &file,
                                 ProjectExplorer::Project *project)
{
    project = project ? project
                      : ProjectExplorer::SessionManager::projectForFile(file);
    if (! project) return; // FIXME: do some debug message

    const auto mappings = m_mapManager->mappingsForProject(project);
    if (! mappings) return;

    const auto local   = project->projectDirectory();
    const auto relpath = file.relativeChildPath(local);

    qDebug() << "Upload file for project" << project->displayName() << ":"
             << relpath.toString();

    auto deviceMgr = ProjectExplorer::DeviceManager::instance();
    for (int i = 0; i < mappings->rowCount(); i++) {
        auto isEnabled = mappings->item(i, Constants::MAP_ENABLED_COLUMN)
                                 ->data(Qt::CheckStateRole).toBool();
        if (! isEnabled) continue;

        auto deviceId = mappings->item(i, Constants::MAP_DEVICE_COLUMN)
                ->data(Constants::DEV_ID_ROLE);
        auto device = deviceMgr->find(Core::Id::fromSetting(deviceId));
        auto mappingName = mappings->item(i, Constants::MAP_NAME_COLUMN)->text();

        auto connection = ConnectionManager::connectionForDevice(device.data());
        if (connection.isNull()) {
            qDebug() << "No connection for mapping" << mappingName
                     << "and device" << device->displayName();
            continue;
        }

        auto remote = Utils::FileName::fromString(mappings->item(i, Constants::MAP_PATH_COLUMN)->text());
        showDebug(QStringLiteral("%1: Upload \"%2\": \"%3\" -> \"%4\"").arg(
                      mappings->item(i, Constants::MAP_NAME_COLUMN)->text(),
                      relpath.toString(), local.toString(), remote.toString())
                 );

        static QSet<Connection *> hasHandler; // FIXME: dirty hack
        if (! hasHandler.contains(connection.data())) {
            hasHandler.insert(connection.data());
            connect(connection.data(), &Connection::uploadFinished,
                [this, connection, local, mappingName] (RemoteJobId job) {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : -1;

                    this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]")
                                    .arg(mappingName, tr("success"), QString::number(elapsed)));
                }
            );
            connect(connection.data(), &Connection::uploadError,
                [this, connection, local, mappingName] (RemoteJobId job, const QString &reason) {
                    auto timer = m_timers.take(job);
                    int elapsed = timer ? timer->elapsed() : -1;

                    this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]: %4")
                                    .arg(mappingName, tr("failure"), QString::number(elapsed), reason));
                }
            );
        }

        auto timer = QSharedPointer<QTime>(new QTime);
        timer->start();

        RemoteJobId job = connection->uploadFile(local, remote, relpath, OverwriteExisting);
        m_timers.insert(job, timer);
    }
}

void RemoteDevPlugin::createOptionsPage()
{
    m_optionsPage = new ConnectionsPage(this);
    // TODO: load Core::ICore::settings();
    addAutoReleasedObject(m_optionsPage);
}

void RemoteDevPlugin::createProjectSettingsPage()
{
    auto panelFactory = new ProjectExplorer::ProjectPanelFactory();
    panelFactory->setPriority(100); // this sets panel order in tabs
    panelFactory->setDisplayName(tr("RemoteDev"));

//    QIcon icon /* = QIcon(QLatin1String(":/projectexplorer/images/EditorSettings.png")) */;
//    panelFactory->setSimpleCreateWidgetFunction<ProjectSettingsWidget>(icon);

    panelFactory->setCreateWidgetFunction(
        [this, panelFactory] (ProjectExplorer::Project *project) -> QWidget * {
            auto panel = new ProjectExplorer::PropertiesPanel ();
            panel->setDisplayName(tr("Remote Mappings"));

            auto widget = new ProjectSettingsWidget(project);

            widget->setMappingsModel(m_mapManager->mappingsForProject(project));
            widget->setDevicesModel(m_devManager->devices());

            panel->setWidget(widget);

            auto panelsWidget = new ProjectExplorer::PanelsWidget();
            panelsWidget->addPropertiesPanel(panel);
            panelsWidget->setFocusProxy(widget);

            return panelsWidget;
        }
    );

    ProjectExplorer::ProjectPanelFactory::registerFactory(panelFactory);
}

void RemoteDevPlugin::createFileMenus()
{
    const Core::Context projectTreeContext(ProjectExplorer::Constants::C_PROJECT_TREE);
    // TODO: opened files context?

    auto fileContextMenu = Core::ActionManager::actionContainer(
                ProjectExplorer::Constants::M_FILECONTEXT);

    // "Upload File" menu
    m_uploadFile = new QAction(tr("Upload File"), this);
    auto cmd = Core::ActionManager::registerAction(
                m_uploadFile, Constants::UPLOAD_FILE, projectTreeContext);

    fileContextMenu->addAction(cmd, ProjectExplorer::Constants::G_FILE_OTHER);
    connect(m_uploadFile, &QAction::triggered,
            this, &RemoteDevPlugin::uploadCurrentNode);

    // "Upload Directory" menu
    auto folderContextMenu = Core::ActionManager::actionContainer(
                ProjectExplorer::Constants::M_FOLDERCONTEXT);
    auto projectContextMenu = Core::ActionManager::actionContainer(
                ProjectExplorer::Constants::M_PROJECTCONTEXT);
    auto subProjectContextMenu = Core::ActionManager::actionContainer(
                ProjectExplorer::Constants::M_SUBPROJECTCONTEXT);

    m_uploadDirectory = new QAction(tr("Upload Directory"), this);
    cmd = Core::ActionManager::registerAction(
                m_uploadDirectory, Constants::UPLOAD_DIRECTORY, projectTreeContext);

    folderContextMenu->addAction(cmd, ProjectExplorer::Constants::G_FOLDER_OTHER);
    projectContextMenu->addAction(cmd, ProjectExplorer::Constants::G_PROJECT_FILES);
    subProjectContextMenu->addAction(cmd, ProjectExplorer::Constants::G_PROJECT_FILES);
    connect(m_uploadDirectory, &QAction::triggered,
            this, &RemoteDevPlugin::uploadCurrentNode);
}

void RemoteDevPlugin::showDebug(const QString &string) const
{
    auto *messageManager = qobject_cast<Core::MessageManager *>(Core::MessageManager::instance());
    messageManager->write(string, { Core::MessageManager::NoModeSwitch });
}

