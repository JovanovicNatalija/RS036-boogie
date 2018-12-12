#ifndef SERVER_H
#define SERVER_H

#include <QFile>
#include <QDir>
#include <QTcpServer>

class Server : public QTcpServer
{
	Q_OBJECT
public:
	Server(quint16 port);

public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	//QTcpSocket *client;
	QVector<QTcpSocket*> m_users;
	bool auth(const QJsonObject&);
	QFile authFile;
	const QString AUTH_FILE_PATH = QDir::currentPath() + "/auth.txt";
	QHash<QString, QString> authData;
	QHash<QString, QTcpSocket*> usernameToSocket;

signals:
	void serverError();
};

#endif // SERVER_H
