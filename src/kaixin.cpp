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
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include "authorization_disabler.h"
#include "fingerprint.h"
#include "jwt.h"
#include "kaixin_api.h"
#include "kaixin_version.h"
#include "logger.h"
#include "rapidjsonhelpers.h"
#include "utils.h"

 // 纠正 EINVAL 被重定义为 WSAEINVAL 的问题。
#ifdef KAIXIN_OS_WINDOWS
#undef EINVAL
#define EINVAL 22
#endif

#ifdef KAIXIN_COMP_MSVC
#ifdef strdup
#undef strdup
#endif
#define strdup(s) _strdup(s)
#endif


kaixin_profile_t *g_profile = nullptr;


// 加载更新令牌
static bool load_refresh_token()
{
#ifdef KAIXIN_OS_WINDOWS
    auto expires_at = utils::get_reg_type_value<int64_t>("kaixin::expires_at");
    auto binary = utils::get_reg_type_value<std::vector<uint8_t>>("kaixin::token");
#endif

    if (g_config == nullptr || binary.empty() || expires_at < utils::get_timestamp())
    {
        return false;
    }

    g_config->refresh_token = utils::unprotect_data(binary);
    return !g_config->refresh_token.empty();
}

// 保存更新令牌
static void save_refresh_token()
{
    if (g_config != nullptr && !g_config->refresh_token.empty()
        && g_config->refresh_token_expires_at > utils::get_timestamp())
    {
        auto binary = utils::protect_data(g_config->refresh_token);
#ifdef KAIXIN_OS_WINDOWS
        utils::set_reg_value("kaixin::token", binary);
        utils::set_reg_value("kaixin::expires_at", g_config->refresh_token_expires_at);
#endif
    }
}

static void refresh_token();

// 处理登录
static int sign_in_handler(const rapidjson::Value &data)
{
    using rapidjson::get;
    auto now = utils::get_timestamp();
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
    get(g_config->username, doc, "name");
    get(g_config->email, doc, "email");
    get(g_config->agent_code, doc, "agent_code");
    get(g_config->secret, doc, "secret");
    get(g_config->id_token_expires_at, doc, "exp");

    // 如果代理编号变了，则清空素材。
    if (g_config->agent_code != utils::get_local_agent_code())
    {
        g_config->materials.clear();
    }

    delete g_profile;
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
    LI() << "Signed in" << g_config->email;
    LI() << "I-code:" << g_config->agent_code;

    // 保存令牌
    save_refresh_token();

    if (!g_config->token_refresher)
    {
        // 自动更新令牌
        g_config->token_refresher = std::make_unique<simple_timer>();
        g_config->token_refresher->set_timeout_callback(refresh_token);

        auto refresh_in = get<int>(data, "expires_in") * 3000 / 4;
        g_config->token_refresher->start(refresh_in);
    }

    return 0;
}

// 更新令牌
static void refresh_token()
{
    if (g_config == nullptr)
    {
        return;
    }

    LI() << "Refreshing tokens.";
    kaixin::string_map form{
        { "refresh_token", g_config->refresh_token }
    };

    g_config->access_token_expires_at = 0;
    g_config->id_token_expires_at = 0;
    kaixin::send_request(ix::HttpClient::kPatch, "/session", form, sign_in_handler);
}


const char *kaixin_version()
{
    return KAIXIN_VERSION_STRING;
}


// 设置日志输出函数。
kaixin_log_output_t kaixin_set_log_output(kaixin_log_output_t output)
{
    return logger::set_output(output);
}


// 初始化
int kaixin_initialize(const char *organization, const char *application, const char *app_key,
                      const char *app_secret, const char *base_url)
{
    if (g_config != nullptr)
    {
        // 已经初始化过了
        LE() << "Kaixin SDK is already initialized.";
        return EPERM;
    }

    if (utils::is_empty(organization) || utils::is_empty(application)
        || utils::is_empty(app_key) || utils::is_empty(app_secret))
    {
        // 组织名、应用名、APP KEY 或 SECRET 为空
        LE() << "Applicatoin parameters are empty.";
        return EINVAL;
    }

    if (!utils::is_empty(base_url) && strstr(base_url, "https://") != base_url)
    {
        // 基础 URL 必须是 HTTPS 协议
        LE() << "The base URL must use HTTPS protocol.";
        return EINVAL;
    }

    LI() << "Initializing kaixin native SDK " KAIXIN_VERSION_STRING ".";
    g_config = new kaixin::Config;
    _ASSERT(g_config != nullptr);
    g_config->organization = organization;
    g_config->application = application;
    g_config->app_key = app_key;
    g_config->app_secret = app_secret;

    if (utils::is_empty(base_url))
    {
        // 默认设置生产环境 URL
        g_config->base_url = "https://api.ubesthelp.com";
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

    LI() << "Locale:" << utils::get_current_locale();
    LI() << "Local i-code:" << utils::get_local_agent_code();
    ix::initNetSystem();

    // 加载上次保存的更新令牌
    if (load_refresh_token())
    {
        LI() << "Loaded token from last session.";
        refresh_token();
    }

    return 0;
}


// 反初始化
void kaixin_uninitialize()
{
    LI() << "Uninitializing kaixin native SDK.";
    delete g_profile;
    g_profile = nullptr;

    delete g_config;
    g_config = nullptr;

    ix::uninitNetSystem();
}


// 登录
int kaixin_sign_in(const char *username, const char *password)
{
    if (g_config == nullptr)
    {
        return EINVAL;
    }

    LI() << "Signing in" << username;
    kaixin::string_map form{
        { "username", username },
        { "password", password }
    };

    return kaixin::send_request(ix::HttpClient::kPost, "/session", form, sign_in_handler);
}


// 注销
int kaixin_sign_out()
{
    if (g_config == nullptr)
    {
        return EINVAL;
    }

    LI() << "Signing out.";
    g_config->notify.reset();

#ifdef KAIXIN_OS_WINDOWS
    utils::delete_reg_value("kaixin::token");
    utils::delete_reg_value("kaixin::expires_at");
#endif

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

    LI() << "Getting device ID.";
    LI() << "What the fuck?!";
    kaixin::string_map form{
        { "fp", fp::generate_simple_fingerprint() },
    };

    kaixin::send_request(ix::HttpClient::kPost, "/device-id", form, [](const rapidjson::Value &data)
    {
        g_config->device_id = data.GetString();
        return 0;
    });

    LI() << "Device ID:" << g_config->device_id;
    return g_config->device_id.c_str();
}


// 获取授权
const kaixin_auth_t *kaixin_get_auth()
{
    LI() << "Getting auth.";
    kaixin_auth_t *auth = nullptr;

    kaixin::send_request(ix::HttpClient::kGet, "/auth", [&auth](const rapidjson::Value &data)
    {
        using rapidjson::get;
        auto *prev = auth;
        get(g_config->secret, data, "secret");
        g_profile->secret = g_config->secret.c_str();

        for (const auto &a : data["auth"].GetArray())
        {
            auto *p = new kaixin_auth_t;
            p->next = nullptr;
            p->module_name = strdup(get<const char *>(a, "module"));
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
void kaixin_free_auth(const kaixin_auth_t *auth)
{
    auto *p = const_cast<kaixin_auth_t *>(auth);

    while (p != nullptr)
    {
        auto next = p->next;
        free(const_cast<char *>(p->module_name));
        delete p;
        p = next;
    }
}


// 获取应用最低版本号
kaixin_version_t kaixin_get_lowest_version()
{
    kaixin_version_t lowest = { 0, 0, 0 };

    kaixin::authorization_disabler atd;
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
    LI() << "Getting material" << type;
    kaixin::string_map queries{
        { "locale", utils::get_current_locale() },
    };

    if (g_profile != nullptr)
    {
        queries.emplace("agent_code", g_profile->invitation_code);
    }
    else
    {
        queries.emplace("agent_code", utils::get_local_agent_code());
    }

    kaixin::send_request(ix::HttpClient::kGet, "/materials", queries, {}, [](const rapidjson::Value &data)
    {
        using rapidjson::get;

        for (const auto &e : data.GetArray())
        {
            auto type = get<std::string>(e, "type");
            auto text = get<std::string>(e, "text");
            g_config->materials.emplace(type, text);
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


// 下行通知
int kaixin_set_notification_callback(kaixin_notification_callback_t func, void *user_data)
{
    if (g_config == nullptr || g_config->notify)
    {
        return EINVAL;
    }

    g_config->notify = std::make_unique<websocket_client>(func, user_data);
    return 0;
}


// Shopee 域名
static std::map<std::string, std::string> to_shopee_hosts_sub_domain(const rapidjson::Value &data)
{
    std::map<std::string, std::string> hosts;

    for (auto iter = data.MemberBegin(); iter != data.MemberEnd(); ++iter)
    {
        hosts.emplace(iter->name.GetString(), iter->value.GetString());
    }

    return hosts;
}

static std::map<kaixin_shopee_hosts_by_sub_domain_t, std::map<std::string, std::string>>
to_shopee_hosts(const rapidjson::Value &data)
{
    std::map<kaixin_shopee_hosts_by_sub_domain_t, std::map<std::string, std::string>> hosts;

    hosts.emplace(KAIXIN_SHOPEE_HOSTS_BUYER, to_shopee_hosts_sub_domain(data["buyer"]));
    hosts.emplace(KAIXIN_SHOPEE_HOSTS_SELLER, to_shopee_hosts_sub_domain(data["seller"]));
    hosts.emplace(KAIXIN_SHOPEE_HOSTS_CDN, to_shopee_hosts_sub_domain(data["cdn"]));

    return hosts;
}

const char *kaixin_get_shopee_host(const char *website, kaixin_shopee_hosts_t hosts,
                                   kaixin_shopee_hosts_by_sub_domain_t sub)
{
    if (g_config == nullptr || website == nullptr)
    {
        return nullptr;
    }

    if (g_config->shopee_hosts.empty())
    {
        kaixin::send_request(ix::HttpClient::kGet, "/shopee-hosts", [](const rapidjson::Value &data)
        {
            g_config->shopee_hosts.emplace(KAIXIN_SHOPEE_HOSTS_GLOBAL, to_shopee_hosts(data["global"]));
            g_config->shopee_hosts.emplace(KAIXIN_SHOPEE_HOSTS_CHINA, to_shopee_hosts(data["china"]));
            return 0;
        });
    }

    if (g_config->shopee_hosts.count(hosts) == 0)
    {
        return nullptr;
    }

    const auto &hs = g_config->shopee_hosts.at(hosts);

    if (hs.count(sub) == 0)
    {
        return nullptr;
    }

    const auto &subs = hs.at(sub);

    if (subs.count(website) == 0)
    {
        return nullptr;
    }

    return subs.at(website).c_str();
}


const char *kaixin_get_shopee_websites()
{
    if (g_config == nullptr)
    {
        return nullptr;
    }

    kaixin_get_shopee_host("tw", KAIXIN_SHOPEE_HOSTS_GLOBAL, KAIXIN_SHOPEE_HOSTS_BUYER);

    const auto &subs = g_config->shopee_hosts.at(KAIXIN_SHOPEE_HOSTS_GLOBAL).at(KAIXIN_SHOPEE_HOSTS_BUYER);
    std::string websites;

    for (const auto &sub : subs)
    {
        websites.append(sub.first);
        websites.append(",");
    }

    auto *s = strdup(websites.c_str());
    s[websites.length() - 1] = '\0';
    return s;
}


// 获取页面地址
const char *kaixin_get_web_url(kaixin_web_page_t page)
{
    if (g_config == nullptr)
    {
        return nullptr;
    }

    LI() << "Getting URL" << page;
    kaixin::string_map queries{
       { "locale", utils::get_current_locale() },
    };

    if (g_profile != nullptr)
    {
        queries.emplace("agent_code", g_profile->invitation_code);
    }
    else
    {
        queries.emplace("agent_code", utils::get_local_agent_code());
    }

    switch (page)
    {
    case KAIXIN_WEB_PAGE_SIGN_UP:
        queries.emplace("page", "sign-up");
        break;
    case KAIXIN_WEB_PAGE_RESET_PASSWORD:
        queries.emplace("page", "reset-password");
        break;
    case KAIXIN_WEB_PAGE_CHANGE_PASSWORD:
        queries.emplace("page", "change-password");
        break;
    case KAIXIN_WEB_PAGE_BUY:
        queries.emplace("page", "buy");
        break;
    case KAIXIN_WEB_PAGE_ACTIVATE:
        queries.emplace("page", "activate");
        break;
    case KAIXIN_WEB_PAGE_BUY_OR_ACTIVATE:
        queries.emplace("page", "buy-or-activate");
        break;
    default:
        LE() << "Unknown page:" << page;
        return nullptr;
    }

    char *url = nullptr;

    kaixin::send_request(ix::HttpClient::kGet, "/web-url", queries, {}, [&url](const rapidjson::Value &data)
    {
        using rapidjson::get;
        url = strdup(data.GetString());
        return 0;
    });

    return url;
}


void kaixin_free_string(const char *s)
{
    free(const_cast<char *>(s));
}


void kaixin_log(const char *msg)
{
    kaixin::string_map form{
       { "msg", msg },
    };
    kaixin::send_request(ix::HttpClient::kPost, "/log", form);
}
