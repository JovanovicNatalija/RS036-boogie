#include "Client.h"
#include <iostream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <utility>
#include <iterator>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <ctime>
#include <tuple>
//192.168.191.128
Client::Client(QObject* parrent)
    :QTcpSocket(parrent)
{
	std::cout << "Client created" << std::endl;
}

void Client::connectToServer(QString ip, quint16 port) {
    connectToHost(ip, port);

    connect(this, SIGNAL(readyRead()), this, SLOT(readMsg()));

    std::cout << "Connected to host" << std::endl;
}

void Client::readMsg(){
    QByteArray messageLength = read(4);
    QJsonDocument jsonMsg = QJsonDocument::fromJson(read(messageLength.toInt()));
    QJsonObject json = jsonMsg.object();
    emit showMsg(json["from"].toString(), json["msg"].toString());
}

void Client::disconnectFromServer() {
    disconnectFromHost();
    std::cout << "Disconected from host" << std::endl;
}

void Client::sendMsg(QString str) {
    write(str.toLocal8Bit().data());
}

//dodajemo poruku u bafer
void Client::addMsgToBuffer(QString from, QString to, QString msg) {
    appendToBuffer(from, to, msg);
}

//pisemo istoriju ceta u xml fajl
void Client::writeInXml(QString username) {
    QString filePath = username + ".xml";
    QFile data(filePath);
        if (!data.open(QFile::WriteOnly | QFile::Truncate))
            return;

    QXmlStreamWriter xml(&data);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("username");
    xml.writeAttribute("user", username);
    std::map<QString, std::vector<std::tuple<QString, QString, QString>>>::iterator it = msgDataBuffer.begin();
        while(it != msgDataBuffer.end())
        {
            xml.writeStartElement("inConversationWith");
            xml.writeAttribute("user", it->first);
            for(auto a: it->second) {
                xml.writeStartElement("message");
                xml.writeTextElement("sender", std::get<0>(a));
                xml.writeTextElement("text", std::get<1>(a));
                xml.writeTextElement("time", std::get<2>(a));
                xml.writeEndElement();
            }
            xml.writeEndElement();
            it++;
        }
    xml.writeEndElement();
    xml.writeEndDocument();
    data.close();
}

//saljemo poruku i podatke o njoj na server
void Client::sendMsgData(QString from, QString to, QString msg) {
    //pravimo json objekat u koji smestamo podatke
    QJsonObject json;
    json.insert("type", "m");
    json.insert("from", from);
    json.insert("to", to);
    json.insert("msg", msg);
    if(!json.empty()) {
        //pravimo QJsonDocument objekat da bi mogli da ga transformisemo u string
        QJsonDocument doc(json);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        QString strJsonLen = QString::number(strJson.length());
        //dodajemo 0 na pocetak stringa do duzine 4
        while(strJsonLen.length() < 4)
            strJsonLen = QString::number(0) + strJsonLen;
        QString fullMsgString = strJsonLen + strJson;
        sendMsg(fullMsgString);
		//qDebug() << strJs << strJs.length() << fullMsgString;
    }
}

//saljemo podatke za proveru username i sifre na server
void Client::sendAuthData(QString username, QString password){
    QJsonObject json;
    //pravimo json objekat u koji smestamo podatke
    json.insert("type", "a");
    json.insert("password", password);
    json.insert("username", username);
    if(!json.empty()) {
        //pravimo QJsonDocument objekat da bi mogli da ga transformisemo u string
        QJsonDocument doc(json);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        QString strJsonLen = QString::number(strJson.length());
        //dodajemo 0 na pocetak stringa do duzine 4
        while(strJsonLen.length() < 4)
            strJsonLen = QString::number(0) + strJsonLen;
        QString fullAuthString = strJsonLen + strJson;
        sendMsg(fullAuthString);
        //qDebug() << strJson << strJson.length() << fullAuthString;
    }
}

