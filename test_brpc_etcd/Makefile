all:discover register
discover:discover.cc
	g++ $^ -o $@ -std=c++17 -lbrpc -lcpprest -lgflags -lssl -lcrypto -lprotobuf -lleveldb -lspdlog -lfmt 
register:register.cc
	g++ $^ -o $@ -std=c++17 -lbrpc -lcpprest -lgflags -lssl -lcrypto -lprotobuf -lleveldb -lspdlog -lfmt

clean:
	rm client server