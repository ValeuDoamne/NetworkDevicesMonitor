
CXX:=g++
CXXFLAGS:=-O2 -Wall -ggdb
LDFLAGS:=-lcrypto -lssl

all: dir network2 network_client network_server network_local_server accepted_client client client_bin

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

client:
	${CXX} ${CXXFLAGS}  -c -o ./out/logger.o ./logger.cc

client_bin:
	${CXX} ${CXXFLAGS} ${LDFLAGS} -o ./out/logger ./out/logger.o ./out/network.o ./out/network_client.o ./out/network_accepted_client.o ./out/network_local_server.o

send:
	scp ./out/logger kali@192.168.1.135:

clean:
	rm -rf ./out
