#include "remoteoptionspage.h"

#include "remoteoptionswidget.h"

using namespace RemoteDev::Internal;

RemoteOptionsPage::RemoteOptionsPage(QObject *parent) :
    IOptionsPage(parent),
    m_widget(nullptr)
{
    setId("remoteoptions");
    setCategory("remoteoptions");
    setDisplayName(tr("Connection Settings"));
    setDisplayCategory(tr("Remote Development"));

    setCategoryIcon(QString::fromLatin1(""));
}


QWidget *RemoteOptionsPage::widget()
{
    if (! m_widget) {
        m_widget = new Internal::RemoteOptionsWidget;
    }
    return m_widget;
}


void RemoteOptionsPage::apply()
{
    // TODO: implement
}


void RemoteOptionsPage::finish()
{
    // TODO: implement: options before dialog is closed
    delete m_widget;
}
