#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class server : public QTcpServer
{
	Q_OBJECT
public:
	server(quint16 port);

public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	//QTcpSocket *client;
	QVector<QTcpSocket*> m_users;

signals:
	void serverError();
};

#endif // SERVER_H
