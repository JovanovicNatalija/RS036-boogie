#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class server : public QTcpServer
{
	Q_OBJECT
public:
	server(qint16 port);

public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	int m_numOfUsers;
	QTcpSocket *client;
	QVector<QTcpSocket*> m_users;
};

#endif // SERVER_H
