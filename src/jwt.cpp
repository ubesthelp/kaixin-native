/*! ***********************************************************************************************
 *
 * \file        jwt.cpp
 * \brief       JWT 函数源文件。
 *
 * \version     0.1
 * \date        2020-07-30
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "jwt.h"

#include <ctime>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <cppcodec/base64_url_unpadded.hpp>

#include "rapidjsonhelpers.h"
#include "utils.h"

namespace jwt {

/// JWK 公钥
static const char JWK_PUBLIC[] =
R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAhCv3iNGsEa7pJ6yprVEN
cMwgRJNjHmlNM5v/gzat5zqkt3Vt29/9Afy89cyQd3emKskKQoNoEHfl48NWjFsO
45Bgwx5ysqNONGcn2+eBTQ2UbuEyu6lPX1cWSkFNMcHkWIYCHXXWcqJ24mAuwDLM
WY4uSu/THYdB3zlCIvADdW8c8BPJP9O9qz92CThoOnd00e56KMnho6kY3IT+V7Uq
GXEaNi58LhCbuxqezi64A50afEv8Qyw5HZNADZt/Z+wOv9cth+OdsT8cwA/BlWbM
fjUjCDD70qsaPbT/uZrzcLDKdywzb/a+D75DDlLAmEby/CsUHch2PDZScylJavj0
xQIDAQAB
-----END PUBLIC KEY-----)";


// Base64Url 解码
template<typename Result = std::string>
inline Result base64_decode(const std::string &encoded)
{
    return cppcodec::base64_url_unpadded::decode<Result>(encoded);
}


// 判断头是否有效
static bool is_header_valid(std::string &header)
{
    using rapidjson::get;
    rapidjson::Document doc;
    doc.ParseInsitu(header.data());

    if (doc.HasParseError())
    {
        // 不是 JSON
        return false;
    }

    // 只支持 RS256 算法
    auto alg = get<std::string>(doc, "alg");
    auto typ = get<std::string>(doc, "typ");
    return alg == "RS256" && typ == "JWT";
}


std::string payload(const std::string &id_token, const std::string &app_key)
{
    assert(!id_token.empty() && !app_key.empty());
    auto pos0 = id_token.find('.');
    auto header = id_token.substr(0, pos0);
    header = base64_decode(header);

    if (!is_header_valid(header))
    {
        // 头无效
        return {};
    }

    // 获取待验证部分和签名
    pos0++;
    auto pos1 = id_token.find('.', pos0);
    header = id_token.substr(0, pos1);
    auto sig = base64_decode<std::vector<uint8_t>>(id_token.substr(pos1 + 1));

    // 验证签名
    auto *bio = BIO_new_mem_buf(JWK_PUBLIC, sizeof(JWK_PUBLIC));
    auto *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    auto *mdctx = EVP_MD_CTX_new();
    std::string data;

    do
    {
        if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) != 1)
        {
            break;
        }

        if (EVP_DigestVerifyUpdate(mdctx, header.c_str(), header.length()) != 1)
        {
            break;
        }

        if (EVP_DigestVerifyFinal(mdctx, sig.data(), sig.size()) != 1)
        {
            break;
        }

        // 签名验证成功
        auto pload = id_token.substr(pos0, pos1 - pos0);
        pload = base64_decode(pload);

        using rapidjson::get;
        rapidjson::Document doc;
        doc.Parse(pload);

        if (doc.HasParseError())
        {
            // 不是 JSON
            break;
        }

        if (get<std::string>(doc, "aud") != app_key)
        {
            // aud 不匹配
            break;
        }

        if (get<int64_t>(doc, "exp") < time(nullptr))
        {
            // 已过期
            break;
        }

        data = std::move(pload);
    } while (false);

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    BIO_free(bio);
    return data;
}


}       // namespace jwt
