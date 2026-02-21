// 网关频道相关 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "channel.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(session_id, "", "登录 session_id（必填）");
DEFINE_string(channel_name, "test_channel", "频道名称");
DEFINE_string(member_ids, "", "成员 user_id 列表，逗号分隔（创建频道）");
DEFINE_string(channel_id, "", "频道 id（发消息时用）");
DEFINE_string(message_content, "hello channel", "频道消息内容");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    httplib::Client cli(FLAGS_gateway_host, FLAGS_gateway_port);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

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
    

    // 1. 创建频道（POST）
    {
        MY_IM::CreateChannelReq req;
        MY_IM::CreateChannelRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_channel_name(FLAGS_channel_name);
        for (const auto& id : split(FLAGS_member_ids))
            req.add_member_id_list(id);
        PostProto(cli, "/chat/channel/create", req, rsp);
        PrintResult("channel_create", rsp.success(), rsp.errmsg());
        if (rsp.success())
            std::cout << "  channel_id=" << rsp.channel_id() << std::endl;
    }

    // 2. 频道发消息（POST）
    if (!FLAGS_channel_id.empty()) {
        MY_IM::SendChannelMessageReq req;
        MY_IM::SendChannelMessageRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_channel_id(FLAGS_channel_id);
        req.mutable_message()->set_message_type(MY_IM::MessageType::STRING);
        req.mutable_message()->mutable_string_message()->set_content(FLAGS_message_content);
        PostProto(cli, "/chat/channel/new_message", req, rsp);
        PrintResult("channel_new_message", rsp.success(), rsp.errmsg());
    }

    return 0;
}
