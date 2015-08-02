#ifndef REMOTEOPTIONSPAGE_H
#define REMOTEOPTIONSPAGE_H

#include "remotedev_global.h"

#include <QPointer>

#include <coreplugin/dialogs/ioptionspage.h>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

// TODO: IOptionsPageProvider should be used

class RemoteOptionsPage : public Core::IOptionsPage
{
    Q_OBJECT
public:
    RemoteOptionsPage(QObject *parent = 0);

    QWidget *widget();
    void apply();
    void finish();

private:
    QPointer<QWidget> m_widget;
};

} // namespace Internal
} // namespace RemoteDev

#endif // REMOTEOPTIONSPAGE_H
