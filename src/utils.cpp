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

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cppcodec/hex_lower.hpp>
#include <ixwebsocket/IXHttpClient.h>


std::string generate_random_hex_string(size_t length)
{
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[length]);
    RAND_pseudo_bytes(bytes.get(), static_cast<int>(length));
    return to_hex(bytes.get(), length);
}


std::string to_hex(const uint8_t *buffer, size_t length)
{
    return cppcodec::hex_lower::encode(buffer, length);
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
