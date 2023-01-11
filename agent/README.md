# Agent

The agent runs in the background as a daemon forwarding json messages between the modules and the server.

The agent is multiplexing using `select()` with a local server using AF_UNIX.

## Building

In this directory run into a terminal: 
```
$ make
```

## Configuration

The agent program it uses the configuration found on `${PWD}/agent_config.toml`
Example config:
```
[agent]
socket_file	= "/tmp/agent.pid" # optional: default set to "/run/agentd.pid"

[server]
hostname        = "127.0.0.1" # required
port            = 8888        # required
secure          = true        # default set to true
```
