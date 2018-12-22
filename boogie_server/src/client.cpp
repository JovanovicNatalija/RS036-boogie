#include "client.h"
#include <QJsonObject>
#include <QJsonDocument>

Client::Client(){}

Client::~Client(){
	free(socket_ptr);

}

Client::Client(QString username, QTcpSocket* socket)
	:username{std::move(username)},socket_ptr{std::move(socket)}
{
	socket = nullptr;
;
}
//move
Client& Client::operator=(Client&& other)
{
	std::swap(other.username, username);
	std::swap(other.socket_ptr, socket_ptr);
	other.socket_ptr=nullptr;


	return *this;
}
//move
Client::Client(Client &&other)
		:username{std::move(other.username)},
		socket_ptr{std::move(other.socket_ptr)}
{
	other.socket_ptr = nullptr;
;
}


void Client::readMessage(){

	//reading first 4 characters, they represent length of message
	QByteArray messageLength = socket_ptr->read(4);
	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(socket_ptr->read(
											messageLength.toInt()));
	QJsonObject jsonMessage = jsonResponse.object();

	if(jsonMessage["type"] == "m"){
		//TODO here we will check if the one we want to send message to
		//is in our contact list

	}
}
void Client::disconnected(){
	qDebug() << "User " << username << " OUT";
}

qint64 Client::sendMessage(const QByteArray& byteArray){
	return socket_ptr->write(byteArray);
}

