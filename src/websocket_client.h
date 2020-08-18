/*! ***********************************************************************************************
 *
 * \file        websocket_client.h
 * \brief       websocket_client 类头文件。
 *
 * \version     0.1
 * \date        2020-08-18
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "noncopyable.h"

#include <functional>
#include <map>
#include <thread>
#include <ixwebsocket/IXHttp.h>
#include <ixwebsocket/IXWebSocketMessage.h>

#include "kaixin.h"

namespace ix {
class WebSocket;
}

class simple_timer;


/*!
 * \brief       WebSocket 客户端类。
 */
class websocket_client : private noncopyable
{
public:
    explicit websocket_client(kaixin_notification_callback_t callback);
    ~websocket_client();

private:
    void create_socket();
    void on_message_callback(const ix::WebSocketMessagePtr &msg);

    void on_register_device_succeeded(const std::string_view &arg);
    void on_register_device_failed(const std::string_view &arg);
    void on_heartbeat_response(const std::string_view &arg);
    void on_notify(const std::string_view &arg);
    void on_flow_control(const std::string_view &arg);
    void on_life_cycle(const std::string_view &arg);

    void heartbeat();

    std::string make_request(const std::string &verb, const std::string &path,
                             const ix::WebSocketHttpHeaders &queries,
                             const ix::WebSocketHttpHeaders &body,
                             const ix::WebSocketHttpHeaders &headers);
    int post(const std::string &path, const ix::WebSocketHttpHeaders &queries,
             const ix::WebSocketHttpHeaders &body, const ix::WebSocketHttpHeaders &headers);

    void handle_response(const std::string &json);

private:
    using command_handler = std::function<void(const std::string_view &)>;
    std::map<std::string, command_handler> handlers_;
    std::mutex mutex_;
    ix::WebSocket *ws_;
    simple_timer *heartbeat_timer_;
    kaixin_notification_callback_t callback_;
    int seq_;
    int reg_seq_;
};
