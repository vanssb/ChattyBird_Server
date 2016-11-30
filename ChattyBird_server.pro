#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T15:36:33
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChattyBird_server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    server.h

FORMS    += mainwindow.ui
