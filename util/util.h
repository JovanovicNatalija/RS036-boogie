#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include <QJsonObject>


QString packMessage(const QJsonObject& dataToSend);

enum MessageType : int {Authentication = 1, Text, Contacts, Server};

#endif // UTIL_H
