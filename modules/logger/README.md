# Logger

The logger program uses inotify system call to watch a given subset of files if they are changed and in case of a change the program sends to the agent the last line from the file. To get the last line from a log file the program is using seeks.
The program is event based and for that it calls `poll()` for searching waiting for events, that makes the program not waste more CPU cyclies.

## Building

In a terminal inside the logger directory run:
```
$ make
```
The logger binary will be produced under `./out/logger` file.

## Configuration

The config file is located in `${PWD}/logger.toml`. An example:

```
[module]
socket_file  = "/tmp/agent.pid"
timeout      = 1
files        = ["/var/log", "/var/log/nginx"]

table_name   = "logs"

create_table = '''[
		           {"column_name" : "file",    "type": "TEXT"}, 
		           {"column_name" : "message", "type": "TEXT"}, 
		 ]'''
```
