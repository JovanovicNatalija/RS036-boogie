#ifndef CLIENT_H
#define CLIENT_H

#include <QSslSocket>
#include <QVector>
#include <map>
#include <vector>
#include <iterator>
#include <tuple>
#include <ctime>
#include <chrono>
#include <QPixmap>

class Client : public QSslSocket
{
    Q_OBJECT
public:
    Client(QObject* parent = nullptr);
    Q_INVOKABLE void connectToServer(const QString& username,
                                     const QString& ip,
                                     quint16 port = 10000);

    Q_INVOKABLE void sendMsg(const QString& str);
    Q_INVOKABLE void sendAuthData(QString password);
    Q_INVOKABLE void sendMsgData(const QString& to,const QString& msg);
    Q_INVOKABLE void addMsgToBuffer(const QString& sender,
                                    const QString& inConversationWith,
                                    const QString& msg);
    Q_INVOKABLE void writeInXml();
    Q_INVOKABLE void readFromXml();
    Q_INVOKABLE void displayOnConvPage(const QString& inConversationWith);
    Q_INVOKABLE void addNewContact(const QString& name, bool online);
    Q_INVOKABLE void checkNewContact(const QString& name);
    Q_INVOKABLE void sendPicture(const QString& filePath, const QString& to);
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE QString username();
    void createXml() const;

signals:
    void showPicture(QString picturePath);
    void showMsg(const QString& msgFrom,const QString& msg);
    void showContacts(const QString& contact, bool online);
    void clearContacts();
    void badPass();
    void alreadyLogIn();
    void badContact(const QString& msg);

public slots:
    void readMsg();
    void sslErrors(const QList<QSslError> &errors);


public:
    int imageNum = 0;
    int m_bytesToRead = 0;

private:
    QString m_username;
    QHash<QString, QVector<std::tuple<QString, QString, QString>>> m_msgDataBuffer;
    QHash<QString, int> m_msgIndexBegin;
    QHash<QString, bool> m_contactInfos;
    unsigned long m_msgCounter = 0;

};

#endif // CLIENT_H
