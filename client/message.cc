// 网关消息存储相关 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "message.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 80, "网关HTTP端口");
DEFINE_string(session_id, "a5ad8006-a86b-4b99-aef8-94ec3f38cbac", "会话id");
DEFINE_string(chat_session_id, "5d3b9962-d1b9-4ee4-b744-385de33d7a29", "聊天回话id");
DEFINE_int64(start_time, 0, "历史消息起始时间");
DEFINE_int64(over_time, 0, "历史消息结束时间");
DEFINE_int32(msg_count, 10, "最近消息条数");
DEFINE_int64(cur_max_message_id, 0, "get_sync 当前最大 message_id");
DEFINE_string(search_key, "", "好");

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
    std::string chat_session_id = FLAGS_chat_session_id;
    if (chat_session_id.empty()) {
        std::cout << "部分接口需 -chat_session_id=xxx" << std::endl;
    }

    // 1. 获取历史消息（GET）
    // if (!chat_session_id.empty()) {
    //     MY_IM::GetHistoryMsgReq req;
    //     MY_IM::GetHistoryMsgRsp rsp;
    //     req.set_request_id(TestUuid());
    //     req.set_session_id(session_id);
    //     req.set_chat_session_id(chat_session_id);
    //     req.set_start_time(FLAGS_start_time);
    //     req.set_over_time(FLAGS_over_time > 0 ? FLAGS_over_time : 1LL << 62);
    //     GetProto(cli, "/chat/message_storage/get_history", req, rsp);
    //     PrintResult("get_history", rsp.success(), rsp.errmsg());
    //     if (rsp.success()) std::cout << "  msg_count=" << rsp.msg_list_size() << std::endl;
    // }

    // 2. 获取最近 N 条消息（POST，避免 GET+body 被代理截断导致 404）
    if (!chat_session_id.empty()) {
        MY_IM::GetRecentMsgReq req;
        MY_IM::GetRecentMsgRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_chat_session_id(chat_session_id);
        req.set_msg_count(FLAGS_msg_count);
        PostProto(cli, "/chat/message_storage/get_recent", req, rsp);
        PrintResult("get_recent", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  msg_count=" << rsp.msg_list_size() << std::endl;
        for (int i = 0; i < rsp.msg_list_size(); i++) {
            std::cout << "  msg_id=" << rsp.msg_list(i).message_id() << std::endl;
            std::cout << "  msg_type=" << rsp.msg_list(i).message().message_type() << std::endl;
            std::cout << "  msg_content=" << rsp.msg_list(i).message().string_message().content() << std::endl;
            std::cout << "  msg_timestamp=" << rsp.msg_list(i).timestamp() << std::endl;
            std::cout << "  msg_sender=" << rsp.msg_list(i).sender().user_id() << std::endl;
        }
    }

    // 3. 搜索历史消息
    // if (!chat_session_id.empty() && !FLAGS_search_key.empty()) {
    //     MY_IM::MsgSearchReq req;
    //     MY_IM::MsgSearchRsp rsp;
    //     req.set_request_id(TestUuid());
    //     req.set_session_id(session_id);
    //     req.set_chat_session_id(chat_session_id);
    //     req.set_search_key(FLAGS_search_key);
    //     PostProto(cli, "/chat/message_storage/search_history", req, rsp);
    //     PrintResult("search_history", rsp.success(), rsp.errmsg());
    //     if (rsp.success()) std::cout << "  msg_count=" << rsp.msg_list_size() << std::endl;
    // }

    //     // 4. 同步消息（POST，按 message_id 拉增量，避免 GET+body 导致 404）
    {
        MY_IM::GetSyncMsgReq req;
        MY_IM::GetSyncMsgRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_cur_max_message_id(FLAGS_cur_max_message_id);
        req.set_msg_count(100);
        PostProto(cli, "/chat/message_storage/get_sync", req, rsp);
        PrintResult("get_sync", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  msg_count=" << rsp.msg_list_size() << std::endl;
        for (int i = 0; i < rsp.msg_list_size(); i++) {
            std::cout << "  msg_id=" << rsp.msg_list(i).message_id() << std::endl;
            std::cout << "  msg_type=" << rsp.msg_list(i).message().message_type() << std::endl;
            std::cout << "  msg_content=" << rsp.msg_list(i).message().string_message().content() << std::endl;
            std::cout << "  msg_timestamp=" << rsp.msg_list(i).timestamp() << std::endl;
            std::cout << "  msg_sender=" << rsp.msg_list(i).sender().user_id() << std::endl;
        }
    }

    return 0;
}
