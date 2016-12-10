#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T15:36:33
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChattyBird_server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    client.cpp

HEADERS  += mainwindow.h \
    server.h \
    codes.h \
    client.h

FORMS    += mainwindow.ui
