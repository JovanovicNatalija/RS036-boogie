#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <map>
#include <vector>
#include <iterator>
class Client : public QTcpSocket
{
    Q_OBJECT
public:
	Client(QObject* parent = nullptr);
    Q_INVOKABLE void connectToServer(QString ip, quint16 port = 10000);
    Q_INVOKABLE void sendMsg(QString str);
    Q_INVOKABLE void sendAuthData(QString username, QString password);
    Q_INVOKABLE void sendMsgData(QString from, QString to, QString msg);
    Q_INVOKABLE void addMsgToBuffer(QString from, QString to, QString msg);
    Q_INVOKABLE void writeInXml(QString username);
    void disconnectFromServer();

signals:
    void showMsg(QString msgFrom, QString msg);
    //void aboutToClose();

public slots:
    void readMsg();

public:
    //pravimo buffer u vidu mape koja sadrzi podatke: kljuc je username one osobe sa kojom je korisnik
    //u konverzaciji, vrednost je par stringova prvi predstavlja username onoga ko je poslao poruku
    // a drugi je sama poruka
    std::map<QString, std::vector<std::pair<QString, QString>>> msgDataBuffer;

    void appendToBuffer(QString msgFrom, QString inConversationWith, QString msg){
        msgDataBuffer[inConversationWith].push_back(std::make_pair(msgFrom, msg));
    }
};

#endif // CLIENT_H
