#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer socket;
    QString message;
    QString errorValue;
public:
    explicit Server(QObject *parent = 0);
    void start(quint16 port);
    QString getMessage();
    QString errorString();
signals:
    void newMessage();
    void error();
public slots:

private slots:
    void newConnection();
    void error(QAbstractSocket::SocketError);
};

#endif // SERVER_H
