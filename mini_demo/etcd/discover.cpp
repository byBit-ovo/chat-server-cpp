#include "../common/etcd.hpp"
#include <gflags/gflags.h>

DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
DEFINE_string(etcd_host,"http://127.0.0.1:2379","etcd服务端地址");
DEFINE_string(base_dir,"/service","服务监控根目录");
void PutCall(const std::string &key,const std::string &value)
{
	LOG_INFO("Recieve a put notification");
}
void DelCall(const std::string &key,const std::string &value)
{
	LOG_INFO("Recieve a delete notification");
}
int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	Discoverer::ptr dicoverer_client = std::make_shared<Discoverer>(FLAGS_etcd_host,FLAGS_base_dir,PutCall,DelCall);

	sleep(60);
	return 0;
}