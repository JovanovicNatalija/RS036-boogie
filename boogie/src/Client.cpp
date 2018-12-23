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

//ok
Client::Client(QObject* parrent)
    :QTcpSocket(parrent)
{
	std::cout << "Client created" << std::endl;
}

QString Client::getUsername() {
    return username;
}

//ok
void Client::connectToServer(QString username, QString ip, quint16 port) {
    connectToHost(ip, port);

    connect(this, SIGNAL(readyRead()), this, SLOT(readMsg()));

    this->username = username;

    std::cout << "Connected to host" << std::endl;
}

//ok
void Client::disconnectFromServer() {
    disconnectFromHost();
    std::cout << "Disconected from host" << std::endl;
}

//ok
void Client::readMsg(){
    QByteArray messageLength = read(4);
    QJsonDocument jsonMsg = QJsonDocument::fromJson(read(messageLength.toInt()));
    QJsonObject jsonMsgObj = jsonMsg.object();
    if(jsonMsgObj["to"].toString() != username) {
        qDebug() << "Received msg is not for " << username;
    } else {
        addMsgToBuffer(jsonMsgObj["from"].toString(), jsonMsgObj["from"].toString(), jsonMsgObj["msg"].toString());
        emit showMsg(jsonMsgObj["from"].toString(), jsonMsgObj["msg"].toString());
    }
}

//ok
void Client::sendMsg(QString str) {
    write(str.toLocal8Bit().data());
}

//dodajemo poruku u bafer
//ok radi ali da li je potrebna ?
void Client::addMsgToBuffer(QString sender, QString inConversationWith, QString msg) {
    //hvatamo trenutno vreme i pretvaramo ga u string zbog lakseg upisivanja u xml
    auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    std::tm * ptm = std::localtime(&end_time);
    char time[32];
    std::strftime(time, 32, "%a, %d.%m.%Y %H:%M:%S", ptm);
    if(msgDataBuffer.find(inConversationWith) == msgDataBuffer.end()) {
        msgInfos[inConversationWith] = 0;
    }
    msgDataBuffer[inConversationWith].push_back(std::make_tuple(sender, msg, time));
    counter++;
    if(counter == 10) {
        writeInXml();
    }
}

//ok
void Client::displayOnConvPage(QString inConversationWith) {
    auto messages = msgDataBuffer.find(inConversationWith);
    if(messages == msgDataBuffer.end())
        return;
    for(auto message: messages->second) {
        emit showMsg(std::get<0>(message), std::get<1>(message));
    }
}

//ok
void Client::createXml() {
    QString filePath = username + ".xml";
    QFile data(filePath);
          if (!data.open(QFile::WriteOnly | QFile::Truncate))
              return;

    QXmlStreamWriter xml(&data);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    QTextStream stream(&data);
    stream << "\n<messages>\n</messages>";

    data.close();
}

// pisemo istoriju ceta u xml fajl
// ok
void Client::writeInXml() {
    QString filePath = username + ".xml";

    if(!(QFileInfo::exists(filePath) && QFileInfo(filePath).isFile())) {
        createXml();
    }

    QFile data(filePath);
        if (!data.open(QFile::ReadWrite | QFile::Text))
            return;

    QXmlStreamWriter xml(&data);
    xml.setAutoFormatting(true);

    qint64 offset = data.size() - QString("</messages>").size() - 1;
    data.seek(offset);

    auto messages = msgDataBuffer.begin();
    while(messages != msgDataBuffer.end())
    {
        auto from = msgInfos[messages->first];
        auto to = messages->second.size();
        for(auto i = from; i < to; i++) {
            auto message = messages->second[i];
            xml.writeStartElement("message");
            xml.writeTextElement("inConversationWith", messages->first);
            xml.writeTextElement("sender", std::get<0>(message));
            xml.writeTextElement("text", std::get<1>(message));
            xml.writeTextElement("time", std::get<2>(message));
            xml.writeEndElement();
        }
        msgInfos[messages->first] = to;
        messages++;
    }

    QTextStream stream(&data);
    stream << "\n</messages>";
    data.close();
    counter = 0;
}

//ok
void Client::readFromXml() {
    QString filePath = username + ".xml";

    QFile data(filePath);
    if (!data.open(QFile::ReadOnly))
        return;

    QXmlStreamReader xml(&data);

    while (!xml.atEnd()) {
        xml.readNextStartElement();
        if(xml.isStartElement() && xml.name() == "message") {
            xml.readNextStartElement();
            QString inConversationWith = xml.readElementText();
            xml.readNextStartElement();
            QString sender = xml.readElementText();
            xml.readNextStartElement();
            QString text = xml.readElementText();
            xml.readNextStartElement();
            QString time = xml.readElementText();
            msgDataBuffer[inConversationWith].push_back(std::make_tuple(sender, text, time));
            if(msgInfos.find(inConversationWith) == msgInfos.end())
                msgInfos[inConversationWith] = 1;
            else
                msgInfos[inConversationWith]++;
       }
    }

    data.close();
}


//saljemo poruku i podatke o njoj na server
void Client::sendMsgData(QString to, QString msg) {
    //pravimo json objekat u koji smestamo podatke
    QJsonObject json;
    json.insert("type", "m");
    json.insert("from", username);
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
void Client::sendAuthData(QString password){
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
