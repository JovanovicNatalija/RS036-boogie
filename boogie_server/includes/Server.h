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
	bool isInitialized() const;
	void showError() const;
	~Server();



public slots:
	void newConnection();
	void userDisconnected();
	void readMessage();

private:
	QDomDocument m_dataDoc;
	const QString DATA_FILE_PATH = QDir::currentPath() + "/data.xml";

	QDomNodeList m_users;
	QHash<QString, QString> m_authData;
	QHash<QString, QVector<QString>> m_contacts;
	QHash<QString, QTcpSocket*> m_usernameToSocket;
	QHash<QString, QVector<QJsonObject>> m_unreadMessages;

	bool m_isInitialized;
	std::string m_errorMessage;



	/*XML FUNCTIONS*/
	void loadData();
	void saveXMLFile() const;
	QDomElement createNewXmlElement(const QString& tagName,
									const QString& text = "",
									const QString& attribute = "",
									const QString& value = "");

	/*CONTACT FUNCTIONS*/
	void addNewContact(const QString& tmpFrom, const QString& tmpTo);
	void notifyContacts(const QString& username, const MessageType& m) const;
	void sendContactsFor(QString username, QTcpSocket* senderSocket) const;

	/*MESSAGE FUNCTIONS*/
	bool sendMessageTo(QTcpSocket* recepient, const QJsonObject& message) const;
	bool sendServerMessageTo(QTcpSocket* receipient, const MessageType& msgType
										,const QString& username = "") const;
	void forwardMessage(const QString& to, const QJsonObject& message);

	/*USER RELATED FUNCTIONS*/
	void createUser(const QString& pass, const QString& username);
	void sendUnreadMessages(const QString& username, QTcpSocket* socket);
	void authentication(QJsonObject jsonResponseObject, QTcpSocket* senderSocket);

	/*HELPER FUNCTIONS*/
	bool userExists(const QString& username) const;
	bool isOnline(const QString& username) const;
	void checkContactExistence(const QString& tmpFrom, const QString& tmpTo);
	bool checkPassword(const QJsonObject&);
	bool hasUnreadMessages(const QString& username) const;

};

#endif // SERVER_H

