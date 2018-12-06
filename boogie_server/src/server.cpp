#include "server.h"
#include <iostream>
#include <QTcpSocket>
#include <QCoreApplication>

server::server(quint16 port)
{
	connect(this, &QTcpServer::newConnection, this, &server::newConnection);
	if(port <= 1024){//first 1024 ports are not to be touched
		qDebug() << "BAD PORT NUMBER";
		QCoreApplication::exit(1);
	}

	bool listening = listen(QHostAddress::Any, port);
	if(listening == false){
		qDebug() << "SERVER ERROR, cant listen on port " << port;
		QCoreApplication::exit(1);
	}
	m_numOfUsers = 0;
	qDebug("server created");
}

void server::newConnection(){
	if(hasPendingConnections()){
		client = nextPendingConnection();

		m_users.push_back(client);

		connect(client, &QTcpSocket::disconnected, this, &server::userDisconnected);
		connect(client, &QTcpSocket::readyRead, this, &server::readMessage);

		m_numOfUsers++;
		qDebug("User connected");
	}

}

void server::userDisconnected(){
	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
	int index = m_users.indexOf(disconnectedClient);

	if(index != -1){
		m_users.removeAt(index);
	}

	disconnectedClient->deleteLater();
}

void server::readMessage(){
	QByteArray buffer;
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
	buffer = senderSocket->readAll();

	int index = m_users.indexOf(senderSocket);
	for(int i = 0;i < m_numOfUsers;++i){
		if(i == index){//if current socket is sender socket
			continue;
		}
		//send message to all users
		m_users[i]->write(buffer);
	}

}
