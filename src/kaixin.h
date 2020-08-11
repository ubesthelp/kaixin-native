/*! ***********************************************************************************************
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
#include <time.h>

#ifdef KAIXIN_EXPORTS
#ifdef _MSC_VER
#define KAIXIN_API      __declspec(dllexport)
#else
#define KAIXIN_API      __attribute__((__visibility__("default")))
#endif
#else
#ifdef _MSC_VER
#define KAIXIN_API      __declspec(dllimport)
#else
#define KAIXIN_API
#endif
#endif

/// 用于测试的基础 URL。
#define KAIXIN_BASE_URL_FOR_TESTING     "https://api-v3-test.hlwcr.cn"

#ifdef __cplusplus
extern "C" {
#endif


/// \brief      用户状态。
typedef enum kaixin_user_status_e
{
    KAIXIN_INVALID_USER = 0,                    ///< 无效用户
    KAIXIN_PASSWORD_CONFIRMED = 2               ///< 用户密码已确认
} kaixin_user_status_t;


/// \brief      用户配置，登录成功后获取。
typedef struct kaixin_profile_s
{
    const char *access_token;                   ///< 访问令牌
    const char *refresh_token;                  ///< 更新令牌，需要保存到本地
    const char *id_token;                       ///< 身份令牌，需要保存到本地，JWT 格式
    const char *username;                       ///< 用户名
    const char *email;                          ///< 电子邮箱
    const char *invitation_code;                ///< 邀请码（上级代理编号）
    const char *secret;                         ///< 本地对称加密密钥
    time_t access_token_expires_at;             ///< 访问令牌过期时间
    time_t refresh_token_expires_at;            ///< 更新令牌过期时间
    time_t id_token_expires_at;                 ///< 身份令牌过期时间
    kaixin_user_status_t status;                ///< 用户状态
} kaixin_profile_t;


/*!
 * \brief       初始化开心 SDK。在调用其它 API 前必须调用此函数。
 *
 * \param[in]   app_key         APP KEY
 * \param[in]   app_secret      APP SECRET
 * \param[in]   base_url        基础 URL。如果设置为 NULL，则使用默认设置（生产环境）；开发及测试时请设置
 *                              为测试环境 URL。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_API int kaixin_initialize(const char *app_key, const char *app_secret, const char *base_url);



/*!
 * \brief       反初始化开心 SDK。
 */
KAIXIN_API void kaixin_uninitialize();


/*!
 * \brief       登录。
 *
 * \param[in]   username        用户名
 * \param[in]   password        密码，明文
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_API int kaixin_sign_in(const char *username, const char *password);


/*!
 * \brief       注销。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
KAIXIN_API int kaixin_sign_out();


/*!
 * \brief       获取用户配置。
 *
 * \return      如果登录成功，则返回用户配置文件；否则返回 `NULL`。
 */
KAIXIN_API const kaixin_profile_t *kaixin_get_profile();


/*!
 * \brief       获取设备 ID。
 *
 * \return      如果成功，则返回设备 ID；否则返回 `NULL`。
 */
KAIXIN_API const char *kaixin_get_device_id();


#ifdef __cplusplus
}       // extern "C"
#endif
