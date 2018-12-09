#include "client.h"

client::client(QString ip, QObject* parrent, quint16 port)
    :QTcpSocket(parrent)
{
    connectToHost(ip, port);
    // TEST IP ADDR TELEFONA 77.243.27.69

    connect(this, &client::readyRead, this, &client::readMsg);
    qDebug("Client created");
}

void client::readMsg(){
    qDebug() << "poy";
}


