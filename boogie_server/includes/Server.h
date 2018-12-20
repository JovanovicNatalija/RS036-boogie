#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <QFile>
#include <QDir>
#include <QTcpServer>
#include "client.h"


class Server : public QTcpServer
{
	Q_OBJECT
public:
	Server(quint16 port);
	bool isInitialized();
	void showError();
	~Server();

public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	QFile authFile;
	const QString AUTH_FILE_PATH = QDir::currentPath() + "/auth.txt";
	QHash<QString, QString> m_authData;
	std::unordered_map<std::string, Client*> m_usernameToClient;
	bool m_isInitialized;
	std::string m_errorMessage;
	bool auth(const QJsonObject&);
	void loadAuthData();
};

#endif // SERVER_H
