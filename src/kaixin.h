﻿/*! ***********************************************************************************************
 *
 * \file        kaixin.h
 * \brief       开心 C SDK 头文件。
 *
 * \version     0.1
 * \date        2020-7-28
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "kaixin_export.h"

#include <stdint.h>
#include <time.h>


// 用于测试的基础 URL。
#define KAIXIN_BASE_URL_FOR_TESTING     "https://api-v3-test.hlwcr.cn"

// 素材类型 - 左下角广告大图片 200x300
#define KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_LARGE   "ad-bottomleft-img-l"
// 素材类型 - 左下角广告小图片 140x100
#define KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_SMALL   "ad-bottomleft-img-s"
// 素材类型 - 左下角广告链接
#define KAIXIN_MATERIAL_AD_BOTTOMLEFT_TARGET        "ad-bottomleft-target"
// 素材类型 - 登录对话框头图
#define KAIXIN_MATERIAL_SIGN_HEADER_IMAGE           "sign-header-img"
// 素材类型 - 登录对话框头图链接
#define KAIXIN_MATERIAL_SIGN_HEADER_TARGET          "sign-header-target"
// 素材类型 - 常见问题链接
#define KAIXIN_MATERIAL_FAQ                         "faq"
// 素材类型 - 提交工单链接
#define KAIXIN_MATERIAL_WORKORDER                   "workorder"
// 素材类型 - 联系我们链接
#define KAIXIN_MATERIAL_CONTACTUS                   "contactus"
// 素材类型 - 下载最新版本链接
#define KAIXIN_MATERIAL_DOWNLOAD                    "download"
// 素材类型 - 导入模板链接
#define KAIXIN_MATERIAL_IMPORT_TEMPLATES            "import-templates"
// 素材类型 - 访问官网
#define KAIXIN_MATERIAL_HOMEPAGE                    "homepage"

// 下行通知动作 - 更新授权
#define KAIXIN_ACTION_REFRESH_AUTH                  "refresh-auth"
// 下行通知动作 - 注销
#define KAIXIN_ACTION_SIGN_OUT                      "sign-out"


#ifdef __cplusplus
extern "C" {
#endif


/// \brief      用户状态。
typedef enum kaixin_user_status_e
{
    KAIXIN_INVALID_USER = 0,                    ///< 无效用户
    KAIXIN_PASSWORD_CONFIRMED = 2               ///< 用户密码已确认
} kaixin_user_status_t;


/// \brief      日志严重性等级。
typedef enum kaixin_log_severity_e
{
    KAIXIN_SEVERITY_DEBUG,
    KAIXIN_SEVERITY_INFO,
    KAIXIN_SEVERITY_WARNING,
    KAIXIN_SEVERITY_ERROR,
    KAIXIN_SEVERITY_CRITICAL,
} kaixin_log_severity_t;


/// \brief      功能页面。
typedef enum kaixin_web_page_e
{
    KAIXIN_WEB_PAGE_SIGN_UP,                    ///< 注册页
    KAIXIN_WEB_PAGE_RESET_PASSWORD,             ///< 重置密码页
    KAIXIN_WEB_PAGE_CHANGE_PASSWORD,            ///< 修改密码页
    KAIXIN_WEB_PAGE_BUY,                        ///< 购买页
    KAIXIN_WEB_PAGE_ACTIVATE,                   ///< 激活页
    KAIXIN_WEB_PAGE_BUY_OR_ACTIVATE,            ///< 购买或激活页
} kaixin_web_page_t;


/// \brief      用户配置，登录成功后获取。
typedef struct kaixin_profile_s
{
    const char *access_token;                   ///< 访问令牌
    const char *refresh_token;                  ///< 更新令牌，需要保存到本地
    const char *id_token;                       ///< 身份令牌，需要保存到本地，JWT 格式
    const char *username;                       ///< 用户名
    const char *email;                          ///< 电子邮箱
    const char *invitation_code;                ///< 邀请码（上级代理编号）
    const char *secret;                         ///< 本地对称加密密钥，base64 编码二进制数据
    time_t access_token_expires_at;             ///< 访问令牌过期时间
    time_t refresh_token_expires_at;            ///< 更新令牌过期时间
    time_t id_token_expires_at;                 ///< 身份令牌过期时间
    kaixin_user_status_t status;                ///< 用户状态
} kaixin_profile_t;


/// \brief      版本号。
typedef struct kaixin_version_s
{
    int major;                                  ///< 主版本号
    int minor;                                  ///< 次版本号
    int patch;                                  ///< 修订号
} kaixin_version_t;


/// \brief      应用授权。
typedef struct kaixin_auth_s
{
    struct kaixin_auth_s *next;                 ///< 下一个授权；如果没有下个授权，则为 `NULL`
    const char *module_name;                    ///< 模块名称
    uint32_t edition;                           ///< 版本
    uint32_t count;                             ///< 数量
    time_t time;                                ///< 过期时间
} kaixin_auth_t;


/// \brief      下行通知参数。
typedef struct kaixin_notification_arguments_s
{
    /*!
     * \brief   客户端需要采取的动作
     * \sa      `KAIXIN_ACTION_REFRESH_AUTH`
     * \sa      `KAIXIN_ACTION_SIGN_OUT`
     */
    const char *action;
} kaixin_notification_arguments_t;


/// \brief      Shopee 子域名。
typedef enum kaixin_shopee_hosts_by_sub_domain_e
{
    KAIXIN_SHOPEE_HOSTS_BUYER,                  ///< 买家子域名
    KAIXIN_SHOPEE_HOSTS_SELLER,                 ///< 卖家子域名
    KAIXIN_SHOPEE_HOSTS_CDN,                    ///< CDN 子域名
} kaixin_shopee_hosts_by_sub_domain_t;


/// \brief      Shopee 域名。
typedef enum kaixin_shopee_hosts_e
{
    KAIXIN_SHOPEE_HOSTS_GLOBAL,                 ///< 全球通用域名
    KAIXIN_SHOPEE_HOSTS_CHINA,                  ///< 中国大陆用域名
} kaixin_shopee_hosts_t;


/// 下行通知回调函数
typedef void(*kaixin_notification_callback_t)(const kaixin_notification_arguments_t *args, void *user_data);

/// 日志输出函数
typedef void(*kaixin_log_output_t)(const char *msg, kaixin_log_severity_t severity);


/*!
 * \brief       获取开心 C SDK 版本号。
 */
KAIXIN_EXPORT const char * kaixin_version();


/*!
 * \brief       设置日志输出函数。
 *
 * \param[in]   output          新的日志输出函数
 *
 * \return      上一个日志输出函数；如果没有，则返回 `NULL`。
 */
KAIXIN_EXPORT kaixin_log_output_t kaixin_set_log_output(kaixin_log_output_t output);


/*!
 * \brief       初始化开心 SDK。在调用其它 API 前必须调用此函数。
 *
 * \param[in]   organization    组织名，用于读写配置文件
 * \param[in]   application     应用名，用于读写配置文件
 * \param[in]   app_key         APP KEY
 * \param[in]   app_secret      APP SECRET
 * \param[in]   base_url        基础 URL。如果设置为 NULL，则使用默认设置（生产环境）；开发及测试时请设置
 *                              为测试环境 URL。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_EXPORT int kaixin_initialize(const char *organization, const char *application,
                                    const char *app_key, const char *app_secret, const char *base_url);



/*!
 * \brief       反初始化开心 SDK。
 */
KAIXIN_EXPORT void kaixin_uninitialize();


/*!
 * \brief       登录。
 *
 * \param[in]   username        用户名
 * \param[in]   password        密码，明文
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_EXPORT int kaixin_sign_in(const char *username, const char *password);


/*!
 * \brief       注销。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_EXPORT int kaixin_sign_out();



/*!
 * \brief       获取用户配置。
 *
 * \return      如果登录成功，则返回用户配置文件；否则返回 `NULL`。
 */
KAIXIN_EXPORT const kaixin_profile_t *kaixin_get_profile();


/*!
 * \brief       获取设备 ID。
 *
 * \return      如果成功，则返回设备 ID；否则返回 `NULL`。
 */
KAIXIN_EXPORT const char *kaixin_get_device_id();


/*!
 * \brief       获取应用授权。
 *
 * \return      应用授权；如果没有授权，则返回 `NULL`。返回的指针需要调用 `kaixin_free_auth` 函数释放。
 */
KAIXIN_EXPORT const kaixin_auth_t *kaixin_get_auth();


/*!
 * \brief       释放授权链表。
 *
 * \param[in]   auth        要释放的链表。
 */
KAIXIN_EXPORT void kaixin_free_auth(const kaixin_auth_t *auth);


/*!
 * \brief       获取应用最低版本号。
 *
 * 如果应用当前版本号低于此版本号，则可能无法正常运行。此时应强制启动升级过程。
 *
 * \note        此函数可在登录前调用。
 * \return      应用最低版本号。
 */
KAIXIN_EXPORT kaixin_version_t kaixin_get_lowest_version();


/*!
 * \brief       获取素材。
 *
 * \param[in]   type            素材类型
 *
 * \return      素材内容，或 `NULL`。
 *
 * \sa          `KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_LARGE`
 * \sa          `KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_SMALL`
 * \sa          `KAIXIN_MATERIAL_AD_BOTTOMLEFT_TARGET`
 * \sa          `KAIXIN_MATERIAL_SIGN_HEADER_IMAGE`
 * \sa          `KAIXIN_MATERIAL_SIGN_HEADER_TARGET`
 * \sa          `KAIXIN_MATERIAL_FAQ`
 * \sa          `KAIXIN_MATERIAL_WORKORDER`
 * \sa          `KAIXIN_MATERIAL_CONTACTUS`
 * \sa          `KAIXIN_MATERIAL_DOWNLOAD`
 * \sa          `KAIXIN_MATERIAL_IMPORT_TEMPLATES`
 * \sa          `KAIXIN_MATERIAL_HOMEPAGE`
 */
KAIXIN_EXPORT const char *kaixin_get_material(const char *type);


/*!
 * \brief       设置下行通知回调函数。
 *
 * \param[in]   func        下行通知到达时要调用的函数
 * \param[in]   user_data   用户数据，用于 `func` 最后一个参数
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_EXPORT int kaixin_set_notification_callback(kaixin_notification_callback_t func,
                                                   void *user_data);


/*!
 * \brief       获取 Shopee 域名。
 *
 * \param[in]   website     站点，2 小写字母
 * \param[in]   hosts       全球/中国
 * \param[in]   sub         子域名
 *
 * \return      Shopee 域名，或 `NULL`。返回的字符串不需要释放。
 *
 * \sa          `KAIXIN_SHOPEE_HOSTS_GLOBAL`
 * \sa          `KAIXIN_SHOPEE_HOSTS_CHINA`
 * \sa          `KAIXIN_SHOPEE_HOSTS_BUYER`
 * \sa          `KAIXIN_SHOPEE_HOSTS_SELLER`
 * \sa          `KAIXIN_SHOPEE_HOSTS_CDN`
 */
KAIXIN_EXPORT const char *kaixin_get_shopee_host(const char *website, kaixin_shopee_hosts_t hosts,
                                                  kaixin_shopee_hosts_by_sub_domain_t sub);


/*!
 * \brief       获取 Shopee 站点列表。
 *
 * \return      Shopee 站点列表，每个站点使用 2 小写字母表示，以半角逗号分隔；或 `NULL`。返回的字符串需要释放。
 *
 * \sa          `kaixin_free_string`
 */
KAIXIN_EXPORT const char *kaixin_get_shopee_websites();


/*!
 * \brief       获取功能页面地址。
 *
 * \param[in]   page        要获取地址的页面。
 *
 * \return      页面地址，不再使用时须调用 `kaixin_free_string` 释放。
 *
 * \sa          `kaixin_free_string`
 */
KAIXIN_EXPORT const char *kaixin_get_web_url(kaixin_web_page_t page);


/*!
 * \brief       释放字符串。
 *
 * \param[in]   s           要释放的字符串。
 */
KAIXIN_EXPORT void kaixin_free_string(const char *s);


/*!
 * \brief       向服务端记录日志。
 *
 * \param[in]   msg         要记录的字符串。
 */
KAIXIN_EXPORT void kaixin_log(const char *msg);


/*!
 * \brief       获取当前时间，UNIX Epoch。
 */
KAIXIN_EXPORT time_t kaixin_get_current_time();


#ifdef __cplusplus
}       // extern "C"
#endif
