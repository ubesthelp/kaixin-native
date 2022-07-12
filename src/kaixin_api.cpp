/*! ***********************************************************************************************
 *
 * \file        kaixin_api.cpp
 * \brief       开心 API 源文件。
 *
 * \version     0.1
 * \date        2020-7-28
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "kaixin_api.h"

#include <openssl/hmac.h>
#include <ixwebsocket/IXHttpClient.h>

#include <chrono>
#include <iomanip>

#include "kaixin_version.h"
#include "logger.h"
#include "rapidjsonhelpers.h"
#include "utils.h"


kaixin::Config *g_config = nullptr;

static time_t g_serverTime = 0;
static std::chrono::steady_clock::time_point g_timestamp;


time_t kaixin_get_current_time()
{
    if (g_serverTime == 0)
    {
        return time(nullptr);
    }

    const auto elapsed = std::chrono::steady_clock::now() - g_timestamp;
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    return g_serverTime + seconds.count();
}


namespace kaixin {


// 计算签名
std::string sign(const std::string &verb, const std::string &path, const string_map &queries,
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
    auto *p = HMAC(EVP_sha256(), key, static_cast<int>(g_config->app_secret.length()), input,
                   sts.length(), output, &output_length);

    if (p == nullptr)
    {
        // 计算出错
        return {};
    }

    return utils::to_hex(output, output_length);
}


std::string url_encode(const std::string &s)
{
    static ix::HttpClient http;
    return http.urlEncode(s);
}


std::string make_form(const string_map &queries)
{
    if (queries.empty())
    {
        return {};
    }

    std::vector<std::string> form;
    form.reserve(queries.size());

    for (const auto &[key, value] : queries)
    {
        form.emplace_back(url_encode(key) + '=' + url_encode(value));
    }

    return utils::join(form, "&");
}


std::string make_url(const std::string &base_url, const std::string &path, const string_map &queries)
{
    auto url = base_url + path;

    if (!queries.empty())
    {
        url += '?' + make_form(queries);
    }

    return url;
}


int send_request(const std::string &verb, const std::string &path, const string_map &queries,
                 const string_map &form, const response_data_handler &handler)
{
    assert(!verb.empty() && !path.empty() && path.at(0) == '/');

    // 设置公共参数：k、t、z
    auto now = utils::get_timestamp_ms();
    string_map params = queries;
    params.emplace("k", g_config->app_key);
    params.emplace("t", std::to_string(now));
    params.emplace("z", utils::generate_random_hex_string(16));

    // 如果有访问令牌，则设置 a 参数
    now /= 1000;

    if (!g_config->access_token.empty() && g_config->access_token_expires_at >= now)
    {
        params.emplace("a", g_config->access_token);
    }

    // 签名
    params.emplace("s", sign(verb, path, params, form));

    // 构造 URL
    auto url = make_url(g_config->base_url, path, params);

    ix::HttpClient http;
    auto args = http.createRequest();
    args->logger = [](const std::string &msg) { logger::debug(msg.c_str()); };

#ifndef NDEBUG
    args->verbose = (utils::get_reg_type_value<uint32_t>("kaixin::verbose") != 0);
#endif

    if (!g_config->id_token.empty() && g_config->id_token_expires_at >= now)
    {
        // 设置认证头
        args->extraHeaders.emplace("Authorization", "Bearer " + g_config->id_token);
    }

    // User agent
    //args->extraHeaders.emplace("User-Agent", "kaixin-native/" KAIXIN_VERSION_STRING);

    // 构造请求体，并发送请求
    auto body = make_form(form);
    auto resp = http.request(url, verb, body, args);

#ifndef NDEBUG
    if (args->verbose)
    {
        for (const auto &[key, value] : resp->headers)
        {
            LD() << key + ":" << value;
        }

        LD() << " ";
        LD() << resp->payload;
    }
#endif

    // 分析服务器时间
    if (g_serverTime == 0)
    {
        g_timestamp = std::chrono::steady_clock::now();
        const auto iter = resp->headers.find("Date");

        if (iter != resp->headers.end())
        {
            std::istringstream input(iter->second);
            input.imbue(std::locale(setlocale(LC_ALL, nullptr)));

            tm t = { 0 };
            input >> std::get_time(&t, "%a, %e %b %Y %T GMT");

            if (input.fail())
            {
                LW() << "Failed to parse date: " << iter->second;
            }
            else
            {
                g_serverTime = _mkgmtime(&t);
            }
        }
        else
        {
            LW() << "No Date in headers.";
        }
    }

    if (resp->payload.empty())
    {
        // 响应为空
        LW() << "Empty body.";
        return -1;
    }

    using rapidjson::get;
    rapidjson::Document doc;
    doc.ParseInsitu(resp->payload.data());

    if (doc.HasParseError())
    {
        // 响应内容不是合法 JSON
        LE() << "Not a JSON string.";
        return -1;
    }

    // 服务端错误代码
    auto code = get<int>(doc, "code");

    if ((resp->statusCode / 100) != 2 || code != 0 || !doc.HasMember("data"))
    {
        // 服务端返回错误
        auto msg = get<std::string>(doc, "msg");
        LE() << "Failed to" << verb << path << ":" << resp->statusCode << code << msg;
        return utils::make_int(resp->statusCode, code);
    }

    // 如果指定了响应处理函数，则调用；否则直接返回 0
    if (handler)
    {
        return handler(doc["data"]);
    }

    return 0;
}


}       // namespace kaixin
