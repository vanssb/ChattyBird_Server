#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include "client.h"
#include <QMap>
#include <QSqlDatabase>

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer socket;
    QString message;
    QString errorValue;
    QMap<int, Client> clients;
    int authorize(Client &client);
    void sendAuthResponse(Client &client, int code);
    void sendPublicMessage(Client &sender, QString message);
    QSqlDatabase data;
    int serverStatus;
public:
    explicit Server(QObject *parent = 0);
    void start(quint16 port);
    void stop();
    QString getMessage();
    QString errorString();
signals:
    void newMessage();
    void error();
public slots:

private slots:
    void newConnection();
    void error(QAbstractSocket::SocketError);
    void readyRead();
    void disconnected();
};

#endif // SERVER_H
