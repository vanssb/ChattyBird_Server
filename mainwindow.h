#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "server.h"
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Server server;
    QTimer timer;
private slots:
    void startServer();
    void stopServer();
    void error();
    void message();
    void updateStats();
    void serverUpdated();
};

#endif // MAINWINDOW_H
