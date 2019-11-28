#ifndef PHONEBOOKMODEL_H
#define PHONEBOOKMODEL_H

#include <QAbstractListModel>
#include <enums.h>

class PhoneBookModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PhoneBookModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    bool insertRows(int position, int count, const QModelIndex& parent = QModelIndex());
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


    void addContact(const QString& firstName, const QString& secondName, const QString& patronym, const QString& sex, const QString& phone);
    void removeContact(const QModelIndex& index);
    void editContact(QModelIndex& index, const QString& firstName, const QString& secondName, const QString& patronym, const QString& sex, const QString& phone);

    void resetModel();
    void updateModelData(const QMap<int, Item>& data);

private:
    QList<Item> _data;
};

#endif  // PHONEBOOKMODEL_H
