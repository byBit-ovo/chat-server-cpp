# ChatClient 状态/消息完整流转图

---

## 1. 总架构：三层数据流

```
┌──────────────────────────────────────────────────────────────────────────┐
│                              UI 层                                       │
│                                                                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │LoginWidget│  │MainWidget│  │SelfInfo  │  │AddFriend │  │History   │  │
│  │          │  │          │  │Widget    │  │Dialog    │  │Message   │  │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  │
│       │             │             │             │             │         │
│       │    调用 xxxAsync()        │             │             │         │
│       └──────────────┼────────────┴─────────────┴─────────────┘         │
│                      │                                                   │
│             监听 xxxDone / xxxChanged 信号                                │
└──────────────────────┼───────────────────────────────────────────────────┘
                       │
          ┌────────────▼────────────┐
          │    DataCenter (单例)     │
          │                        │
          │  ┌──────────────────┐  │
          │  │   内存状态:       │  │
          │  │  · myself        │  │
          │  │  · friendList    │  │
          │  │  · chatSessionList│  │
          │  │  · recentMessages │  │
          │  │  · applyList     │  │
          │  │  · memberList    │  │
          │  │  · unreadCount   │  │
          │  │  · loginSessionId│  │
          │  └──────────────────┘  │
          │                        │
          │  ┌──────────────────┐  │
          │  │   持久化状态:     │  │
          │  │  ChatClient.json │  │
          │  │  · loginSessionId│  │
          │  │  · unreadCount   │  │
          │  └──────────────────┘  │
          │                        │
          │  持有 NetClient ────────┼───▶
          └────────────────────────┘
                                    │
                       ┌────────────▼────────────┐
                       │     NetClient            │
                       │                          │
                       │  HTTP POST               │
                       │  127.0.0.1:8000          │──────▶ 后端
                       │  (24 个 API)              │
                       │                          │
                       │  WebSocket               │
                       │  127.0.0.1:8001          │◀────── 后端
                       │  (8 种推送)               │
                       └──────────────────────────┘
```

---

## 2. 登录 → 主窗口 完整流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                         启动阶段                                      │
│                                                                      │
│  main.cpp                                                            │
│    │                                                                 │
│    ├── TEST_SKIP_LOGIN=0 ──────────────────────────────┐            │
│    │                                                    │            │
│    ▼                                                    ▼            │
│  LoginWidget                                    MainWidget           │
│  (显示登录界面)                                  (跳过登录)            │
│    │                                                    │            │
│    │  用户输入用户名/密码                                  │            │
│    │  点击 [登录]                                        │            │
│    ▼                                                    │            │
│  DataCenter::userLoginAsync(name, pwd)                  │            │
│    │                                                    │            │
│    ▼                                                    │            │
│  NetClient::userLogin()                                 │            │
│    │  POST /service/user/username_login                 │            │
│    │  body: { nickname, password }                      │            │
│    ▼                                                    │            │
│  ┌─────────────┐                                        │            │
│  │  后端服务器   │                                        │            │
│  │  返回:       │                                        │            │
│  │  success=true│                                        │            │
│  │  loginSession│                                        │            │
│  │  Id="test.." │                                        │            │
│  └──────┬──────┘                                        │            │
│         │                                               │            │
│         ▼                                               │            │
│  handleHttpResponse<UserLoginRsp>()                     │            │
│    │                                                    │            │
│    ▼                                                    │            │
│  DataCenter::resetLoginSessionId("testLoginSessionId")  │            │
│    │  · 更新内存 loginSessionId                          │            │
│    │  · saveDataFile() → 持久化到 ChatClient.json        │            │
│    │                                                    │            │
│    ▼                                                    │            │
│  emit dataCenter->userLoginDone(true, "")               │            │
│    │                                                    │            │
│    ▼                                                    │            │
│  LoginWidget::userLoginDone(true, "")                   │            │
│    │  · 创建 MainWidget::getInstance()                   │            │
│    │  · close() LoginWidget                             │            │
│    │  · show() MainWidget                               │            │
│    ▼                                                    │            │
│  ┌──────────────────────────────────────┐               │            │
│  │        MainWidget 初始化              │◀──────────────┘            │
│  │                                      │                            │
│  │  构造函数自动执行:                     │                            │
│  │  ① initMainWindow()   布局+样式       │                            │
│  │  ② initLeftWindow()   左侧栏          │                            │
│  │  ③ initMidWindow()    中间栏          │                            │
│  │  ④ initRightWindow()  右侧栏          │                            │
│  │  ⑤ initSignalSlot()   绑定全部信号槽   │                            │
│  │  ⑥ initWebsocket()   启动WS连接       │                            │
│  └──────────────────────────────────────┘                            │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. 主窗口初始数据加载

```
MainWidget 显示后
  │
  ├── initWebsocket()
  │     │  NetClient::initWebsocket()
  │     │  连接到 ws://127.0.0.1:8001/ws
  │     │  连接成功 → sendAuth() → 发送 loginSessionId
  │     │  此后可以接收实时推送
  │     │
  │     └── WebSocket 就绪，等待推送事件
  │
  ├── getMyselfAsync()  ← 构造函数中直接调用
  │     │  POST /service/user/get_user_info
  │     │  响应 → resetMyself() → 写入内存
  │     │  响应 → emit getMyselfDone()
  │     │         └── MainWidget lambda: userAvatar->setIcon(myself->avatar)
  │     │
  │     └── 状态: myself ✅ 已加载
  │
  ├── loadSessionList()
  │     │  已有缓存? → 直接 updateChatSessionList() 渲染
  │     │  无缓存?   → getChatSessionListAsync()
  │     │              POST /service/friend/get_chat_session_list
  │     │              响应 → resetChatSessionList()
  │     │              响应 → emit getChatSessionListDone()
  │     │                     └── updateChatSessionList()
  │     │                         SessionFriendArea 渲染 N 个 SessionItem
  │     │
  │     └── 状态: chatSessionList ✅ 已加载 (30个单聊 + 1个群聊)
  │
  ├── loadFriendList()
  │     │  已有缓存? → 直接 updateFriendList() 渲染
  │     │  无缓存?   → getFriendListAsync()
  │     │              POST /service/friend/get_friend_list
  │     │              响应 → resetFriendList()
  │     │              响应 → emit getFriendListDone()
  │     │                     └── updateFriendList()
  │     │                         SessionFriendArea 渲染 N 个 FriendItem
  │     │
  │     └── 状态: friendList ✅ 已加载 (20个好友)
  │
  └── loadApplyList()
        │  已有缓存? → 直接 updateApplyList() 渲染
        │  无缓存?   → getApplyListAsync()
        │              POST /service/friend/get_pending_friend_events
        │              响应 → resetApplyList()
        │              响应 → emit getApplyListDone()
        │                     └── updateApplyList()
        │                         SessionFriendArea 渲染 N 个 ApplyItem
        │
        └── 状态: applyList ✅ 已加载 (5个待处理申请)
```

---

## 4. 消息发送完整流程

```
┌──────────────────────────────────────────────────────────────────┐
│                     文本消息发送                                   │
│                                                                   │
│  用户在 MessageEditArea 输入 "Hello" 按 Enter                      │
│    │                                                              │
│    ▼                                                              │
│  MessageEditArea::sendTextMessage()                               │
│    │                                                              │
│    ▼                                                              │
│  DataCenter::sendTextMessageAsync(chatSessionId, "Hello")         │
│    │                                                              │
│    ▼                                                              │
│  NetClient::sendMessage(                                          │
│      loginSessionId = "testLoginSessionId",                       │
│      chatSessionId  = "2000",                                     │
│      messageType    = TEXT_TYPE,                                  │
│      content        = "Hello".toUtf8(),                           │
│      extraInfo      = ""                                          │
│  )                                                                │
│    │                                                              │
│    │  ① 构造 NewMessageReq {                                      │
│    │       requestId    = "Rxxxxxxxxxxxx",                        │
│    │       sessionId    = "testLoginSessionId",                   │
│    │       chatSessionId= "2000",                                  │
│    │       message {                                              │
│    │         messageType = STRING,                                │
│    │         stringMessage { content = "Hello" }                  │
│    │       }                                                      │
│    │     }                                                        │
│    │  ② 序列化 → QByteArray (Protobuf binary)                     │
│    │  ③ sendHttpRequest("/service/message_transmit/new_message")  │
│    │  ④ httpClient.post("http://127.0.0.1:8000/...", body)       │
│    ▼                                                              │
│  ═══════════ 网络传输 ═══════════                                 │
│    │                                                              │
│    ▼                                                              │
│  后端处理 → 存储消息 → 返回响应                                     │
│    │                                                              │
│    ▼                                                              │
│  QNetworkReply::finished 信号触发                                  │
│    │                                                              │
│    ▼                                                              │
│  handleHttpResponse<NewMessageRsp>(resp, &ok, &reason)            │
│    │  ⑤ 检查 HTTP 状态码                                          │
│    │  ⑥ 反序列化响应 body → NewMessageRsp                         │
│    │  ⑦ 检查 success 字段                                         │
│    │  ⑧ 返回 shared_ptr<NewMessageRsp>                            │
│    │                                                              │
│    ▼                                                              │
│  emit dataCenter->sendMessageDone(TEXT_TYPE, "Hello", "")         │
│    │                                                              │
│    ├────────────────────────────────────────────┐                 │
│    ▼                                            ▼                 │
│  MessageEditArea::addSelfMessage()     emit updateLastMessage(    │
│    │  ① 构造 Message 对象                       "2000")           │
│    │  ② messageShowArea->addMessage(msg)        │                 │
│    │  ③ 清空输入框                              ▼                 │
│    │  ④ scrollToEnd()              SessionFriendArea             │
│    │                               SessionItem::updateLastMessage│
│    ▼                                  · 更新会话列表中的消息预览   │
│  UI 显示: 右侧蓝色气泡 "Hello"                                     │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  此时 DataCenter 状态变化:                                    │ │
│  │  recentMessages["2000"] 末尾新增一条 Message                  │ │
│  │  chatSessionList 中会话 "2000" 的 lastMessage 已更新          │ │
│  └─────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

---

## 5. 消息接收（WebSocket 推送）完整流程

```
┌──────────────────────────────────────────────────────────────────┐
│              WebSocket 实时推送：收到新消息                         │
│                                                                   │
│  后端推送 NotifyMessage (Protobuf 二进制)                          │
│    │                                                              │
│    ▼                                                              │
│  websocketClient 收到 binaryMessageReceived 信号                   │
│    │                                                              │
│    ▼                                                              │
│  ① 反序列化 → bite_im::NotifyMessage {                            │
│       notifyType = CHAT_MESSAGE_NOTIFY,                           │
│       newMessageInfo {                                            │
│         messageInfo {                                             │
│           messageId = "3000",                                     │
│           chatSessionId = "2000",                                 │
│           sender { userId="1000", nickname="张三0", ... },        │
│           message {                                               │
│             messageType = STRING,                                 │
│             stringMessage { content = "这是一条消息内容0" }        │
│           }                                                       │
│         }                                                         │
│       }                                                           │
│     }                                                             │
│    │                                                              │
│    ▼                                                              │
│  ② handleWsResponse(notifyMessage)                                │
│    │  notifyType == CHAT_MESSAGE_NOTIFY                           │
│    │  ③ Message::load(messageInfo)  → 转为客户端 Message 对象      │
│    │  ④ handleWsMessage(message)                                  │
│    ▼                                                              │
│  ┌── 判断: recentMessages 中是否已有 chatSessionId="2000" 的消息?  │
│  │                                                                 │
│  ├── 有 (会话已加载) ──────────────────────┐                      │
│  │                                         │                      │
│  │  ⑤ recentMessages["2000"].push_back(msg)│                      │
│  │  ⑥ receiveMessage("2000")              │                      │
│  │     │                                   │                      │
│  │     ├── chatSessionId=="2000"           │                      │
│  │     │   等于 currentChatSessionId?       │                      │
│  │     │                                   │                      │
│  │     ├── 是当前会话                       │                      │
│  │     │   ⑦ emit receiveMessageDone(msg)  │                      │
│  │     │      └── MessageEditArea          │                      │
│  │     │           addOtherMessage()       │                      │
│  │     │           → MessageShowArea       │                      │
│  │     │              渲染白色气泡 "这是一条消息" │                 │
│  │     │                                   │                      │
│  │     └── 不是当前会话                     │                      │
│  │          DataCenter::addUnread("2000")  │                      │
│  │          unreadMessageCount["2000"]++   │                      │
│  │          → 会话列表显示未读红点           │                      │
│  │                                         │                      │
│  │  ⑧ emit updateLastMessage("2000")       │                      │
│  │     └── SessionFriendArea               │                      │
│  │          SessionItem::updateLastMessage │                      │
│  │          → 更新会话列表的消息预览         │                      │
│  │                                         │                      │
│  └── 没有 (会话未加载) ────────────────────┐                      │
│                                            │                      │
│     ⑤ connect(getRecentMessageListDoneNoUI,│                      │
│               receiveMessage)              │                      │
│     ⑥ getRecentMessageListAsync("2000")    │                      │
│        → 加载完成 → receiveMessage("2000")  │                      │
│        → 回到上面的分支逻辑                  │                      │
│                                            │                      │
│  └────────────────────────────────────────┘                      │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  此时 DataCenter 状态变化:                                    │ │
│  │  recentMessages["2000"] 末尾新增一条 Message                  │ │
│  │  chatSessionList 中会话 "2000" 的 lastMessage 已更新          │ │
│  │  如果非当前会话: unreadMessageCount["2000"]++                │ │
│  └─────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

---

## 6. 好友管理：完整生命周期

```
┌──────────────────────────────────────────────────────────────────┐
│                    好友管理状态机                                  │
│                                                                   │
│                          ┌──────────┐                             │
│                          │  陌生人    │                             │
│                          └────┬─────┘                             │
│                               │                                   │
│              ① 搜索用户        │  ② 发送好友申请                  │
│              searchUserAsync  │  addFriendApplyAsync             │
│              ("张三")         │  (userId="1001")                  │
│                               │                                   │
│              ┌────────────────┼────────────────┐                 │
│              ▼                ▼                ▼                 │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │  搜索结果列表  │   │  申请已发送    │   │  对方收到通知  │         │
│  │  (30个用户)   │   │  Toast 提示   │   │  (WebSocket)  │         │
│  └──────┬───────┘   └──────────────┘   └──────┬───────┘         │
│         │                                      │                  │
│         │ 点击 "添加好友"                         │ ③ 对方同意      │
│         ▼                                      │ acceptFriend   │
│  addFriendApplyAsync                           │ ApplyAsync     │
│  POST /service/friend/add_friend_apply         ▼                  │
│         │                            ┌──────────────┐           │
│         ▼                            │  你收到通知    │           │
│  emit addFriendApplyDone             │  (WebSocket)  │           │
│    └── Toast "好友申请已发送"          │              │           │
│                                      │ ④ 你的 DataCenter:        │
│         ┌────────────────────────────│  friendList 新增该用户     │
│         │                            │  applyList 移除该申请     │
│         │                            │  emit acceptFriendApplyDone│
│         ▼                            └──────┬───────┘           │
│  ┌──────────────┐                           │                    │
│  │   已是好友     │◀──────────────────────────┘                   │
│  │              │                                                │
│  │  可以:        │   ⑤ 删除好友                                   │
│  │  · 发消息     │   deleteFriendAsync                           │
│  │  · 查看信息   │   POST /service/friend/remove_friend         │
│  │  · 创建群聊   │       │                                        │
│  └──────────────┘       ▼                                        │
│                    ┌──────────────┐                              │
│                    │  DataCenter:  │                              │
│                    │  friendList   │                              │
│                    │  移除该用户    │                              │
│                    │  chatSessionList│                            │
│                    │  移除对应会话   │                              │
│                    │              │                              │
│                    │  emit deleteFriendDone                       │
│                    │    └── 刷新好友列表                           │
│                    │    └── 刷新会话列表                           │
│                    └──────┬───────┘                              │
│                           │                                       │
│                           ▼                                       │
│                      ┌──────────┐                                │
│                      │  陌生人    │  (回到起点)                     │
│                      └──────────┘                                │
└──────────────────────────────────────────────────────────────────┘
```

---

## 7. 修改个人资料流程

```
┌──────────────────────────────────────────────────────────────────┐
│                    修改个人资料                                    │
│                                                                   │
│  SelfInfoWidget 中用户操作:                                       │
│                                                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐       │
│  │  修改头像     │    │  修改昵称     │    │  修改手机号    │       │
│  │              │    │              │    │              │       │
│  │ 选择图片文件  │    │ 输入新昵称    │    │ 输入新手机号   │       │
│  │     │        │    │     │        │    │     │        │       │
│  │     ▼        │    │     ▼        │    │     ▼        │       │
│  │changeAvatar  │    │changeNickname│    │getVerifyCode  │       │
│  │Async(bytes)  │    │Async("新名")  │    │Async(phone)   │       │
│  │     │        │    │     │        │    │     │        │       │
│  │     │        │    │     │        │    │     ▼        │       │
│  │     │        │    │     │        │    │ 收到验证码ID  │       │
│  │     │        │    │     │        │    │     │        │       │
│  │     │        │    │     │        │    │     ▼        │       │
│  │     │        │    │     │        │    │changePhone   │       │
│  │     │        │    │     │        │    │Async(phone,  │       │
│  │     │        │    │     │        │    │  codeId,code)│       │
│  │     │        │    │     │        │    │     │        │       │
│  └─────┼────────┘    └─────┼────────┘    └─────┼────────┘       │
│        │                   │                   │                  │
│        ▼                   ▼                   ▼                  │
│  ┌─────────────────────────────────────────────────────┐        │
│  │              NetClient 发送 HTTP POST                │        │
│  │  /service/user/set_avatar                           │        │
│  │  /service/user/set_nickname                         │        │
│  │  /service/user/set_phone                            │        │
│  └───────────────────────┬─────────────────────────────┘        │
│                          │                                        │
│                          ▼                                        │
│  ┌─────────────────────────────────────────────────────┐        │
│  │              后端返回 success=true                    │        │
│  └───────────────────────┬─────────────────────────────┘        │
│                          │                                        │
│        ┌─────────────────┼──────────────────┐                    │
│        ▼                 ▼                  ▼                    │
│  resetAvatar()    resetNickname()    resetPhone()               │
│  myself->avatar   myself->nickname   myself->phone              │
│  = new QIcon()    = "新名"           = "新手机号"                │
│        │                 │                  │                    │
│        ▼                 ▼                  ▼                    │
│  emit changeAvatar emit changeNickname emit changePhone         │
│       Done               Done                Done                │
│        │                 │                  │                    │
│        ├──────────┐      ├──────────┐       ├──────────┐        │
│        ▼          ▼      ▼          ▼       ▼          ▼        │
│  MainWidget  Message  SelfInfo  Message  SelfInfo              │
│  刷新头像     ShowArea 关闭编辑态 ShowArea 关闭编辑态             │
│             刷新消息中         刷新消息中                        │
│             的发送者头像       的发送者昵称                       │
└──────────────────────────────────────────────────────────────────┘
```

---

## 8. 群聊创建流程

```
ChooseFriendDialog (选择至少2个好友)
  │
  ▼
DataCenter::createGroupChatSessionAsync(selectedUserIdList)
  │
  ▼
NetClient::createGroupChatSession()
  │  POST /service/friend/create_chat_session
  │  body: { sessionId, memberIdList, chatSessionName="新的群聊" }
  ▼
后端处理...
  │
  ▼
handleHttpResponse<ChatSessionCreateRsp>()
  │
  ▼
emit dataCenter->createGroupChatSessionDone()
  └── MainWidget → Toast "创建群聊会话请求已经发送!"

  ═══════════ 同时，后端通过 WebSocket 推送 ═══════════

WebSocket 推送: notifyType = CHAT_SESSION_CREATE_NOTIFY
  │
  ▼
handleWsSessionCreate(chatSessionInfo)
  │  ① chatSessionList->push_front(chatSessionInfo)
  │  ② emit dataCenter->receiveSessionCreateDone()
  └── MainWidget → updateChatSessionList() → Toast "您被拉入到新的群聊中!"
```

---

## 9. 文件下载 & 语音转文字流程

```
┌──────────────────────────────────────────────────────────────────┐
│  场景A: 图片消息需要显示缩略图                                     │
│                                                                   │
│  MessageShowArea 收到一条 IMAGE_TYPE 消息                         │
│  消息中有 fileId="testImage"，但没有图片 content                  │
│    │                                                              │
│    ▼                                                              │
│  DataCenter::getSingleFileAsync("testImage")                      │
│    │  POST /service/file/get_single_file                         │
│    ▼                                                              │
│  后端返回: fileData { fileId="testImage", fileContent=<二进制> }   │
│    │                                                              │
│    ▼                                                              │
│  emit dataCenter->getSingleFileDone("testImage", <imageBytes>)    │
│    └── MessageImageLabel::updateUI()                             │
│         · 二进制 → QPixmap → 缩放 → 显示在气泡中                   │
│                                                                   │
├──────────────────────────────────────────────────────────────────┤
│  场景B: 语音消息转文字                                             │
│                                                                   │
│  用户在语音消息气泡上右键 → "转文字"                                │
│    │                                                              │
│    ▼                                                              │
│  DataCenter::speechConvertTextAsync(fileId, audioContent)         │
│    │  POST /service/speech/recognition                           │
│    ▼                                                              │
│  后端返回: recognitionResult = "你好你好这是一段语音消息"           │
│    │                                                              │
│    ▼                                                              │
│  emit dataCenter->speechConvertTextDone(fileId, text)             │
│    └── MessageContentLabel::speechConvertTextDone()              │
│         · 在语音气泡下方显示转换后的文字                            │
└──────────────────────────────────────────────────────────────────┘
```

---

## 10. 状态持久化：应用重启恢复

```
┌──────────────────────────────────────────────────────────────────┐
│                     持久化机制                                    │
│                                                                   │
│  持久化文件: ~/Library/Application Support/<AppName>/ChatClient.json│
│                                                                   │
│  保存时机 (saveDataFile):                                         │
│    · resetLoginSessionId() 被调用时 (每次登录成功)                 │
│    · addUnread() 被调用时 (收到非当前会话的消息)                   │
│    · clearUnread() 被调用时 (点击会话进入聊天)                     │
│                                                                   │
│  保存内容:                                                        │
│  {                                                                │
│    "loginSessionId": "testLoginSessionId",                        │
│    "unread": {                                                    │
│      "2000": 3,    ← 会话 2000 有 3 条未读                        │
│      "2005": 1     ← 会话 2005 有 1 条未读                        │
│    }                                                              │
│  }                                                                │
│                                                                   │
│  加载时机 (loadDataFile):                                         │
│    · DataCenter 构造函数中自动调用                                  │
│                                                                   │
│  应用重启后:                                                       │
│    · loginSessionId 恢复 → 可以复用之前的登录态? (取决于后端)       │
│    · unreadMessageCount 恢复 → 会话列表红点数字不变                │
│    · 其他数据 (好友列表/会话列表/消息) 不持久化，需要重新从网络加载  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 11. 完整状态生命周期总览

```
                        ┌─────────┐
                        │ 应用启动  │
                        └────┬────┘
                             │
                    ┌────────▼────────┐
                    │   未登录状态     │
                    │ loginSessionId  │
                    │    = ""         │
                    └────────┬───────┘
                             │ 登录成功
                             │ userLoginDone(true, "")
                             ▼
                    ┌────────────────┐
                    │   已登录状态     │
                    │ loginSessionId  │
                    │ = "testLogin..."│
                    │                │
                    │ myself ✅       │
                    └───────┬────────┘
                            │
              ┌─────────────┼─────────────┐
              ▼             ▼             ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │ 加载会话列表│ │ 加载好友列表│ │ 加载申请列表│
        │ (30+1个)  │ │ (20个)    │ │ (5个)     │
        └────┬─────┘ └────┬─────┘ └────┬─────┘
             │            │            │
             └────────────┼────────────┘
                          │
                          ▼
                 ┌────────────────┐
                 │   主窗口就绪     │
                 │   WebSocket    │
                 │   已连接        │
                 └───────┬────────┘
                         │
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
   ┌──────────┐    ┌──────────┐    ┌──────────┐
   │ 点击会话  │    │ 点击好友  │    │ 修改资料  │
   │ 加载消息  │    │ 切到会话  │    │ 发送请求  │
   └────┬─────┘    └────┬─────┘    └────┬─────┘
        │               │               │
        ▼               ▼               ▼
   ┌──────────┐    ┌──────────┐    ┌──────────┐
   │消息展示区  │    │消息编辑区  │    │个人信息页  │
   │渲染气泡   │    │可以输入   │    │编辑完成   │
   └────┬─────┘    └────┬─────┘    └──────────┘
        │               │
        │  发送消息      │  收到 WebSocket 推送
        ▼               ▼
   ┌────────────────────────────┐
   │  DataCenter 状态持续更新:   │
   │  · recentMessages 增删     │
   │  · chatSessionList 更新    │
   │  · friendList 变化         │
   │  · unreadCount 变化        │
   └────────────────────────────┘
                │
                │ 退出登录 / 关闭窗口
                ▼
         ┌──────────────┐
         │  closeWebsocket│
         │  持久化未读数据 │
         │  应用退出       │
         └──────────────┘
```
