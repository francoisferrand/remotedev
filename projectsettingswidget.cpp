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

    ui->tblMappings->setModel(model);

    connect(ui->tblMappings, &QTableView::activated,
            mapper, &QDataWidgetMapper::setCurrentModelIndex);

    connect(m_project, &ProjectExplorer::Project::aboutToSaveSettings,
            this, &ProjectSettingsWidget::saveSettings);

    // TBD: is this required?
    //connect(m_project, &ProjectExplorer::Project::settingsLoaded,
    //        this, &ProjectSettingsWidget::initData);
    this->initData();
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

    model->appendRow({
        nameItem, enabledItem,
        new QStandardItem(), new QStandardItem(test)
    });
}

void ProjectSettingsWidget::initData()
{
    qDebug() << "Initializing project settings:" << m_project->displayName();

    // QVariantMap settings;
    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    auto section = settings.value(QLatin1String(Constants::MAPPINGS_GROUP)).toMap();

    qDebug() << "Settings:" << settings;

    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    model->clear();
    for (auto &name : section.keys()) {
        qDebug() << "Adding mapping" << name;
        auto config = section.value(name).toMap();

        auto nameItem    = new QStandardItem(config[QStringLiteral("name")].toString());
        auto enabledItem = new QStandardItem();
        enabledItem->setData(config[QStringLiteral("enabled")], Qt::CheckStateRole);

        auto deviceItem  = new QStandardItem(config[QStringLiteral("device")].toString());
        auto pathItem    = new QStandardItem(config[QStringLiteral("path")].toString());

        model->appendRow({ nameItem, enabledItem, deviceItem, pathItem });
    }

    // 2 visible columns: name, enabled
    // FIXME: somehow this does not work when model is empty (in the constructor)
    ui->tblMappings->setColumnHidden(2, true);
    ui->tblMappings->setColumnHidden(3, true);
}

void ProjectSettingsWidget::saveSettings()
{
    qDebug() << "Saving settings for" << m_project->displayName();

    QVariantMap mappings;
    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    for (int i = 0; i < model->rowCount(); i++) {
        const auto &name = model->item(i, 0)->text();
        // FIXME: name is supposed to be unique?
        mappings[name] = QVariantMap({
            { QStringLiteral("name"),    name },
            { QStringLiteral("enabled"), model->item(i, 1)->data(Qt::CheckStateRole) },
            // FIXME: are device ID's fixed?
            { QStringLiteral("device"),  model->item(i, 2)->text() },
            { QStringLiteral("path"),    model->item(i, 3)->text() }
        });
    }

    auto settings = m_project->namedSettings(QLatin1String(Constants::SETTINGS_GROUP)).toMap();
    settings[QLatin1String(Constants::MAPPINGS_GROUP)] = mappings;
    m_project->setNamedSettings(QLatin1String(Constants::SETTINGS_GROUP), settings);
}
