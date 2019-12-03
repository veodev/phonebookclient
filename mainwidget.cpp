#include "mainwidget.h"
#include "enums.h"
#include "ui_mainwidget.h"

#include <QHostAddress>
#include <QtEndian>
#include <QTimer>
#include <QtXml>

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
    _dialogWidget->popup();
}

void MainWidget::on_deleteButton_clicked()
{
    _model->removeContact(_currentModelIndex);
    _currentModelIndex = QModelIndex();
}

void MainWidget::on_editButton_clicked()
{
    if (_model) {
        int row = _currentModelIndex.row();
        _dialogWidget->setSecondName(_model->data(_model->index(row, 0)).toString());
        _dialogWidget->setFirstName(_model->data(_model->index(row, 1)).toString());
        _dialogWidget->setPatronym(_model->data(_model->index(row, 2)).toString());
        _dialogWidget->setSex(_model->data(_model->index(row, 3)).toString());
        _dialogWidget->setPhone(_model->data(_model->index(row, 4)).toString());
        _dialogWidget->changeMode(DyalogWidgetModes::EditMode);
        _dialogWidget->popup();
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
                case Headers::SingleContact: {
                    QByteArray data = _messagesBuffer.left(size - sizeof(Headers));
                    parseContact(data);
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

    updateServerData();
}

void MainWidget::createModel()
{
    _model = new PhoneBookModel(this);
    connect(_model, &QAbstractItemModel::dataChanged, this, &MainWidget::modelDataChanged);
    connect(_model, &PhoneBookModel::doAddNewContact, this, &MainWidget::updateServerData);
}

void MainWidget::sendMessage(QByteArray& message)
{
    if (_tcpSocket != nullptr) {
        quint16 size = qToLittleEndian<quint16>(static_cast<quint16>(message.size()));
        _tcpSocket->write(reinterpret_cast<char*>(&size), sizeof(quint16));
        _tcpSocket->write(message);
        _tcpSocket->flush();
    }
}

void MainWidget::updateServerData()
{
    copyDataFromModel();

    QByteArray message;
    message.append(Headers::ClearContacts);
    sendMessage(message);

    for (int i = 0; i < _contacts.size(); ++i) {
        QByteArray data;
        QXmlStreamWriter xmlWriter(&data);
        xmlWriter.writeStartElement("item");
        xmlWriter.writeAttribute("id", QString("%1").arg(i));

        xmlWriter.writeStartElement("secondname");
        xmlWriter.writeCharacters(_contacts[i].secondName);
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("firstname");
        xmlWriter.writeCharacters(_contacts[i].firstName);
        xmlWriter.writeEndElement();


        xmlWriter.writeStartElement("patronym");
        xmlWriter.writeCharacters(_contacts[i].patronym);
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("sex");
        xmlWriter.writeCharacters(_contacts[i].sex);
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("phone");
        xmlWriter.writeCharacters(_contacts[i].phone);
        xmlWriter.writeEndElement();

        xmlWriter.writeEndElement();

        message.clear();
        message.append(Headers::SingleContact);
        message.append(data);
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

void MainWidget::copyDataFromModel()
{
    _contacts.clear();
    for (int i = 0; i < _model->rowCount(); ++i) {
        Item item;
        item.secondName = _model->data(_model->index(i, 0)).toString();
        item.firstName = _model->data(_model->index(i, 1)).toString();
        item.patronym = _model->data(_model->index(i, 2)).toString();
        item.sex = _model->data(_model->index(i, 3)).toString();
        item.phone = _model->data(_model->index(i, 4)).toString();
        _contacts.push_back(item);
    }
}

void MainWidget::parseContact(QByteArray& data)
{
    QXmlStreamReader xmlReader(data);
    bool isContactFilled = false;
    while (!xmlReader.atEnd()) {
        if (xmlReader.isStartElement()) {
            if (xmlReader.name() == "item") {
                _itemIndex = xmlReader.attributes().at(0).value().toInt();
                _bufferItem.reset();
                isContactFilled = false;
            }
            if (xmlReader.isStartElement() && xmlReader.name() == "secondname") {
                _bufferItem.secondName = xmlReader.readElementText();
            }
            if (xmlReader.isStartElement() && xmlReader.name() == "firstname") {
                _bufferItem.firstName = xmlReader.readElementText();
            }
            if (xmlReader.isStartElement() && xmlReader.name() == "patronym") {
                _bufferItem.patronym = xmlReader.readElementText();
            }
            if (xmlReader.isStartElement() && xmlReader.name() == "sex") {
                _bufferItem.sex = xmlReader.readElementText();
            }
            if (xmlReader.isStartElement() && xmlReader.name() == "phone") {
                _bufferItem.phone = xmlReader.readElementText();
                isContactFilled = true;
            }
        }

        if (isContactFilled) {
            _contacts.push_back(_bufferItem);
            isContactFilled = false;
        }
        xmlReader.readNext();
    }
}
