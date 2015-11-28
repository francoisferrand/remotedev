#include "projectsettingswidget.h"
#include "ui_projectsettingswidget.h"

#include <QStandardItemModel>
//#include <QItemDelegate> // TBD: which one to use?
#include <QStyledItemDelegate>
#include <QDataWidgetMapper>

#include <QDebug>

#include "remotedevconstants.h"

//static const int myrole = Qt::UserRole + 1;

using namespace RemoteDev::Internal;

ProjectSettingsWidget::ProjectSettingsWidget(ProjectExplorer::Project *prj) :
    ui(new Ui::ProjectSettingsWidget),
    m_project(prj)
{
    ui->setupUi(this);

    //  0   |    1    |   2    |  3
    // name | enabled | device | path
    auto model = new QStandardItemModel(0, 4, this);

    auto mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
//    mapper->addMapping(ui->cbxDevice, 2); // FIXME: how to make this work?
    mapper->addMapping(ui->edtPath, 3);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    // 2 visible columns: name, enabled
    ui->tblMappings->setModel(model);
    ui->tblMappings->setColumnHidden(2, true);
    ui->tblMappings->setColumnHidden(3, true);

    connect(ui->tblMappings, &QTableView::activated,
            mapper, &QDataWidgetMapper::setCurrentModelIndex);

    // FIXME: somehow project has invalid pointer

//    qDebug() << "creating settings for:" << m_project->displayName();

//    connect(m_project, &ProjectExplorer::Project::settingsLoaded,
//            [project] () -> void {
//                qDebug() << "settings loaded:" << project->displayName();
//            });

//    connect(m_project, SIGNAL(settingsLoaded()), this, SLOT(handleSettings()));

//    connect(m_project, &ProjectExplorer::Project::settingsLoaded,
//            this, &ProjectSettingsWidget::initData);
    // TBD: connect(project, settinsLoaded, this, initData)
}

ProjectSettingsWidget::~ProjectSettingsWidget()
{
    delete ui;
}

void ProjectSettingsWidget::newMapping()
{
    qDebug() << "new mapping for project:" << m_project->displayName();

    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());

    auto nameItem = new QStandardItem(QStringLiteral("<new mapping>"));

    auto enabledItem = new QStandardItem();
    enabledItem->setData(true, Qt::CheckStateRole);

    static int i = 0;
    auto test = QStringLiteral("test%1").arg(i++);

    model->appendRow({ nameItem, enabledItem, new QStandardItem(), new QStandardItem(test) });
}

void ProjectSettingsWidget::initData()
{
    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());

    qDebug() << "Project settings loaded:" << m_project->displayName();

    // QVariantMap settings;
    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    auto section = settings.value(QLatin1String(Constants::MAPPINGS_GROUP)).toMap();


    for (auto &mapping : section.keys()) {
        auto mapSettings = section.value(mapping).toMap();

//        if (! key.startsWith(prefix))
//            continue;

//        auto nameItem = new QStandardItem(key.section('.', 1, 1));


//        auto enableItem = new QStandardItem();
//        enableItem->setDa

//        auto combo = new QComboBox();
//        combo->setAutoFillBackground(true);
//        auto itemWidget = new QStyledItemDelegate();

//        ui->tblMappings->setIndexWidget(model->indexFromItem(item), itemWidget);
//        auto item = new QStandardItem()

//        model->appendRow({ nameItem, enabledItem });
    }

    // TODO: use QDataWidgetMapper

    //    m_project->setNamedSettings(Constants::SETTINGS_GROUP, settigs);
}

void ProjectSettingsWidget::handleSettings()
{

}
