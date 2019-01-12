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
#include <QImageReader>
#include <QPixmap>
#include <QBuffer>
#include <QUrl>
#include <iostream>
#include <QImageWriter>


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


void Client::connectToServer(const QString& ip,
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
        emit showContacts(i.key(), i.value(), -1);
    }
    for(auto group : m_groupInfos) {
        emit showGroups(group.groupName, true, group.id);
    }
}

void Client::readMsg(){
    if(m_bytesToRead == 0)//we dont have any unread data
    {
		m_bytesToRead = readMessageLenghtFromSocket(this);
    }

    if(bytesAvailable() < m_bytesToRead){
        return;
    }


    QJsonDocument jsonMsg = QJsonDocument::fromJson(read(m_bytesToRead));
    m_bytesToRead = 0;//we have read all there is for this message

    QJsonObject jsonMsgObj = jsonMsg.object();
    auto msgType = jsonMsgObj["type"];

    if(jsonMsgObj.contains("to") && jsonMsgObj["to"].toString() != m_username) {
        qDebug() << "Received msg is not for " << m_username;
		return;
	}

	if(msgType == MessageType::Text){
        QString message = jsonMsgObj["msg"].toString();
        if(jsonMsgObj.contains("groupId")){
            int groupId = jsonMsgObj["groupId"].toInt();
            addMsgToBuffer( true,
                            jsonMsgObj["from"].toString(),
                            jsonMsgObj["groupId"].toVariant().toString(),
                            message, QString("text"));

            emit showMsgForGroup(groupId,
                                 jsonMsgObj["from"].toString(),
                                 message);
        } else {
            addMsgToBuffer( false,
                            jsonMsgObj["from"].toString(),
                            jsonMsgObj["from"].toString(),
                            message, QString("text"));

            emit showMsg(jsonMsgObj["from"].toString(),
                         message);
        }
    }
    else if(msgType == MessageType::Contacts){
        QJsonArray contactsJsonArray = jsonMsgObj["contacts"].toArray();
        for(auto con : contactsJsonArray){
            QJsonObject jsonObj = con.toObject();
            m_contactInfos[jsonObj["contact"].toString()] = jsonObj["online"].toBool();
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
			addNewContact(jsonMsgObj["username"].toString(),
							jsonMsgObj["online"].toBool());
        else {
			emit badContact(QString("Traženi kontakt ne postoji!"));
        }
    }
    else if(msgType == MessageType::CreateGroup) {
        addGroup(jsonMsgObj);
    }
    else if(msgType == MessageType::Groups) {
        QJsonArray groupsArray = jsonMsgObj["groups"].toArray();
        for(auto gr: groupsArray){
            addGroup(gr.toObject());
        }
    }
    else if (msgType == MessageType::Image){
        QPixmap p;
        p.loadFromData(QByteArray::fromBase64(jsonMsgObj["msg"].toString().toLatin1()));
        QString path = m_username + QString("/img") +
                                QString::number(m_imgCounter) + QString(".png");
        QImageWriter writer(path, "png");
        QImage image = p.toImage();
        writer.write(image);

        if(jsonMsgObj.contains("groupId")){
            int groupId = jsonMsgObj["groupId"].toInt();
            showPictureForGroup(groupId,
                                jsonMsgObj["from"].toString(),
                                "file://" + QFileInfo(path).absoluteFilePath());

            addMsgToBuffer( true,
                            jsonMsgObj["from"].toString(),
                            jsonMsgObj["groupId"].toVariant().toString(),
                            "file://" + QFileInfo(path).absoluteFilePath(),
                            QString("image"));
        } else {
            showPicture(jsonMsgObj["from"].toString(),
                        "file://" + QFileInfo(path).absoluteFilePath());

            addMsgToBuffer( false,
                            jsonMsgObj["from"].toString(),
                            jsonMsgObj["from"].toString(),
                            "file://" + QFileInfo(path).absoluteFilePath(),
                            QString("image"));
        }

        m_imgCounter++;
    }
    else if(msgType == MessageType::BadPass){
        emit badPass();
    }
    else if(msgType == MessageType::AllreadyLoggedIn){
        emit alreadyLogIn();
    }
    else if(msgType == MessageType::BadMessageFormat){
		qDebug() << "Ne znaš da programiraš :p";
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

void Client::addGroup(const QJsonObject& grInfos) {
    QJsonArray membersJsonArray = grInfos["members"].toArray();
    QSet<QString> groupMembers;
    for(auto member: membersJsonArray){
        groupMembers.insert(member.toVariant().toString());
    }
    chatGroup gr;
    gr.groupName = grInfos["groupName"].toString();
    gr.members = groupMembers;
    gr.id = grInfos["groupId"].toInt();
    for(auto groups: m_groupInfos) {
        if(groups.id == gr.id){
            return;
        }
    }
    m_groupInfos.push_back(gr);
    refreshContactsAndGroups();
}

void Client::sendMsg(const QString& str) {
	writeMessageLengthToSocket(this, str.size());
    write(str.toLocal8Bit().data());
}

//dodajemo poruku u bafer
void Client::addMsgToBuffer(bool group,
                            const QString& sender,
                            const QString& inConversationWith,
                            const QString& msg,
                            const QString& type) {
    auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    std::tm* ptm = std::localtime(&end_time);
    char time[32];
    std::strftime(time, 32, "%d.%m.%Y %H:%M:%S", ptm);
    if(m_msgDataBuffer.find(inConversationWith) == m_msgDataBuffer.end()) {
        m_msgIndexBegin[inConversationWith] = 0;
    }
    auto pair = QPair<bool, std::tuple<QString, QString, QString, QString>>
            (group, std::make_tuple(type, sender, msg, time));
    m_msgDataBuffer[inConversationWith].push_back(pair);
    m_msgCounter++;
    if(m_msgCounter == MAX_UNWRITTEN_MESSAGES) {
        writeInXml();
    }
}

void Client::displayOnConvPage(QString inConversationWith) {
    auto messages = m_msgDataBuffer.find(inConversationWith);
    if(messages == m_msgDataBuffer.end())
        return;
    for(auto pair: messages.value()) {
        auto group = pair.first;
        auto message = pair.second;
        auto type = std::get<0>(message);
        auto sender = std::get<1>(message);
        auto msg = std::get<2>(message);
        if(group == true) {
            if(type == "text") {
                emit showMsgForGroup(inConversationWith.toInt(), sender, msg);
            } else {
                emit showPictureForGroup(inConversationWith.toInt(), sender, msg);
            }
        } else {
            if(type == "text") {
                emit showMsg(sender, msg);
            } else {
                emit showPicture(sender, msg);
            }
        }
    }
}


void Client::createXml() const {
    QString filePath = m_username + "/" + "Messages.xml";
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

void Client::writeInXml() {
    QString filePath = m_username + "/" + "Messages.xml";

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
            auto pair = messages.value()[i];
            auto inConversationWith = messages.key();
            auto message = pair.second;
            auto group = pair.first;
            auto type = std::get<0>(message);
            auto sender = std::get<1>(message);
            auto msg = std::get<2>(message);
            auto time = std::get<3>(message);

            xml.writeStartElement("message");
            xml.writeAttribute("type", type);
            xml.writeAttribute("group", group ? "true" : "false");
            xml.writeTextElement("inConversationWith", inConversationWith);
            xml.writeTextElement("sender", sender);
            xml.writeTextElement("content", msg);
            xml.writeTextElement("time", time);
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
    QString filePath = m_username + "/Messages.xml";

    QFile data(filePath);
    if (!data.open(QFile::ReadOnly))
        return;

    QXmlStreamReader xml(&data);

    while (!xml.atEnd()) {
        xml.readNextStartElement();
        if(xml.isStartElement() && xml.name() == "message") {
            auto attributes = xml.attributes();
            auto type = attributes[0].value().toString();
            auto group = attributes[1].value().toString();
            xml.readNextStartElement();
            QString inConversationWith = xml.readElementText();
            xml.readNextStartElement();
            QString sender = xml.readElementText();
            xml.readNextStartElement();
            QString msg = xml.readElementText();
            xml.readNextStartElement();
            QString time = xml.readElementText();
            auto pair = QPair<bool, std::tuple<QString, QString, QString, QString>>
                    (group == "true" ? true : false ,std::make_tuple(type, sender, msg, time));
            m_msgDataBuffer[inConversationWith].push_back(pair);

            if(m_msgIndexBegin.find(inConversationWith) == m_msgIndexBegin.end())
                m_msgIndexBegin[inConversationWith] = 1;
            else
                m_msgIndexBegin[inConversationWith]++;

            if(type == "image" && sender != m_username) {
                m_imgCounter++;
            }
       }
    }

    data.close();
}

void Client::checkNewContact(const QString& name) {
    QJsonObject jsonObject;
    jsonObject.insert("type", setMessageType(MessageType::AddNewContact));
    jsonObject.insert("username", name);
    jsonObject.insert("from", m_username);
    if(!jsonObject.empty()) {
        QString fullMsgString = packMessage(jsonObject);
        sendMsg(fullMsgString);
    }
}

void Client::addContactToGroupSet(const QString& contact) {
    if(!m_contactsInGroups.contains(contact)){
        m_contactsInGroups.insert(contact);
    }
}

void Client::removeContactFromGroupSet(const QString& contact) {
    if(m_contactsInGroups.contains(contact)){
        m_contactsInGroups.remove(contact);
    }
}

void Client::sendGroupInfos(const QString& groupName) {
    if(m_contactsInGroups.count() < 2){
        emit tooFewPeople(QString("Za grupu su potrebna bar 3 korisnika"));
        return;
    }
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
    refreshContactsAndGroups();
}

template <typename T>
void Client::sendImage(bool group, T to, const QString& filePath) {
    QImageReader reader(QUrl(filePath).toLocalFile());
    QImage img = reader.read();
    auto pix = QPixmap::fromImage(img);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, "png");
    auto const data = buffer.data().toBase64();

    QJsonObject jsonMessageObject;
    jsonMessageObject.insert("type", setMessageType(MessageType::Image));
    jsonMessageObject.insert("from", m_username);
    if(group == true) {
        jsonMessageObject.insert("groupId", to);
    } else {
        jsonMessageObject.insert("to", to);
    }
    jsonMessageObject.insert("id", QString("img") + QString::number(m_imageNum));
    jsonMessageObject.insert("msg", QLatin1String(data));
    QString msgString = packMessage(jsonMessageObject);
    sendMsg(msgString);
    flush();
    m_imageNum++;
}

void Client::sendPicture(const QString& to, const QString& filePath) {
    sendImage(false, to, filePath);
}

void Client::sendGroupPictureData(int groupId, const QString& filePath) {
    sendImage(true, groupId, filePath);
}

template <typename T>
void Client::sendMessage(bool group, T to, const QString& msg) {
    QJsonObject jsonMessageObject;
    jsonMessageObject.insert("type", setMessageType(MessageType::Text));
    jsonMessageObject.insert("from", m_username);
    if(group == true) {
        jsonMessageObject.insert("groupId", to);
    } else {
        jsonMessageObject.insert("to", to);
    }
    jsonMessageObject.insert("msg", msg);
    if(!jsonMessageObject.empty()) {
        QString fullMsgString = packMessage(jsonMessageObject);
        sendMsg(fullMsgString);
    }
}

void Client::sendGroupMsgData(int groupId, const QString& msg) {
    sendMessage(true, groupId, msg);
}

void Client::sendMsgData(const QString& to,const QString& msg) {
   sendMessage(false, to, msg);
}

void Client::sendAuthData(const QString& username, const QString& password){
    m_username = username;

    QDir dir(m_username);
    if (!dir.exists())
        QDir().mkdir(m_username);

    QJsonObject jsonAuthObject;
    jsonAuthObject.insert("type", setMessageType(MessageType::Authentication));
    QString hashedPassword =
            QString(QCryptographicHash::hash((password.toLocal8Bit().data()),
                                             QCryptographicHash::Md5).toHex());
    jsonAuthObject.insert("password", hashedPassword);
    jsonAuthObject.insert("username", m_username);
    if(!jsonAuthObject.empty()) {
        QString fullAuthString = packMessage(jsonAuthObject);
        sendMsg(fullAuthString);
    }
}

QString Client::groupNameFromId(int id) {
    for(auto group: m_groupInfos) {
        if(group.id == id)
            return group.groupName;
    }
    return "";
}



