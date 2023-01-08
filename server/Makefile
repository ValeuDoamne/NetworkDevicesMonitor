
CXX:=g++
CXXFLAGS:=-O2 -Wall -ggdb -std=c++23
LDFLAGS:=-lcrypto -lssl -lpq -lpqxx -ljsoncpp

all: dir network2 network_client network_server network_local_server accepted_client server commands desktop_server server_bin

dir:
	mkdir -p ./out


network_client:
	${CXX} ${CXXFLAGS}  -c -o ./out/network_client.o ./network/client.cc

network2:
	${CXX} ${CXXFLAGS}  -c -o ./out/network.o ./network/network.cc

network_server:
	${CXX} ${CXXFLAGS}  -c -o ./out/network_server.o ./network/server.cc

network_local_server:
	${CXX} ${CXXFLAGS}  -c -o ./out/network_local_server.o ./network/local_server.cc

accepted_client:
	${CXX} ${CXXFLAGS}  -c -o ./out/network_accepted_client.o ./network/accepted_client.cc

server:
	${CXX} ${CXXFLAGS}  -c -o ./out/server.o server.cc 

commands:
	${CXX} ${CXXFLAGS} -c -o ./out/desktop_commands.o ./components/desktop_commands.cc

desktop_server:
	${CXX} ${CXXFLAGS} -c -o ./out/desktop_server.o ./components/desktop_server.cc

server_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/server ./out/server.o ./out/desktop_commands.o ./out/desktop_server.o ./out/network_local_server.o ./out/network_server.o ./out/network.o ./out/network_accepted_client.o


clean:
	rm -rf ./out
