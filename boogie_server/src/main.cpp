#include <QCoreApplication>

#include "server.h"
#include <QObject>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	quint16 port = quint16(atoi(argv[1]));
	server s(port);
	//still not exiting when this serverError is emited, TODOQ
	QObject::connect(&s, &server::serverError, &a, &QCoreApplication::quit, Qt::QueuedConnection);


	return a.exec();
}
