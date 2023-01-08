#pragma once

#include <string>
#include <vector>
#include <json/json.h>
#include <pqxx/pqxx>

class Command
{
protected:
	pqxx::connection *conn;
public:
	Command(pqxx::connection *connection)
		:conn(connection) { }
	Command() { }
	virtual Json::Value command(const Json::Value& root) = 0;
	virtual std::string name() const = 0;
};

class LogIn : public Command {
public:
	LogIn(pqxx::connection *conn)
		:Command(conn) { }
	virtual Json::Value command(const Json::Value& root) override;
	virtual std::string name() const override {return "login";}
};

class LogOut : public Command {
public:
	virtual Json::Value command(const Json::Value& root) override;
	virtual std::string name() const override {return "logout";}
};

class GetData: public Command {
public:
	GetData(pqxx::connection *conn)
		:Command(conn) { }
	virtual Json::Value command(const Json::Value& root) override;
	virtual std::string name() const override {return "getdata";}
};

class CommandManager
{
private:
	std::vector<Command *> commands;
public:
	void add_command(Command *);
	const std::vector<Command *>& get_commands() const;
};
