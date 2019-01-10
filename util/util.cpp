#include "util.h"

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>


QString packMessage(const QJsonObject& dataToSend){
	QJsonDocument tmp(dataToSend);
	QString jsonToString{tmp.toJson(QJsonDocument::Compact)};
	//QString strJsonLen = QString::number(jsonToString.length());

//	while(strJsonLen.length() < 4)
//		strJsonLen = QString::number(0) + strJsonLen;

	return /*strJsonLen+*/jsonToString;
}

QJsonValue setMessageType(const MessageType& type){
	return QJsonValue(static_cast<int>(type));
}

bool operator==(const QJsonValue &v, const MessageType& type){
	return v.toInt() == static_cast<int>(type);
}
int readMessageLenghtFromSocket(QSslSocket* socket)
{
	int length;
	QByteArray messageLength = socket->read(4);
	QDataStream stream(&messageLength, QIODevice::ReadOnly);
	stream.setVersion(QDataStream::Qt_5_10); //to ensure that new version of QDataStream wouldn't change much
	stream >> length;
	return length;
}
bool writeMessageLengthToSocket(QSslSocket* socket, int length)
{
	QByteArray byteArray;
	QDataStream stream(&byteArray, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_5_10); //to ensure that new version of DataStream wouldn't change much
	stream << length;
	bool success = socket->write(byteArray, 4) != -1;
	if(success)
		socket->flush();
	return success;

}

