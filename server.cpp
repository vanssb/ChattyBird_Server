#include "server.h"
#include <QDataStream>
#include <codes.h>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QTime>

Server::Server(QObject *parent) : QObject(parent)
{
    connect( &socket, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    serverStatus = 0;
}

void Server::start(quint16 port){
    if (socket.listen(QHostAddress::LocalHost, port) ){
        message = "Server succesfully started on port " + QString::number(port);
        serverStatus = 1;
        emit newMessage();
    }
    else{
        errorValue = socket.errorString();
        serverStatus = 0;
        emit error();
    }
}

void Server::newConnection(){
    QTcpSocket* clientSocket = socket.nextPendingConnection();
    int key = clientSocket->socketDescriptor();
    Client client;
    client.socket = clientSocket;
    clients[key] = client;
    key = clients[key].socket->socketDescriptor();
    connect( client.socket, SIGNAL(disconnected()), SLOT(disconnected()) );
    connect( client.socket, SIGNAL(readyRead()), SLOT(readyRead()) );
    message = "New user connected: " + QString::number(key);
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

void Server::readyRead(){
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int key = clientSocket->socketDescriptor();
    QDataStream in(clients[key].socket);
    if (clients[key].blockSize == 0) {
        //Ожидаем пока будет получен хотя бы размер блока
        if (clients[key].socket->bytesAvailable() < (quint64)sizeof(quint64))
            return;
        //считываем размер блока
            in >> clients[key].blockSize;
    }
    //ждем пока блок прийдет полностью
    if (clients[key].socket->bytesAvailable() < clients[key].blockSize)
        return;
    else
        //можно принимать новый блок
        clients[key].blockSize = 0;
    //3 байта - команда серверу
    quint8 command;
    in >> command;
    switch (command) {
    case Codes::auth:
        in >> clients[key].name;
        in >> clients[key].password;
        sendAuthResponse( clients[key], authorize(clients[key]) );
    break;
    case Codes::messagePublic:
        if (clients[key].isAuthed){
            QString text;
            in >> text;
            sendPublicMessage( clients[key], text );
        }
    break;
    }
}

int Server::authorize(Client &client){
    data = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath()+"/Data/Information.db";
    data.setDatabaseName(path);
    if (data.open()){
        QString request = "SELECT Nickname, COUNT(*) AS count FROM Users WHERE Name='" + client.name + "' AND Password='" + client.password +"'";
        QSqlQuery q( request );
        q.first();
        int count = q.value("count").toInt();
        if(count > 0){
            client.nickname = q.value("Nickname").toString();
            client.isAuthed = true;
            return Codes::authSuccess;
        }
        else
            return Codes::authIncorrectPair;
    }
    else{
        return Codes::authProblem;
    }
}

void Server::sendAuthResponse(Client &client, int code){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0 << Codes::auth << code;
    out.device()->seek(0);
    if (code == Codes::authSuccess){
        message = "Client " + QString::number(client.socket->socketDescriptor()) +
                  " logined as " + client.name;
        emit newMessage();
    }
//вписываем размер блока на зарезервированное место
    out << (quint64)(block.size() - sizeof(quint64));
    client.socket->write(block);
}

void Server::disconnected(){
    QTcpSocket* clientSocket = (QTcpSocket*)sender();
    int key = clientSocket->socketDescriptor();
}

void Server::stop(){
    if (serverStatus == 1){
        foreach( Client value, clients ){
            value.socket->close();
        }
        clients.clear();
        socket.close();
        message = "Server stopped";
        emit newMessage();
    }
}

void Server::sendPublicMessage(Client &sender,QString message){
    QString text = "[" + QTime::currentTime().toString("hh:mm:ss") + "] " +
                   sender.nickname + ": " + message;
    foreach( Client value, clients ){
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        // надо придумать коды ко всем операциям
        quint8 command;
        command = Codes::messagePublic;
        out << (quint64)0 << command  << text;
        out.device()->seek(0);
    //вписываем размер блока на зарезервированное место
        out << (quint64)(block.size() - sizeof(quint64));
        value.socket->write(block);
    }
}

