#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "network/client.h"
#include "mainpage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setClient(net::client *new_client);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    net::client    *client;
    mainpage       *page;
};
#endif // MAINWINDOW_H
