#include "desktop_commands.h"

std::vector<std::string> tokens;

bool in_tokens(const std::string& token)
{
	for(const auto& logged_token : tokens)
	{
		if(token == logged_token)
		{
			return true;
		}
	}
	return false;
}

void remove_token(const std::string& token)
{
	for(auto& logged_token : tokens)
	{
		if(token == logged_token)
		{
			logged_token.erase();
			break;
		}
	}
}
Json::Value LogIn::command(const Json::Value& root)
{
	auto username = root["username"].as<std::string>();
	auto password = root["password"].as<std::string>();
	pqxx::work txn{*this->conn};
	auto result = txn.exec_prepared("login", username, password);
	Json::Value ret;
	
	ret["status"] = "fail";
	if(result.size() == 1) {
		srand(time(0));
		ret["status"] = "success";
		std::string token;
		for(int i = 0; i < 128; i++)
		{
			token += (char)'0' + rand() % 74;
		}
		tokens.emplace_back(token);
		ret["token"] = token;
	}
	
	return ret;
}


Json::Value LogOut::command(const Json::Value& root)
{
	auto token = root["token"].as<std::string>();
	Json::Value ret;
	if(in_tokens(token))
	{
		remove_token(token);
		ret["status"] = "succes";
	} else {
		ret["status"] = "fail";
	}
	return ret;
}

Json::Value GetData::command(const Json::Value& root)
{
	Json::Value ret;
	if(in_tokens(root["token"].as<std::string>())) {
		auto data = root["data"].as<std::string>();
		if(data == "get_tables")
		{
			ret["status"] = "success";
			pqxx::work txn{*this->conn};
			
			auto rows = txn.exec_prepared("get_tables");
			
			for(int i = 0; i < rows.size(); i++)
			{
				Json::Value table;
				std::string table_name;
				std::string column_name;
				
				for(const auto& field : rows[i]) table_name = field.as<std::string>(); 
				table["table_name"] = table_name;
				
				auto row = txn.exec_prepared1("get_columns", table_name);
				for(const auto& field : row) column_name = field.as<std::string>();
				table["columns"] = column_name;

				ret["tables"][i] = table; 
			}	
		} else if(data == "get_query")
		{
			pqxx::work txn{*this->conn};
			auto table_name = root["table"].as<std::string>();
			auto where_clause = root["query"].as<std::string>();
		
			std::string query;
			if(where_clause.length() == 0) {
				query = "SELECT * FROM "+table_name+ " ;"; 
			} else {
				query = "SELECT * FROM "+table_name+ " WHERE "+where_clause+" ;";
			}
			auto rows = txn.exec(query);
			
			for(int i = 0; i < rows.size(); i++)
			{
				for(int j = 0; j < rows[i].size(); j++)
				{
					ret["data"][i][j] = rows[i][j].as<std::string>();
				}
			}
			ret["status"] = "success";
		
		} else {
			ret["status"] = "fail";
		}
	} else {
		ret["status"] = "fail";
	}
	return ret;
}

void CommandManager::add_command(Command *command)
{
	for(int i = 0; i < this->commands.size(); i++)
	{
		if(command->name() == this->commands[i]->name())
		{
			if(dynamic_cast<LogIn*>(command)!=nullptr)
			{
				delete (LogIn *)command;
			} else if(dynamic_cast<LogOut*>(command)!=nullptr)
			{
				delete (LogOut *)command;
			} else if(dynamic_cast<GetData*>(command)!=nullptr)
			{
				delete (GetData*)command;
			}
			return;
		}
	}
	this->commands.push_back(command);
}

const std::vector<Command *>& CommandManager::get_commands() const
{
	return this->commands;
}
