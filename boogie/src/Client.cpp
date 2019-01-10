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
#include <QSet>

Client::Client(QObject* parrent)
	:QSslSocket(parrent)
{

	//setting ssl certificates
	setLocalCertificate("../certs/blue_local.pem");
	setPrivateKey("../certs/blue_local.key");
	setPeerVerifyMode(QSslSocket::VerifyPeer);
	connect(this,  SIGNAL(sslErrors(QList<QSslError>)), this,
			SLOT(sslErrors(QList<QSslError>)));

	std::cout << "Client created" << std::endl;
}

QString Client::username() {
    return m_username;
}

void Client::sslErrors(const QList<QSslError> &errors)
{
	foreach (const QSslError &error, errors){
		if(error.error() == QSslError::SelfSignedCertificate){//ignoring self signed cert
			QList<QSslError> expectedSslErrors;
			expectedSslErrors.append(error);
			ignoreSslErrors(expectedSslErrors);
		}
	}
}


void Client::connectToServer(const QString& username, const QString& ip,
							 quint16 port) {
	connectToHostEncrypted(ip, port);

	if(waitForEncrypted(3000)){
		qDebug() << "Encrypted connection established";
	}
	else{
		std::cerr << "Unable to connect to server" << std::endl;
		return;
	}
	connect(this, SIGNAL(readyRead()), this, SLOT(readMsg()));

    this->m_username = username;

}

void Client::disconnectFromServer() {
    disconnectFromHost();
    std::cout << "Disconected from host" << std::endl;
}

void Client::addNewContact(const QString& name, bool online) {

    if(m_contactInfos[name] != true)
        m_contactInfos[name] = online;
    refreshContactsAndGroups();
}

void Client::refreshContactsAndGroups() {
    emit clearContacts();
    for(auto i = m_contactInfos.cbegin(); i != m_contactInfos.cend(); i++){
        emit showContacts(i.key(), i.value());
    }
    for(auto group : m_groupInfos) {
        emit showContacts(group.groupName, true);
    }
}

void Client::readMsg() {
    QByteArray messageLength = read(4);
    QJsonDocument jsonMsg = QJsonDocument::fromJson(read(messageLength.toInt()));
    QJsonObject jsonMsgObj = jsonMsg.object();
	auto msgType = jsonMsgObj["type"];

    if(jsonMsgObj.contains("to") && jsonMsgObj["to"].toString() != m_username) {
        qDebug() << "Received msg is not for " << m_username;
		return;
	}
	if(msgType == MessageType::Text){
        QString message = jsonMsgObj["msg"].toString();
		//message = splitMessage(message);
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
            m_contactInfos[jsonObj["contact"].toString()] = jsonObj["online"].toBool();
            //emit showContacts(jsonObj["contact"].toString(), jsonObj["online"].toBool());
		}
        refreshContactsAndGroups();
	}
	else if(msgType == MessageType::ContactLogin){
        m_contactInfos[jsonMsgObj["contact"].toString()] = true;
        refreshContactsAndGroups();
	}
	else if(msgType == MessageType::ContactLogout){
        m_contactInfos[jsonMsgObj["contact"].toString()] = false;
        refreshContactsAndGroups();
	}
	else if(msgType == MessageType::AddNewContact) {
        if(jsonMsgObj["exists"].toBool() == true)
            addNewContact(jsonMsgObj["username"].toString(), jsonMsgObj["online"].toBool());
        else {
            emit badContact(QString("Trazeni kontakt ne postoji!"));
        }
    }
    else if(msgType == MessageType::CreateGroup) {
        QJsonArray membersJsonArray = jsonMsgObj["members"].toArray();
        QSet<QString> groupMembers;
        for(auto member: membersJsonArray){
            groupMembers.insert(member.toVariant().toString());
        }
        chatGroup gr;
        gr.groupName = jsonMsgObj["groupName"].toString();
        gr.members = groupMembers;
        gr.id = jsonMsgObj["id"].toInt();
        m_groupInfos.push_back(gr);
        refreshContactsAndGroups();
    }
	else if(msgType == MessageType::BadPass){
        emit badPass();
	}
	else if(msgType == MessageType::AllreadyLoggedIn){
        emit alreadyLogIn();
	}
	else if(msgType == MessageType::BadMessageFormat){
        qDebug() << "Ne znas da programiras :p";
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

void Client::sendMsg(const QString& str) {
    write(str.toLocal8Bit().data());
}

//dodajemo poruku u bafer
void Client::addMsgToBuffer(const QString& sender,const QString& inConversationWith,const QString& msg) {
    //hvatamo trenutno vreme i pretvaramo ga u string zbog lakseg upisivanja u xml
    auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    std::tm * ptm = std::localtime(&end_time);
    char time[32];
    std::strftime(time, 32, "%d.%m.%Y %H:%M:%S", ptm);
    if(m_msgDataBuffer.find(inConversationWith) == m_msgDataBuffer.end()) {
        m_msgIndexBegin[inConversationWith] = 0;
    }
    m_msgDataBuffer[inConversationWith].push_back(std::make_tuple(sender, msg, time));
    m_msgCounter++;
    if(m_msgCounter == 10) {
        writeInXml();
    }
}

void Client::displayOnConvPage(const QString& inConversationWith) {
    auto messages = m_msgDataBuffer.find(inConversationWith);
    if(messages == m_msgDataBuffer.end())
        return;
    for(auto message: messages.value()) {
        emit showMsg(std::get<0>(message), std::get<1>(message));
    }
}


void Client::createXml() const {
    QString filePath = m_username + ".xml";
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

void Client::writeInXml() {
    QString filePath = m_username + ".xml";

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

    auto messages = m_msgDataBuffer.begin();
    while(messages != m_msgDataBuffer.end())
    {
        auto from = m_msgIndexBegin[messages.key()];
        auto to = messages.value().size();
        for(auto i = from; i < to; i++) {
            auto message = messages.value()[i];
            xml.writeStartElement("message");
            xml.writeTextElement("inConversationWith", messages.key());
            xml.writeTextElement("sender", std::get<0>(message));
            xml.writeTextElement("text", std::get<1>(message));
            xml.writeTextElement("time", std::get<2>(message));
            xml.writeEndElement();
        }
        m_msgIndexBegin[messages.key()] = to;
        messages++;
    }

    QTextStream stream(&data);
    stream << "\n</messages>";
    data.close();
    m_msgCounter = 0;
}

void Client::readFromXml() {
    QString filePath = m_username + ".xml";

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
            m_msgDataBuffer[inConversationWith].push_back(std::make_tuple(sender, text, time));
            if(m_msgIndexBegin.find(inConversationWith) == m_msgIndexBegin.end())
                m_msgIndexBegin[inConversationWith] = 1;
            else
                m_msgIndexBegin[inConversationWith]++;
       }
    }

    data.close();
}

//saljemo serveru username novog kontakta, da bi proverili da li taj kontakt postoji
void Client::checkNewContact(const QString& name) {
    if(name == m_username) {
        badContact(QString("Ne mozete pricati sami sa sobim,\n nije socijalno prihvatljivo"));
    }
    QJsonObject jsonObject;
	jsonObject.insert("type", setMessageType(MessageType::AddNewContact));
    jsonObject.insert("username", name);
    jsonObject.insert("from", this->m_username);
    if(!jsonObject.empty()) {
        QString fullMsgString = packMessage(jsonObject);
        sendMsg(fullMsgString);
    }
}

void Client::addContactToGroupSet(QString contact) {
    if(!m_contactsInGroups.contains(contact)){
        m_contactsInGroups.insert(contact);
    }
}

void Client::removeContactFromGroupSet(QString contact) {
    if(m_contactsInGroups.contains(contact)){
        m_contactsInGroups.remove(contact);
    }
}

void Client::sendGroupInfos(QString groupName) {
    m_contactsInGroups.insert(m_username);
    QJsonObject groupDataJson;
    QJsonArray membersArrayJson;
    std::copy(m_contactsInGroups.begin(),
                   m_contactsInGroups.end(),
                   std::back_insert_iterator<QJsonArray>(membersArrayJson));
    groupDataJson.insert("type", setMessageType(MessageType::CreateGroup));
	groupDataJson.insert("members", membersArrayJson);
    groupDataJson.insert("groupName", groupName);
    if(!groupDataJson.empty()) {
        QString fullMsgString = packMessage(groupDataJson);
        sendMsg(fullMsgString);
    }
    refreshContactsAndGroups();
}

void Client::clearGroupSet() {
    m_contactsInGroups.clear();
}
void Client::sendPicture(const QString& filePath)  {
    qDebug() << filePath;
}

//saljemo poruku i podatke o njoj na server
void Client::sendMsgData(const QString& to,const QString& msg) {
	QJsonObject jsonMessageObject;
	jsonMessageObject.insert("type", setMessageType(MessageType::Text));
    jsonMessageObject.insert("from", m_username);
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
    jsonAuthObject.insert("username", m_username);
	if(!jsonAuthObject.empty()) {
		QString fullAuthString = packMessage(jsonAuthObject);
        sendMsg(fullAuthString);
        //qDebug() << strJson << strJson.length() << fullAuthString;
    }
}
