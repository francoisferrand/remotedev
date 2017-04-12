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
//#include <projectexplorer/propertiespanel.h>
#include <projectexplorer/projectpanelfactory.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>

#include "connectionhelper.h"
#include "connectionmanager.h"
#include "optionspage.h"
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

    //auto *editorManager = Core::EditorManager::instance();
    // NOTE: currentEditorChanged is also triggered upon editorOpened
    // connect(editorManager, &Core::EditorManager::editorOpened,
    //         this, &RemoteDevPlugin::uploadCurrentDocument);
    // connect(editorManager, &Core::EditorManager::currentEditorChanged,
    //         this, &RemoteDevPlugin::uploadCurrentDocument);

    QAction *saveAction = Core::ActionManager::command(Core::Constants::SAVE)->action();
    connect(saveAction, &QAction::triggered,
            this, &RemoteDevPlugin::uploadCurrentDocument);

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
    disconnect(this, SLOT(uploadCurrentDocument()));
    disconnect(this, SLOT(uploadCurrentNode()));

    disconnect(m_uploadFile, SIGNAL(triggered(bool)));
    disconnect(m_uploadDirectory, SIGNAL(triggered(bool)));

    return SynchronousShutdown;
}

void RemoteDevPlugin::uploadCurrentDocument()
{
    auto document = Core::EditorManager::currentDocument();
    if (document) {
        auto *project = ProjectExplorer::SessionManager::projectForFile(document->filePath());
        if (project) {
            upload(document->filePath(), *project, &Connection::uploadFile);
        } else {
            qDebug() << "No project for file: " << document->filePath();
        }
    }
}

void RemoteDevPlugin::uploadCurrentNode()
{
    auto node = ProjectExplorer::ProjectTree::currentNode();
    if (!node) return;

    auto *project = ProjectExplorer::ProjectTree::projectForNode(node);
    if (!project) return;

    switch (node->nodeType()) {
    case ProjectExplorer::FileNodeType:
        upload(node->filePath(), *project, &Connection::uploadFile);
        break;
    case ProjectExplorer::FolderNodeType:
        upload(node->filePath(), *project, &Connection::uploadDirectory);
        break;
    case ProjectExplorer::ProjectNodeType:
        upload(project->projectDirectory(), *project, &Connection::uploadDirectory);
        break;
    case ProjectExplorer::VirtualFolderNodeType:
    case ProjectExplorer::SessionNodeType:
    default:
        showDebug(
          QStringLiteral("FIXME: unsupported node type ProjectExplorer::%1").arg(node->nodeType()));
    }
}

void RemoteDevPlugin::downloadCurrentNode()
{
    auto node = ProjectExplorer::ProjectTree::currentNode();
    if (!node) return;

    auto *project = ProjectExplorer::ProjectTree::projectForNode(node);
    if (!project) return;

    switch (node->nodeType()) {
    case ProjectExplorer::FileNodeType:
        download(node->filePath(), *project, &Connection::downloadFile);
        break;
    case ProjectExplorer::FolderNodeType:
    case ProjectExplorer::ProjectNodeType:
    case ProjectExplorer::VirtualFolderNodeType:
    case ProjectExplorer::SessionNodeType:
    default:
        showDebug(
          QStringLiteral("FIXME: unsupported node type ProjectExplorer::%1").arg(node->nodeType()));
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

void RemoteDevPlugin::upload(const Utils::FileName &file,
                             ProjectExplorer::Project &project,
                             RemoteDevPlugin::UploadMethod uploadMethod)
{
    //    FIXME:
    //    CMocka-1: Upload "src/cmocka.c": "/home/elvenfighter/Projects/cmocka" -> "/tmp/cmocka-1"
    //    CMocka-2: Upload "src/cmocka.c": "/home/elvenfighter/Projects/cmocka" -> "/var/tmp/cmocka-2"
    //    CMockaTest-2: failure [25 ms]: /home/elvenfighter/Projects/cmocka-test: No such file
    //    CMockaTest-1: failure [41 ms]: /home/elvenfighter/Projects/cmocka-test: No such file

    const auto local = project.projectDirectory();
    const auto relpath = file.relativeChildPath(local);

    qDebug() << "Upload file for project" << project.displayName() << ":"
             << relpath.toString();

    auto deviceMgr = ProjectExplorer::DeviceManager::instance();

    for (const Mapping& mapping : m_mapManager->mappingsForProject(project)) {
        if (!mapping.isEnabled()) continue;

        const auto mappingName = mapping.name();

        auto device = deviceMgr->find(mapping.deviceId());
        if (device.isNull()) {
            qDebug() << "No device can be found for mapping" << mappingName;
            continue;
        }

        auto connection = ConnectionManager::connectionForDevice(*device);
        if (connection.isNull()) {
            qDebug() << "No connection for mapping" << mappingName
                     << "and device" << device->displayName();
            continue;
        }

        const auto remote = mapping.remotePath();
        showDebug(QStringLiteral("%1: Upload \"%2\": \"%3\" -> \"%4\"").arg(
            mappingName, relpath.toString(), local.toString(), remote.toString()
        ));

        auto &helper = getConnectionHelper(*connection, mappingName, local.toString());
        helper.startJob([&]() {
            auto job = (connection.data()->*uploadMethod)(local, remote, relpath, OverwriteExisting);
            qDebug() << "Started job" << mappingName << "->" << job;
            return job;
        });
    }
}

void RemoteDevPlugin::download(const Utils::FileName &file,
                               ProjectExplorer::Project &project,
                               RemoteDevPlugin::DownloadMethod downloadMethod)
{
    const auto localDir = project.projectDirectory();
    const auto relpath = file.relativeChildPath(localDir);

    qDebug() << "Download file for project " << project.displayName() << ":"
             << relpath.toString();



    // FIXME: name of mapping (default for download) should be passed
    // FIXME: for now, grab first enabled mapping
    //        (although it is not necessary for the mapping to be enabled here)
    // FIXME: decide if Mapping::isEnabled() controls only upload
    const auto &mappings = m_mapManager->mappingsForProject(project);
    const auto mapping = std::find_if(mappings.begin(),
                                      mappings.end(),
                                      [](const Mapping &m) { return m.isEnabled(); });

    if (mapping == mappings.end()) {
        qDebug() << "Download mapping not selected";
        return;
    }

    const auto mappingName = mapping->name();

    auto deviceMgr = ProjectExplorer::DeviceManager::instance();
    auto device = deviceMgr->find(mapping->deviceId());
    if (device.isNull()) {
        qDebug() << "No device can be found for mapping" << mappingName;
        return;
    }

    auto connection = ConnectionManager::connectionForDevice(*device);
    if (connection.isNull())
        return;

    const auto remote = mapping->remotePath();
    showDebug(QStringLiteral("%1: Download \"%2\": \"%3\" -> \"%4\"").arg(
        mappingName, relpath.toString(), localDir.toString(), remote.toString()
    ));

    auto &helper = getConnectionHelper(*connection, mappingName, localDir.toString());
    helper.startJob([&]() {
        auto job = (connection.data()->*downloadMethod)(localDir, remote, relpath, OverwriteExisting);
        qDebug() << "Started job" << mappingName << "->" << job;
        return job;
    });
}

ConnectionHelper &RemoteDevPlugin::getConnectionHelper(RemoteDev::Connection &connection,
                                                       const QString &mapping,
                                                       const QString &target) const
{
    auto helper = connection.findChild<ConnectionHelper *>();
    if (!helper) {
        helper = new ConnectionHelper(&connection);

        // FIXME: transform to methods
        connect(&connection, &Connection::uploadFinished,
            [this, helper, target, mapping] (RemoteJobId job) {
                auto elapsed = helper->endJob(job);

                this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]: %4")
                                .arg(mapping, tr("success"),
                                     QString::number(elapsed), target));
            }
        );
        connect(&connection, &Connection::uploadError,
            [this, helper, target, mapping] (RemoteJobId job, const QString &reason) {
                auto elapsed = helper->endJob(job);

                this->showDebug(QString::fromLatin1("%1: %2 [%3 ms]: %4: %5")
                                .arg(mapping, tr("failure"),
                                     QString::number(elapsed), target, reason));
            }
        );
    }

    return *helper;
}

void RemoteDevPlugin::createOptionsPage()
{
    m_optionsPage = new OptionsPage(this);
    // TODO: load Core::ICore::settings();
    addAutoReleasedObject(m_optionsPage);
}

void RemoteDevPlugin::createProjectSettingsPage()
{
    auto panelFactory = new ProjectExplorer::ProjectPanelFactory();
    panelFactory->setPriority(100); // this sets panel order in tabs
    panelFactory->setDisplayName(tr("RemoteDev"));

    // QIcon icon /* = QIcon(QLatin1String(":/projectexplorer/images/EditorSettings.png")) */;
    // panelFactory->setSimpleCreateWidgetFunction<ProjectSettingsWidget>(icon);

    panelFactory->setCreateWidgetFunction(
        [this, panelFactory] (ProjectExplorer::Project *project) -> QWidget * {
            if (!project) return nullptr;
            // auto panel = new ProjectExplorer::PropertiesPanel;
            // panel->setDisplayName(tr("Remote Mappings"));

            auto widget = new ProjectSettingsWidget(*project, *this->m_mapManager);
            widget->setDevicesModel(m_devManager->devices());

            // panel->setWidget(widget);

            auto panelsWidget = new ProjectExplorer::PanelsWidget();
            panelsWidget->addPropertiesPanel(tr("RemoteDev"), QIcon(), widget /* panel*/);
            // panelsWidget->addPropertiesPanel(panel);
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
    auto *cmd = Core::ActionManager::registerAction(m_uploadFile, Constants::UPLOAD_FILE,
                                                   projectTreeContext);

    fileContextMenu->addAction(cmd, ProjectExplorer::Constants::G_FILE_OTHER);
    connect(m_uploadFile, &QAction::triggered,
            this, &RemoteDevPlugin::uploadCurrentNode);

    // "Download File" menu
    m_downloadFile = new QAction(tr("Download File"), this);
    cmd = Core::ActionManager::registerAction(m_downloadFile, Constants::DOWNLOAD_FILE,
                                              projectTreeContext);

    fileContextMenu->addAction(cmd, ProjectExplorer::Constants::G_FILE_OTHER);
    connect(m_downloadFile, &QAction::triggered,
            this, &RemoteDevPlugin::downloadCurrentNode);

    // "Upload Directory" menu
    auto folderContextMenu =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_FOLDERCONTEXT);
    auto projectContextMenu =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_PROJECTCONTEXT);
    auto subProjectContextMenu =
            Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_SUBPROJECTCONTEXT);

    m_uploadDirectory = new QAction(tr("Upload Directory"), this);
    cmd = Core::ActionManager::registerAction(m_uploadDirectory, Constants::UPLOAD_DIRECTORY, projectTreeContext);

    folderContextMenu->addAction(cmd, ProjectExplorer::Constants::G_FOLDER_OTHER);
    projectContextMenu->addAction(cmd, ProjectExplorer::Constants::G_PROJECT_FILES);
    subProjectContextMenu->addAction(cmd, ProjectExplorer::Constants::G_PROJECT_FILES);
    connect(m_uploadDirectory, &QAction::triggered,
            this, &RemoteDevPlugin::uploadCurrentNode);
}

void RemoteDevPlugin::showDebug(const QString &string) const
{
    auto messageManager = Core::MessageManager::instance();
    messageManager->write(string, Core::MessageManager::Silent);
}

