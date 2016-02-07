#ifndef REMOTEOPTIONSWIDGET_H
#define REMOTEOPTIONSWIDGET_H

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

#endif // REMOTEOPTIONSWIDGET_H
