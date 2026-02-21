// 网关文件相关 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "file.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(session_id, "", "登录 session_id（必填）");
DEFINE_string(file_id, "", "文件 id（get_single 时用）");
DEFINE_string(file_ids, "", "文件 id 列表，逗号分隔（get_multi）");
DEFINE_string(file_name, "test_upload.txt", "上传文件名");
DEFINE_string(file_content, "hello file content", "上传文件内容");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    httplib::Client cli(FLAGS_gateway_host, FLAGS_gateway_port);
    cli.set_read_timeout(10, 0);
    cli.set_write_timeout(10, 0);

    std::string session_id = FLAGS_session_id;
    if (session_id.empty()) {
        std::cout << "请提供 -session_id=xxx" << std::endl;
        return -1;
    }

    auto split = [](const std::string& s) {
        std::vector<std::string> out;
        std::string t;
        for (char c : s) {
            if (c == ',') { if (!t.empty()) out.push_back(t); t.clear(); }
            else t += c;
        }
        if (!t.empty()) out.push_back(t);
        return out;
    };

    // 1. 获取单个文件（GET）
    if (!FLAGS_file_id.empty()) {
        MY_IM::GetSingleFileReq req;
        MY_IM::GetSingleFileRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_file_id(FLAGS_file_id);
        GetProto(cli, "/chat/file/get_single_file", req, rsp);
        PrintResult("get_single_file", rsp.success(), rsp.errmsg());
        if (rsp.success() && rsp.has_file_data())
            std::cout << "  file_id=" << rsp.file_data().file_id()
                      << " size=" << rsp.file_data().file_content().size() << std::endl;
    }

    // 2. 获取多个文件（GET）
    if (!FLAGS_file_ids.empty()) {
        MY_IM::GetMultiFileReq req;
        MY_IM::GetMultiFileRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        for (const auto& id : split(FLAGS_file_ids))
            req.add_file_id_list(id);
        GetProto(cli, "/chat/file/get_multi_file", req, rsp);
        PrintResult("get_multi_file", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  (file_data map 已返回)" << std::endl;
    }

    // 3. 上传单个文件（POST）
    {
        MY_IM::PutSingleFileReq req;
        MY_IM::PutSingleFileRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        auto* fd = req.mutable_file_data();
        fd->set_file_name(FLAGS_file_name);
        fd->set_file_content(FLAGS_file_content);
        fd->set_file_size(static_cast<int64_t>(FLAGS_file_content.size()));
        PostProto(cli, "/chat/file/put_single_file", req, rsp);
        PrintResult("put_single_file", rsp.success(), rsp.errmsg());
        if (rsp.success() && rsp.has_file_info())
            std::cout << "  file_id=" << rsp.file_info().file_id() << std::endl;
    }

    // 4. 上传多个文件（POST）
    {
        MY_IM::PutMultiFileReq req;
        MY_IM::PutMultiFileRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        auto* fd1 = req.add_file_data();
        fd1->set_file_name("multi_a.txt");
        fd1->set_file_content("content a");
        fd1->set_file_size(9);
        auto* fd2 = req.add_file_data();
        fd2->set_file_name("multi_b.txt");
        fd2->set_file_content("content b");
        fd2->set_file_size(9);
        PostProto(cli, "/chat/file/put_multi_file", req, rsp);
        PrintResult("put_multi_file", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  file_count=" << rsp.file_info_size() << std::endl;
    }

    return 0;
}
