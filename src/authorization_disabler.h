/*! ***********************************************************************************************
 *
 * \file        authorization_disabler.h
 * \brief       authorization_disabler 类头文件。
 *
 * \version     0.1
 * \date        2020-08-14
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "noncopyable.h"
#include "kaixin_api.h"

namespace kaixin {


class authorization_disabler : private noncopyable
{
public:
    authorization_disabler()
        : aExp_(g_config->access_token_expires_at)
        , idExp_(g_config->id_token_expires_at)
    {
        g_config->access_token_expires_at = 0;
        g_config->id_token_expires_at = 0;
    }

    ~authorization_disabler()
    {
        g_config->access_token_expires_at = aExp_;
        g_config->id_token_expires_at = idExp_;
    }

private:
    const time_t aExp_;
    const time_t idExp_;
};


}       // namespace kaixin
