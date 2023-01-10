#ifndef DESKTOP_CONFIG_H
#define DESKTOP_CONFIG_H
#include <cstdio>
#include <string>
#include <unistd.h>
#include <cstring>
#include "toml.hpp"

struct Config
{
    std::string server_ip;
    uint16_t    server_port;
    bool        server_secure;
};

void parse_configuration(Config& configuration, const std::string& path)
{
    toml::table tbl = toml::parse_file(path);
    configuration.server_ip     = tbl["client"]["hostname"].value_or("");
    configuration.server_port   = tbl["client"]["port"].value_or(0);
    configuration.server_secure = tbl["client"]["secure"].value_or(true);
}


void show_help()
{
    printf("Help for the desktop utility:\n");
    printf("\t--config -c: [path]\t Set the server configuration path, default: client.toml\n");
    printf("\t--help   -h:\t\t Show this help.\n");
    _exit(0);
}

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

#endif // DESKTOP_CONFIG_H
