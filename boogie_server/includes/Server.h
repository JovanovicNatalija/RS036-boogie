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
	bool isInitialized();
	void showError();

public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	QFile authFile;
	const QString AUTH_FILE_PATH = QDir::currentPath() + "/auth.txt";
	QHash<QString, QString> m_authData;
	QHash<QString, QTcpSocket*> m_usernameToSocket;
	bool m_isInitialized;
	std::string m_errorMessage;
	bool auth(const QJsonObject&);
	void loadAuthData();
};

#endif // SERVER_H
