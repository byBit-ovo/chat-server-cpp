# minimum required cmake version 
cmake_minimum_required(VERSION 3.18)

# project name
project(helloWorld)

# build target
add_executable(main main.cc)

# add test
include(CTest)
add_test(
	Name testAdd
	COMMAND main
)

# install local
include(GNUInstallDirs)
install(TARGETS main)

