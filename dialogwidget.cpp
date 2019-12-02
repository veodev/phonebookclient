#include "dialogwidget.h"
#include "ui_dialogwidget.h"

DialogWidget::DialogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DialogWidget)
    , _currentMode(DyalogWidgetModes::AddMode)
    , _parentWidget(parent)
{
    ui->setupUi(this);
    hide();
}

DialogWidget::~DialogWidget()
{
    delete ui;
}

void DialogWidget::changeMode(DyalogWidgetModes mode)
{
    switch (mode) {
    case AddMode:
        ui->titleLabel->setText("Adding a new contact:");
        ui->addButton->show();
        ui->saveButton->hide();
        ui->firstNameLineEdit->clear();
        ui->secondNameLineEdit->clear();
        ui->patronymLineEdit->clear();
        ui->sexLineEdit->clear();
        ui->phoneLineEdit->clear();
        break;
    case EditMode:
        ui->titleLabel->setText("Contact editing:");
        ui->addButton->hide();
        ui->saveButton->show();
        break;
    }
    _currentMode = mode;
}

QString DialogWidget::getFirstName()
{
    return ui->firstNameLineEdit->text();
}

QString DialogWidget::getSecondName()
{
    return ui->secondNameLineEdit->text();
}

QString DialogWidget::getPatronym()
{
    return ui->patronymLineEdit->text();
}

QString DialogWidget::getSex()
{
    return ui->sexLineEdit->text();
}

QString DialogWidget::getPhone()
{
    return ui->phoneLineEdit->text();
}

DyalogWidgetModes DialogWidget::getMode()
{
    return _currentMode;
}

void DialogWidget::setFirstName(QString text)
{
    ui->firstNameLineEdit->setText(text);
}

void DialogWidget::setSecondName(QString text)
{
    ui->secondNameLineEdit->setText(text);
}

void DialogWidget::setPatronym(QString text)
{
    ui->patronymLineEdit->setText(text);
}

void DialogWidget::setSex(QString text)
{
    ui->sexLineEdit->setText(text);
}

void DialogWidget::setPhone(QString text)
{
    ui->phoneLineEdit->setText(text);
}

void DialogWidget::popup()
{
    QSize parentSize = _parentWidget->size();
    setMinimumSize(parentSize);
    setMaximumSize(parentSize);
    raise();
    show();
}

void DialogWidget::on_cancelButton_clicked()
{
    hide();
}

void DialogWidget::on_addButton_clicked()
{
    hide();
    emit doDialogWidgetClosed();
}

void DialogWidget::on_saveButton_clicked()
{
    hide();
    emit doDialogWidgetClosed();
}
