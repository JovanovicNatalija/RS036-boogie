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

QString packMessage(QJsonObject dataToSend){
	QJsonDocument tmp(dataToSend);
	QString jsonToString{tmp.toJson(QJsonDocument::Compact)};
	QString strJsonLen = QString::number(jsonToString.length());

	while(strJsonLen.length() < 4)
		strJsonLen = QString::number(0) + strJsonLen;

	return strJsonLen+jsonToString;
}

QJsonArray createContactsJson(QVector<QString> contacts){
	QJsonArray ret;
	for(auto str : contacts){
		ret.append(str);
	}
	return ret;
}


Server::~Server(){}

Server::Server(quint16 port)
{
	m_isInitialized = true;
	if(port <= 1024){//first 1024 ports are not to be touched
		m_errorMessage =  "BAD PORT NUMBER";
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
		dataFile.close(); //CHECK
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

		qDebug() << currUsername << " ima pass " << password;

		//get list of contacts
		auto contacts = currUserNode.firstChildElement("contacts")
				.elementsByTagName("contact");
		for(int j = 0; j < contacts.count(); ++j){
			qDebug() << "kontakt: " << contacts.at(j).toElement().text() << " \n";
			m_contacts[currUsername].append(contacts.at(j).toElement().text());
		}


	}


}
void Server::newConnection(){
	if(this->hasPendingConnections()){
		QTcpSocket* client = this->nextPendingConnection();

		//TODO can i do it without these connections?!
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

bool Server::sendMessageTo(Client* recepient, QJsonObject message){
	//TODO write this to some log file i guess
	QString tmp = packMessage(message);
	if(recepient->sendMessage(tmp.toLocal8Bit().data()) != -1)
		return true;

	else return false;
}

void Server::saveXMLFile(){
	QFile dataFile{DATA_FILE_PATH};
	if(!dataFile.open(QIODevice::WriteOnly)){
		qDebug() << "FAILED OPENING OF DATA XML";

	}
	QTextStream output{&dataFile};
	output << m_dataDoc.toString();
	qDebug() << "FILE SAVED";

}

//adds contact to array of contacts and to DOM
//TODO should this add to client as well
void Server::addNewContact(QString user, QString contact)
{
	m_contacts[user].append(contact);

	auto userDomElement = m_users.at(0);
	int i = 1;
	while(userDomElement.attributes().namedItem("username").nodeValue()
			!= user){
		userDomElement = m_users.at(i++);
	}
	QDomElement newContact = m_dataDoc.createElement("contact");
	QDomText contactText = m_dataDoc.createTextNode(contact);
	newContact.appendChild(contactText);
	userDomElement.firstChildElement("contacts").appendChild(newContact);
	saveXMLFile();
}

void Server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	//reading first 4 characters, they represent length of message
	QByteArray messageLength = senderSocket->read(4);
	//TODO check if received data is number

	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(
				senderSocket->read(messageLength.toInt()));
	QJsonObject json = jsonResponse.object();

	//if sent json object is auth object
	if(json["type"] == "a"){
		if(auth(json) == false){
			qDebug() << "BAD PASS";
			senderSocket->write("BAD PASS");
			senderSocket->disconnectFromHost();
		}
		else{
			qDebug() << "AUTH SUCCESSFUL";
			//helper var, just for nicer code;
			QString tmpUsername = json["username"].toString();
			Client *client = new Client(tmpUsername,
					senderSocket);
			QJsonObject contactsDataJson;
			QJsonArray contactsArrayJson = createContactsJson(
						m_contacts[tmpUsername]);

			//TODO MAKE ENUM!!!!!!!!!
			contactsDataJson.insert("type","c");
			contactsDataJson.insert("from","SERVER");
			contactsDataJson.insert("to",tmpUsername);
			contactsDataJson.insert("contacts", contactsArrayJson);

			sendMessageTo(client, contactsDataJson);

			m_usernameToClient[tmpUsername.toStdString()] =
					std::move(client);
		}
	}
	//if sent data is message, forward it only to the intended recepient
	//TODO this works only when recepient is online
	else if(json["type"] == "m"){
		auto tmpTo = json["to"].toString();
		auto tmpFrom = json["from"].toString();

		//Contact creation
		//TODO This will be changed later
		if(!m_contacts[tmpFrom].contains(tmpTo)){
			addNewContact(tmpFrom, tmpTo);
		}
		if(!m_contacts[tmpTo].contains(tmpFrom)){
			addNewContact(tmpTo, tmpFrom);
		}

		qDebug() << jsonResponse.toJson(QJsonDocument::Compact);
		if(m_usernameToClient.find(json["to"].toString().toStdString())
				!= m_usernameToClient.end()){
			//TODO write functions that does this because it will need to write in file as well
			auto ret = m_usernameToClient[tmpTo.toStdString()]->sendMessage
					(messageLength + jsonResponse.toJson(QJsonDocument::Compact));
			if(ret == -1){
				//send message to sender socket that writing failed
			}
		}
	}
}


void Server::createUser(QString pass, QString username)
{
	QDomElement newUser = m_dataDoc.createElement("user");
	newUser.setAttribute("username", username);

	QDomElement password = m_dataDoc.createElement("password");
	QDomText passwordText = m_dataDoc.createTextNode(pass);
	password.appendChild(passwordText);

	QDomElement contacts = m_dataDoc.createElement("contacts");
	newUser.appendChild(password);
	newUser.appendChild(contacts);

	m_dataDoc.documentElement().appendChild(newUser);

	m_authData[username] = pass;
	saveXMLFile();
}

bool Server::auth(const QJsonObject & json){
	QString username = json["username"].toString();
	QString pass = json["password"].toString();
	//if username was allready entered, check password

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


