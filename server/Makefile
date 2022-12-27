
CXX:=g++
CXXFLAGS:=-O2 -Wall -ggdb -std=c++23
LDFLAGS:=-lcrypto -lssl -lpq -lpqxx -ljsoncpp

all: dir network2 network_client network_server network_local_server accepted_client server server_bin

dir:
	mkdir -p ./out


network_client:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_client.o ./network/client.cc

network2:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network.o ./network/network.cc

network_server:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_server.o ./network/server.cc

network_local_server:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_local_server.o ./network/local_server.cc

accepted_client:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_accepted_client.o ./network/accepted_client.cc

server:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/server.o server.cc

server_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/server ./out/server.o ./out/network_local_server.o ./out/network_server.o ./out/network.o ./out/network_accepted_client.o


clean:
	rm -rf ./out
