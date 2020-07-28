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


struct Config
{
    std::string app_key;
    std::string app_secret;
    std::string base_url;
};

static Config *g_config = nullptr;


static inline bool is_empty(const char *s)
{
    return s == nullptr || strlen(s) == 0;
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
