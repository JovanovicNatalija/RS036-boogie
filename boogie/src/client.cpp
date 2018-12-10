#include "client.h"
#include <iostream>

client::client(QObject* parrent)
    :QTcpSocket(parrent)
{
    std::cout << "Client created" << std::endl;
}

void client::connectToServer(QString ip, quint16 port) {
    connectToHost(ip, port);

    connect(this, SIGNAL(readyRead()), this, SLOT(readMsg()));

    std::cout << "Connected to host" << std::endl;
}

void client::readMsg(){
    std::cout << "read msg:" << std::endl;
    std::cout << readAll().toStdString() << std::endl;
}

void client::disconnectFromServer() {
    disconnectFromHost();
    std::cout << "Disconected from host" << std::endl;
}

void client::sendMsg(QString str) {
    write(str.toLocal8Bit().data());
}


