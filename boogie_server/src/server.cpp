#include "server.h"
#include <iostream>

server::server()
{
	connect(this, &QTcpServer::newConnection, this, &server::newConnection);

	listen(QHostAddress::Any, 12345);
	m_numOfUsers = 0;
	qDebug("server created");
}

void server::newConnection(){
	qDebug("User connected");
	m_numOfUsers++;
}
