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
class Client : public QSslSocket

{
    Q_OBJECT
public:
	Client(QObject* parent = nullptr);
	Q_INVOKABLE void connectToServer(const QString& username,const QString& ip,
									 quint16 port = 10000);
    Q_INVOKABLE void sendMsg(const QString& str);
    Q_INVOKABLE void sendAuthData(QString password);
    Q_INVOKABLE void sendMsgData(const QString& to,const QString& msg);
	Q_INVOKABLE void addMsgToBuffer(const QString& sender,
									const QString& inConversationWith,
									const QString& msg);
    Q_INVOKABLE void writeInXml();
    Q_INVOKABLE void readFromXml();
    Q_INVOKABLE void displayOnConvPage(const QString& inConversationWith);
    Q_INVOKABLE void addNewContact(const QString& name, bool online);
    Q_INVOKABLE void checkNewContact(const QString& name);
    Q_INVOKABLE void sendPicture(const QString& filePath);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE QString username();
    Q_INVOKABLE void addContactToGroupSet(QString contact);
    Q_INVOKABLE void removeContactFromGroupSet(QString contact);
    Q_INVOKABLE void sendGroupInfos(QString groupName);
    Q_INVOKABLE void clearGroupSet();
    Q_INVOKABLE void refreshContactsAndGroups();
    Q_INVOKABLE void addGroup(QJsonObject grInfos);
    void createXml() const;

signals:
    void showMsg(const QString& msgFrom,const QString& msg);
    void showContacts(const QString& contact, bool online);
    void clearContacts();
    void badPass();
    void alreadyLogIn();
    void badContact(const QString& msg);

public slots:
    void readMsg();
	void sslErrors(const QList<QSslError> &errors);

public:
    //pravimo buffer u vidu mape koja sadrzi podatke: kljuc je username one osobe sa kojom je korisnik
    //u konverzaciji, vrednost je touple od 3 stringa prvi predstavlja username onoga ko je poslao poruku,
    // drugi je sama poruka a treci vreme slanja poruke
    std::map<QString, std::vector<std::tuple<QString, QString, QString>>> msgDataBuffer;
    std::map<QString, unsigned long> msgInfos;
    std::map<QString, bool> contactInfos;
    unsigned long counter = 0;

private:
    QString m_username;
    QHash<QString, QVector<std::tuple<QString, QString, QString>>> m_msgDataBuffer;
    QHash<QString, int> m_msgIndexBegin;
    QHash<QString, bool> m_contactInfos;
    std::vector<chatGroup> m_groupInfos;
    unsigned long m_msgCounter = 0;
    QSet<QString> m_contactsInGroups;

};

#endif // CLIENT_H
