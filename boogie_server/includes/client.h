#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>


class Client : public QObject
{
public:
	Client();
	Client(QString, QTcpSocket*);
	Client(Client&&);
	Client& operator=(Client&&);
	~Client();
	qint64 sendMessage(const QByteArray& byteArray);

private:
	QString username;
	QTcpSocket* socket_ptr;

public slots:
	void readMessage();
	void disconnected();
};

#endif // CLIENT_H
