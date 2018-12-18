#include <QCoreApplication>

#include "Server.h"
#include <QObject>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if(argc < 2){
		qDebug() << "BAD CALL! add port number as argument";
		return 1;
	}

	quint16 port = quint16(atoi(argv[1]));
	Server s(port);
	//still not exiting when this ServerError is emited, TODO
	QObject::connect(&s, &Server::serverError, &a, &QCoreApplication::quit, Qt::QueuedConnection);


	return a.exec();
}
