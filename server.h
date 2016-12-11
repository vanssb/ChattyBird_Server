#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include "client.h"
#include <QMap>
#include <QSqlDatabase>
#include <QTimer>
#include <QStringList>
#include <QDataStream>

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer socket;
    QString message;
    QString errorValue;
    QMap<int, Client> clients;
    quint64 inputTrafficSize;
    quint64 outputTrafficSize;
    int authorize(Client &client);
    void sendAuthResponse(Client &client, quint8 code);
    void sendPublicMessage(Client &sender, QString message);
    QSqlDatabase data;
    int serverStatus;
    QTimer timer;
    void updateState();
    void parseData(quint8 command, QDataStream &in, Client &currentClient);
    int checkSignUpData(Client &client);
    void signUpUser(Client &client);
    void sendSignUpResponce(Client &client);
public:
    explicit Server(QObject *parent = 0);
    ~Server();
    void start(quint16 port);
    void stop();
    QString getMessage();
    QString errorString();
    QString getInputTrafficSize();
    QString getOutputTrafficSize();
    QStringList getUsersNameList();
signals:
    void newMessage();
    void error();
    void stateUpdated();
public slots:

private slots:
    void newConnection();
    void error(QAbstractSocket::SocketError);
    void readyRead();
    void disconnected();

};

#endif // SERVER_H
