// 网关消息转发 HTTP 接口调用测试（发送单条消息）
#include "gateway_test_common.hpp"
#include "transmite.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(session_id, "", "登录 session_id（必填）");
DEFINE_string(chat_session_id, "", "会话 id（必填）");
DEFINE_string(content, "hello gateway transmit test", "消息文本内容");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    httplib::Client cli(FLAGS_gateway_host, FLAGS_gateway_port);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

    std::string session_id = FLAGS_session_id;
    std::string chat_session_id = FLAGS_chat_session_id;
    if (session_id.empty() || chat_session_id.empty()) {
        std::cout << "请提供 -session_id=xxx -chat_session_id=xxx" << std::endl;
        return -1;
    }

    // 发送新消息（POST）
    MY_IM::NewMessageReq req;
    MY_IM::NewMessageRsp rsp;
    req.set_request_id(TestUuid());
    req.set_session_id(session_id);
    req.set_chat_session_id(chat_session_id);
    req.mutable_message()->set_message_type(MY_IM::MessageType::STRING);
    req.mutable_message()->mutable_string_message()->set_content(FLAGS_content);

    PostProto(cli, "/chat/message_transmit/new_message", req, rsp);
    PrintResult("new_message", rsp.success(), rsp.errmsg());

    return 0;
}
