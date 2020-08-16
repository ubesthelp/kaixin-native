/*! ***********************************************************************************************
 *
 * \file        kaixin_api.h
 * \brief       开心 API 头文件。
 *
 * \version     0.1
 * \date        2020-7-28
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "kaixin.h"

#include <ctime>
#include <functional>
#include <map>
#include <string>

#include <ixwebsocket/IXWebSocketHttpHeaders.h>
#include <rapidjson/document.h>


namespace kaixin {


/// 全局配置参数。
struct Config
{
    std::string organization;                   ///< 组织名
    std::string application;                    ///< 应用名
    std::string app_key;                        ///< APP KEY
    std::string app_secret;                     ///< APP SECRET
    std::string base_url;                       ///< 基础 URL
    std::string access_token;                   ///< 访问令牌
    std::string refresh_token;                  ///< 更新令牌
    std::string id_token;                       ///< 身份令牌
    std::string username;                       ///< 用户名
    std::string email;                          ///< Email
    std::string agent_code;                     ///< 上级代理编号
    std::string secret;                         ///< 本地对称加密密钥
    std::string device_id;                      ///< 设备 ID
    std::map<std::string, std::string> materials;       ///< 素材
    time_t access_token_expires_at = 0;         ///< 访问令牌过期时间
    time_t refresh_token_expires_at = 0;        ///< 更新令牌过期时间
    time_t id_token_expires_at = 0;             ///< 身份令牌过期时间
};


/// 字符串到字符串的映射。
using string_map = ix::WebSocketHttpHeaders;

/// 响应数据处理函数类型。
using response_data_handler = std::function<int(const rapidjson::Value &)>;


/*!
 * \brief       URL 编码。
 *
 * \param[in]   s       要编码的字符串
 *
 * \return      编码后的字符串。
 */
std::string url_encode(const std::string &s);


/*!
 * \brief       根据查询映射生成表单字符串（application/x-www-form-urlencoded）。
 *
 * \param[in]   queries         查询键值映射
 *
 * \return      表单字符串。
 */
std::string make_form(const string_map &queries);


/*!
 * \brief       生成完整 URL。
 *
 * \param[in]   base_url        基础 URL
 * \param[in]   path            路径
 * \param[in]   queries         查询键值映射
 *
 * \return      完整 URL。
 */
std::string make_url(const std::string &base_url, const std::string &path, const string_map &queries);


/*!
 * \brief       同步发送请求并返回结果。
 *
 * \param[in]   verb            请求方法，全大写
 * \param[in]   path            请求路径，以“/”开头
 * \param[in]   queries         查询映射，可以为空
 * \param[in]   form            POST 表单，可以为空
 * \param[in]   handler         响应处理函数。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
int send_request(const std::string &verb, const std::string &path, const string_map &queries,
                 const string_map &form, const response_data_handler &handler = {});


/*!
 * \brief       同步发送请求并返回结果。
 *
 * \param[in]   verb            请求方法，全大写
 * \param[in]   path            请求路径，以“/”开头
 * \param[in]   form            POST 表单，可以为空
 * \param[in]   handler         响应处理函数。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
inline int send_request(const std::string &verb, const std::string &path, const string_map &form,
                        const response_data_handler &handler = {})
{
    return send_request(verb, path, {}, form, handler);
}


/*!
 * \brief       同步发送请求并返回结果。
 *
 * \param[in]   verb            请求方法，全大写
 * \param[in]   path            请求路径，以“/”开头
 * \param[in]   handler         响应处理函数。
 *
 * \return      如果成功，则返回零；否则返回非零。
 */
inline int send_request(const std::string &verb, const std::string &path,
                        const response_data_handler &handler = {})
{
    return send_request(verb, path, {}, handler);
}


}       // namespace kaixin


extern kaixin::Config *g_config;
