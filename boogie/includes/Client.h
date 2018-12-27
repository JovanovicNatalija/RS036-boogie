#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <map>
#include <vector>
#include <iterator>
#include <tuple>
#include <ctime>
#include <chrono>
class Client : public QTcpSocket
{
    Q_OBJECT
public:
	Client(QObject* parent = nullptr);
    Q_INVOKABLE void connectToServer(QString username, QString ip, quint16 port = 10000);
    Q_INVOKABLE void sendMsg(QString str);
    Q_INVOKABLE void sendAuthData(QString password);
    Q_INVOKABLE void sendMsgData(QString to, QString msg);
    Q_INVOKABLE void addMsgToBuffer(QString sender, QString inConversationWith, QString msg);
    Q_INVOKABLE void writeInXml();
    Q_INVOKABLE void readFromXml();
    Q_INVOKABLE void displayOnConvPage(QString inConversationWith);
    Q_INVOKABLE QString getUsername();
    Q_INVOKABLE QString fixMsg(QString message);
    Q_INVOKABLE void addNewContact(QString name);
    void createXml();
    void disconnectFromServer();


signals:
    void showMsg(QString msgFrom, QString msg);
    void showContacts(QString contact, bool online);
    void clearContacts();
    void badPass();
    Q_INVOKABLE void pushConvPage(QString);
    //void aboutToClose();

public slots:
    void readMsg();


public:
    //pravimo buffer u vidu mape koja sadrzi podatke: kljuc je username one osobe sa kojom je korisnik
    //u konverzaciji, vrednost je touple od 3 stringa prvi predstavlja username onoga ko je poslao poruku,
    // drugi je sama poruka a treci vreme slanja poruke
    std::map<QString, std::vector<std::tuple<QString, QString, QString>>> msgDataBuffer;
    std::map<QString, unsigned long> msgInfos;
    std::map<QString, bool> contactInfos;
    unsigned long counter = 0;

private:
    QString username;

};

#endif // CLIENT_H
