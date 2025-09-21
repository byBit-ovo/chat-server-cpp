all:put get 
test:dclient rclient
dclient:discover.cpp	
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags
rclient:register.cpp	
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest -lspdlog -lfmt -lgflags
put:put.cc
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest 
get:get.cc
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest 
clean:
	rm put get dclient rclient