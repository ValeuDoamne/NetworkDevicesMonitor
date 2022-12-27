
CXX:=g++
CXXFLAGS:=-O3 -Wall -ggdb
LDFLAGS:=-lcrypto -lssl

all: dir network2 network_client network_server accepted_client client client_bin server server_bin

dir:
	mkdir -p ./out


network_client:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_client.o ./network/network_client.cc

network2:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network.o ./network/network.cc

network_server:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_server.o ./network/network_server.cc

accepted_client:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/network_accepted_client.o ./network/network_accepted_client.cc
client:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/client.o ./client.cc 

client_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/client ./out/client.o ./out/network.o ./out/network_client.o

server:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -c -o ./out/server.o server.cc

server_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/server ./out/server.o ./out/network_server.o ./out/network.o ./out/network_accepted_client.o


clean:
	rm -rf ./out
