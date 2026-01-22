#include "../common/etcd.hpp"
#include <gflags/gflags.h>

DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");
DEFINE_int64(ttl,10,"连接保活时长");
DEFINE_string(etcd_host,"http://127.0.0.1:2379","etcd服务端地址");
DEFINE_string(base_dir,"/service","服务监控根目录");
DEFINE_string(service_url,"game/shoot","射击游戏目录");
using namespace MY_IM;
int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	Registerant::ptr rclient = std::make_shared<Registerant>(FLAGS_etcd_host,FLAGS_ttl);
	rclient->Register(FLAGS_base_dir + FLAGS_service_url,"shoot!");

	sleep(10);
	return 0;
}