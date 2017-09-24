CC = g++

server-client: server.out client.out

client.out: client.cpp
	$(CC) -o $@ $^

server.out: server.cpp
	$(CC) -o $@ $^