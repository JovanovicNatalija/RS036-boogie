#include <QTcpSocket>

#include <QCoreApplication>
#include <QDir>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDomDocument>

#include <iostream>

#include "Server.h"
#include "../util/util.h"

Server::~Server(){}

Server::Server(quint16 port)
{
	m_isInitialized = true;
	if(port <= 1024){//first 1024 ports are not to be touched
		m_errorMessage = "BAD PORT NUMBER";
		m_isInitialized = false;
	}

	bool listening = listen(QHostAddress::Any, port);
	if(listening == false){
		m_errorMessage = "SERVER ERROR, CAN'T LISTEN ON PORT "
						 + std::to_string(port);
		m_isInitialized = false;
	}

	loadData();

	connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
	qDebug("server created");
}

void Server::loadData(){
	QFile dataFile{DATA_FILE_PATH};

	if(!dataFile.open(QIODevice::ReadWrite | QIODevice::Text)){
		qDebug() << "OPENING FILE AT " + DATA_FILE_PATH + " FAILED";
		m_errorMessage = "OPENING DATA FILE FAILED";
		m_isInitialized = false;
		return;
	}
	else{
		//if file is empty, intialize it with empty xml tag so that setContent
		//would'nt fail
		if(dataFile.size() == 0){
			dataFile.write("<xml></xml>");
			dataFile.flush();
			dataFile.close();
			dataFile.open(QIODevice::ReadWrite | QIODevice::Text);
		}
		if(!m_dataDoc.setContent(&dataFile)){
			qDebug() << "LOADING FILE AT " + DATA_FILE_PATH + " FAILED";
			m_errorMessage = "LOADING DATA FILE FAILED";
			m_isInitialized = false;
			return;
		}
		dataFile.close();
	}
	QDomElement root = m_dataDoc.firstChildElement();
	m_users = root.elementsByTagName("user");

	//for each user tag
	for(int i = 0; i < m_users.count(); ++i){
		auto currUserNode = m_users.at(i);

		//store username and password
		auto currUsername =
				currUserNode.attributes().namedItem("username").nodeValue();
		auto password = currUserNode.firstChildElement("password").text();
		m_authData[currUsername] = password;

		//get list of contacts
		auto contacts = currUserNode.firstChildElement("contacts")
				.elementsByTagName("contact");
		for(int j = 0; j < contacts.count(); ++j){
			m_contacts[currUsername].append(contacts.at(j).toElement().text());
		}
	}
}
void Server::newConnection(){
	if(this->hasPendingConnections()){
		QTcpSocket* client = this->nextPendingConnection();

		connect(client, &QTcpSocket::disconnected, this,
				&Server::userDisconnected);
		connect(client, &QTcpSocket::readyRead, this, &Server::readMessage);

	}
}

void Server::notifyContacts(const QString& username, const MessageType& m) const
{
	for(auto user : m_contacts[username]){
		QTcpSocket* tmp = m_usernameToSocket[user];
		if(tmp == nullptr)//contact not online
			continue;
		QJsonObject notification;
		notification.insert("type", setMessageType(m));
		notification.insert("to", user);
		notification.insert("contact", username);
		sendMessageTo(tmp, notification);
	}
}


void Server::userDisconnected(){

	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
	QString username = m_usernameToSocket.key(disconnectedClient);//linear time but its called rarely so its ok
	if(username == ""){
		qDebug() << "WHOOPS! Disconected signal came from socket that was not connected.CHECK!";
		return;
	}
	m_usernameToSocket.remove(username);

	//notify all contacts that user is offline
	notifyContacts(username, MessageType::ContactLogout);

	qDebug() << "User " << username << " OUT";
	disconnectedClient->deleteLater();
}

bool Server::sendMessageTo(QTcpSocket* recepient, const QJsonObject& message) const
{
	QString tmp = packMessage(message);
	if(recepient->write(tmp.toLocal8Bit().data()) != -1){
		recepient->flush();
		return true;
	}

	else return false;
}

bool Server::sendServerMessageTo(QTcpSocket* receipient, const MessageType& msgType
								 , const QString& username) const
{
	QJsonObject response;
	response.insert("type", setMessageType(msgType));
	if(username != ""){
		response.insert("to", username);
	}
	bool ret =  -1 != receipient->write(packMessage(response).toLocal8Bit().data());
	if(ret){
		receipient->flush();
	}
	return ret;

}

void Server::saveXMLFile() const
{
	QFile dataFile{DATA_FILE_PATH};
	if(!dataFile.open(QIODevice::WriteOnly)){
		qDebug() << "FAILED OPENING XML DATA";
		return;
	}
	QTextStream output{&dataFile};
	output << m_dataDoc.toString();
	qDebug() << "FILE SAVED";
	//QFile is RAII, no need to close file

}

QDomElement Server::createNewXmlElement(const QString& tagName,
										const QString& text,
										const QString& attribute,
										const QString& value){
	QDomElement newElement = m_dataDoc.createElement(tagName);
	if(text != ""){
		QDomText newElementText = m_dataDoc.createTextNode(text);
		newElement.appendChild(newElementText);
	}
	if(attribute != ""){
		newElement.setAttribute(attribute, value);
	}

	return newElement;
}

//adds contact to array of contacts and to DOM
void Server::addNewContact(const QString& user, const QString& contact)
{
	m_contacts[user].append(contact);

	//first user in xml
	auto userDomElement = m_users.at(0);
	int i = 1;
	//find dom element for given user
	while(userDomElement.attributes().namedItem("username").nodeValue()
			!= user){
		userDomElement = m_users.at(i++);
	}
	QDomElement newContact = createNewXmlElement("contact", contact);
	userDomElement.firstChildElement("contacts").appendChild(newContact);
	saveXMLFile();
}

bool isNumeric(QByteArray arr){
	return std::all_of(arr.begin(), arr.end(),
					   [](char c){return isdigit(c);});
}

void Server::sendContactsFor(QString username, QTcpSocket* senderSocket) const
{
	QJsonObject contactsDataJson;
	QJsonArray contactsArrayJson;
	std::for_each(m_contacts[username].begin(),
				  m_contacts[username].end(),
				  [&](QString s){
						QJsonObject tmp;
						tmp.insert("contact", s);
						tmp.insert("online", m_usernameToSocket.contains(s));
						contactsArrayJson.append(tmp);
					}
					);

	contactsDataJson.insert("type", setMessageType(MessageType::Contacts));
	contactsDataJson.insert("to",username);
	contactsDataJson.insert("contacts", contactsArrayJson);
	sendMessageTo(senderSocket, contactsDataJson);
}

void Server::sendUnreadMessages(const QString& username, QTcpSocket* socket)
{
	qDebug() << "Sending unread messages for " << username;
	for(auto message : m_unreadMessages[username]){
		qDebug() << QJsonDocument(message).toJson(QJsonDocument::Compact);
		sendMessageTo(socket, message);
	}
	m_unreadMessages.remove(username);

}

bool Server::hasUnreadMessages(const QString& username) const
{
	return m_unreadMessages.contains(username);
}

void Server::authentication(QJsonObject jsonResponseObject, QTcpSocket* senderSocket)
{
	if(m_usernameToSocket.contains(jsonResponseObject["username"].toString())){
		qDebug() << "ALLREADY LOGGED IN";
		sendServerMessageTo(senderSocket,MessageType::AllreadyLoggedIn);
		senderSocket->disconnectFromHost();
	}
	else if(checkPassword(jsonResponseObject) == false){
		qDebug() << "BAD PASS";
		sendServerMessageTo(senderSocket,MessageType::BadPass);
		senderSocket->disconnectFromHost();
	}
	else{
		//helper var, just for nicer code;
		QString tmpUsername = jsonResponseObject["username"].toString();

		qDebug() << "AUTH FOR USER " << tmpUsername << " SUCCESSFUL";

		sendContactsFor(tmpUsername, senderSocket);
		notifyContacts(tmpUsername, MessageType::ContactLogin);
		if(hasUnreadMessages(tmpUsername)){
			sendUnreadMessages(tmpUsername, senderSocket);
		}
		m_usernameToSocket[tmpUsername] = senderSocket;
	}
}

void Server::checkContactExistence(QString tmpFrom, QString tmpTo)
{
	if(!m_contacts[tmpFrom].contains(tmpTo)){
		addNewContact(tmpFrom, tmpTo);
	}
	if(!m_contacts[tmpTo].contains(tmpFrom)){
		addNewContact(tmpTo, tmpFrom);
	}
}

void Server::forwardMessage(const QString& to, const QJsonObject& message)
{
	bool ret = sendMessageTo(	m_usernameToSocket[to],
								message);
	if(!ret){
		qDebug() << "WEIRD! User " << to << "is not online, afterall";
		//adding message to buffer for next time user logs in
		m_unreadMessages[to].append(message);
	}
}

bool Server::isOnline(QString username) const
{
	return m_usernameToSocket.contains(username);
}

void Server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	//reading first 4 characters, they represent length of json encoded message
	QByteArray messageLength = senderSocket->read(4);

	//check if sent data represents a number
	if(!isNumeric(messageLength))
	{
		//reading all but not saving it since it is not in valid format
		senderSocket->readAll();
		sendServerMessageTo(senderSocket, MessageType::BadMessageFormat);
		return;
	}

	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(
				senderSocket->read(messageLength.toInt()));

	//check if message is in valid json format
	if(jsonResponse.isNull()){
		sendServerMessageTo(senderSocket, MessageType::BadMessageFormat);
		return;
	}

	QJsonObject jsonResponseObject = jsonResponse.object();

	//if sent json object is auth object
	if(jsonResponseObject["type"] == MessageType::Authentication){
		authentication(jsonResponseObject, senderSocket);
	}

	//if sent data is text message, forward it only to the intended recepient
	else if(jsonResponseObject["type"] == MessageType::Text){
		QString tmpTo = jsonResponseObject["to"].toString();
		QString tmpFrom = jsonResponseObject["from"].toString();

		checkContactExistence(tmpFrom, tmpTo);

		//qDebug() << jsonResponse.toJson(QJsonDocument::Compact);
		if(isOnline(tmpTo)){
			forwardMessage(tmpTo, jsonResponseObject);
		}
		else{
			//adding message to buffer for next time user logs in
			m_unreadMessages[tmpTo].append(jsonResponseObject);
		}
	}
}

void Server::createUser(const QString& pass, const QString& username)
{
	QDomElement newUser = createNewXmlElement("user", "", "username", username);

	QDomElement password = createNewXmlElement("password", pass);

	QDomElement contacts = createNewXmlElement("contacts");
	newUser.appendChild(password);
	newUser.appendChild(contacts);

	m_dataDoc.documentElement().appendChild(newUser);

	m_authData[username] = pass;
	saveXMLFile();
}

bool Server::checkPassword(const QJsonObject & json){
	QString username = json["username"].toString();
	QString pass = json["password"].toString();

	//if username exists
	if(m_authData.contains(username)){
		return m_authData[username] == pass;
	}
	//first login, append to file and to current map
	//TODO create better system for making account
	else{
		createUser(pass, username);
		return true;
	}
}

bool Server::isInitialized() const{
	return m_isInitialized;
}
void Server::showError() const{
	std::cerr << m_errorMessage << std::endl;
}
