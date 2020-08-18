/*! ***********************************************************************************************
 *
 * \file        websocket_client.cpp
 * \brief       websocket_client 类源文件。
 *
 * \version     0.1
 * \date        2020-08-18
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "websocket_client.h"

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUrlParser.h>

#include "kaixin_api.h"
#include "rapidjsonhelpers.h"
#include "simple_timer.h"
#include "utils.h"

using std::placeholders::_1;


websocket_client::websocket_client(kaixin_notification_callback_t callback)
    : ws_(nullptr)
    , heartbeat_timer_(nullptr)
    , callback_(callback)
{
    assert(callback != nullptr);

    handlers_.emplace("RO", std::bind(&websocket_client::on_register_device_succeeded, this, _1));
    handlers_.emplace("RF", std::bind(&websocket_client::on_register_device_failed, this, _1));
    handlers_.emplace("HO", std::bind(&websocket_client::on_heartbeat_response, this, _1));
    handlers_.emplace("NF", std::bind(&websocket_client::on_notify, this, _1));
    handlers_.emplace("OS", std::bind(&websocket_client::on_flow_control, this, _1));
    handlers_.emplace("CR", std::bind(&websocket_client::on_life_cycle, this, _1));
}


websocket_client::~websocket_client()
{
    delete heartbeat_timer_;
    delete ws_;
}


void websocket_client::create_socket()
{
    delete ws_;
    ws_ = new ix::WebSocket;

    std::string protocol;
    std::string host;
    std::string path;
    std::string query;
    int port = 0;
    ix::UrlParser::parse(g_config->base_url, protocol, host, path, query, port);

    ws_->setUrl("ws://" + host + ":8080/");
    ws_->setOnMessageCallback(std::bind(&websocket_client::on_message_callback, this, _1));
    ws_->start();
}


void websocket_client::on_message_callback(const ix::WebSocketMessagePtr &msg)
{
    std::lock_guard lock(mutex_);

    switch (msg->type)
    {
    case ix::WebSocketMessageType::Open:
        {
            // 命令字：RG
            // 含义：在API网关注册长连接，携带DeviceId
            // 命令类型：请求
            // 发送端：客户端
            // 格式：RG#DeviceId
            // 示例：RG#ffd3234343dae324342@12344133
            std::string rg("RG#");
            rg += utils::generate_random_hex_string(16);
            rg += "@";
            rg += g_config->app_key;
            ws_->sendText(rg);
        }
        break;

    case ix::WebSocketMessageType::Close:
        //ws_->start();
        break;

    case ix::WebSocketMessageType::Message:
        {
            auto cmd = msg->str.substr(0, 2);
            auto iter = handlers_.find(cmd);

            if (iter != handlers_.end())
            {
                iter->second(msg->str.substr(2));
            }
        }
        break;
    }
}


void websocket_client::on_register_device_succeeded(const std::string_view &arg)
{
    // 命令字：RO
    // 含义：DeviceId注册成功时，API网关返回成功，并将连接唯一标识和心跳间隔配置返回
    // 命令类型：应答
    // 发送端：API网关
    // 格式：RO#ConnectionCredential#keepAliveInterval
    // 示例：RO#1534692949977#25000
    auto index = arg.find_last_of('#');
    auto interval = std::strtol(arg.substr(index + 1).data(), nullptr, 0);

    assert(heartbeat_timer_ == nullptr);
    heartbeat_timer_ = new simple_timer;
    heartbeat_timer_->set_timeout_callback(std::bind(&websocket_client::heartbeat, this));
    heartbeat_timer_->start(interval * 1000);
}


void websocket_client::on_register_device_failed(const std::string_view &/*arg*/)
{
    // 失败命令字：RF
    // 含义：API网关返回注册失败应答
    // 命令类型：应答
    // 发送端：API网关
    // 格式：RF#ErrorMessage
    // 示例：RF#ServerError
}


void websocket_client::on_heartbeat_response(const std::string_view &/*arg*/)
{
    // Do nothing.
}


void websocket_client::on_notify(const std::string_view &arg)
{
    // 命令字：NO
    // 含义：客户端返回接收下行通知应答
    // 命令类型：应答
    // 发送端：客户端
    // 没有其他参数，直接发送命令字
    ws_->sendText("NO");

    // 命令字：NF
    // 含义：API网关发送下行通知请求
    // 命令类型：请求
    // 发送端：API网关
    // 格式为NF#MESSAGE
    // 示例：NF#HELLO WORLD!
    rapidjson::Document doc;
    doc.ParseInsitu(const_cast<char *>(arg.data()));

    if (!doc.HasParseError() && doc.IsObject())
    {
        kaixin_notification_arguments_t args;
        rapidjson::get(args.action, doc, "action");
        callback_(&args);
    }
}


void websocket_client::on_flow_control(const std::string_view &/*arg*/)
{
    // 命令字：OS
    // 含义：客户端请求量达到API网关流控阈值，API网关会给客户端发送这个命令，需要客户端主动断掉连接，主动重连。主动重连将不会影响用户体验。否则API网关不久后会主动断链长连接。
    // 命令类型：请求
    // 发送端：API网关
    // 没有其他参数，直接发送命令字
    ws_->close();
}


void websocket_client::on_life_cycle(const std::string_view &/*arg*/)
{
    // 命令字：CR
    // 含义：连接达到长连接生命周期，API网关会给客户端发送这个命令，需要客户端主动断掉连接，主动重连。主动重连将不会影响用户体验。否则API网关不久后会主动断链长连接。
    // 命令类型：请求
    // 发送端：API网关
    // 没有其他参数，直接发送命令字
    ws_->close();
}


void websocket_client::heartbeat()
{
    // 命令字：H1
    // 含义：客户端心跳请求信令
    // 命令类型：请求
    // 发送端：客户端
    // 没有其他参数，直接发送命令字
    std::lock_guard lock(mutex_);
    ws_->sendText("H1");
}
