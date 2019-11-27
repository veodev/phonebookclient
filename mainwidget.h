#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>

#include "dialogwidget.h"
#include "phonebookmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private slots:
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void on_editButton_clicked();
    void dialogWidgetClosed();
    void itemActivated(const QModelIndex& index);
    void tcpSocketStateChanged(QAbstractSocket::SocketState state);
    void tcpSocketReadyRead();
    void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());

private:
    void connectToServer();
    void disconnectToServer();
    void readMessageFromBuffer();
    void createModel();
    void sendMessage(QByteArray& message);
    void updateServerData();
    void updatePhoneBookModel();

private:
    Ui::MainWidget* ui;
    QTcpSocket* _tcpSocket;
    PhoneBookModel* _model;
    QModelIndex _currentModelIndex;
    DialogWidget* _dialogWidget;
    QByteArray _messagesBuffer;
    //    struct Item
    //    {
    //        QString secondName;
    //        QString firstName;
    //        QString patronym;
    //        QString sex;
    //        QString phone;
    //        void reset()
    //        {
    //            secondName.clear();
    //            firstName.clear();
    //            patronym.clear();
    //            sex.clear();
    //            phone.clear();
    //        }
    //    };

    QMap<int, Item> _contacts;
    Item _bufferItem;
    int _itemIndex;
};
#endif  // MAINWIDGET_H
