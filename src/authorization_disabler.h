/*! ***********************************************************************************************
 *
 * \file        authorization_disabler.h
 * \brief       AuthorizationDisabler 类头文件。
 *
 * \version     0.1
 * \date        2020-08-14
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "kaixin_api.h"

namespace kaixin {


class AuthorizationDisabler
{
public:
    AuthorizationDisabler()
        : aExp_(g_config->access_token_expires_at)
        , idExp_(g_config->id_token_expires_at)
    {
        g_config->access_token_expires_at = 0;
        g_config->id_token_expires_at = 0;
    }

    ~AuthorizationDisabler()
    {
        g_config->access_token_expires_at = aExp_;
        g_config->id_token_expires_at = idExp_;
    }

    AuthorizationDisabler(const AuthorizationDisabler &) = delete;
    AuthorizationDisabler(AuthorizationDisabler &&) = delete;
    AuthorizationDisabler &operator=(const AuthorizationDisabler &) = delete;
    AuthorizationDisabler &operator=(AuthorizationDisabler &&) = delete;

private:
    const time_t aExp_;
    const time_t idExp_;
};


}       // namespace kaixin
