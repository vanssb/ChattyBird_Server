#ifndef CLIENT_H
#define CLIENT_H
#include <QTcpSocket>

class Client
{
public:
    Client();
    QTcpSocket* socket;
    bool isAuthed;
    quint64 blockSize;
    QString name;
    QString nickname;
    QString password;
};

#endif // CLIENT_H
