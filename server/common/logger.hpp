#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

std::shared_ptr<spdlog::logger> g_logger;
//  @mode: true -> release-version, 
//         false -> debug-version,flush to stdout
void init_logger(bool mode,const std::string &file_name="",int level=spdlog::level::level_enum::trace)
{
    if(mode == false){
        g_logger = spdlog::stdout_color_mt("default-logger");
        g_logger->set_level(spdlog::level::level_enum::trace);
        g_logger->flush_on(spdlog::level::level_enum::trace);
    }
    else
    {
        g_logger= spdlog::basic_logger_mt("file-logger",file_name);
        g_logger->set_level(static_cast<spdlog::level::level_enum>(level));
        g_logger->flush_on(static_cast<spdlog::level::level_enum>(level));
    } 
    g_logger->set_pattern("[%n][%H:%M:%S][thread: %t][%-8l]%v");

}

#define LOG_TRACE(format, ...)   g_logger->trace("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__);    //0
#define LOG_DEBUG(format, ...)   g_logger->debug("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__);    //1
#define LOG_INFO(format, ...)    g_logger->info("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__);     //2
#define LOG_WARNING(format, ...) g_logger->warn("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__);     //3
#define LOG_ERROR(format, ...)   g_logger->error("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__);    //4
#define LOG_FATAL(format, ...)   g_logger->critical("[{}:{}] " format,__FILE__,__LINE__,##__VA_ARGS__); //5