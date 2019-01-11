#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket>
#include <QVector>
#include <map>
#include <vector>
#include <iterator>
#include <tuple>
#include <ctime>
#include <chrono>
#include <QSet>
#include "../util/util.h"
#include <QPixmap>


class Client : public QSslSocket
{
    Q_OBJECT
public:
    Client(QObject* parent = nullptr);
    Q_INVOKABLE void connectToServer(const QString& ip,
                                     quint16 port = 10000);
    Q_INVOKABLE void sendMsg(const QString& str);
    Q_INVOKABLE void sendAuthData(const QString& username,const QString& password);
    Q_INVOKABLE void sendMsgData(const QString& to,const QString& msg);
    Q_INVOKABLE void addMsgToBuffer(const QString& sender,
                                    const QString& inConversationWith,
                                    const QString& msg,
                                    const QString& type);
    Q_INVOKABLE void writeInXml();
    Q_INVOKABLE void readFromXml();
    Q_INVOKABLE void displayOnConvPage(const QString& inConversationWith);
    Q_INVOKABLE void addNewContact(const QString& name, bool online);
    Q_INVOKABLE void checkNewContact(const QString& name);
    Q_INVOKABLE void sendPicture(const QString& to, const QString& filePath);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE QString username();
    Q_INVOKABLE void addContactToGroupSet(QString contact);
    Q_INVOKABLE void removeContactFromGroupSet(QString contact);
    Q_INVOKABLE void sendGroupInfos(QString groupName);
    Q_INVOKABLE void clearGroupSet();
    Q_INVOKABLE void refreshContactsAndGroups();
    Q_INVOKABLE void addGroup(QJsonObject grInfos);
    Q_INVOKABLE void sendGroupMsgData(int groupId, const QString& msg);
    void createXml() const;

signals:
	void showPicture(const QString& msgFrom, const QString& path);
    void showMsg(const QString& msgFrom,const QString& msg);
	void showMsgForGroup(int groupId, const QString& msgFrom, const QString& msg);
    void showContacts(const QString& contact, bool online,int groupId);
    void showGroups(const QString& contact, bool online, int groupId);
    void clearContacts();
    void badPass();
    void alreadyLogIn();
    void badContact(const QString& msg);
    Q_INVOKABLE int unreadMsg(const QString& username);


public slots:
    void readMsg();
    void sslErrors(const QList<QSslError> &errors);

private:
    QString m_username;
    QHash<QString, QVector<QPair<QString, std::tuple<QString, QString, QString>>>> m_msgDataBuffer;
    QHash<QString, int> m_msgIndexBegin;
    QHash<QString, bool> m_contactInfos;
    std::vector<chatGroup> m_groupInfos;
	unsigned long m_msgCounter = 0;
	QSet<QString> m_contactsInGroups;
    unsigned long m_imgCounter = 0;
    int m_imageNum = 0;
	int m_bytesToRead = 0;
};

#endif // CLIENT_H
