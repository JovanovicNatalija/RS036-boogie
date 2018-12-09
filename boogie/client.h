#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
//TODO refactor client->Client
class client : public QTcpSocket
{
    Q_OBJECT
public:
    client(QString ip, QObject* parrent = nullptr, quint16 port = 10000);

public slots:
    void readMsg();
};

#endif // CLIENT_H
