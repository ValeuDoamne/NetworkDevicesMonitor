
CXX:=g++
CXXFLAGS:=-O2 -Wall -ggdb -std=c++23
LDFLAGS:=-lcrypto -lssl -lpq -lpqxx -ljsoncpp

all: dir network_unmade  desktop_server agent_server main server_bin

dir:
	mkdir -p ./out

network_unmade:
	${CXX} ${CXXFLAGS} -c -o ./out/network.o ./network/network.cc
	${CXX} ${CXXFLAGS} -c -o ./out/network_server.o ./network/server.cc
	${CXX} ${CXXFLAGS} -c -o ./out/network_accepted_client.o ./network/accepted_client.cc

desktop_server:
	${CXX} ${CXXFLAGS} -c -o ./out/desktop_commands.o ./components/desktop_server/desktop_commands.cc
	${CXX} ${CXXFLAGS} -c -o ./out/desktop_server.o ./components/desktop_server/desktop_server.cc

agent_server:
	${CXX} ${CXXFLAGS} -c -o ./out/agent_server.o ./components/agent_server/agent_server.cc

main:
	${CXX} ${CXXFLAGS} -c -o ./out/main.o ./server.cc

server_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/server ./out/main.o ./out/agent_server.o ./out/desktop_server.o ./out/desktop_commands.o ./out/network_server.o ./out/network.o ./out/network_accepted_client.o


clean:
	rm -rf ./out
