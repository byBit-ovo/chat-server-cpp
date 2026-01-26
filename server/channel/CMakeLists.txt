# 1. 添加cmake版本说明
cmake_minimum_required(VERSION 3.1.3)
# 2. 声明工程名称
project(channel_server)

set(target "channel_server")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 3. 检测并生成protobuf框架代码
set(proto_path ${CMAKE_CURRENT_SOURCE_DIR}/../proto)
set(proto_files base.proto transmite.proto channel.proto)
set(proto_srcs "")
foreach(proto_file ${proto_files})
    string(REPLACE ".proto" ".pb.cc" proto_cc ${proto_file})
    add_custom_command(
        PRE_BUILD
        COMMAND protoc
        ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I ${proto_path} --experimental_allow_proto3_optional ${proto_path}/${proto_file}
        DEPENDS ${proto_path}/${proto_file}
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc}
        COMMENT "生成Protobuf框架代码文件:" ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc}
    )
    list(APPEND proto_srcs ${CMAKE_CURRENT_BINARY_DIR}/${proto_cc})
endforeach()

# 4. 检测并生成ODB框架代码
set(odb_path ${CMAKE_CURRENT_SOURCE_DIR}/../odb)
set(odb_files chat_session_member.hxx chat_session.hxx channel.hxx)
set(odb_srcs "")
foreach(odb_file ${odb_files})
    string(REPLACE ".hxx" "-odb.cxx" odb_cxx ${odb_file})
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx})
        add_custom_command(
            PRE_BUILD
            COMMAND odb
            ARGS -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time ${odb_path}/${odb_file}
            DEPENDS ${odb_path}/${odb_file}
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}
            COMMENT "生成ODB框架代码文件:" ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx}
        )
    endif()
    list(APPEND odb_srcs ${CMAKE_CURRENT_BINARY_DIR}/${odb_cxx})
endforeach()

# 5. 获取源码目录下的所有源码文件
set(src_files "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source src_files)

# 6. 声明目标及依赖
add_executable(${target} ${src_files} ${proto_srcs} ${odb_srcs})

# 7. 设置需要连接的库
target_link_libraries(${target} -lgflags 
    -lspdlog -lfmt -lbrpc -lssl -lcrypto 
    -lprotobuf -lleveldb -letcd-cpp-api 
    -lcpprest -lcurl -lodb-mysql -lodb -lodb-boost
    /usr/lib/x86_64-linux-gnu/libjsoncpp.so.19
    -lcpr)

# channel 测试客户端
set(channel_client "channel_client")
set(channel_client_files ${CMAKE_CURRENT_SOURCE_DIR}/test/channe_client.cc)
add_executable(${channel_client} ${channel_client_files} ${proto_srcs})
target_link_libraries(${channel_client}
    -pthread -lgflags -lprotobuf
    -lspdlog -lfmt -lbrpc -lssl -lcrypto
    -lleveldb -letcd-cpp-api -lcpprest -lcurl)

# 8. 设置头文件默认搜索路径
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../odb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third/include)

# 9. 设置安装路径
INSTALL(TARGETS ${target} ${channel_client} RUNTIME DESTINATION bin)
