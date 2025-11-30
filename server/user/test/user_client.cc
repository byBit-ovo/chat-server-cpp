#include "etcd.hpp"
#include "channel.hpp"
#include "utils.hpp"
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <thread>
#include "user.pb.h"
#include "base.pb.h"

DEFINE_bool(run_mode, false, "程序的运行模式，false-调试； true-发布；");
DEFINE_string(log_file, "", "发布模式下，用于指定日志的输出文件");
DEFINE_int32(log_level, 0, "发布模式下，用于指定日志输出等级");

DEFINE_string(etcd_host, "http://127.0.0.1:2379", "服务注册中心地址");
DEFINE_string(base_service, "/service", "服务监控根目录");
DEFINE_string(user_service, "/service/user_service", "服务监控根目录");

MY_IM::ServiceManager::ptr _user_channels;

MY_IM::UserInfo user_info;

std::string login_ssid;
std::string new_nickname = "chen浩南";

// TEST(用户子服务测试, 用户注册测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);
//     user_info.set_nickname("猪妈妈");

//     MY_IM::UserRegisterReq req;
//     MY_IM::UserRegisterRsp rsp;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_nickname(user_info.nickname());
//     req.set_password("1122334455");

//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.UserRegister(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }
// TEST(用户子服务测试, 用户登录测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::UserLoginReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_nickname("陈浩南");
//     req.set_password("24681012");
//     MY_IM::UserLoginRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.UserLogin(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
//     login_ssid = rsp.login_session_id();
// }
// TEST(用户子服务测试, 用户头像设置测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::SetUserAvatarReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_user_id("1ea5-cad3-c849-0005");
//     req.set_session_id(login_ssid);
//     req.set_avatar(user_info.avatar());
//     MY_IM::SetUserAvatarRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.SetUserAvatar(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }
// TEST(用户子服务测试, 用户签名设置测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::SetUserDescriptionReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(login_ssid);
//     req.set_description(user_info.description());
//     MY_IM::SetUserDescriptionRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.SetUserDescription(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }
// TEST(用户子服务测试, 用户昵称设置测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::SetUserNicknameReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(login_ssid);
//     req.set_nickname(new_nickname);
//     MY_IM::SetUserNicknameRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.SetUserNickname(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }


// TEST(用户子服务测试, 用户信息获取测试) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::GetUserInfoReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_user_id(user_info.user_id());
//     req.set_session_id(login_ssid);
//     MY_IM::GetUserInfoRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.GetUserInfo(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
//     ASSERT_EQ(user_info.user_id(), rsp.user_info().user_id());
//     ASSERT_EQ(new_nickname, rsp.user_info().nickname());
//     ASSERT_EQ(user_info.description(), rsp.user_info().description());
//     ASSERT_EQ("", rsp.user_info().phone());
//     ASSERT_EQ(user_info.avatar(), rsp.user_info().avatar());
// }

// void set_user_avatar(const std::string &uid, const std::string &avatar) {
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);
//     MY_IM::SetUserAvatarReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.set_user_id(uid);
//     req.set_session_id(login_ssid);
//     req.set_avatar(avatar);
//     MY_IM::SetUserAvatarRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.SetUserAvatar(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }

// TEST(用户子服务测试, 批量用户信息获取测试) {
//     set_user_avatar("1ea5-cad3-c849-0005", "chen浩南的头像数据");
//     set_user_avatar("9e35-44dd-9f00-0000", "小猪佩奇的头像数据");
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::GetMultiUserInfoReq req;
//     req.set_request_id(MY_IM::Uuid());
//     req.add_users_id("1ea5-cad3-c849-0005");
//     req.add_users_id("9e35-44dd-9f00-0000");
//     req.add_users_id("0118-3f18-a576-0001");
//     MY_IM::GetMultiUserInfoRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.GetMultiUserInfo(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
//     auto users_map = rsp.mutable_users_info();
//     MY_IM::UserInfo fuser = (*users_map)["1ea5-cad3-c849-0005"];
//     ASSERT_EQ(fuser.user_id(), "1ea5-cad3-c849-0005");
//     ASSERT_EQ(fuser.nickname(), "chen浩南");
//     ASSERT_EQ(fuser.description(), "这是铜锣湾扛把子");
//     ASSERT_EQ(fuser.phone(), "");	
//     ASSERT_EQ(fuser.avatar(), "chen浩南的头像数据");

//     MY_IM::UserInfo puser = (*users_map)["9e35-44dd-9f00-0000"];
//     ASSERT_EQ(puser.user_id(), "9e35-44dd-9f00-0000");
//     ASSERT_EQ(puser.nickname(), "佩奇");
//     ASSERT_EQ(puser.description(), "");
//     ASSERT_EQ(puser.phone(), "");
//     ASSERT_EQ(puser.avatar(), "小猪佩奇的头像数据");
    
//     MY_IM::UserInfo quser = (*users_map)["0118-3f18-a576-0001"];
//     ASSERT_EQ(quser.user_id(), "0118-3f18-a576-0001");
//     ASSERT_EQ(quser.nickname(), "猪妈妈");
//     ASSERT_EQ(quser.description(), "");
//     ASSERT_EQ(quser.phone(), "");
//     ASSERT_EQ(quser.avatar(), "");
// }

std::string code_id;
void get_code() {
    auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
    ASSERT_TRUE(channel);

    MY_IM::PhoneVerifyCodeReq req;
    req.set_request_id(MY_IM::Uuid());
    req.set_phone_number(user_info.phone());
    MY_IM::PhoneVerifyCodeRsp rsp;
    brpc::Controller cntl;
    MY_IM::UserService_Stub stub(channel.get());
    stub.GetPhoneVerifyCode(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
    code_id = rsp.verify_code_id();
}


// TEST(用户子服务测试, 手机号注册) {
//     get_code();
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::PhoneRegisterReq req;
//     req.set_request_id(MY_IM::uuid());
//     req.set_phone_number(user_info.phone());
//     req.set_verify_code_id(code_id);
//     std::cout << "手机号注册，输入验证码：" << std::endl;
//     std::string code;
//     std::cin >> code;
//     req.set_verify_code(code);
//     MY_IM::PhoneRegisterRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.PhoneRegister(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
// }
// TEST(用户子服务测试, 手机号登录) {
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     get_code();
//     auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
//     ASSERT_TRUE(channel);

//     MY_IM::PhoneLoginReq req;
//     req.set_request_id(MY_IM::uuid());
//     req.set_phone_number(user_info.phone());
//     req.set_verify_code_id(code_id);
//     std::cout << "手机号登录，输入验证码：" << std::endl;
//     std::string code;
//     std::cin >> code;
//     req.set_verify_code(code);
//     MY_IM::PhoneLoginRsp rsp;
//     brpc::Controller cntl;
//     MY_IM::UserService_Stub stub(channel.get());
//     stub.PhoneLogin(&cntl, &req, &rsp, nullptr);
//     ASSERT_FALSE(cntl.Failed());
//     ASSERT_TRUE(rsp.success());
//     std::cout << "手机登录会话ID：" << rsp.login_session_id() << std::endl;
// }
TEST(用户子服务测试, 手机号设置) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    get_code();
    auto channel = _user_channels->GetChannel(FLAGS_user_service);//获取通信信道
    ASSERT_TRUE(channel);

    MY_IM::SetUserPhoneNumberReq req;
    req.set_request_id(MY_IM::Uuid());
    std::cout << "手机号设置时，输入用户ID：" << std::endl;
    std::string user_id;
    std::cin >> user_id;
    req.set_user_id(user_id);
    req.set_phone_number("18888888888");
    req.set_phone_verify_code_id(code_id);
    std::cout << "手机号设置时，输入验证码：" << std::endl;
    std::string code;
    std::cin >> code;
    req.set_phone_verify_code(code);
    MY_IM::SetUserPhoneNumberRsp rsp;
    brpc::Controller cntl;
    MY_IM::UserService_Stub stub(channel.get());
    stub.SetUserPhoneNumber(&cntl, &req, &rsp, nullptr);
    ASSERT_FALSE(cntl.Failed());
    ASSERT_TRUE(rsp.success());
}

int main(int argc, char *argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger(FLAGS_run_mode, FLAGS_log_file, FLAGS_log_level);

    //1. 先构造Rpc信道管理对象
    _user_channels = std::make_shared<MY_IM::ServiceManager>();
    _user_channels->FollowOn(FLAGS_user_service);
    auto put_cb = std::bind(&MY_IM::ServiceManager::OnlineCall, _user_channels.get(), 
	std::placeholders::_1, std::placeholders::_2);
    auto del_cb = std::bind(&MY_IM::ServiceManager::OfflineCall, _user_channels.get(), 
	std::placeholders::_1, std::placeholders::_2);
    
    //2. 构造服务发现对象
    MY_IM::Discoverer::ptr dclient = std::make_shared<MY_IM::Discoverer>(FLAGS_etcd_host, FLAGS_base_service, put_cb, del_cb);
    
    user_info.set_nickname("猪妈妈");
    user_info.set_user_id("1ea5-cad3-c849-0005");
    user_info.set_description("这是铜锣湾扛把子");
    user_info.set_phone("15009356594");
    user_info.set_avatar("扛把子头像新数据");
	login_ssid = "218e-55f3-8e23-0006";
    testing::InitGoogleTest(&argc, argv);
    LOG_DEBUG("开始测试！");
    return RUN_ALL_TESTS();
}