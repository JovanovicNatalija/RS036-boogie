#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include <QFile>
#include <QDir>

#include <QTcpServer>

#include <QDomDocument>

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
	QDomDocument m_dataDoc;
	QDomNodeList m_users;
	QDomElement createNewXmlElement(QString tagName, QString text = "",
									QString attribute = "", QString value = "");

	const QString DATA_FILE_PATH = QDir::currentPath() + "/data.xml";
	QHash<QString, QString> m_authData;
	QHash<QString, QVector<QString>> m_contacts;
	std::unordered_map<std::string, Client*> m_usernameToClient;
	bool m_isInitialized;
	std::string m_errorMessage;
	bool sendMessageTo(Client*, QJsonObject);


	bool auth(const QJsonObject&);
	void loadData();
	void saveXMLFile();
	void addNewContact(QString tmpFrom, QString tmpTo);
	void createUser(QString pass, QString username);
};

#endif // SERVER_H

