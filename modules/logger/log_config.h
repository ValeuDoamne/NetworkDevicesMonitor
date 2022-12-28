#pragma once
#include "toml.hpp"

struct Config {
	/* Agent Setup */
	std::string agent_socket_file;
	std::vector<std::string> files;
	int timeout = 0;
	std::string create_table;
	std::string table_name;
};

void parse_configuration(Config& configuration, const std::string& config_path)
{
	toml::table tbl;
	try {
		tbl = toml::parse_file(config_path);	

		configuration.agent_socket_file   = tbl["module"]["socket_file"].value_or("");
		configuration.timeout             = tbl["module"]["timeout"].value_or(0);
		configuration.create_table        = tbl["module"]["create_table"].value_or("");
		configuration.table_name          = tbl["module"]["table_name"].value_or("logs");

		if(toml::array *arr = tbl["module"]["files"].as_array())
		{
			arr->for_each([&configuration](auto&& el)
					{
						if constexpr (toml::is_string<decltype(el)>)
							configuration.files.push_back(*el);
					});
		}
	
	} catch (const toml::parse_error& err)
	{
		std::cerr << "[\033[31mERROR\033[0m]: Error parsing file '" << *err.source().path << "':\n" << err.description() << "\n  (" << err.source().begin << ")\n";
	}
}

void show_help();

void parse_command_line_arguments(int argc, char **argv, std::string& config_path) {
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			show_help();
		}
		if(strcmp(argv[i], "--config") == 0 || strcmp(argv[i], "-c") == 0) {
			if(i+1 == argc)	show_help();
			config_path = argv[i+1];	
		}
	}
}

void show_help()
{
	printf("Help for the logger utility:\n");
	printf("\t--config -c: [path]\t Set the server configuration path, default: ./logger.toml\n");
	printf("\t--help   -h:\t\t Show this help.\n");
	_exit(0);
}

