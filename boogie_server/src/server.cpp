#include <QTcpSocket>
#include <QCoreApplication>

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

	for(auto u : m_users){
		if(u == senderSocket){
			continue;
		}
		u->write(buffer);
	}

	qDebug() << buffer;

}
