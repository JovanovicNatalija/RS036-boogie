#include "util.h"

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>


QString packMessage(const QJsonObject& dataToSend){
	QJsonDocument tmp(dataToSend);
	QString jsonToString{tmp.toJson(QJsonDocument::Compact)};
	QString strJsonLen = QString::number(jsonToString.length());

	while(strJsonLen.length() < 4)
		strJsonLen = QString::number(0) + strJsonLen;

	return strJsonLen+jsonToString;
}
