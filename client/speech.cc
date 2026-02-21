// 网关语音识别 HTTP 接口调用测试
#include "gateway_test_common.hpp"
#include "speech.pb.h"
#include <gflags/gflags.h>

DEFINE_string(gateway_host, "127.0.0.1", "网关地址");
DEFINE_int32(gateway_port, 9000, "网关HTTP端口");
DEFINE_string(session_id, "", "登录 session_id（可选）");
DEFINE_string(speech_content, "hello speech", "语音数据（此处用字符串模拟，实际可为二进制）");

int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    httplib::Client cli(FLAGS_gateway_host, FLAGS_gateway_port);
    cli.set_read_timeout(10, 0);
    cli.set_write_timeout(10, 0);

    MY_IM::SpeechRecognitionReq req;
    MY_IM::SpeechRecognitionRsp rsp;
    req.set_request_id(TestUuid());
    if (!FLAGS_session_id.empty())
        req.set_session_id(FLAGS_session_id);
    req.set_speech_content(FLAGS_speech_content);

    PostProto(cli, "/chat/speech/recognition", req, rsp);
    PrintResult("speech_recognition", rsp.success(), rsp.errmsg());
    if (rsp.success() && rsp.has_recognition_result())
        std::cout << "  result=" << rsp.recognition_result() << std::endl;

    return 0;
}
