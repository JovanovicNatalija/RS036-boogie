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

	if(!s.isInitialized()){
		s.showError();
		return -1;
	}

	return a.exec();
}
