#ifndef GATEWAY_TEST_COMMON_HPP
#define GATEWAY_TEST_COMMON_HPP

#include "httplib.h"
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <sstream>

inline std::string TestUuid() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen) << "-" << std::chrono::steady_clock::now().time_since_epoch().count();
    return oss.str();
}

template <typename Req, typename Rsp>
static bool PostProto(httplib::Client& cli, const std::string& path, const Req& req, Rsp& rsp) {
    std::string body;
    if (!req.SerializeToString(&body)) {
        std::cout << "[ERROR] 序列化失败: " << path << std::endl;
        return false;
    }
    auto res = cli.Post(path.c_str(), body, "application/x-protbuf");
    if (!res) {
        std::cout << "[ERROR] HTTP POST 失败: " << path << std::endl;
        return false;
    }
    if (res->status != 200) {
        std::cout << "[WARN] HTTP " << path << " status=" << res->status << std::endl;
    }
    if (!rsp.ParseFromString(res->body)) {
        std::cout << "[ERROR] 反序列化失败: " << path << std::endl;
        return false;
    }
    return true;
}

template <typename Req, typename Rsp>
static bool GetProto(httplib::Client& cli, const std::string& path, const Req& req, Rsp& rsp) {
    std::string body;
    if (!req.SerializeToString(&body)) {
        std::cout << "[ERROR] 序列化失败: " << path << std::endl;
        return false;
    }
    httplib::Request req_http;
    req_http.method = "GET";
    req_http.path = path;
    req_http.set_header("Content-Type", "application/x-protbuf");
    req_http.body = body;
    httplib::Response res_http;
    httplib::Error err;
    if (!cli.send(req_http, res_http, err)) {
        std::cout << "[ERROR] HTTP GET 失败: " << path << " " << (int)err << std::endl;
        return false;
    }
    if (res_http.status != 200) {
        std::cout << "[WARN] HTTP " << path << " status=" << res_http.status << std::endl;
    }
    if (!rsp.ParseFromString(res_http.body)) {
        std::cout << "[ERROR] 反序列化失败: " << path << std::endl;
        return false;
    }
    return true;
}

static void PrintResult(const std::string& name, bool ok, const std::string& errmsg) {
    std::cout << "[" << name << "] success=" << (ok ? "true" : "false");
    if (!errmsg.empty()) std::cout << " errmsg=" << errmsg;
    std::cout << std::endl;
}

#endif
