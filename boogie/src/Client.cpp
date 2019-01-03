#include "Client.h"
#include <iostream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>
#include <utility>
#include <iterator>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <ctime>
#include <tuple>
#include "../util/util.h"
#include <map>
#include <QCryptographicHash>

Client::Client(QObject* parrent)
    :QTcpSocket(parrent)
{
	std::cout << "Client created" << std::endl;
}

QString Client::getUsername() {
    return username;
}

void Client::connectToServer(QString username, QString ip, quint16 port) {
    connectToHost(ip, port);

	connect(this, SIGNAL(readyRead()), this, SLOT(readMsg()));

    this->username = username;

    std::cout << "Connected to host" << std::endl;
}

void Client::disconnectFromServer() {
    disconnectFromHost();
    std::cout << "Disconected from host" << std::endl;
}

QString Client::splitMessage(QString message) {
    int helper = 0;
    for(int i = 0; i < message.length(); i++){
        if(i - helper ==  50) {
            message.insert(i, "\n");
            helper = i;
        }
    }
    return message;
}

void Client::addNewContact(QString name, bool online) {

    if(contactInfos[name] != true)
        contactInfos[name] = online;
    emit clearContacts();
    for(auto &pair: contactInfos){
        emit showContacts(pair.first, pair.second);
    }
}

void Client::readMsg(){
    QByteArray messageLength = read(4);
    QJsonDocument jsonMsg = QJsonDocument::fromJson(read(messageLength.toInt()));
    QJsonObject jsonMsgObj = jsonMsg.object();
	auto msgType = jsonMsgObj["type"];

    if(jsonMsgObj.contains("to") && jsonMsgObj["to"].toString() != username) {
		qDebug() << "Received msg is not for " << username;
		return;
	}
	if(msgType == MessageType::Text){
        QString message = jsonMsgObj["msg"].toString();
		//message = splitMessage(message);
		qDebug() << message;
		addMsgToBuffer(jsonMsgObj["from"].toString(),
						jsonMsgObj["from"].toString(),
                        message);

		emit showMsg(jsonMsgObj["from"].toString(),
                    message);

	}
	else if(msgType == MessageType::Contacts){
		QJsonArray contactsJsonArray = jsonMsgObj["contacts"].toArray();
		for(auto con : contactsJsonArray){
			QJsonObject jsonObj = con.toObject();
            contactInfos[jsonObj["contact"].toString()] = jsonObj["online"].toBool();
            //emit showContacts(jsonObj["contact"].toString(), jsonObj["online"].toBool());
		}

        for(auto &pair: contactInfos){
            emit showContacts(pair.first, pair.second);
        }
	}
	else if(msgType == MessageType::ContactLogin){
        contactInfos[jsonMsgObj["contact"].toString()] = true;
        emit clearContacts();
        for(auto &pair: contactInfos){
            emit showContacts(pair.first, pair.second);
        }

	}
	else if(msgType == MessageType::ContactLogout){
        contactInfos[jsonMsgObj["contact"].toString()] = false;
        emit clearContacts();
        for(auto &pair: contactInfos){
            emit showContacts(pair.first, pair.second);
        }
	}
	else if(msgType == MessageType::AddNewContact) {
        if(jsonMsgObj["exists"].toBool() == true)
            addNewContact(jsonMsgObj["username"].toString(), jsonMsgObj["online"].toBool());
    }
	else if(msgType == MessageType::BadPass){
        qDebug() << "losa lozinka";
        emit badPass();
	}
	else if(msgType == MessageType::AllreadyLoggedIn){
		//TODO
	}
	else if(msgType == MessageType::BadMessageFormat){
		//TODO
	}
	else if(msgType == MessageType::UnknownUser){
		//TODO
	}
	else{
        qDebug() << "UNKNOWN MESSAGE TYPE";
	}

	//GOTTA CATCH 'EM ALL
	//when multiple messages arive at small time frame,
	//either signal is not emited or slot is not called(not sure)
	//so this is necessary for reading them all
	if(this->bytesAvailable() != 0){
		emit readyRead();
	}

}


void Client::sendMsg(QString str) {
    write(str.toLocal8Bit().data());
}

//dodajemo poruku u bafer
void Client::addMsgToBuffer(QString sender, QString inConversationWith, QString msg) {
    //hvatamo trenutno vreme i pretvaramo ga u string zbog lakseg upisivanja u xml
    auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    std::tm * ptm = std::localtime(&end_time);
    char time[32];
    std::strftime(time, 32, "%d.%m.%Y %H:%M:%S", ptm);
    if(msgDataBuffer.find(inConversationWith) == msgDataBuffer.end()) {
        msgInfos[inConversationWith] = 0;
    }
    msgDataBuffer[inConversationWith].push_back(std::make_tuple(sender, msg, time));
    counter++;
    if(counter == 10) {
        writeInXml();
    }
}

void Client::displayOnConvPage(QString inConversationWith) {
    auto messages = msgDataBuffer.find(inConversationWith);
    if(messages == msgDataBuffer.end())
        return;
    for(auto message: messages->second) {
        emit showMsg(std::get<0>(message), std::get<1>(message));
    }
}


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

//saljemo serveru username novog kontakta, da bi proverili da li taj kontakt postoji
void Client::checkNewContact(QString name){
    QJsonObject jsonObject;
	jsonObject.insert("type", setMessageType(MessageType::AddNewContact));
    jsonObject.insert("username", name);
	jsonObject.insert("from", this->username);
    if(!jsonObject.empty()) {
        QString fullMsgString = packMessage(jsonObject);
        sendMsg(fullMsgString);
    }
}

void Client::sendPicture(QString filePath) {
    qDebug() << filePath;
}

//saljemo poruku i podatke o njoj na server
void Client::sendMsgData(QString to, QString msg) {
	QJsonObject jsonMessageObject;
	jsonMessageObject.insert("type", setMessageType(MessageType::Text));
    jsonMessageObject.insert("from", username);
	jsonMessageObject.insert("to", to);
	jsonMessageObject.insert("msg", msg);
	if(!jsonMessageObject.empty()) {
		QString fullMsgString = packMessage(jsonMessageObject);
        sendMsg(fullMsgString);
    }
}

//saljemo podatke za proveru username i sifre na server
void Client::sendAuthData(QString password){
    QJsonObject jsonAuthObject;
    //pravimo json objekat u koji smestamo podatke
	jsonAuthObject.insert("type", setMessageType(MessageType::Authentication));
	QString hashedPassword =
			QString(QCryptographicHash::hash((password.toLocal8Bit().data()),
											 QCryptographicHash::Md5).toHex());
	jsonAuthObject.insert("password", hashedPassword);
	jsonAuthObject.insert("username", username);
	if(!jsonAuthObject.empty()) {
		QString fullAuthString = packMessage(jsonAuthObject);
        sendMsg(fullAuthString);
        //qDebug() << strJson << strJson.length() << fullAuthString;
    }
}
