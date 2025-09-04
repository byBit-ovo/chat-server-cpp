all:put get
put:put.cc
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest 
get:get.cc
	g++ $^ -o $@ -std=c++17 -letcd-cpp-api -lcpprest 
clean:
	rm put get