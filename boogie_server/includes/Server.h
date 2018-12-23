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
	QDomElement createNewXmlElement(const QString& tagName,
									const QString& text = "",
									const QString& attribute = "",
									const QString& value = "");
	const QString DATA_FILE_PATH = QDir::currentPath() + "/data.xml";
	QHash<QString, QString> m_authData;
	QHash<QString, QVector<QString>> m_contacts;
	std::unordered_map<std::string, Client*> m_usernameToClient;
	bool m_isInitialized;
	std::string m_errorMessage;
	bool sendMessageTo(Client*, const QJsonObject &) const;

	bool sendServerMessageTo(QTcpSocket* receipient, const QString& message) const;

	bool auth(const QJsonObject&);
	void loadData();
	void saveXMLFile() const;
	void addNewContact(const QString& tmpFrom, const QString& tmpTo);
	void createUser(const QString& pass, const QString& username);
};

#endif // SERVER_H

