/*! ***********************************************************************************************
 *
 * \file        fingerprint.h
 * \brief       fingerprint 类头文件。
 *
 * \version     1.0
 * \date        2020-08-08
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include <string>

namespace fp {


/*!
 * \brief       生成简单设备指纹。
 * \return      简单设备指纹字符串。
 */
std::string generate_simple_fingerprint();


}       // namespace fp
