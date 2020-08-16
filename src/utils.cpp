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

#ifdef KAIXIN_OS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <cppcodec/hex_lower.hpp>

#include "kaixin_api.h"



namespace utils {


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


const std::string &get_current_locale()
{
    static std::string loc;

    if (loc.empty())
    {
#ifdef KAIXIN_OS_WINDOWS
        WCHAR locName[LOCALE_NAME_MAX_LENGTH] = { 0 };
        GetUserDefaultLocaleName(locName, LOCALE_NAME_MAX_LENGTH);

        for (auto &c : locName)
        {
            if (c == L'-')
            {
                c = L'_';
            }
            else if (c == L'\0')
            {
                break;
            }
        }

        loc = to_narrow(locName);
#endif
    }

    return loc;
}


std::string get_local_agent_code()
{
    if (g_config == nullptr)
    {
        return {};
    }

#ifdef KAIXIN_OS_WINDOWS
    auto icode = get_reg_string_value("icode");
    auto vcode = get_reg_binary_value("vcode");
#endif

    if (icode.empty() || vcode.empty())
    {
        return icode;
    }

    BIO *bio = nullptr;
    EVP_PKEY *pkey = nullptr;
    EVP_MD_CTX *mdctx = nullptr;

    do
    {
        const std::string public_key(R"(-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEjb1LQwlx54Te6AUC+vmDhZ1k8cs8
76glS+KbuoSI1t5GgetS72GryBW9/XsrlmEF87LEaN5Gs0qnTHBBj2BmPg==
-----END PUBLIC KEY-----)");

        bio = BIO_new_mem_buf(public_key.data(), static_cast<int>(public_key.length()));

        if (bio == nullptr)
        {
            break;
        }

        pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);

        if (pkey == nullptr)
        {
            break;
        }

        mdctx = EVP_MD_CTX_new();

        if (mdctx == nullptr)
        {
            break;
        }

        if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) != 1)
        {
            break;
        }

        if (EVP_DigestVerifyUpdate(mdctx, icode.c_str(), icode.length()) != 1)
        {
            break;
        }

        if (EVP_DigestVerifyFinal(mdctx, vcode.data(), vcode.size()) == 1)
        {
            // 本地验证成功
            vcode.clear();
        }
    } while (false);

    if (mdctx)
    {
        EVP_MD_CTX_free(mdctx);
    }

    if (pkey)
    {
        EVP_PKEY_free(pkey);
    }

    if (bio)
    {
        BIO_free(bio);
    }

    return vcode.empty() ? icode : std::string();
}


// vaCI:skip+*:nullptr
#ifdef KAIXIN_OS_WINDOWS
std::string to_narrow(const std::wstring &wide)
{
    auto size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    std::string narrow(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, narrow.data(), size, NULL, NULL);
    narrow.resize(narrow.size() - 1);
    return narrow;
}


std::wstring to_wide(const std::string &narrow)
{
    auto size = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, NULL, 0);
    std::wstring wide(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, wide.data(), size);
    wide.resize(wide.size() - 1);
    return wide;
}


reg_value get_reg_value(const std::string &value_name, const reg_value &default_value /*= {}*/)
{
    std::wstring subkey(L"SOFTWARE\\");
    subkey += to_wide(g_config->organization);
    subkey += L"\\";
    subkey += to_wide(g_config->application);

    auto wide_name = to_wide(value_name);
    DWORD dwType = 0;
    DWORD cbData = 0;
    RegGetValueW(HKEY_LOCAL_MACHINE, subkey.c_str(), wide_name.c_str(), RRF_RT_ANY, &dwType, NULL,
                 &cbData);

    if (cbData == 0)
    {
        return default_value;
    }

    std::vector<uint8_t> value(cbData, 0);
    RegGetValueW(HKEY_LOCAL_MACHINE, subkey.c_str(), wide_name.c_str(), RRF_RT_ANY, NULL,
                 value.data(), &cbData);

    reg_value rvalue;

    switch (dwType)
    {
    case REG_SZ:
        rvalue = to_narrow(reinterpret_cast<wchar_t *>(value.data()));
        break;
    case REG_BINARY:
        rvalue = std::move(value);
        break;
    case REG_DWORD:
        rvalue = *reinterpret_cast<DWORD *>(value.data());
        break;
    case REG_QWORD:
        rvalue = *reinterpret_cast<uint64_t *>(value.data());
        break;
    default:
        break;
    }

    return rvalue;
}
#endif
// vaCI:skip-*:nullptr


}       // namespace utils
