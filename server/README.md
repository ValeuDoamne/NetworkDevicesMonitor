# Server

The server uses PostGres as a database to store the data provided by agent.

The server at the beggining of the program forks itself to create 2 separate TCP servers: the first one is for accepting clients with desktop interfaces and the second one is for accepting agents connections from the modules.

The server is communicating ecrypted using TLS with the agents and the clients.

## Building

In a terminal inside the server directory run:
```
$ make
```
The server binary will be produced under `./out/server` file.

After installing postgres two databases must be used.
One database purpuse is for collecting the information for the modules and the other one is for adding users.

The users passwords are encrypted using sha512.
For setting up the users database the following commands must be made inside `psql -d [users_database]`, where users_database is the new database used.
```
CREATE TABLE users(id SERIAL PRIMARY KEY, username TEXT, password TEXT);
CREATE EXTENSION pgcrypto;
```
The commands create the table and get the pgcrypto extension for hashing

To insert a users in to the database:
```
INSERT INTO users(username, password) VALUES ('[username]', encode(digest('[password]', 'sha512'), 'hex'));
```
Where [username] and [password] are the desired credentials.

## Configuration

The server program uses TOML as configuraiton files, the default configuration is lcoated in the file `${PWD}/server_config.toml`. An example configuration:

```
[client_server]
hostname = "0.0.0.0"
port     = 7777 
secure   = true 
certificate_file     = "./cert.pem"
certificate_key_file = "./key.pem"

[agent_server]
hostname = "0.0.0.0"
port     = 8888
secure   = true
certificate_file     = "./cert.pem"
certificate_key_file = "./key.pem"

[database]
hostname = "localhost"
port     = 5432
username = "user"
password = "password"
db_name  = "modules" # the database used for modules
db_users = "users"   # the database used for users
```
