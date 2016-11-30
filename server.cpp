#include "server.h"

Server::Server(QObject *parent) : QObject(parent)
{
    connect( &socket, SIGNAL(newConnection()), this, SLOT(newConnection()) );
}

void Server::start(quint16 port){
    if (socket.listen(QHostAddress::Any, port) ){
        message = "Server succesfully started...";
        emit newMessage();
    }
}

void Server::newConnection(){
    message = "New user connected";
    emit newMessage();
}

void Server::error(QAbstractSocket::SocketError){
    errorValue = socket.errorString();
    emit error();
}

QString Server::getMessage(){
    return message;
}

QString Server::errorString(){
    return errorValue;
}
