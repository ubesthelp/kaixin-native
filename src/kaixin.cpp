/*! ***********************************************************************************************
 *
 * \file        kaixin.cpp
 * \brief       开心 C SDK 源文件。
 *
 * \version     0.1
 * \date        2018-3-4
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2018 Roy QIU。
 *
 **************************************************************************************************/
#include "kaixin.h"

#include <cassert>
#include <cerrno>
#include <string>
#include <ixwebsocket/IXHttpClient.h>
#include <rapidjson/document.h>

 // 纠正 EINVAL 被重定义为 WSAEINVAL 的问题。
#ifdef WIN32
#undef EINVAL
#define EINVAL 22
#endif

// 字符串到字符串的映射
using string_map = ix::WebSocketHttpHeaders;

// 全局配置参数
struct Config
{
    std::string app_key;            // APP KEY
    std::string app_secret;         // APP SECRET
    std::string base_url;           // 基础 URL
    std::string access_token;       // 访问令牌
    std::string refresh_token;      // 更新令牌
    std::string id_token;           // 身份令牌
    time_t expires_at = 0;          // 访问令牌过期时间
    time_t refresh_token_expires_at = 0;        // 更新令牌过期时间
};

static Config *g_config = nullptr;


static inline bool is_empty(const char *s)
{
    return s == nullptr || strlen(s) == 0;
}


static int send_request(const std::string &verb, const std::string &path, const string_map &quries,
                        const string_map &form)
{
    assert(!verb.empty() && !path.empty() && path.at(0) == '/');
    auto url = g_config->base_url + path;

    ix::HttpClient http;
    auto req = http.createRequest(url, verb);

    if (!g_config->id_token.empty())
    {
        req->extraHeaders.insert(std::make_pair("Authorization", "Bearer " + g_config->id_token));
    }

    return 0;
}


int kaixin_initialize(const char *app_key, const char *app_secret, const char *base_url)
{
    if (g_config != nullptr)
    {
        // 已经初始化过了
        return EPERM;
    }

    if (is_empty(app_key) || is_empty(app_secret))
    {
        // APP KEY 或 SECRET 为空
        return EINVAL;
    }

    g_config = new Config;
    _ASSERT(g_config != nullptr);
    g_config->app_key = app_key;
    g_config->app_secret = app_secret;
    g_config->base_url = base_url;

    return 0;
}


void kaixin_uninitialize()
{
    delete g_config;
    g_config = nullptr;
}


int kaixin_sign_in(const char *username, const char *password)
{

}
