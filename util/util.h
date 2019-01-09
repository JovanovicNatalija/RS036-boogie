#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QJsonObject>

typedef struct{
    QString groupName;
    QSet members;
    int id;
    QSet admins;
} chatGroup;

QString packMessage(const QJsonObject& dataToSend);

enum class MessageType : int {Authentication = 1, Text, Contacts, ContactLogout,
							  ContactLogin, BadPass, AllreadyLoggedIn,
                              BadMessageFormat, AddNewContact, CreateGroup};

bool operator==(const QJsonValue &v, const MessageType& type);


QJsonValue setMessageType(const MessageType& type);

#endif // UTIL_H
