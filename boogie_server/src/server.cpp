#include <QTcpSocket>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include <iostream>
#include "server.h"

server::server(quint16 port)
{
	connect(this, &QTcpServer::newConnection, this, &server::newConnection);
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
	qDebug() << AUTH_FILE_PATH;
	authFile.setFileName(AUTH_FILE_PATH);

	if(!authFile.open(QIODevice::ReadWrite)){
		qDebug() << "opening auth file failed";
		emit serverError();
		return;
	}

	qint64 lineLength;
	char data[1024];
	while((lineLength = authFile.readLine(data, sizeof (data))) != -1){
		QString tmp(data);
		auto data = tmp.split(":");
		authData[data[0]] = data[1];
	}
	//reading whole file will set seek at the end and then any writing will be
	//same as opening in append mode

	qDebug("server created");
}

void server::newConnection(){
	if(hasPendingConnections()){
		QTcpSocket* client = nextPendingConnection();

		m_users.push_back(client);

		connect(client, &QTcpSocket::disconnected, this, &server::userDisconnected);
		connect(client, &QTcpSocket::readyRead, this, &server::readMessage);

		qDebug("User connected");
	}
}

void server::userDisconnected(){
	qDebug() << "User OUT";
	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
	int index = m_users.indexOf(disconnectedClient);

	if(index != -1){
		m_users.removeAt(index);
	}
	disconnectedClient->deleteLater();
}

void server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	QByteArray buff = senderSocket->read(4);

	QJsonDocument jsonResponse = QJsonDocument::fromJson(senderSocket->read(buff.toInt()));
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
		}
	}

//	for(auto u : m_users){
//		if(u == senderSocket){
//			continue;
//		}
//	}

}

bool server::auth(const QJsonObject & json){
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
