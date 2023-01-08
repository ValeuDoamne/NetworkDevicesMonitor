#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QMainWindow>
#include <QString>
#include "network/client.h"

namespace Ui {
class mainpage;
}

class mainpage : public QMainWindow
{
    Q_OBJECT
public:
    explicit mainpage(QWidget *parent = nullptr);
    void setClient(net::client *client) {this->client = client;}
    void setToken(const std::string& token) {this->token = token;}
    ~mainpage();

private:
    Ui::mainpage *ui;
    std::string token;
    net::client* client;
};

#endif // MAINPAGE_H
