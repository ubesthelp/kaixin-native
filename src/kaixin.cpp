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

#include <chrono>
#include <functional>
#include <sstream>

#include <openssl/hmac.h>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXNetSystem.h>

#include "rapidjsonhelpers.h"
#include "utils.h"

 // 纠正 EINVAL 被重定义为 WSAEINVAL 的问题。
#ifdef WIN32
#undef EINVAL
#define EINVAL 22
#endif


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
static kaixin_profile_t *g_profile = nullptr;


// 计算签名
static std::string sign(const std::string &verb, const std::string &path, const string_map &queries,
                        const string_map &form)
{
    // 签名字符串：请求方法 + 路径
    std::string sts = verb + path;

    // 所有请求参数（查询 + 表单）合并排序
    string_map params = queries;
    params.insert(form.begin(), form.end());

    // + 参数名称 + 参数值
    for (const auto &[key, value] : params)
    {
        sts += key + value;
    }

    // 计算 HMAC SHA256
    auto *input = reinterpret_cast<const uint8_t *>(sts.c_str());
    auto *key = reinterpret_cast<const uint8_t *>(g_config->app_secret.c_str());
    uint8_t output[EVP_MAX_MD_SIZE] = { 0 };
    unsigned int output_length = EVP_MAX_MD_SIZE;
    auto *p = HMAC(EVP_sha256(), key, g_config->app_secret.length(), input, sts.length(), output,
                   &output_length);

    if (p == nullptr)
    {
        // 计算出错
        return {};
    }

    return to_hex(output, output_length);
}


// 同步发送请求并返回结果
static int send_request(const std::string &verb, const std::string &path, const string_map &queries,
                        const string_map &form, const response_data_handler &handler)
{
    assert(!verb.empty() && !path.empty() && path.at(0) == '/' && !!handler);

    // 设置公共参数：k、t、z，并签名
    string_map params = queries;
    params.emplace("k", g_config->app_key);
    params.emplace("t", std::to_string(timestamp()));
    params.emplace("z", generate_random_hex_string(16));
    params.emplace("s", sign(verb, path, params, form));

    // 构造 URL
    auto url = make_url(g_config->base_url, path, params);

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

    if (is_empty(base_url))
    {
        // TODO: 设置默认生产环境 URL
    }
    else
    {
        g_config->base_url = base_url;

        if (g_config->base_url.at(g_config->base_url.length() - 1) == '/')
        {
            // 删除结尾处的“/”
            g_config->base_url.erase(g_config->base_url.length() - 1, 1);
        }
    }

    ix::initNetSystem();
    return 0;
}


// 反初始化
void kaixin_uninitialize()
{
    ix::uninitNetSystem();

    delete g_profile;
    g_profile = nullptr;

    delete g_config;
    g_config = nullptr;
}


// 登录
static int sign_in_handler(const rapidjson::Value &data)
{
    using rapidjson::get;
    auto now = time(nullptr);
    get(g_config->access_token, data, "access_token");
    get(g_config->refresh_token, data, "refresh_token");
    get(g_config->id_token, data, "id_token");
    g_config->expires_at = now + get<int>(data, "expires_in");
    g_config->refresh_token_expires_at = now + get<int>(data, "refresh_token_expires_in");

    assert(g_profile == nullptr);
    g_profile = new kaixin_profile_t;
    memset(g_profile, 0, sizeof(kaixin_profile_t));
    g_profile->access_token = g_config->access_token.c_str();
    g_profile->refresh_token = g_config->refresh_token.c_str();
    g_profile->id_token = g_config->id_token.c_str();
    g_profile->access_token_expires_at = g_config->expires_at;
    g_profile->refresh_token_expires_at = g_config->refresh_token_expires_at;
    return 0;
}

int kaixin_sign_in(const char *username, const char *password)
{
    string_map form{
        { "username", username },
        { "password", password }
    };

    return send_request(ix::HttpClient::kPost, "/session", {}, form, sign_in_handler);
}


const kaixin_profile_t *kaixin_get_profile()
{
    return g_profile;
}
