#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>

#include "../util/util.h"

#include <QFile>
#include <QDir>

#include <QTcpServer>

#include <QDomDocument>

#include <QSslKey>
#include <QSslCertificate>


class Server : public QTcpServer
{
	Q_OBJECT
public:
	Server(quint16 port);
	bool isInitialized() const;
	void showError() const;
	~Server() override;



public slots:
	void newConnection();
	void userDisconnected();
	void sslErrors(const QList<QSslError> &errors);
	void readMessage();

private:

	QSslKey key;
	QSslCertificate cert;

	QDomDocument m_dataDoc;
	const QString DATA_FILE_PATH = QDir::currentPath() + "/data.xml";

	QDomNodeList m_users;
	QDomNodeList m_groupsNodeList;
	QHash<QString, QString> m_authData;
	QHash<QString, QVector<QString>> m_contacts;
	QHash<QString, QVector<int>> m_usernameToGroups;
	QHash<QString, QSslSocket*> m_usernameToSocket;
	QHash<QTcpSocket*, int> m_socketBytesLeft;

	QHash<QString, QVector<QJsonObject>> m_unreadMessages;
	QHash<int,chatGroup> m_groups;
	int m_nextGroupId;
	bool m_isInitialized;
	std::string m_errorMessage;

	void incomingConnection(qintptr socketDescriptor) override;

	/*XML FUNCTIONS*/
	void loadData();
	void saveXMLFile() const;
	QDomElement createNewXmlElement(const QString& tagName,
									const QString& text = "",
									const QString& attribute = "",
									const QString& value = "");
	void addGroupToXml(const chatGroup& gr);

	/*CONTACT FUNCTIONS*/
	void addNewContact(const QString& tmpFrom, const QString& tmpTo);
	void notifyContacts(const QString& username, const MessageType& m) const;
	void sendContactsFor(QString username, QSslSocket* senderSocket) const;
	void sendGroupsFor(QString username, QSslSocket* socket) const;

	/*MESSAGE FUNCTIONS*/
	bool sendMessageToSocket(QSslSocket* recepient, const QJsonObject& message) const;
	bool sendServerMessageTo(QSslSocket* receipient, const MessageType& msgType
										,const QString& username = "") const;

	void forwardMessage(const QString& to, const QJsonObject& message);

	/*USER RELATED FUNCTIONS*/
	void createUser(const QString& pass, const QString& username);
	void sendUnreadMessages(const QString& username, QSslSocket* socket);
	void authentication(QJsonObject jsonResponseObject, QSslSocket* senderSocket);
	void createGroup(QJsonObject &jsonResponseObject);

	/*HELPER FUNCTIONS*/
	bool userExists(const QString& username) const;
	bool isOnline(const QString& username) const;
	void checkContactExistence(const QString& tmpFrom, const QString& tmpTo);
	bool checkPassword(const QJsonObject&);
	bool hasUnreadMessages(const QString& username) const;


};

#endif // SERVER_H

