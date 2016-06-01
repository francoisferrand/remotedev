#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

namespace Ui { class OptionsWidget; }

class OptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsWidget(QWidget *parent = 0);
    ~OptionsWidget();

    void updateWithSettings(QSettings &settings);

private:

private:
    Ui::OptionsWidget *ui;
};

} // namespace Interal
} // namespace RemoteDev

#endif // OPTIONSWIDGET_H
