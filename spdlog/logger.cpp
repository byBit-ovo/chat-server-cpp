#include "logger.hpp"
#include <gflags/gflags.h>

DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");

int main(int argc,char *argv[])
{
    google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
    LOG_TRACE("Hello {},{}","Good morning",1); 
    LOG_DEBUG("Hello {},{}","Good morning",1); 
    LOG_INFO("Hello {},{}","Good morning",1); 
    LOG_WARNING("Hello {},{}","Good morning",1); 
    LOG_ERROR("Hello {},{}","Good morning",1); 
    LOG_FATAL("Hello {},{}","Good morning",1); 
    return 0;
}