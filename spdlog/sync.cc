#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
int main()
{
    //set global flush strategy
    spdlog::flush_every(std::chrono::seconds(1));
    //flush immediately above debug
    spdlog::flush_on(spdlog::level::level_enum::debug);
    spdlog::set_level(spdlog::level::level_enum::debug);
    //sync logger default
    auto logger = spdlog::stdout_color_mt("default-logger");
    // auto logger = spdlog::basic_logger_mt("file-logger","./log/sync.log");
    logger->set_pattern("[%n][%Hh:%Mm:%Ss] [thread: %t] [%-8l] --%v");
    logger->trace("hello!,{}","jack");
    logger->debug("hello!,{}","jack");
    logger->info("hello!,{}","jack");
    logger->error("hello!,{}","jack");
    logger->warn("hello!,{}","jack");
    logger->critical("hello!,{}","jack");
    logger->debug("hello");
    std::cout<<"Log over!"<<std::endl;
    
    return 0;
}
