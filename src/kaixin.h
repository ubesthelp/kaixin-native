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


#ifdef __cplusplus
}       // extern "C"
#endif
