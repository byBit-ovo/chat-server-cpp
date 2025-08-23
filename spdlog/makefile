sync:sync.cc
	g++ $^ -o $@ -std=c++17 -lspdlog -lfmt -lgflags
async::async.cc
	g++ $^ -o $@ -std=c++17 -lspdlog -lfmt -lgflags
logger: logger.cpp
	g++ $^ -o $@ -std=c++17 -lspdlog -lfmt -lgflags
.PHONY:clean
clean:
	rm sync async logger