#include "../../common/search.hpp"
#include "../../common/logger.hpp"
#include <gflags/gflags.h>
#include <iostream>
#include <thread>
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");


int main(int argc, char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	auto client = MY_IM::ESClientFactory::create({"http://127.0.0.1:9200/"});
	MY_IM::ES_User es_user(client);
	es_user.createIndex();
	es_user.appendData("ID1","133","小猪佩奇1","我爱吃薯条","头像1");
	es_user.appendData("ID2","101","小猪佩奇2","我爱吃炸鸡","头像2");
	es_user.appendData("ID3","222","小猪佩奇3","我爱吃汉堡","头像3");
	es_user.appendData("ID4","833","小猪佩奇4","我爱喝可乐","头像4");
	auto res = es_user.search("小猪佩奇",{"ID2"});
	for(auto &user: res){
		std::cout << user.nickname() << std::endl;
		std::cout << user.user_id() << std::endl;
		std::cout << user.description() << std::endl;
		std::cout << user.phone() << std::endl;
	}
	

	return 0;
}