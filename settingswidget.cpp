#include "settingswidget.h"
#include "ui_settingswidget.h"

#include <QSettings>
#include <QStringList>

using namespace RemoteDev::Internal;

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::updateWithSettings(QSettings &settings)
{
    settings.beginGroup(QStringLiteral("connections"));
    QStringList connections = settings.childGroups();

    for (const QString &connection : connections) {
        settings.beginGroup(connection);

        // this->connectionOptionsPage(connection)

        settings.endGroup();
    }

    settings.endGroup();
}
