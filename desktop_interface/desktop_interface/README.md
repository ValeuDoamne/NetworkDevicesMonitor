# Desktop Interface

The desktop interface uses Qt6.

The interface connects to the server and can query the WHERE clause of a table to filter the date to event searched.

## Building

To build the program:

```
cmake -S . -B build && cd build && make
```
In the build directory will be produced the desktop_interface binary.

## Configuration

The program uses the configuration file found in `${PWD}/client.toml` to connect to the server to fetch the data
Example config file:
```
[client]
hostname = "192.168.1.10" # IP or Domain to the server 
port = 7777
secure = true
```
