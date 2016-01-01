#ifndef REMOTEOPTIONSWIDGET_H
#define REMOTEOPTIONSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace RemoteDev {
namespace Internal {

namespace Ui { class SettingsWidget; }

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();

    void updateWithSettings(QSettings &settings);

private:

private:
    Ui::SettingsWidget *ui;
};

} // namespace Interal
} // namespace RemoteDev

#endif // REMOTEOPTIONSWIDGET_H
