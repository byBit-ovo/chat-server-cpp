
#include "../common/asr.hpp"
#include "../common/logger.hpp"
#include <gflags/gflags.h>
void asr(aip::Speech& client)
{
    // 无可选参数调用接口
    std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);
    Json::Value result = client.recognize(file_content, "pcm", 16000, aip::null);
	if(result["err_no"].asInt() != 0)
	{
		std::cout << result["err_msg"].asString() << std::endl;
		return;
	}
	std::cout << result["result"][0].asString() << std::endl;
//     // 极速版调用函数
//     // Json::Value result = client.recognize_pro(file_content, "pcm", 16000, aip::null);

//     // 如果需要覆盖或者加入参数
//     std::map<std::string, std::string> options;
//     options["dev_pid"] = "1537";
//     Json::Value result = client.recognize(file_content, "pcm", 16000, options);
}
DEFINE_bool(enable_debug,true,"是否启用调试模式，格式:true/false");
DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");

int main(int argc, char *argv[])
{
	    // 设置APPID/AK/SK
    google::ParseCommandLineFlags(&argc,&argv,true);

	init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
    std::string app_id = "120114390";
    std::string api_key = "EFjD50piY2fiQps5ZzLSmQEE";
    std::string secret_key = "ANTsDIMZOGglxE6m6HQ6f0hI9zOzXNNj";
	MY_IM::ASRClient client(app_id,api_key,secret_key);
	std::string err;
	std::string file_content;
    aip::get_file_content("16k.pcm", &file_content);
	std::cout << client.Recognize(file_content,err) << std::endl;
	std::cout << err << std::endl;
    // aip::Speech client(app_id, api_key, secret_key);
	// asr(client);
	return 0;
}