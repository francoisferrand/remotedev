#include "optionswidget.h"
#include "ui_optionswidget.h"

#include <QSettings>
#include <QStringList>

using namespace RemoteDev::Internal;

OptionsWidget::OptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsWidget)
{
    ui->setupUi(this);
}

OptionsWidget::~OptionsWidget()
{
    delete ui;
}

void OptionsWidget::updateWithSettings(QSettings &settings)
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
