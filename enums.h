#ifndef ENUMS_H
#define ENUMS_H
#include <QString>

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
    SaveContacts,
    SingleContact
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
