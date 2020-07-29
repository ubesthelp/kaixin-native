/*! ***********************************************************************************************
 *
 * \file        kaixin.cpp
 * \brief       开心 C SDK 源文件。
 *
 * \version     0.1
 * \date        2020-7-28
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "kaixin.h"

#include <functional>
#include <sstream>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXNetSystem.h>

#include "rapidjsonhelpers.h"

 // 纠正 EINVAL 被重定义为 WSAEINVAL 的问题。
#ifdef WIN32
#undef EINVAL
#define EINVAL 22
#endif


// 字符串到字符串的映射
using string_map = ix::WebSocketHttpHeaders;

// 响应数据处理函数类型
using response_data_handler = std::function<int(const rapidjson::Value &)>;

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

// 判断字符串是否为空
static inline bool is_empty(const char *s)
{
    return s == nullptr || strlen(s) == 0;
}

// 连接字符串
template <typename Range, typename Value = typename Range::value_type>
static std::string join(Range const &elements, const char *const delimiter)
{
    std::ostringstream os;
    auto b = std::begin(elements), e = end(elements);

    if (b != e)
    {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }

    if (b != e)
    {
        os << *b;
    }

    return os.str();
}

// 根据查询映射生成表单字符串
static std::string make_form(const string_map &queries)
{
    static ix::HttpClient http;

    if (queries.empty())
    {
        return {};
    }

    std::vector<std::string> form;
    form.reserve(queries.size());

    for (const auto &[key, value] : queries)
    {
        form.emplace_back(http.urlEncode(key + '=' + value));
    }

    return join(form, "&");
}

// 根据高 16 位和低 16 位拼成 32 位整数
static inline int make_int(int upper, int lower)
{
    return static_cast<int>((static_cast<uint32_t>(upper & 0xffff) << 16) | (lower & 0xffff));
}

// 根据路径和查询映射生成完整 URL
static std::string make_url(const std::string &path, const string_map &queries)
{
    auto url = g_config->base_url;
    url += path;

    if (!queries.empty())
    {
        url += '?' + make_form(queries);
    }

    return url;
}

// 同步发送请求并返回结果
static int send_request(const std::string &verb, const std::string &path, const string_map &queries,
                        const string_map &form, const response_data_handler &handler)
{
    assert(!verb.empty() && !path.empty() && path.at(0) == '/' && !!handler);

    // 构造 URL
    auto url = make_url(path, queries);

    ix::HttpClient http;
    auto args = http.createRequest();

    if (!g_config->id_token.empty())
    {
        // 设置认证头
        args->extraHeaders.insert(std::make_pair("Authorization", "Bearer " + g_config->id_token));
    }

    // 构造请求体，并发送请求
    auto body = make_form(form);
    auto resp = http.request(url, verb, body, args);

    if (resp->payload.empty())
    {
        // 响应为空
        return -1;
    }

    rapidjson::Document doc;
    doc.ParseInsitu(resp->payload.data());

    if (doc.HasParseError())
    {
        // 响应内容不是合法 JSON
        return -1;
    }

    int code = get(doc, "code");

    if ((resp->statusCode / 100) != 2 || code != 0)
    {
        // 服务端返回错误
        return make_int(resp->statusCode, get(doc, "code"));
    }

    return handler(doc["data"]);
}


// 初始化
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

    if (!is_empty(base_url) && strstr(base_url, "https://") != base_url)
    {
        // 基础 URL 必须是 HTTPS 协议
        return EINVAL;
    }

    g_config = new Config;
    _ASSERT(g_config != nullptr);
    g_config->app_key = app_key;
    g_config->app_secret = app_secret;
    g_config->base_url = base_url;

    ix::initNetSystem();
    return 0;
}


// 反初始化
void kaixin_uninitialize()
{
    ix::uninitNetSystem();
    delete g_config;
    g_config = nullptr;
}


// 登录
static int sign_in_handler(const rapidjson::Value &data)
{
    return 0;
}

int kaixin_sign_in(const char *username, const char *password)
{
    return send_request(ix::HttpClient::kPost, "/session", {}, {}, sign_in_handler);
}
