#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <iostream>
#include "desktop_config.h"
#include "network/client.h"

#include "mainpage.h"

int main(int argc, char *argv[])
{
    std::string configuration_path = "client.toml";
    Config configuration;
    parse_command_line_arguments(argc, argv, configuration_path);
    parse_configuration(configuration, configuration_path);


    QApplication a(argc, argv);

    MainWindow w;
    net::client *desktop_connection;

    try{
        desktop_connection = new net::client(configuration.server_ip, configuration.server_port, net::protocol::TCP, configuration.server_secure);
        desktop_connection->connect();
    } catch(const net::network_error& e)
    {
        qDebug() << e.what();
    }
    w.setClient(desktop_connection);

    w.show();
    return a.exec();
}
