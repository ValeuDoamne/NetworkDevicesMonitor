# Network Devices Monitor

This project consists of a 4 components: server, desktop interface, agent and modules

The server is the middle man between agents and the desktop interfaces.
The agents collect the data given by a module.
The desktop interfaces talk with the server to query data provided by the agents.

## Building

The building was done in an Arch Based Linux Distro (I recommend Manjaro for that).
The following packages are needed for building the project
```
$ sudo pacman -S jsoncpp openssl libpqxx postgresql
```
