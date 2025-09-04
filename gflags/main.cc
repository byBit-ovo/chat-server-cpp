#include <gflags/gflags.h>
#include <iostream>
//--flagflie=filename,用配置文件读取flag
DEFINE_string(ip,"127.0.0.1","服务器监听地址，格式:127.0.0.1");
DEFINE_int32(port,8989,"服务器监听端口,格式: 8080");
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");

int main(int argc,char *argv[]){
    google::ParseCommandLineFlags(&argc,&argv,true);
    std::cout<<FLAGS_ip<<std::endl;
    std::cout<<FLAGS_port<<std::endl;
    std::cout<<FLAGS_enable_debug<<std::endl;
    return 0;
}
