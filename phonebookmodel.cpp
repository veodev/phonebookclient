#include "phonebookmodel.h"

#include <enums.h>
#include <QDebug>

PhoneBookModel::PhoneBookModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PhoneBookModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return _data.size();
}

int PhoneBookModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 5;
}

QVariant PhoneBookModel::data(const QModelIndex& index, int role) const
{
    int currentRow = index.row();
    if (index.isValid() == false || currentRow >= _data.size()) {
        return QVariant();
    }

    int currentColumn = index.column();
    if (index.isValid() == false || currentColumn >= 5) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        switch (currentColumn) {
        case 0:
            return _data[currentRow].secondName;
        case 1:
            return _data[currentRow].firstName;
        case 2:
            return _data[currentRow].patronym;
        case 3:
            return _data[currentRow].sex;
        case 4:
            return _data[currentRow].phone;
        }
    }


    switch (role) {
    case FirstNameRole:
        return _data[currentRow].firstName;
    case SecondNameRole:
        return _data[currentRow].secondName;
    case PatronymRole:
        return _data[currentRow].patronym;
    case SexRole:
        return _data[currentRow].sex;
    case PhoneRole:
        return _data[currentRow].phone;
    }

    return QVariant();
}

bool PhoneBookModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int currentRow = index.row();
    if (index.isValid() == false || currentRow >= _data.size()) {
        return false;
    }

    switch (role) {
    case FirstNameRole:
        _data[currentRow].firstName = value.toString();
        break;
    case SecondNameRole:
        _data[currentRow].secondName = value.toString();
        break;
    case PatronymRole:
        _data[currentRow].patronym = value.toString();
        break;
    case SexRole:
        _data[currentRow].sex = value.toString();
        break;
    case PhoneRole:
        _data[currentRow].phone = value.toString();
        break;
    }
    emit dataChanged(index, index, QVector<int>() << role);
    return true;
}

bool PhoneBookModel::insertRows(int position, int count, const QModelIndex& parent)
{
    Q_UNUSED(parent)
    beginInsertRows(QModelIndex(), position, position + count - 1);
    for (int i = position; i < position + count; ++i) {
        _data.insert(_data.begin() + i, Item());
    }
    endInsertRows();
    return true;
}

QVariant PhoneBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return section + 1;
    }

    switch (section) {
    case SECONDNAME:
        return QString("Secondname");
    case FIRSTNAME:
        return QString("Firstname");
    case PATRONYM:
        return QString("Patronym");
    case SEX:
        return QString("Sex");
    case PHONE:
        return QString("Phone");
    }

    return QVariant();
}

void PhoneBookModel::addContact(const QString& firstName, const QString& secondName, const QString& patronym, const QString& sex, const QString& phone)
{
    Item item;
    item.firstName = firstName;
    item.secondName = secondName;
    item.patronym = patronym;
    item.sex = sex;
    item.phone = phone;

    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    _data.append(item);
    endInsertRows();

    emit dataChanged(index(0, 0), index(_data.size() - 1, 0));
}

void PhoneBookModel::removeContact(const QModelIndex& index)
{
    if (index.isValid()) {
        int row = index.row();
        beginRemoveRows(QModelIndex(), row, row);
        _data.removeAt(row);
        endRemoveRows();
        emit dataChanged(this->index(0, 0), this->index(_data.size() - 1, 0));
    }
}

void PhoneBookModel::editContact(QModelIndex& index, const QString& firstName, const QString& secondName, const QString& patronym, const QString& sex, const QString& phone)
{
    setData(index, firstName, FirstNameRole);
    setData(index, secondName, SecondNameRole);
    setData(index, patronym, PatronymRole);
    setData(index, sex, SexRole);
    setData(index, phone, PhoneRole);
}

void PhoneBookModel::resetModel()
{
    beginResetModel();
    _data.clear();
    endResetModel();
}

void PhoneBookModel::updateModelData(const QMap<int, Item>& data)
{
    resetModel();
    for (const auto& item : data) {
        _data.append(item);
    }
    emit dataChanged(this->index(0, 0), this->index(_data.size() - 1, 0));
}
