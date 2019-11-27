#ifndef DIALOGWIDGET_H
#define DIALOGWIDGET_H

#include <QWidget>
#include <enums.h>

namespace Ui
{
class DialogWidget;
}

class DialogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DialogWidget(QWidget* parent = nullptr);
    ~DialogWidget();

    void changeMode(DyalogWidgetModes mode);
    QString getFirstName();
    QString getSecondName();
    QString getPatronym();
    QString getSex();
    QString getPhone();
    DyalogWidgetModes getMode();

    void setFirstName(QString text);
    void setSecondName(QString text);
    void setPatronym(QString text);
    void setSex(QString text);
    void setPhone(QString text);

signals:
    void doDialogWidgetClosed();

private slots:
    void on_cancelButton_clicked();
    void on_addButton_clicked();
    void on_saveButton_clicked();

private:
    Ui::DialogWidget* ui;
    DyalogWidgetModes _currentMode;
};

#endif  // DIALOGWIDGET_H
