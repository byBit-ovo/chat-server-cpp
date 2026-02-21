// 网关好友/会话相关 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "user.pb.h"
#include "friend.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(session_id, "", "登录 session_id（必填）");
DEFINE_string(respondent_id, "", "被申请人 user_id（添加好友时用）");
DEFINE_string(notify_event_id, "", "好友申请事件 id（处理申请时用）");
DEFINE_string(apply_user_id, "", "申请人 user_id（处理申请时用）");
DEFINE_string(peer_id, "", "要删除的好友 user_id");
DEFINE_string(search_key, "test", "搜索关键字");
DEFINE_string(chat_session_id, "", "会话 id（获取成员/已有会话时用）");
DEFINE_string(chat_session_name, "测试会话", "创建会话名称");
DEFINE_string(member_ids, "", "成员 user_id 列表，逗号分隔");
DEFINE_string(add_member_ids, "", "待添加成员 id，逗号分隔（add_chat_session_member）");
DEFINE_bool(agree, true, "是否同意好友申请");

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

    // 1. 获取好友列表（GET）
    {
        MY_IM::GetFriendListReq req;
        MY_IM::GetFriendListRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        GetProto(cli, "/chat/friend/get_friend_list", req, rsp);
        PrintResult("get_friend_list", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  friend_count=" << rsp.friend_list_size() << std::endl;
    }

    // 2. 发送好友申请
    if (!FLAGS_respondent_id.empty()) {
        MY_IM::FriendAddReq req;
        MY_IM::FriendAddRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_respondent_id(FLAGS_respondent_id);
        PostProto(cli, "/chat/friend/add_friend_apply", req, rsp);
        PrintResult("add_friend_apply", rsp.success(), rsp.errmsg());
    }

    // 3. 处理好友申请
    if (!FLAGS_notify_event_id.empty() && !FLAGS_apply_user_id.empty()) {
        MY_IM::FriendAddProcessReq req;
        MY_IM::FriendAddProcessRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_notify_event_id(FLAGS_notify_event_id);
        req.set_apply_user_id(FLAGS_apply_user_id);
        req.set_agree(FLAGS_agree);
        PostProto(cli, "/chat/friend/add_friend_process", req, rsp);
        PrintResult("add_friend_process", rsp.success(), rsp.errmsg());
    }

    // 4. 删除好友
    if (!FLAGS_peer_id.empty()) {
        MY_IM::FriendRemoveReq req;
        MY_IM::FriendRemoveRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_peer_id(FLAGS_peer_id);
        PostProto(cli, "/chat/friend/remove_friend", req, rsp);
        PrintResult("remove_friend", rsp.success(), rsp.errmsg());
    }

    // 5. 搜索好友
    {
        MY_IM::FriendSearchReq req;
        MY_IM::FriendSearchRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_search_key(FLAGS_search_key);
        PostProto(cli, "/chat/friend/search_friend", req, rsp);
        PrintResult("search_friend", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  user_count=" << rsp.user_info_size() << std::endl;
    }

    // 6. 获取待处理好友申请列表（GET）
    {
        MY_IM::GetPendingFriendEventListReq req;
        MY_IM::GetPendingFriendEventListRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        GetProto(cli, "/chat/friend/get_pending_friend_events", req, rsp);
        PrintResult("get_pending_friend_events", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  event_count=" << rsp.event_size() << std::endl;
    }

    // 7. 获取会话列表（GET）
    {
        MY_IM::GetChatSessionListReq req;
        MY_IM::GetChatSessionListRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        GetProto(cli, "/chat/friend/get_chat_session_list", req, rsp);
        PrintResult("get_chat_session_list", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  session_count=" << rsp.chat_session_info_list_size() << std::endl;
    }

    // 8. 创建会话
    {
        MY_IM::ChatSessionCreateReq req;
        MY_IM::ChatSessionCreateRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_chat_session_name(FLAGS_chat_session_name);
        for (const auto& id : split(FLAGS_member_ids))
            req.add_member_id_list(id);
        PostProto(cli, "/chat/friend/create_chat_session", req, rsp);
        PrintResult("create_chat_session", rsp.success(), rsp.errmsg());
    }

    // 9. 添加会话成员
    if (!FLAGS_chat_session_id.empty() && !FLAGS_add_member_ids.empty()) {
        MY_IM::AddChatSessionMemberReq req;
        MY_IM::AddChatSessionMemberRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_chat_session_id(FLAGS_chat_session_id);
        for (const auto& id : split(FLAGS_add_member_ids))
            req.add_member_id_list(id);
        PostProto(cli, "/chat/friend/add_chat_session_member", req, rsp);
        PrintResult("add_chat_session_member", rsp.success(), rsp.errmsg());
    }

    // 10. 获取会话成员列表（GET）
    if (!FLAGS_chat_session_id.empty()) {
        MY_IM::GetChatSessionMemberReq req;
        MY_IM::GetChatSessionMemberRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_chat_session_id(FLAGS_chat_session_id);
        GetProto(cli, "/chat/friend/get_chat_session_member", req, rsp);
        PrintResult("get_chat_session_member", rsp.success(), rsp.errmsg());
        if (rsp.success()) std::cout << "  member_count=" << rsp.member_info_list_size() << std::endl;
    }

    return 0;
}
