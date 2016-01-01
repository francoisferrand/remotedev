#include "connectionspage.h"

#include "settingswidget.h"

using namespace RemoteDev::Internal;

ConnectionsPage::ConnectionsPage(QObject *parent) :
    IOptionsPage(parent),
    m_widget(nullptr)
{
    setId("remoteoptions");
    setCategory("remoteoptions");
    setDisplayName(tr("Connection Settings"));
    setDisplayCategory(tr("Remote Development"));

    setCategoryIcon(QString::fromLatin1(""));
}


QWidget *ConnectionsPage::widget()
{
    if (! m_widget) {
        m_widget = new Internal::SettingsWidget;
    }
    return m_widget;
}


void ConnectionsPage::apply()
{
    // TODO: implement
}


void ConnectionsPage::finish()
{
    // TODO: implement: options before dialog is closed
    delete m_widget;
}
