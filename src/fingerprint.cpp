/*! ***********************************************************************************************
 *
 * \file        fingerprint.cpp
 * \brief       fingerprint 类源文件。
 *
 * \version     1.0
 * \date        2020-08-08
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "fingerprint.h"

#include <locale>
#include <sstream>

#include <openssl/evp.h>
#include <cppcodec/base64_url_unpadded.hpp>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "utils.h"

#ifdef _WIN32
#include "wmi_client.h"
#else
#error "Unsupported platform."
#endif

namespace fp {

static std::string &to_lower(std::string &s, const std::locale &loc = {})
{
    const auto &facet = std::use_facet<std::ctype<char>>(loc);
    facet.tolower(s.data(), s.data() + s.length());
    return s;
}

static std::string md5(std::string &s)
{
    EVP_MD_CTX *ctx = nullptr;
    std::string hexed;

    do
    {
        ctx = EVP_MD_CTX_new();

        if (ctx == nullptr)
        {
            break;
        }

        if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1)
        {
            break;
        }

        to_lower(s);

        if (EVP_DigestUpdate(ctx, s.c_str(), s.length()) != 1)
        {
            break;
        }

        unsigned char digest[EVP_MAX_MD_SIZE] = { 0 };
        unsigned int len = 0;

        if (EVP_DigestFinal_ex(ctx, digest, &len) != 1)
        {
            break;
        }

        hexed = utils::to_hex(digest, len);
    } while (false);

    EVP_MD_CTX_free(ctx);
    return hexed;
}


std::string generate_simple_fingerprint()
{
    std::ostringstream oss;
    rapidjson::OStreamWrapper buffer(oss);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(buffer);
    writer.StartObject();

#ifdef _WIN32
    wmi_client wmic;
    writer.Key("bi");
    writer.String(md5(wmic.query(L"BIOS", L"SerialNumber").front()));
    writer.Key("bb");
    writer.String(md5(wmic.query(L"BaseBoard", L"SerialNumber").front()));
    writer.Key("sm");
    writer.String(md5(wmic.query(L"ComputerSystemProduct", L"IdentifyingNumber").front()));
    writer.Key("ma");
    writer.String(md5(wmic.query(L"ComputerSystem", L"Manufacturer").front()));
    writer.Key("mo");
    writer.String(md5(wmic.query(L"ComputerSystem", L"Model").front()));
    writer.Key("ni");
    writer.StartArray();

    for (auto &ni : wmic.query(L"NetworkAdapter", L"MACAddress", L"PNPDeviceID like 'PCI\\\\%'"))
    {
        writer.String(md5(ni));
    }

    writer.EndArray();
    writer.Key("uu");
    writer.String(md5(wmic.query(L"ComputerSystemProduct", L"UUID").front()));
#endif

    writer.EndObject();

    auto s = oss.str();
    return cppcodec::base64_url_unpadded::encode(s);
}


}       // namespace fp
