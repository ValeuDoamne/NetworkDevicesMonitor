#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include "network/client.h"

namespace Ui {
class mainpage;
}

class mainpage : public QMainWindow
{
    Q_OBJECT
public:
    explicit mainpage(QWidget *parent, net::client *client, const std::string& token);
    ~mainpage();

private slots:
    void on_table_button_clicked();
    void set_text(int index)
    {
        this->index_table    = index;
        this->table_selected = table_button->itemText(index);
    }

    void on_search_button_clicked();

    void on_logout_button_clicked();

private:
    void getTableNames();
    int index_table;
    std::vector<std::pair<std::string, std::vector<std::string>>> table_name_and_columns;

    Ui::mainpage *ui;
    std::string token;
    net::client* client;
    QString table_selected;
    QComboBox *table_button;
};

#endif // MAINPAGE_H
