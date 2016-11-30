#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect( ui->start, SIGNAL(clicked(bool)), this, SLOT(startServer()) );
    connect( &server, SIGNAL(newMessage()), this, SLOT(message()) );
    connect( &server, SIGNAL(error()), this, SLOT(error()) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startServer(){
    server.start( ui->portEdit->text().toInt() );
}

void MainWindow::error(){
    ui->logs->append( "[" + QTime::currentTime().toString("hh:mm:ss") + "] " + "Error: " + server.errorString() );
}

void MainWindow::message(){
    ui->logs->append( "[" + QTime::currentTime().toString("hh:mm:ss") + "] " +
                      server.getMessage() );
}
