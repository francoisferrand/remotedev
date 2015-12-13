#include "projectsettingswidget.h"
#include "ui_projectsettingswidget.h"

#include <QStandardItemModel>
//#include <QItemDelegate> // TBD: which one to use?
#include <QStyledItemDelegate>
#include <QDataWidgetMapper>

#include <QDebug>

#include "remotedevconstants.h"

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

    static int i = 0;
    this->createMapping(QStringLiteral("<new mapping>"),
                        true,
                        QString(),
                        QStringLiteral("test%1").arg(i++));
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
        qDebug() << "Restoring mapping" << name;
        auto config = section.value(name).toMap();

        this->createMapping(config[QStringLiteral("name")].toString(),
                            config[QStringLiteral("enabled")].toBool(),
                            config[QStringLiteral("device")].toString(),
                            config[QStringLiteral("path")].toString());
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

void ProjectSettingsWidget::createMapping(const QString &name, bool enabled, const QString &device, const QString &path)
{
    auto nameItem = new QStandardItem(name);

    auto enabledItem = new QStandardItem();
    enabledItem->setData(enabled, Qt::CheckStateRole);

    auto deviceItem = new QStandardItem(device);

    auto pathItem = new QStandardItem(path);

    auto model = qobject_cast<QStandardItemModel *>(ui->tblMappings->model());
    model->appendRow({ nameItem, enabledItem, deviceItem, pathItem });
}
