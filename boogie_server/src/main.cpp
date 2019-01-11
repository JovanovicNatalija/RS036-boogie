#include <QCoreApplication>

#include "Server.h"

#include <QObject>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	quint16 port;
	if(argc == 2){
		port = quint16(atoi(argv[1]));
	}
	else{
		port = 10000;
	}
	Server s(port);

	if(!s.isInitialized()){
		s.showError();
		return -1;
	}

	return a.exec();
}
