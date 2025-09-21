all:client server
client:client.cc main.pb.cc
	g++ $^ -o $@ -std=c++17 -lbrpc -letcd-cpp-api -lcpprest -lleveldb -lgflags -lssl -lcrypto -lprotobuf -lspdlog -lfmt -g
server:server.cc main.pb.cc
	g++ $^ -o $@ -std=c++17 -lbrpc -letcd-cpp-api -lcpprest -lleveldb -lgflags -lssl -lcrypto -lprotobuf -lspdlog -lfmt -g

clean:
	rm client server