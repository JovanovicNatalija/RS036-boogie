#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class server : public QTcpServer
{
	Q_OBJECT
public:
	server();

public slots:
	void newConnection();

private:
	int m_numOfUsers;
};

#endif // SERVER_H
