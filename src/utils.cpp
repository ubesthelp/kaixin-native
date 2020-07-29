/*! ***********************************************************************************************
 *
 * \file        utils.cpp
 * \brief       工具函数源文件。
 *
 * \version     0.1
 * \date        2020-07-29
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "utils.h"

#include <iomanip>
#include <sstream>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include <ixwebsocket/IXHttpClient.h>


std::string generate_random_hex_string(size_t length)
{
    std::string hex;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    do
    {
        if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0) != 0)
        {
            // 初始化种子错误？
            break;
        }

        std::unique_ptr<uint8_t[]> bytes(new uint8_t[length]);

        if (mbedtls_ctr_drbg_random(&ctr_drbg, bytes.get(), length) != 0)
        {
            // 生成数据错误
            break;
        }

        hex = to_hex(bytes.get(), length);
    } while (false);

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return hex;
}


std::string to_hex(const uint8_t *buffer, size_t length)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (size_t i = 0; i < length; i++)
    {
        oss << std::setw(2) << static_cast<int>(buffer[i]);
    }

    return oss.str();
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

    return join(form, "&");
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
