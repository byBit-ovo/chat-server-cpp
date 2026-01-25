<!DOCTYPE html>
<html lang="en">
 <head>
 <meta charset="UTF-8">
 <meta http-equiv="X-UA-Compatible" content="IE=edge">
 <meta name="viewport" content="width=device-width, 
initial-scale=1.0">
 <title>Test Websocket</title>
 </head>
 <body>
 <input type="text" id="message">
 <button id="submit">提交</button>
 <script>
 let websocket = new WebSocket("ws://81.71.17.201:9090");
 
 websocket.onopen = function() {
 console.log("连接建立");
 } 
 websocket.onmessage = function(e) {
 console.log("收到消息: " + e.data);
 } 
 websocket.onerror = function() {
 console.log("连接异常");
 } 
 websocket.onclose = function() {
 console.log("连接关闭");
 } 
 let input = document.querySelector('#message');
 let button = document.querySelector('#submit');
 button.onclick = function() {
	console.log("发送消息: " + input.value);
 websocket.send(input.value);
 } 
 </script>
</body>
</html>