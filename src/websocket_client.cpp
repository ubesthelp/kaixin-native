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

#include <chrono>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUrlParser.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "kaixin_api.h"
#include "kaixin_version.h"
#include "logger.h"
#include "rapidjsonhelpers.h"
#include "simple_timer.h"
#include "utils.h"

using std::placeholders::_1;


static std::string get_host(const std::string &url)
{
    std::string protocol;
    std::string host;
    std::string path;
    std::string query;
    int port = 0;
    ix::UrlParser::parse(url, protocol, host, path, query, port);
    return host;
}

template<typename W, typename K, typename V>
static inline void write(W &w, const K &key, const V &value)
{
    w.Key(key);
    w.String(value);
}

template<typename W, typename K, typename V>
static inline void write_array(W &w, const K &key, const V &value)
{
    w.Key(key);
    w.StartArray();
    w.String(value);
    w.EndArray();
}


websocket_client::websocket_client(kaixin_notification_callback_t callback, void *user_data)
    : ws_(nullptr)
    , heartbeat_timer_(nullptr)
    , callback_(callback)
    , user_data_(user_data)
    , seq_(0)
    , reg_seq_(-1)
    , dereg_seq_(-1)
    , registered_(false)
{
    assert(callback != nullptr);

    handlers_.emplace("RO", std::bind(&websocket_client::on_register_device_succeeded, this, _1));
    handlers_.emplace("RF", std::bind(&websocket_client::on_register_device_failed, this, _1));
    handlers_.emplace("HO", std::bind(&websocket_client::on_heartbeat_response, this, _1));
    handlers_.emplace("NF", std::bind(&websocket_client::on_notify, this, _1));
    handlers_.emplace("OS", std::bind(&websocket_client::on_flow_control, this, _1));
    handlers_.emplace("CR", std::bind(&websocket_client::on_life_cycle, this, _1));

    create_socket();
}


websocket_client::~websocket_client()
{
    delete heartbeat_timer_;
    heartbeat_timer_ = nullptr;

    if (registered_)
    {
        // 注销下行通知
        LD() << "Deregistering notifications.";
        dereg_seq_ = del("/notification", {}, {}, { {"x-ca-websocket_api_type", "UNREGISTER"} });

        using namespace std::chrono_literals;
        std::unique_lock lock(mutex_);
        cond_.wait_for(lock, 5s);
    }

    ws_->stop();
    delete ws_;
}


void websocket_client::create_socket()
{
    delete ws_;
    ws_ = new ix::WebSocket;
    //ws_->setExtraHeaders({ {"User-Agent", "kaixin-native/" KAIXIN_VERSION_STRING } });
    ws_->setUrl("ws://" + get_host(g_config->base_url) + ":8080/");
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
            LD() << "Socket connected.";

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
            LD() << rg;
            ws_->sendText(rg);
        }
        break;

    case ix::WebSocketMessageType::Close:
        LD() << "Socket disconnected.";

        if (heartbeat_timer_ != nullptr)
        {
            delete heartbeat_timer_;
            heartbeat_timer_ = nullptr;
        }

        break;

    case ix::WebSocketMessageType::Message:
        {
            auto cmd = msg->str.substr(0, 2);
            auto iter = handlers_.find(cmd);

            if (iter != handlers_.end())
            {
                iter->second(msg->str.substr(2));
            }
            else
            {
                handle_response(msg->str);
            }
        }
        break;

    case ix::WebSocketMessageType::Error:
        LE() << "Socket error:" << msg->errorInfo.http_status << msg->errorInfo.reason;
        break;
    }
}


void websocket_client::on_register_device_succeeded(const std::string_view &arg)
{
    LD() << "Device registered.";
    // 命令字：RO
    // 含义：DeviceId注册成功时，API网关返回成功，并将连接唯一标识和心跳间隔配置返回
    // 命令类型：应答
    // 发送端：API网关
    // 格式：RO#ConnectionCredential#keepAliveInterval
    // 示例：RO#1534692949977#25000
    auto index = arg.find_last_of('#');
    auto interval = std::strtol(arg.substr(index + 1).data(), nullptr, 0);

    // 开启心跳计时
    assert(heartbeat_timer_ == nullptr);
    heartbeat_timer_ = new simple_timer;
    heartbeat_timer_->set_timeout_callback(std::bind(&websocket_client::heartbeat, this));
    heartbeat_timer_->start(interval);

    // 注册下行通知
    LD() << "Registering notifications.";
    reg_seq_ = post("/notification", {}, {}, { { "x-ca-websocket_api_type", "REGISTER" } });
}


void websocket_client::on_register_device_failed(const std::string_view &/*arg*/)
{
    // 失败命令字：RF
    // 含义：API网关返回注册失败应答
    // 命令类型：应答
    // 发送端：API网关
    // 格式：RF#ErrorMessage
    // 示例：RF#ServerError
    LE() << "Device registration failed.";
}


void websocket_client::on_heartbeat_response(const std::string_view &/*arg*/)
{
    // Do nothing.
}


void websocket_client::on_notify(const std::string_view &arg)
{
    LD() << "Notified.";

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
    if (arg.length() > 1)
    {
        rapidjson::Document doc;
        doc.ParseInsitu(const_cast<char *>(arg.data() + 1));

        if (!doc.HasParseError() && doc.IsObject())
        {
            kaixin_notification_arguments_t args;
            rapidjson::get(args.action, doc, "action");
            callback_(&args, user_data_);
        }
    }
}


void websocket_client::on_flow_control(const std::string_view &/*arg*/)
{
    LD() << "Flow control.";
    // 命令字：OS
    // 含义：客户端请求量达到API网关流控阈值，API网关会给客户端发送这个命令，需要客户端主动断掉连接，主动重连。主动重连将不会影响用户体验。否则API网关不久后会主动断链长连接。
    // 命令类型：请求
    // 发送端：API网关
    // 没有其他参数，直接发送命令字
    ws_->stop();
    ws_->start();
}


void websocket_client::on_life_cycle(const std::string_view &/*arg*/)
{
    LD() << "Life cycle.";
    // 命令字：CR
    // 含义：连接达到长连接生命周期，API网关会给客户端发送这个命令，需要客户端主动断掉连接，主动重连。主动重连将不会影响用户体验。否则API网关不久后会主动断链长连接。
    // 命令类型：请求
    // 发送端：API网关
    // 没有其他参数，直接发送命令字
    ws_->stop();
    ws_->start();
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


std::string websocket_client::make_request(const std::string &verb, const std::string &path,
                                           const ix::WebSocketHttpHeaders &queries,
                                           const ix::WebSocketHttpHeaders &body,
                                           const ix::WebSocketHttpHeaders &headers)
{
    if (ws_ == nullptr || ws_->getReadyState() != ix::ReadyState::Open)
    {
        create_socket();
    }

    auto now = utils::get_timestamp_ms();
    std::ostringstream oss;
    rapidjson::OStreamWrapper buffer(oss);
    rapidjson::Writer<rapidjson::OStreamWrapper> w(buffer);
    w.StartObject();
    {
        write(w, "method", verb);
        write(w, "host", get_host(g_config->base_url));
        write(w, "path", path);
        w.Key("querys");
        w.StartObject();
        {
            // 设置公共参数：a、k、t、z
            auto params = queries;
            params.emplace("a", g_config->access_token);
            params.emplace("k", g_config->app_key);
            params.emplace("t", std::to_string(now));
            params.emplace("z", utils::generate_random_hex_string(16));

            // 签名
            params.emplace("s", kaixin::sign(verb, path, params, body));

            for (const auto &[key, value] : params)
            {
                write(w, key, value);
            }
        }
        w.EndObject();
        w.Key("headers");
        w.StartObject();
        {
            for (const auto &[key, value] : headers)
            {
                write_array(w, key, value);
            }

#ifndef NDEBUG
            write_array(w, "x-ca-request-mode", "debug");
#endif
            write_array(w, "x-ca-seq", std::to_string(seq_++));

            // 设置认证头
            write(w, "authorization", "Bearer " + g_config->id_token);
        }
        w.Key("isBase64");
        w.Int(0);
        write(w, "body", kaixin::make_form(body));
        w.EndObject();
    }
    w.EndObject();

    return oss.str();
}


int websocket_client::post(const std::string &path, const ix::WebSocketHttpHeaders &queries,
                           const ix::WebSocketHttpHeaders &body,
                           const ix::WebSocketHttpHeaders &headers)
{
    auto req = make_request(ix::HttpClient::kPost, path, queries, body, headers);
    ws_->sendText(req);
    return seq_ - 1;
}


int websocket_client::del(const std::string &path, const ix::WebSocketHttpHeaders &queries,
                          const ix::WebSocketHttpHeaders &body,
                          const ix::WebSocketHttpHeaders &headers)
{
    auto req = make_request("DELETE", path, queries, body, headers);
    ws_->sendText(req);
    return seq_ - 1;
}


void websocket_client::handle_response(const std::string &json)
{
    using rapidjson::get;
    rapidjson::Document doc;
    doc.ParseInsitu(const_cast<char *>(json.c_str()));

    if (doc.HasParseError())
    {
        return;
    }

    auto status = get<int>(doc, "status");
    const auto &headers = doc["header"];
    auto seq = std::strtol(get<const char *>(headers, "x-ca-seq"), nullptr, 0);

    if (seq == reg_seq_)
    {
        if ((status / 100) == 2)
        {
            LD() << "Notification registered.";
            reg_seq_ = -1;
            registered_ = true;
        }
    }
    else if (seq == dereg_seq_)
    {
        LD() << "Notification deregistered.";
        registered_ = false;
        cond_.notify_all();
    }
}
