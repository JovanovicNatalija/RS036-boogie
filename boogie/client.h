#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
//TODO refactor client->Client
class client : public QTcpSocket
{
    Q_OBJECT
public:
    client(QObject* parent = nullptr);
    Q_INVOKABLE void connectToServer(QString ip, quint16 port = 10000);
    Q_INVOKABLE void sendMsg(QString str);
    Q_INVOKABLE void sendAuthData(QString username, QString password);
    Q_INVOKABLE void sendMsgData(QString from, QString to, QString msg);
    void disconnectFromServer();

public slots:
    void readMsg();
};

#endif // CLIENT_H
