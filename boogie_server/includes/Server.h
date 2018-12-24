#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include "../util/util.h"

#include <QFile>
#include <QDir>

#include <QTcpServer>

#include <QDomDocument>


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
	QHash<QString, QTcpSocket*> m_usernameToSocket;
	bool m_isInitialized;
	std::string m_errorMessage;

	bool sendMessageTo(QTcpSocket* recepient, const QJsonObject& message) const;
	bool sendServerMessageTo(QTcpSocket* receipient, const QString& message
										,const QString& username = "") const;

	bool auth(const QJsonObject&);
	void loadData();

	void saveXMLFile() const;
	void addNewContact(const QString& tmpFrom, const QString& tmpTo);
	void createUser(const QString& pass, const QString& username);
	void notifyContacts(const QString& username, const MessageType& m) const;
};

#endif // SERVER_H

