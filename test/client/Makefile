
all: dir network2 server bin

dir:
	mkdir -p ./out

network2:
	g++ -O2 -std=c++23 -c -o ./out/network.o ./network/network.cc
	g++ -O2 -std=c++23 -c -o ./out/client.o ./network/client.cc

server:
	g++ -O2 -std=c++23 -c -o ./out/main.o client.cc

bin:
	g++ -lcrypto -lssl -O2 -std=c++23 -o ./out/main ./out/main.o  ./out/client.o ./out/network.o

clean:
	rm -rf ./out
