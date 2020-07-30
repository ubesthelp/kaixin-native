/*! ***********************************************************************************************
 *
 * \file        utils.cpp
 * \brief       工具函数源文件。
 *
 * \version     0.1
 * \date        2020-07-29
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "utils.h"

#include <openssl/rand.h>
#include <cppcodec/hex_lower.hpp>


namespace utils {


std::string generate_random_hex_string(size_t length)
{
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[length]);
    RAND_pseudo_bytes(bytes.get(), static_cast<int>(length));
    return to_hex(bytes.get(), length);
}


std::string to_hex(const uint8_t *buffer, size_t length)
{
    return cppcodec::hex_lower::encode(buffer, length);
}


}       // namespace utils
