#include "connectionswidget.h"
#include "ui_connectionswidget.h"

#include <QSettings>
#include <QStringList>

using namespace RemoteDev::Internal;

ConnectionsWidget::ConnectionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionsWidget)
{
    ui->setupUi(this);
}

ConnectionsWidget::~ConnectionsWidget()
{
    delete ui;
}

void ConnectionsWidget::updateWithSettings(QSettings &settings)
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
