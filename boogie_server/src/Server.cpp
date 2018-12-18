#include <QTcpSocket>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include <iostream>
#include "Server.h"

Server::Server(quint16 port)
{
	connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
	if(port <= 1024){//first 1024 ports are not to be touched
		qDebug() << "BAD PORT NUMBER";
		emit serverError();
		return;
	}

	bool listening = listen(QHostAddress::Any, port);
	if(listening == false){
		qDebug() << "SERVER ERROR, cant listen on port " << port;
		emit serverError();
		return;
	}
	loadAuthData();;

	qDebug("server created");
}

void Server::loadAuthData(){
	authFile.setFileName(AUTH_FILE_PATH);

	if(!authFile.open(QIODevice::ReadWrite)){
		qDebug() << "Opening auth file failed";
		emit serverError();
		return;
	}
	QTextStream fileTextStream(&authFile);
	QString line;
	while(fileTextStream.readLineInto(&line, 50)){
		if(!line.contains(":")){
			qDebug() << "BAD AUTH FILE FORMAT";
			emit serverError();
			return;
		}
		auto data = line.split(":");
		authData[data[0]] = data[1];
	}
	//reading whole file will set seek at the end and then any writing will
	//be same as opening in append mode
}

void Server::newConnection(){
	if(hasPendingConnections()){
		QTcpSocket* client = nextPendingConnection();

		connect(client, &QTcpSocket::disconnected, this, &Server::userDisconnected);
		connect(client, &QTcpSocket::readyRead, this, &Server::readMessage);

		qDebug("User connected");
	}
}

void Server::userDisconnected(){

	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
	auto username = usernameToSocket.key(disconnectedClient);//linear time but its called rarely so its ok
	usernameToSocket.remove(username);
	qDebug() << "User " << username << " OUT";

	disconnectedClient->deleteLater();
}

void Server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	//reading first 4 characters, they represent length of message
	QByteArray messageLength = senderSocket->read(4);

	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(senderSocket->read(messageLength.toInt()));
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
			usernameToSocket[json["username"].toString()] = senderSocket;
		}
	}
	//if sent data is message, forward it only to the intended recepient
	//TODO this works only when recepient is online
	else if(json["type"] == "m"){
		qDebug() << jsonResponse.toJson(QJsonDocument::Compact);
		if(usernameToSocket.contains(json["to"].toString())){
			usernameToSocket[json["to"].toString()]->write
					(messageLength + jsonResponse.toJson(QJsonDocument::Compact));
		}
	}
}

bool Server::auth(const QJsonObject & json){
	QString username = json["username"].toString();
	QString pass = json["password"].toString();
	//if username was allready entered, check password

	if(authData.contains(username)){

		return authData[username] == pass;
	}

	//first login, append to file and to current map
	//TODO create better system for making account
	else{
		authFile.write((username + ":" + pass + "\n").toStdString().data());
		authFile.flush();
		authData[username] = pass;
		return true;
	}

}
