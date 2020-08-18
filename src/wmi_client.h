/*! ***********************************************************************************************
 *
 * \file        wmi_client.h
 * \brief       wmi_client 类头文件。
 *
 * \version     1.0
 * \date        2020-08-08
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "noncopyable.h"

#include <string>
#include <vector>

#ifndef _WIN32
#error "Only support on Windows."
#endif

class wmi_client_private;


/// WMI 客户端类。
class wmi_client : private noncopyable
{
public:
    wmi_client();
    ~wmi_client();

    /*!
     * \brief       查询 WMI。
     *
     * \param[in]   cls     类
     * \param[in]   field   字段
     * \param[in]   cond    过滤条件
     *
     * \return      查询结果。
     */
    std::vector<std::string> query(const wchar_t *cls, const wchar_t *field,
                                   const wchar_t *cond = nullptr) const;

private:
    wmi_client_private *d;
};
