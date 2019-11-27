#ifndef ENUMS_H
#define ENUMS_H
#include <QString>

enum Roles
{
    FirstNameRole = Qt::UserRole + 1,
    SecondNameRole,
    PatronymRole,
    SexRole,
    PhoneRole
};

enum DyalogWidgetModes
{
    EditMode,
    AddMode
};

enum ColumnHeaders
{
    SECONDNAME,
    FIRSTNAME,
    PATRONYM,
    SEX,
    PHONE,
};

enum Headers : char
{
    Unknown,
    ClearContacts,
    StartSingleContact,
    SecondName,
    FirstName,
    Patronym,
    Sex,
    Phone,
    EndSingleContact,
    SaveContacts
};

struct Item
{
    Item()
        : secondName("")
        , firstName("")
        , patronym("")
        , sex("")
        , phone("")
    {
    }
    void reset()
    {
        secondName.clear();
        firstName.clear();
        patronym.clear();
        sex.clear();
        phone.clear();
    }
    QString secondName;
    QString firstName;
    QString patronym;
    QString sex;
    QString phone;
};

#endif  // ENUMS_H
