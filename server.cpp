#include "server.h"
#include <codes.h>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QTime>

Server::Server(QObject *parent) : QObject(parent)
{
    connect( &socket, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    serverStatus = 0;
    inputTrafficSize = 0;
    outputTrafficSize = 0;
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

Server::~Server(){
    stop();
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
    else{
        //можно принимать новый блок
        inputTrafficSize += ( clients[key].blockSize + sizeof(clients[key].blockSize) );
        clients[key].blockSize = 0;
    }
    //3 байта - команда серверу
    quint8 command;
    in >> command;
    parseData( command, in , clients[key]);
}

int Server::authorize(Client &client){
    data = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath()+"/Data/Information.db";
    foreach( Client value, clients ){
        if (value.name == client.name && value.isAuthed)
            return Codes::authAlreadyOnline;
    }
    data.setDatabaseName(path);
    if (data.open()){
        QString request = "SELECT Nickname, COUNT(*) AS count FROM Users WHERE Name='" + client.name + "' AND Password='" + client.password +"'";
        QSqlQuery q( request );
        q.first();
        int count = q.value("count").toInt();
        QString nickname = q.value("Nickname").toString();
        data.close();
        if(count > 0){
            client.nickname = nickname;
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

void Server::sendAuthResponse(Client &client, quint8 code){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0 << code;
    out.device()->seek(0);
    if (code == Codes::authSuccess){
        message = "Client " + QString::number(client.socket->socketDescriptor()) +
                  " logined as " + client.name;
        emit newMessage();
        emit stateUpdated();
    }
//вписываем размер блока на зарезервированное место
    out << (quint64)(block.size() - sizeof(quint64));
    outputTrafficSize += ( block.size() );
    client.socket->write(block);
}

void Server::disconnected(){
    updateState();
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
    foreach( Client value, clients ){
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        // надо придумать коды ко всем операциям
        quint8 command;
        command = Codes::messagePublic;
        out << (quint64)0 << command  << sender.nickname << message;
        out.device()->seek(0);
    //вписываем размер блока на зарезервированное место
        out << (quint64)(block.size() - sizeof(quint64));
        outputTrafficSize += ( block.size() );
        value.socket->write(block);
    }
}

QString Server::getInputTrafficSize(){
    return QString::number( inputTrafficSize );
}

QString Server::getOutputTrafficSize(){
    return QString::number( outputTrafficSize );
}

void Server::updateState(){
    foreach( int key, clients.keys() ){
        if (clients[key].socket->socketDescriptor() == -1 || !clients[key].isAuthed){
            clients[key].socket->deleteLater();
            clients.remove(key);
        }
    }
    emit stateUpdated();
}

QStringList Server::getUsersNameList(){
    QStringList list;
    foreach( Client value, clients ){
        list.append(value.name);
    }
    return list;
}

void Server::parseData(quint8 command, QDataStream &in, Client &currentClient){
    switch (command) {
    case Codes::authRequest:
        in >> currentClient.name;
        in >> currentClient.password;
        sendAuthResponse( currentClient, authorize(currentClient) );
    break;
    case Codes::messagePublic:
        if (currentClient.isAuthed){
            QString text;
            in >> text;
            sendPublicMessage( currentClient, text );
        }
    break;
    case Codes::signUpRequest:
        in >> currentClient.name;
        in >> currentClient.password;
        in >> currentClient.nickname;
        sendSignUpResponce(currentClient);
    break;
    }
}

int Server::checkSignUpData(Client &client){
    data = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath()+"/Data/Information.db";
    data.setDatabaseName(path);
    if (data.open()){
        QString request = "SELECT Nickname, COUNT(*) AS count FROM Users WHERE Name='" + client.name + "' OR Nickname='" + client.nickname +"'";
        QSqlQuery q( request );
        q.first();
        int count = q.value("count").toInt();
        data.close();
        if (count)
            return Codes::signUpNameNotVacant;
        else{
            signUpUser(client);
            return Codes::signUpSuccess;
        }
    }
    return Codes::signUpProblem;
}

void Server::signUpUser(Client &client){
    data = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath()+"/Data/Information.db";
    data.setDatabaseName(path);
    if (data.open()){
        QString request = "INSERT INTO Users (Name,Password,Nickname) VALUES('"+client.name+"','"+client.password+"','"+client.nickname+"')";
        QSqlQuery q;
        q.exec( request );
        data.close();
    }
}

void Server::sendSignUpResponce(Client &client){
    quint8 code = checkSignUpData(client);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint64)0 << code;
    out.device()->seek(0);
//вписываем размер блока на зарезервированное место
    out << (quint64)(block.size() - sizeof(quint64));
    outputTrafficSize += ( block.size() );
    client.socket->write(block);
}
