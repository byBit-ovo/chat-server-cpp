#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

//    0. 定义server类型
typedef websocketpp::server<websocketpp::config::asio> server_t;

void onOpen(websocketpp::connection_hdl hdl) {
    std::cout << "websocket长连接建立成功！\n";
}
void onClose(websocketpp::connection_hdl hdl) {
    std::cout << "websocket长连接断开！\n";
    
}
void onMessage(server_t *server, websocketpp::connection_hdl hdl, server_t::message_ptr msg) {
    //1. 获取有效消息载荷数据，进行业务处理
    std::string body = msg->get_payload();
    std::cout << "收到消息：" << body << std::endl;
    //2. 对客户端进行响应
    //获取通信连接
    auto conn = server->get_con_from_hdl(hdl);
    //发送数据
    conn->send(body + "-Hello!", websocketpp::frame::opcode::value::text);
}
 
int main()
{
    //    1. 实例化服务器对象
    server_t server;
    //    2. 初始化日志输出 --- 关闭日志输出
    server.set_access_channels(websocketpp::log::alevel::none);
    //    3. 初始化asio框架
    server.init_asio();
    //    4. 设置消息处理/连接握手成功/连接关闭回调函数
    server.set_open_handler(onOpen);
    server.set_close_handler(onClose);
    auto msg_hadler = std::bind(onMessage, &server, std::placeholders::_1, std::placeholders::_2);
    server.set_message_handler(msg_hadler);
    //    5. 启用地址重用
    server.set_reuse_addr(true);
    //    5. 设置监听端口
    server.listen(9090);
    //    6. 开始监听
    server.start_accept();
    //    7. 启动服务器
    server.run();
    return 0;
}


// <!DOCTYPE html>
// <html lang="en">
//  <head>
//  <meta charset="UTF-8">
//  <meta http-equiv="X-UA-Compatible" content="IE=edge">
//  <meta name="viewport" content="width=device-width, 
// initial-scale=1.0">
//  <title>Test Websocket</title>
//  </head>
//  <body>
//  <input type="text" id="message">
//  <button id="submit">提交</button>
//  <script>
//  let websocket = new WebSocket("ws://81.71.17.201:9090");
 
//  websocket.onopen = function() {
//  console.log("连接建立");
//  } 
//  websocket.onmessage = function(e) {
//  console.log("收到消息: " + e.data);
//  } 
//  websocket.onerror = function() {
//  console.log("连接异常");
//  } 
//  websocket.onclose = function() {
//  console.log("连接关闭");
//  } 
//  let input = document.querySelector('#message');
//  let button = document.querySelector('#submit');
//  button.onclick = function() {
// 	console.log("发送消息: " + input.value);
//  websocket.send(input.value);
//  } 
//  </script>
// </body>
// </html>
