#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <json/json.h>
#include <QString>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

Json::Value to_json(std::string s)
{
    Json::Value json_message;
    try {
        std::istringstream stream(s);
        stream >> json_message;
    } catch(const std::exception& e)
    {
        qDebug() << e.what();
    }
    return json_message;
}

void MainWindow::on_pushButton_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    Json::Value root;
    root["command"] = "login";
    root["username"] = username.toStdString();
    root["password"] = password.toStdString();

    client->send_message(root.toStyledString());
    std::string message = client->receive_message();

    Json::Value json_message = to_json(message);

    if(json_message["status"].as<std::string>() == "success")
    {
        std::string token = json_message["token"].as<std::string>();
        this->hide();
        this->page = new mainpage(this, this->client, token);
        this->page->show();
    }
}

void MainWindow::setClient(net::client *new_client)
{
    this->client = new_client;
}
