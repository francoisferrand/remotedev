#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#pragma once

#include "remotedev_global.h"

#include <QPointer>

#include <coreplugin/dialogs/ioptionspage.h>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

// TODO: IOptionsPageProvider should be used

class OptionsPage : public Core::IOptionsPage
{
    Q_OBJECT
public:
    OptionsPage(QObject *parent = 0);

    QWidget *widget();
    void apply();
    void finish();

private:
    QPointer<QWidget> m_widget;
};

} // namespace Internal
} // namespace RemoteDev

#endif // OPTIONSPAGE_H
