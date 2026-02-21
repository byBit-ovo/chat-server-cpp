// 网关用户相关 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "user.pb.h"
#include "base.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(nickname, "test_user", "登录昵称");
DEFINE_string(password, "123456", "密码");
DEFINE_string(session_id, "", "已有 session_id（可选，不填则先登录获取）");
DEFINE_string(device_type, "pc", "设备类型");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    httplib::Client cli(FLAGS_gateway_host, FLAGS_gateway_port);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

    std::string session_id = FLAGS_session_id;

    // 1. 用户名注册
    {
        MY_IM::UserRegisterReq req;
        MY_IM::UserRegisterRsp rsp;
        req.set_request_id(TestUuid());
        req.set_nickname(FLAGS_nickname);
        req.set_password(FLAGS_password);
        PostProto(cli, "/chat/user/username_register", req, rsp);
        PrintResult("username_register", rsp.success(), rsp.errmsg());
    }

    // 2. 用户名登录
    {
        MY_IM::UserLoginReq req;
        MY_IM::UserLoginRsp rsp;
        req.set_request_id(TestUuid());
        req.set_nickname(FLAGS_nickname);
        req.set_password(FLAGS_password);
        req.set_device_type(FLAGS_device_type);
        PostProto(cli, "/chat/user/username_login", req, rsp);
        PrintResult("username_login", rsp.success(), rsp.errmsg());
        if (session_id.empty() && rsp.success()) {
            session_id = rsp.login_session_id();
            std::cout << "[username_login] session_id=" << session_id << std::endl;
        }
    }

    if (session_id.empty()) {
        std::cout << "无 session_id，跳过需鉴权接口" << std::endl;
        return 0;
    }

    // 3. 获取当前用户信息（GET）
    {
        MY_IM::GetUserInfoReq req;
        MY_IM::GetUserInfoRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        GetProto(cli, "/chat/user/get_user_info", req, rsp);
        PrintResult("get_user_info", rsp.success(), rsp.errmsg());
        if (rsp.success() && rsp.has_user_info()) {
            std::cout << "  user_id=" << rsp.user_info().user_id()
                      << " nickname=" << rsp.user_info().nickname() << std::endl;
        }
    }

    // 4. 修改昵称
    {
        MY_IM::SetUserNicknameReq req;
        MY_IM::SetUserNicknameRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_nickname(FLAGS_nickname + "_updated");
        PostProto(cli, "/chat/user/set_nickname", req, rsp);
        PrintResult("set_nickname", rsp.success(), rsp.errmsg());
    }

    // 5. 修改签名
    {
        MY_IM::SetUserDescriptionReq req;
        MY_IM::SetUserDescriptionRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_description("hello gateway test");
        PostProto(cli, "/chat/user/set_description", req, rsp);
        PrintResult("set_description", rsp.success(), rsp.errmsg());
    }

    // 6. 修改头像（空字节表示占位，实际可传图片二进制）
    {
        MY_IM::SetUserAvatarReq req;
        MY_IM::SetUserAvatarRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_avatar("");
        PostProto(cli, "/chat/user/set_avatar", req, rsp);
        PrintResult("set_avatar", rsp.success(), rsp.errmsg());
    }

    // 7. 修改手机号（需验证码时可留空）
    {
        MY_IM::SetUserPhoneNumberReq req;
        MY_IM::SetUserPhoneNumberRsp rsp;
        req.set_request_id(TestUuid());
        req.set_session_id(session_id);
        req.set_phone_number("");
        PostProto(cli, "/chat/user/set_phone", req, rsp);
        PrintResult("set_phone", rsp.success(), rsp.errmsg());
    }

    return 0;
}
