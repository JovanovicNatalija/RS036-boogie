#include <QTcpSocket>

#include <QCoreApplication>
#include <QDir>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QSslSocket>

#include <QDomDocument>

#include <iostream>

#include "Server.h"
#include "../util/util.h"

Server::~Server(){}

Server::Server(quint16 port)
{
	QFile keyFile("../certs/red_local.key");
	keyFile.open(QIODevice::ReadOnly);
	key = QSslKey(keyFile.readAll(), QSsl::Rsa);
	keyFile.close();

	QFile certFile("../certs/red_local.pem");
	certFile.open(QIODevice::ReadOnly);
	cert = QSslCertificate(certFile.readAll());
	certFile.close();


	m_isInitialized = true;
	if(port <= 1024){//first 1024 ports are not to be touched
		m_errorMessage = "BAD PORT NUMBER";
		m_isInitialized = false;
	}

	bool listening = listen(QHostAddress::Any, port);
	if(listening == false){
		m_errorMessage = "SERVER ERROR, CAN'T LISTEN ON PORT "
						 + std::to_string(port);
		m_isInitialized = false;
	}

	loadData();

	connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
	qDebug("server created");
}

void Server::incomingConnection(qintptr socketDescriptor)
{
	QSslSocket *newConnection = new QSslSocket(this);

	//this has to be in qt4 syntax
	connect(newConnection, SIGNAL(sslErrors(QList<QSslError>)),
			this, SLOT(sslErrors(QList<QSslError>)));

	newConnection->setSocketDescriptor(socketDescriptor);

	//encryption keys and certificates
	newConnection->setPrivateKey(key);
	newConnection->setLocalCertificate(cert);
	newConnection->addCaCertificates("../certs/blue_ca.pem");
	newConnection->setPeerVerifyMode(QSslSocket::VerifyPeer);
	newConnection->startServerEncryption();

	addPendingConnection(newConnection);
}

void Server::sslErrors(const QList<QSslError> &errors)
{
	QSslSocket *senderSocket = qobject_cast<QSslSocket *>(QObject::sender());
	foreach (const QSslError &error, errors){
		if(error.error() == QSslError::SelfSignedCertificate){//ignoring self signed cert
			QList<QSslError> expectedSslErrors;
			expectedSslErrors.append(error);
			senderSocket->ignoreSslErrors(expectedSslErrors);
		}
	}

}

void Server::loadData(){
	m_nextGroupId = 0;
	QFile dataFile{DATA_FILE_PATH};

	if(!dataFile.open(QIODevice::ReadWrite | QIODevice::Text)){
		qDebug() << "OPENING FILE AT " + DATA_FILE_PATH + " FAILED";
		m_errorMessage = "OPENING DATA FILE FAILED";
		m_isInitialized = false;
		return;
	}
	else{
		//if file is empty, intialize it with empty xml tag so that setContent
		//would'nt fail
		if(dataFile.size() == 0){
			dataFile.write("<xml></xml>");
			dataFile.flush();
			dataFile.close();
			dataFile.open(QIODevice::ReadWrite | QIODevice::Text);
		}
		if(!m_dataDoc.setContent(&dataFile)){
			qDebug() << "LOADING FILE AT " + DATA_FILE_PATH + " FAILED";
			m_errorMessage = "LOADING DATA FILE FAILED";
			m_isInitialized = false;
			return;
		}
		dataFile.close();
	}
	QDomElement root = m_dataDoc.firstChildElement();
	m_users = root.elementsByTagName("user");

	//for each user tag
	for(int i = 0; i < m_users.count(); ++i){
		auto currUserNode = m_users.at(i);

		//store username and password
		auto currUsername =
				currUserNode.attributes().namedItem("username").nodeValue();
		auto password = currUserNode.firstChildElement("password").text();
		m_authData[currUsername] = password;

		//get list of contacts
		auto contacts = currUserNode.firstChildElement("contacts")
				.elementsByTagName("contact");
		for(int j = 0; j < contacts.count(); ++j){
			m_contacts[currUsername].append(contacts.at(j).toElement().text());
		}
	}
	m_groupsNodeList = root.elementsByTagName("group");

	for(int i = 0; i< m_groupsNodeList.count(); ++i){
		auto currGroupNode = m_groupsNodeList.at(i);
		auto currGroupName = currGroupNode.attributes().namedItem("groupName")
													.nodeValue();
		auto currGroupId = currGroupNode.firstChildElement("id").text().toInt();
		//searching for biggest id among groups and taking +1 for next group id
		if(currGroupId >= m_nextGroupId){
			m_nextGroupId = currGroupId + 1;
		}
		auto membersList = currGroupNode.firstChildElement("members")
										.elementsByTagName("member");

		QSet<QString> members;
		chatGroup gr;
		gr.groupName = currGroupName;
		gr.id = currGroupId;
		for(int j = 0; j < membersList.count(); ++j){
			auto member = membersList.at(j).toElement().text();
			members.insert(member);
			m_usernameToGroups[member].push_back(currGroupId);
		}
		gr.members = members;
		members.clear();//clear it for next iteration
		qDebug() << "grupa " << gr.groupName << " ima id " << gr.id << "i clanove " << gr.members;
		m_groups[currGroupId] = gr;
	}

}
void Server::newConnection(){
	if(this->hasPendingConnections()){
		QTcpSocket* client = this->nextPendingConnection();

		connect(client, &QTcpSocket::disconnected, this,
				&Server::userDisconnected);
		connect(client, &QTcpSocket::readyRead, this, &Server::readMessage);

	}
}

void Server::notifyContacts(const QString& username, const MessageType& m) const
{
	for(auto user : m_contacts[username]){
		QTcpSocket* tmp = m_usernameToSocket[user];
		if(tmp == nullptr)//contact not online
			continue;
		QJsonObject notification;
		notification.insert("type", setMessageType(m));
		notification.insert("to", user);
		notification.insert("contact", username);
		sendMessageTo(tmp, notification);
	}
}


void Server::userDisconnected(){

	QTcpSocket *disconnectedClient = qobject_cast<QTcpSocket *>(QObject::sender());
	QString username = m_usernameToSocket.key(disconnectedClient);//linear time but its called rarely so its ok
	if(username != ""){
		m_usernameToSocket.remove(username);

		//notify all contacts that user is offline
		notifyContacts(username, MessageType::ContactLogout);

		qDebug() << "User " << username << " OUT";
	}

	disconnectedClient->deleteLater();
}

bool Server::sendMessageTo(QTcpSocket* recepient, const QJsonObject& message) const
{
	QString tmp = packMessage(message);
	if(recepient->write(tmp.toLocal8Bit().data()) != -1){
		recepient->flush();
		return true;
	}

	else return false;
}

bool Server::sendServerMessageTo(QTcpSocket* receipient, const MessageType& msgType
								 , const QString& username) const
{
	QJsonObject response;
	response.insert("type", setMessageType(msgType));
	if(username != ""){
		response.insert("to", username);
	}
	bool ret = -1 != receipient->write(packMessage(response).toLocal8Bit().data());
	if(ret){
		receipient->flush();
	}
	return ret;

}

void Server::saveXMLFile() const
{
	QFile dataFile{DATA_FILE_PATH};
	if(!dataFile.open(QIODevice::WriteOnly)){
		qDebug() << "FAILED OPENING XML DATA";
		return;
	}
	QTextStream output{&dataFile};
	output << m_dataDoc.toString();
	qDebug() << "FILE SAVED";
	//QFile is RAII, no need to close file

}

QDomElement Server::createNewXmlElement(const QString& tagName,
										const QString& text,
										const QString& attribute,
										const QString& value){
	QDomElement newElement = m_dataDoc.createElement(tagName);
	if(text != ""){
		QDomText newElementText = m_dataDoc.createTextNode(text);
		newElement.appendChild(newElementText);
	}
	if(attribute != ""){
		newElement.setAttribute(attribute, value);
	}

	return newElement;
}

//adds contact to array of contacts and to DOM
void Server::addNewContact(const QString& user, const QString& contact)
{
	m_contacts[user].append(contact);

	QDomNode userDomElement;
	int i = 0;

	//find dom element for given user
	for(i = 0;i < m_users.count();++i){
		userDomElement = m_users.at(i);
		if(userDomElement.attributes().namedItem("username").nodeValue() == user)
		{
			QDomElement newContact = createNewXmlElement("contact", contact);
			userDomElement.firstChildElement("contacts").appendChild(newContact);
			saveXMLFile();
			break;
		}
	}
}

bool isNumeric(QByteArray arr){
	return std::all_of(arr.begin(), arr.end(),
					   [](char c){return isdigit(c);});
}

void Server::sendContactsFor(QString username, QTcpSocket* socket) const
{
	QJsonObject contactsDataJson;
	QJsonArray contactsArrayJson;
	std::transform(m_contacts[username].begin(),
				   m_contacts[username].end(),
				   std::back_insert_iterator<QJsonArray>(contactsArrayJson),
				   [&](QString s){
						return QJsonObject({
											{"contact", s},
											{"online", isOnline(s)}
										   });
					});
	contactsDataJson.insert("type", setMessageType(MessageType::Contacts));
	contactsDataJson.insert("to",username);
	contactsDataJson.insert("contacts", contactsArrayJson);
	sendMessageTo(socket, contactsDataJson);
}
void Server::sendGroupsFor(QString username, QTcpSocket* socket) const{
	QJsonObject groupsDataJson;
	QJsonArray groupsArrayJson;
	std::transform(m_usernameToGroups[username].begin(),
				   m_usernameToGroups[username].end(),
				   std::back_insert_iterator<QJsonArray>(groupsArrayJson),
				   [&](int groupId){
						chatGroup gr = m_groups[groupId];
						qDebug() << "grupa " << gr.groupName << " ima id " << gr.id << "i clanove " << gr.members;
						QJsonArray membersArray;
						for(auto member : gr.members){
							membersArray.append(member);
						}
						return QJsonObject({
											{"groupId", groupId},
											{"groupName", gr.groupName},
											{"members", membersArray}
										   });
					});
	groupsDataJson.insert("type", setMessageType(MessageType::Groups));
	groupsDataJson.insert("to",username);
	groupsDataJson.insert("groups", groupsArrayJson);
	sendMessageTo(socket, groupsDataJson);
}

void Server::sendUnreadMessages(const QString& username, QTcpSocket* socket)
{
	qDebug() << "Sending unread messages for " << username;
	for(auto message : m_unreadMessages[username]){
		qDebug() << QJsonDocument(message).toJson(QJsonDocument::Compact);
		sendMessageTo(socket, message);
	}
	m_unreadMessages.remove(username);

}

bool Server::hasUnreadMessages(const QString& username) const
{
	return m_unreadMessages.contains(username);
}

void Server::authentication(QJsonObject jsonResponseObject, QTcpSocket* senderSocket)
{
	if(m_usernameToSocket.contains(jsonResponseObject["username"].toString())){
		qDebug() << "ALLREADY LOGGED IN";
		sendServerMessageTo(senderSocket,MessageType::AllreadyLoggedIn);
        //senderSocket->disconnectFromHost();
	}
	else if(checkPassword(jsonResponseObject) == false){
		qDebug() << "BAD PASS";
        sendServerMessageTo(senderSocket,MessageType::BadPass);
        //senderSocket->disconnectFromHost();
	}
	else{
		//helper var, just for nicer code;
		QString tmpUsername = jsonResponseObject["username"].toString();

		qDebug() << "AUTH FOR USER " << tmpUsername << " SUCCESSFUL";

		sendContactsFor(tmpUsername, senderSocket);
		sendGroupsFor(tmpUsername, senderSocket);
		notifyContacts(tmpUsername, MessageType::ContactLogin);
		if(hasUnreadMessages(tmpUsername)){
			sendUnreadMessages(tmpUsername, senderSocket);
		}
		m_usernameToSocket[tmpUsername] = senderSocket;
	}
}

void Server::checkContactExistence(const QString& tmpFrom, const QString& tmpTo)
{
	if(!m_contacts[tmpFrom].contains(tmpTo)){
		addNewContact(tmpFrom, tmpTo);
	}
	if(!m_contacts[tmpTo].contains(tmpFrom)){
		addNewContact(tmpTo, tmpFrom);
	}
}

void Server::forwardMessage(const QString& to, const QJsonObject& message)
{
	bool ret = sendMessageTo(	m_usernameToSocket[to],
								message);
	if(!ret){
		qDebug() << "WEIRD! User " << to << "is not online, afterall";
		//adding message to buffer for next time user logs in
		m_unreadMessages[to].append(message);
	}
}

bool Server::isOnline(const QString& username) const
{
	return m_usernameToSocket.contains(username);
}

void Server::addGroupToXml(const chatGroup& group){
	QDomElement groupTag = createNewXmlElement("group", "",
											   "groupName", group.groupName);
	QDomElement groupId = createNewXmlElement("id", QString::number(group.id));
	QDomElement members = createNewXmlElement("members");
	for(auto member : group.members){
		members.appendChild(createNewXmlElement("member", member));
	}
	groupTag.appendChild(groupId);
	groupTag.appendChild(members);
	m_dataDoc.documentElement().appendChild(groupTag);
	saveXMLFile();

}

void Server::readMessage(){
	QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());

	//reading first 4 characters, they represent length of json encoded message
	QByteArray messageLength = senderSocket->read(4);

	//check if sent data represents a number
	if(!isNumeric(messageLength))
	{
		//reading all but not saving it since it is not in valid format
		senderSocket->readAll();
		sendServerMessageTo(senderSocket, MessageType::BadMessageFormat);
		return;
	}

	//reading next messageLength bytes
	QJsonDocument jsonResponse = QJsonDocument::fromJson(
				senderSocket->read(messageLength.toInt()));

	//check if message is in valid json format
	if(jsonResponse.isNull()){
		sendServerMessageTo(senderSocket, MessageType::BadMessageFormat);
		return;
	}

	QJsonObject jsonResponseObject = jsonResponse.object();

	auto msgType = jsonResponseObject["type"];
	//if sent json object is auth object
	if(msgType == MessageType::Authentication){
		authentication(jsonResponseObject, senderSocket);
	}
	else if(msgType == MessageType::AddNewContact){
		QString newContact = jsonResponseObject["username"].toString();
		QString user =  jsonResponseObject["from"].toString();
		if(!m_authData.contains(newContact)){
			jsonResponseObject["exists"] = false;
		}
		else{
			jsonResponseObject["exists"] = true;
			jsonResponseObject["online"] = isOnline(newContact);
			checkContactExistence(user, newContact);
		}
		forwardMessage(user, jsonResponseObject);
	}
	//if sent data is text message, forward it only to the intended recepient
	else if(msgType == MessageType::Text){
		if(jsonResponseObject.contains("groupId")){
			int groupId = jsonResponseObject["groupId"].toInt();
			auto gr = m_groups[groupId];
			for(auto member : gr.members){
                if(member != jsonResponseObject["from"].toString()){
                    if(isOnline(member)){
                        forwardMessage(member, jsonResponseObject);
                    }
                    else{
                        //adding message to buffer for next time user logs in
                        m_unreadMessages[member].append(jsonResponseObject);
                    }
                }
			}
		}
		else{
			QString tmpTo = jsonResponseObject["to"].toString();
			QString tmpFrom = jsonResponseObject["from"].toString();
			if(isOnline(tmpTo)){
				forwardMessage(tmpTo, jsonResponseObject);
			}
			else{
				//adding message to buffer for next time user logs in
				m_unreadMessages[tmpTo].append(jsonResponseObject);
			}
		}
	}
	else if(msgType == MessageType::CreateGroup){
		QJsonArray jsonMemebers = jsonResponseObject["members"].toArray();
		QSet<QString> groupMembers;
		for(auto member: jsonMemebers){
			groupMembers.insert(member.toVariant().toString());
		}
		QString groupName = jsonResponseObject["groupName"].toString();
		chatGroup gr;
		gr.groupName = groupName;
		gr.members = groupMembers;
		gr.id = m_nextGroupId;
		m_nextGroupId++;
		m_groups[gr.id] = gr;
		jsonResponseObject["id"] = gr.id;
		for(auto member: groupMembers){
			if(isOnline(member)){
				forwardMessage(member, jsonResponseObject);
			}
			else{
				m_unreadMessages[member].append(jsonResponseObject);
			}
		}
		addGroupToXml(gr);
	}
	else
	{
		qDebug() << "UNKNOWN MESSAGE TYPE";
	}
}

void Server::createUser(const QString& pass, const QString& username)
{
	QDomElement newUser = createNewXmlElement("user", "", "username", username);

	QDomElement password = createNewXmlElement("password", pass);

	QDomElement contacts = createNewXmlElement("contacts");
	newUser.appendChild(password);
	newUser.appendChild(contacts);

	m_dataDoc.documentElement().appendChild(newUser);

	m_authData[username] = pass;
	saveXMLFile();
}

bool Server::checkPassword(const QJsonObject & json){
	QString username = json["username"].toString();
	QString pass = json["password"].toString();

	//if username exists
	if(m_authData.contains(username)){
		return m_authData[username] == pass;
	}
	//first login, append to file and to current map
	//TODO create better system for making account
	else{
		createUser(pass, username);
		return true;
	}
}

bool Server::isInitialized() const{
	return m_isInitialized;
}
void Server::showError() const{
	std::cerr << m_errorMessage << std::endl;
}
