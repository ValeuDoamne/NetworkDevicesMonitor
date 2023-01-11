#include "mainpage.h"
#include "ui_mainpage.h"
#include <json/json.h>
#include <QDebug>
#include <iostream>
#include <QTableView>
#include <QStandardItem>
#include "mainwindow.h"

Json::Value get_json(const std::string& s)
{
    Json::Value ret;
    try {
        std::istringstream stream(s);
        stream >> ret;
    }  catch(const std::exception& e)
    {
        qDebug() << e.what();
    }
    return ret;
}

std::vector<std::string> split_string(std::string s, const std::string& delimiter)
{
    std::vector<std::string> ret;

    size_t pos = 0;
    std::string token;
    while((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        ret.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    ret.push_back(s);
    return ret;
}

void mainpage::getTableNames()
{
    Json::Value data;
    data["command"] = "get_data";
    data["data"] = "get_tables";
    data["token"] = this->token;

    this->client->send_message(data.toStyledString());
    auto json_msg = get_json(this->client->receive_message());
    if(json_msg["status"].as<std::string>() == "success")
    {
        auto table_array = json_msg["tables"];
        for(unsigned int i = 0; i < table_array.size(); i++)
        {
            auto table_name = table_array[i]["table_name"].as<std::string>();
            auto column_name = table_array[i]["columns"].as<std::string>();
            this->table_name_and_columns.push_back(make_pair(table_name, split_string(column_name, ", ")));
        }
    } else qDebug() << "[Debug]: Trying to get table names failed\n";

    for(int i = 0; i < this->table_name_and_columns.size(); i++)
    {
        this->table_button->addItem(this->table_name_and_columns[i].first.c_str());
    }
}

mainpage::mainpage(QWidget *parent, net::client *client, const std::string& token) :
    QMainWindow(parent),
    ui(new Ui::mainpage),
    client(client),
    token(token)
{
    ui->setupUi(this);
    this->table_button = ui->table_button;
    this->getTableNames();

    connect(this->table_button, SIGNAL(activated(int)), this, SLOT(set_text(int)));
    set_text(0);
}

mainpage::~mainpage()
{
    delete ui;
}

void mainpage::on_table_button_clicked()
{
}


void mainpage::on_search_button_clicked()
{
    Json::Value data;
    data["command"] = "get_data";
    data["data"] = "get_query";
    data["token"] = this->token;
    data["table"] = this->table_selected.toStdString();
    auto query = ui->search_bar->text();
    data["query"] = query.toStdString();

    this->client->send_message(data.toStyledString());
    auto received_message = this->client->receive_message();

    auto json_msg = new Json::Value;
    try{
        std::istringstream stream(received_message);
        stream >> *json_msg;
    } catch(const std::exception& e)
    {
        std::cerr << "[Error]: Not valid json\n";
    }

    if((*json_msg)["status"].as<std::string>() == "success"){
        QStandardItemModel *mod = new QStandardItemModel;
        int index = 0;
        for(auto const &columns_name : this->table_name_and_columns[this->index_table].second){
            QStandardItem *it = new QStandardItem(QObject::tr(columns_name.c_str()));
            mod->setHorizontalHeaderItem(index++,it);
        }
        auto data = (*json_msg)["data"];
        for(int i = 0; i < data.size(); i++)
        {
            for(int j = 0; j < data[i].size(); j++)
            {
                QStandardItem *it = new QStandardItem(QObject::tr(data[i][j].asCString()));
                mod->setItem(i,j,it);
            }
        }
        ui->label_number_of_records->setText(QString(std::string("Records shown: "+std::to_string(data.size())).c_str()));
        ui->tableView->setModel(mod);
    }
    delete json_msg;
}

void mainpage::on_logout_button_clicked()
{
    Json::Value message;
    message["command"] = "logout";
    message["token"]   = this->token;
    this->client->send_message(message.toStyledString());
    auto json_msg = get_json(this->client->receive_message());
    auto window = (MainWindow *)this->parent();
    this->hide();
    window->show();
    delete this;
}

