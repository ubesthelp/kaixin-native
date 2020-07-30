/*! ***********************************************************************************************
 *
 * \file        jwt.h
 * \brief       JWT 函数头文件。
 *
 * \version     0.1
 * \date        2020-07-30
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include <string>


namespace jwt {


/*!
 * \brief       获取 JWT 的负载数据。
 *
 * \param[in]   id_token        JWT 字符串
 * \param[in]   app_key         APP KEY
 *
 * \return      如果签名有效、未过期且目标对象正确，则返回负载字符串；否则返回空字符串。
 */
std::string payload(const std::string &id_token, const std::string &app_key);


}       // namespace jwt
