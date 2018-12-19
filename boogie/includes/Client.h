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
    //u konverzaciji, vrednost je touple od 3 stringa prvi predstavlja username onoga ko je poslao poruku,
    // drugi je sama poruka a treci vreme slanja poruke
    std::map<QString, std::vector<std::tuple<QString, QString, QString>>> msgDataBuffer;

    void appendToBuffer(QString msgFrom, QString inConversationWith, QString msg){
        //hvatamo trenutno vreme i pretvaramo ga u string zbog lakseg upisivanja u xml
        auto start = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(start);
        std::tm * ptm = std::localtime(&end_time);
        char buffer[32];
        std::strftime(buffer, 32, "%a, %d.%m.%Y %H:%M:%S", ptm);
        msgDataBuffer[inConversationWith].push_back(std::make_tuple(msgFrom, msg, buffer));
    }
};

#endif // CLIENT_H
