#include <QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	quint16 port = quint16(atoi(argv[1]));
	server s(port);

	return a.exec();
}
