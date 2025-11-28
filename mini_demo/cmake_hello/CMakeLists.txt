# minimum required cmake version 
cmake_minimum_required(VERSION 3.18)

# project name
project(
	helloWorld
	VERSION 1.2.3
	LANGUAGES CXX
)

# build target
add_executable(main main.cc)
add_executable(testAdd test.cc)

# add test
include(CTest)
add_test(Case_add testAdd)

# install local
include(GNUInstallDirs)
install(TARGETS main)

# pack
include(CPack)
include(sub/sub.cmake)