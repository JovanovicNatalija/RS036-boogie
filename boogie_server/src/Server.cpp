#include <QTcpSocket>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include <iostream>
#include "Server.h"
#include "client.h"

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
	loadAuthData();

	connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
	qDebug("server created");
}

void Server::loadAuthData(){
	authFile.setFileName(AUTH_FILE_PATH);

	if(!authFile.open(QIODevice::ReadWrite)){
		m_errorMessage =  "OPENING AUTH FILE FAILED";
		m_isInitialized = false;
	}
	QTextStream fileTextStream(&authFile);
	QString line;
	while(fileTextStream.readLineInto(&line, 50)){
		if(!line.contains(":")){
			m_errorMessage =  "BAD AUTH FILE FORMAT";
			m_isInitialized = false;
		}
		auto data = line.split(":");
		m_authData[data[0]] = data[1];
	}
	//reading whole file will set seek at the end and then any writing will
	//be same as opening in append mode
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
			Client *client = new Client(json["username"].toString(),
					senderSocket);

			//disconnect all signals from this socket beacuse they will be
			//connected in Client class
			senderSocket->disconnect();
			m_usernameToClient[json["username"].toString().toStdString()] =
					std::move(client);

			connect (senderSocket, &QTcpSocket::readyRead,client,
					 &Client::readMessage);
			connect (senderSocket, &QTcpSocket::disconnected,client,
					 &Client::disconnected);
		}
	}
	//if sent data is message, forward it only to the intended recepient
	//TODO this works only when recepient is online
	else if(json["type"] == "m"){
		qDebug() << "THIS SHOULD NOT HAVE HAPPEND!!!!";
//		qDebug() << jsonResponse.toJson(QJsonDocument::Compact);
//		if(m_usernameToClient.contains(json["to"].toString())){
//			//TODO check for ret val of write
//			m_usernameToClient[json["to"].toString()].write
//					(messageLength + jsonResponse.toJson(QJsonDocument::Compact));
//		}
	}
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
		authFile.write((username + ":" + pass + "\n").toStdString().data());
		authFile.flush();
		m_authData[username] = pass;
		return true;
	}
}

bool Server::isInitialized(){
	return m_isInitialized;
}
void Server::showError(){
	std::cerr << m_errorMessage << std::endl;
}
