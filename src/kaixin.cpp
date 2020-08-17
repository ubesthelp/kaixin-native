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

#include <ixwebsocket/IXNetSystem.h>

#include "authorization_disabler.h"
#include "fingerprint.h"
#include "jwt.h"
#include "kaixin_api.h"
#include "rapidjsonhelpers.h"
#include "utils.h"

 // 纠正 EINVAL 被重定义为 WSAEINVAL 的问题。
#ifdef KAIXIN_OS_WINDOWS
#undef EINVAL
#define EINVAL 22
#endif


kaixin_profile_t *g_profile = nullptr;


// 初始化
int kaixin_initialize(const char *organization, const char *application, const char *app_key,
                      const char *app_secret, const char *base_url)
{
    if (g_config != nullptr)
    {
        // 已经初始化过了
        return EPERM;
    }

    if (utils::is_empty(organization) || utils::is_empty(application)
        || utils::is_empty(app_key) || utils::is_empty(app_secret))
    {
        // 组织名、应用名、APP KEY 或 SECRET 为空
        return EINVAL;
    }

    if (!utils::is_empty(base_url) && strstr(base_url, "https://") != base_url)
    {
        // 基础 URL 必须是 HTTPS 协议
        return EINVAL;
    }

    g_config = new kaixin::Config;
    _ASSERT(g_config != nullptr);
    g_config->organization = organization;
    g_config->application = application;
    g_config->app_key = app_key;
    g_config->app_secret = app_secret;

    if (utils::is_empty(base_url))
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
    g_config->access_token_expires_at = now + get<int>(data, "expires_in");
    g_config->refresh_token_expires_at = now + get<int>(data, "refresh_token_expires_in");

    auto payload = jwt::payload(g_config->id_token, g_config->app_key);

    if (payload.empty())
    {
        // 身份令牌无效
        return 1;
    }

    using rapidjson::get;
    rapidjson::Document doc;
    doc.ParseInsitu(payload.data());
    get(g_config->username, doc, "sub");
    get(g_config->email, doc, "email");
    get(g_config->agent_code, doc, "agent_code");
    get(g_config->secret, doc, "secret");
    get(g_config->id_token_expires_at, doc, "exp");

    if (g_config->agent_code.empty())
    {
        g_config->agent_code = utils::get_local_agent_code();
    }

    assert(g_profile == nullptr);
    g_profile = new kaixin_profile_t;
    memset(g_profile, 0, sizeof(kaixin_profile_t));
    g_profile->access_token = g_config->access_token.c_str();
    g_profile->refresh_token = g_config->refresh_token.c_str();
    g_profile->id_token = g_config->id_token.c_str();
    g_profile->username = g_config->username.c_str();
    g_profile->email = g_config->email.c_str();
    g_profile->invitation_code = g_config->agent_code.c_str();
    g_profile->secret = g_config->secret.c_str();
    g_profile->access_token_expires_at = g_config->access_token_expires_at;
    g_profile->refresh_token_expires_at = g_config->refresh_token_expires_at;
    g_profile->id_token_expires_at = g_config->id_token_expires_at;
    get(g_profile->status, doc, "status");

    return 0;
}

int kaixin_sign_in(const char *username, const char *password)
{
    kaixin::string_map form{
        { "username", username },
        { "password", password }
    };

    return kaixin::send_request(ix::HttpClient::kPost, "/session", form, sign_in_handler);
}


int kaixin_sign_out()
{
    return kaixin::send_request("DELETE", "/session");
}


// 获取用户配置
const kaixin_profile_t *kaixin_get_profile()
{
    return g_profile;
}


// 获取设备 ID
const char *kaixin_get_device_id()
{
    if (g_config == nullptr)
    {
        return nullptr;
    }

    if (!g_config->device_id.empty())
    {
        return g_config->device_id.c_str();
    }

    kaixin::string_map form{
        { "fp", fp::generate_simple_fingerprint() },
    };

    kaixin::send_request(ix::HttpClient::kPost, "/device-id", form, [](const rapidjson::Value &data)
    {
        g_config->device_id = data.GetString();
        return 0;
    });

    return g_config->device_id.c_str();
}


// 获取授权
kaixin_auth_t * kaixin_get_auth()
{
    kaixin_auth_t *auth = nullptr;

    kaixin::send_request(ix::HttpClient::kGet, "/auth", [&auth](const rapidjson::Value &data)
    {
        using rapidjson::get;
        auto *prev = auth;

        for (const auto &a : data.GetArray())
        {
            auto *p = new kaixin_auth_t;
            p->next = nullptr;
#ifdef KAIXIN_COMP_MSVC
            p->module_name = _strdup(get<const char *>(a, "module"));
#else
            p->module_name = strdup(get<const char *>(a, "module"));
#endif
            get(p->edition, a, "edition");
            get(p->count, a, "count");
            get(p->time, a, "time");

            if (auth == nullptr)
            {
                auth = p;
            }
            else
            {
                prev->next = p;
            }

            prev = p;
        }

        return 0;
    });

    return auth;
}


// 释放授权链表
void kaixin_free_auth(kaixin_auth_t *auth)
{
    auto *p = auth;

    while (p != nullptr)
    {
        auth = p->next;
        free(const_cast<char *>(p->module_name));
        delete p;
        p = auth;
    }
}


// 获取应用最低版本号
kaixin_version_t kaixin_get_lowest_version()
{
    kaixin_version_t lowest = { 0, 0, 0 };

    kaixin::AuthorizationDisabler atd;
    kaixin::send_request(ix::HttpClient::kGet, "/lowest-version", [&lowest](const rapidjson::Value &data)
    {
        get(lowest.major, data, "major");
        get(lowest.minor, data, "minor");
        get(lowest.patch, data, "patch");
        return 0;
    });

    return lowest;
}


// 获取素材
const char *kaixin_get_material(const char *type)
{
    if (g_config == nullptr)
    {
        return nullptr;
    }

    if (!g_config->materials.empty())
    {
        // 素材已经获取过了，直接查找返回
        auto iter = g_config->materials.find(type);

        if (iter == g_config->materials.end())
        {
            return nullptr;
        }

        return iter->second.c_str();
    }

    // 获取素材
    kaixin::string_map queries{
        { "locale", utils::get_current_locale() },
        { "agent_code", g_config->agent_code.empty() ? utils::get_local_agent_code() : g_config->agent_code },
    };

    kaixin::send_request(ix::HttpClient::kGet, "/materials", queries, {}, [](const rapidjson::Value &data)
    {
        using rapidjson::get;

        for (const auto &e : data.GetArray())
        {
            auto name = get<std::string>(e, "name");
            auto text = get<std::string>(e, "text");
            g_config->materials.emplace(name, text);
        }

        return 0;
    });

    auto iter = g_config->materials.find(type);

    if (iter == g_config->materials.end())
    {
        return nullptr;
    }

    return iter->second.c_str();
}
