#include "mainwidget.h"
#include "enums.h"
#include "ui_mainwidget.h"

#include <QHostAddress>
#include <QtEndian>
#include <QTimer>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , _tcpSocket(nullptr)
    , _model(nullptr)
    , _currentModelIndex(QModelIndex())
    , _itemIndex(-1)
{
    ui->setupUi(this);
    _dialogWidget = new DialogWidget(this);
    _dialogWidget->setMinimumSize(this->size());
    _dialogWidget->setMaximumSize(this->size());
    connect(_dialogWidget, &DialogWidget::doDialogWidgetClosed, this, &MainWidget::dialogWidgetClosed);
    createModel();
    ui->tableView->setModel(_model);
    connect(ui->tableView, &QTableView::pressed, this, &MainWidget::itemActivated);
    connectToServer();
}


MainWidget::~MainWidget()
{
    if (_tcpSocket != nullptr) {
        disconnect(_tcpSocket, &QTcpSocket::stateChanged, this, &MainWidget::tcpSocketStateChanged);
    }
    delete ui;
}

void MainWidget::on_addButton_clicked()
{
    _dialogWidget->changeMode(DyalogWidgetModes::AddMode);
    _dialogWidget->show();
}

void MainWidget::on_deleteButton_clicked()
{
    _model->removeContact(_currentModelIndex);
    _currentModelIndex = QModelIndex();
}

void MainWidget::on_editButton_clicked()
{
    if (_model) {
        _dialogWidget->setFirstName(_model->data(_currentModelIndex, FirstNameRole).toString());
        _dialogWidget->setSecondName(_model->data(_currentModelIndex, SecondNameRole).toString());
        _dialogWidget->setPatronym(_model->data(_currentModelIndex, PatronymRole).toString());
        _dialogWidget->setSex(_model->data(_currentModelIndex, SexRole).toString());
        _dialogWidget->setPhone(_model->data(_currentModelIndex, PhoneRole).toString());
        _dialogWidget->changeMode(DyalogWidgetModes::EditMode);
        _dialogWidget->show();
    }
}

void MainWidget::dialogWidgetClosed()
{
    if (_model == nullptr) {
        return;
    }
    switch (_dialogWidget->getMode()) {
    case DyalogWidgetModes::AddMode:
        _model->addContact(_dialogWidget->getFirstName(), _dialogWidget->getSecondName(), _dialogWidget->getPatronym(), _dialogWidget->getSex(), _dialogWidget->getPhone());
        break;
    case DyalogWidgetModes::EditMode:
        _model->editContact(_currentModelIndex, _dialogWidget->getFirstName(), _dialogWidget->getSecondName(), _dialogWidget->getPatronym(), _dialogWidget->getSex(), _dialogWidget->getPhone());
        break;
    }
}

void MainWidget::itemActivated(const QModelIndex& index)
{
    _currentModelIndex = index;
}

void MainWidget::connectToServer()
{
    if (_tcpSocket == nullptr) {
        _tcpSocket = new QTcpSocket(this);
        connect(_tcpSocket, &QTcpSocket::stateChanged, this, &MainWidget::tcpSocketStateChanged);
        connect(_tcpSocket, &QTcpSocket::readyRead, this, &MainWidget::tcpSocketReadyRead);
        _tcpSocket->connectToHost(QHostAddress::LocalHost, 50002);
    }
}

void MainWidget::disconnectToServer()
{
    if (_tcpSocket != nullptr) {
        _tcpSocket->disconnectFromHost();
        disconnect(_tcpSocket, &QTcpSocket::stateChanged, this, &MainWidget::tcpSocketStateChanged);
        disconnect(_tcpSocket, &QTcpSocket::readyRead, this, &MainWidget::tcpSocketReadyRead);
        _tcpSocket->deleteLater();
        _tcpSocket = nullptr;
    }
}

void MainWidget::readMessageFromBuffer()
{
    Headers header = Unknown;
    while (true) {
        if (_messagesBuffer.size() >= static_cast<int>(sizeof(qint16))) {
            quint16 size = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(_messagesBuffer.left(sizeof(quint16)).data()));
            if (_messagesBuffer.size() >= size) {
                _messagesBuffer.remove(0, sizeof(quint16));
                header = static_cast<Headers>(_messagesBuffer.at(0));
                _messagesBuffer.remove(0, sizeof(Headers));
                switch (header) {
                case Headers::ClearContacts:
                    _contacts.clear();
                    _itemIndex = 0;
                    break;
                case Headers::SaveContacts:
                    updatePhoneBookModel();
                    break;
                case Headers::StartSingleContact:
                    _bufferItem.reset();
                    break;
                case Headers::EndSingleContact:
                    _contacts.push_back(_bufferItem);
                    ++_itemIndex;
                    break;
                case Headers::SecondName: {
                    _bufferItem.secondName = QString::fromUtf8(_messagesBuffer.left(size - sizeof(Headers)));
                    _messagesBuffer.remove(0, size - sizeof(Headers));
                    break;
                }
                case Headers::FirstName: {
                    _bufferItem.firstName = QString::fromUtf8(_messagesBuffer.left(size - sizeof(Headers)));
                    _messagesBuffer.remove(0, size - sizeof(Headers));
                    break;
                }
                case Headers::Patronym: {
                    _bufferItem.patronym = QString::fromUtf8(_messagesBuffer.left(size - sizeof(Headers)));
                    _messagesBuffer.remove(0, size - sizeof(Headers));
                    break;
                }
                case Headers::Sex: {
                    _bufferItem.sex = QString::fromUtf8(_messagesBuffer.left(size - sizeof(Headers)));
                    _messagesBuffer.remove(0, size - sizeof(Headers));
                    break;
                }
                case Headers::Phone: {
                    _bufferItem.phone = QString::fromUtf8(_messagesBuffer.left(size - sizeof(Headers)));
                    _messagesBuffer.remove(0, size - sizeof(Headers));
                    break;
                }
                default:
                    break;
                }
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }
}

void MainWidget::tcpSocketStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        ui->connectionLabel->setStyleSheet("color: red");
        ui->connectionLabel->setText("Disconnected");
        disconnectToServer();
        QTimer::singleShot(3000, this, &MainWidget::connectToServer);
        break;
    case QAbstractSocket::ConnectingState:
        ui->connectionLabel->setText("Connecting ...");
        break;
    case QAbstractSocket::ConnectedState:
        ui->connectionLabel->setStyleSheet("color: green");
        ui->connectionLabel->setText("Connected");
        break;
    default:
        break;
    }
}

void MainWidget::tcpSocketReadyRead()
{
    while (_tcpSocket->bytesAvailable()) {
        _messagesBuffer.append(_tcpSocket->readAll());
    }
    readMessageFromBuffer();
}

void MainWidget::modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    _contacts.clear();
    for (int i = 0; i < _model->rowCount(); ++i) {
        QModelIndex index = _model->index(i, 0);
        Item item;
        item.secondName = _model->data(index, SecondNameRole).toString();
        item.firstName = _model->data(index, FirstNameRole).toString();
        item.patronym = _model->data(index, PatronymRole).toString();
        item.sex = _model->data(index, SexRole).toString();
        item.phone = _model->data(index, PhoneRole).toString();
        _contacts.push_back(item);
    }
    updateServerData();
}

void MainWidget::createModel()
{
    _model = new PhoneBookModel(this);
    connect(_model, &QAbstractItemModel::dataChanged, this, &MainWidget::modelDataChanged);
}

void MainWidget::sendMessage(QByteArray& message)
{
    if (_tcpSocket != nullptr) {
        quint16 size = static_cast<quint16>(message.size());
        _tcpSocket->write(reinterpret_cast<char*>(&size), sizeof(quint16));
        _tcpSocket->write(message);
        _tcpSocket->flush();
    }
}

void MainWidget::updateServerData()
{
    QByteArray message;
    message.append(Headers::ClearContacts);
    sendMessage(message);

    for (auto& contact : _contacts) {
        QByteArray message;
        message.append(Headers::StartSingleContact);
        sendMessage(message);

        message.clear();
        message.append(Headers::SecondName);
        message.append(contact.secondName.toUtf8());
        sendMessage(message);

        message.clear();
        message.append(Headers::FirstName);
        message.append(contact.firstName.toUtf8());
        sendMessage(message);

        message.clear();
        message.append(Headers::Patronym);
        message.append(contact.patronym.toUtf8());
        sendMessage(message);

        message.clear();
        message.append(Headers::Sex);
        message.append(contact.sex.toUtf8());
        sendMessage(message);

        message.clear();
        message.append(Headers::Phone);
        message.append(contact.phone.toUtf8());
        sendMessage(message);

        message.clear();
        message.append(Headers::EndSingleContact);
        sendMessage(message);
    }

    message.clear();
    message.append(Headers::SaveContacts);
    sendMessage(message);
}

void MainWidget::updatePhoneBookModel()
{
    _model->updateModelData(_contacts);
}
