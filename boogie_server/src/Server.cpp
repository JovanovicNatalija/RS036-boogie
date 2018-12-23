#include <QTcpSocket>

#include <QCoreApplication>
#include <QDir>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDomDocument>

#include <iostream>

#include "Server.h"
#include "client.h"
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

		qDebug("User connected");
	}
}

void Server::userDisconnected(){

//	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
//	auto username = m_usernameToSocket.key(disconnectedClient);//linear time but its called rarely so its ok
//	m_usernameToSocket.remove(username);
//	qDebug() << "User " << username << " OUT";

//	disconnectedClient->deleteLater();
}

bool Server::sendMessageTo(Client* recepient, const QJsonObject& message) const
{
	//TODO write this to some log file i guess
	QString tmp = packMessage(message);
	if(recepient->sendMessage(tmp.toLocal8Bit().data()) != -1)
		return true;

	else return false;
}

bool Server::sendServerMessageTo(QTcpSocket* receipient, const QString& message) const
{
	QJsonObject response;
	response.insert("type", MessageType::Server);
	response.insert("msg", message);
	qint64 ret = receipient->write(packMessage(response).toLocal8Bit().data());
	return ret != -1;
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
//TODO should this add to client as well
void Server::addNewContact(const QString& user, const QString& contact)
{
	m_contacts[user].append(contact);

	auto userDomElement = m_users.at(0);
	int i = 1;
	while(userDomElement.attributes().namedItem("username").nodeValue()
			!= user){
		userDomElement = m_users.at(i++);
	}
	QDomElement newContact = createNewXmlElement("contact", contact);
	userDomElement.firstChildElement("contacts").appendChild(newContact);
	saveXMLFile();
}

void Server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	//reading first 4 characters, they represent length of message
	QByteArray messageLength = senderSocket->read(4);
	if(!std::all_of(
				messageLength.begin(), messageLength.end(),
				[](char c){return isdigit(c);})
			)
	{
		//reading all but not saving it since it is not in valid format
		senderSocket->readAll();
		sendServerMessageTo(senderSocket,
							"BAD MESSAGE FORMAT, FIRST 4 BYTES ARE NOT NUMBER");
		return;
	}

	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(
				senderSocket->read(messageLength.toInt()));
	if(jsonResponse.isNull()){
		sendServerMessageTo(senderSocket,
							"BAD MESSAGE FORMAT, APPEARS NOT TO BE JSON");
		return;
	}
	QJsonObject jsonResponseObject = jsonResponse.object();

	//if sent json object is auth object
	if(jsonResponseObject["type"] == MessageType::Authentication){
		if(auth(jsonResponseObject) == false){
			qDebug() << "BAD PASS";
			sendServerMessageTo(senderSocket,"BAD PASS");
			senderSocket->disconnectFromHost();
		}
		else{
			qDebug() << "AUTH SUCCESSFUL";
			//helper var, just for nicer code;
			QString tmpUsername = jsonResponseObject["username"].toString();
			Client *client = new Client(tmpUsername,
					senderSocket);

			QJsonObject contactsDataJson;
			QJsonArray contactsArrayJson;
			std::for_each(m_contacts[tmpUsername].begin(),
						  m_contacts[tmpUsername].end(),
						  [&contactsArrayJson](QString s){
								contactsArrayJson.append(s);
							}
							);

			contactsDataJson.insert("type", MessageType::Contacts);
			contactsDataJson.insert("to",tmpUsername);
			contactsDataJson.insert("contacts", contactsArrayJson);
			sendMessageTo(client, contactsDataJson);

			m_usernameToClient[tmpUsername.toStdString()] =
					std::move(client);
		}
	}
	//if sent data is text message, forward it only to the intended recepient
	//TODO this works only when recepient is online
	else if(jsonResponseObject["type"] == MessageType::Text){
		QString tmpTo = jsonResponseObject["to"].toString();
		QString tmpFrom = jsonResponseObject["from"].toString();

		//Contact creation
		//TODO This will be changed later
		if(!m_contacts[tmpFrom].contains(tmpTo)){
			addNewContact(tmpFrom, tmpTo);
		}
		if(!m_contacts[tmpTo].contains(tmpFrom)){
			addNewContact(tmpTo, tmpFrom);
		}

		//qDebug() << jsonResponse.toJson(QJsonDocument::Compact);
		if(m_usernameToClient.find(jsonResponseObject["to"].toString().toStdString())
				!= m_usernameToClient.end()){

			bool ret = sendMessageTo(	m_usernameToClient[tmpTo.toStdString()],
										jsonResponseObject);
			if(!ret){
				qDebug() << "WEIRD! User " << tmpTo << "is not online, afterall";
			}
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

bool Server::auth(const QJsonObject & json){
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

bool Server::isInitialized(){
	return m_isInitialized;
}
void Server::showError(){
	std::cerr << m_errorMessage << std::endl;
}

