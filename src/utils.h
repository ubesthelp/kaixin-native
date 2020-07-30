/*! ***********************************************************************************************
 *
 * \file        utils.h
 * \brief       工具函数头文件。
 *
 * \version     0.1
 * \date        2020-07-29
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include <chrono>
#include <sstream>
#include <string>

#include <ixwebsocket/IXWebSocketHttpHeaders.h>


/// 字符串到字符串的映射。
using string_map = ix::WebSocketHttpHeaders;


/*!
 * \brief       生成随机二进制数据。
 *
 * \param[in]   length      要生成的数据长度，以字节为单位。
 *
 * \return      以十六进制字符串表示的二进制数据，长度为 lenght * 2。
 */
std::string generate_random_hex_string(size_t length);


/*!
 * \brief       将二进制数据转换为十六进制字符串。
 *
 * \param[in]   buffer      要转换的二进制数据
 * \param[in]   length      二进制数据长度，以字节为单位。
 *
 * \return      转换后的十六进制字符串。
 */
std::string to_hex(const uint8_t *buffer, size_t length);


/*!
 * \brief       获取当时时间毫秒数。
 *
 * \return      UNIX 时间，毫秒。
 */
inline constexpr int64_t timestamp()
{
    using namespace std::chrono;
    auto ms = time_point_cast<milliseconds>(system_clock::now());
    return ms.time_since_epoch().count();
}


/*!
 * \brief       判断字符串是否为空。
 *
 * \param[in]   s   要判断的字符串
 *
 * \return      如果为空指针或空字符串，则返回 `true`；否则返回 `false`。
 */
inline constexpr bool is_empty(const char *s)
{
    return s == nullptr || strlen(s) == 0;
}


// 根据高 16 位和低 16 位拼成 32 位整数
/*!
 * \brief       根据高 16 位和低 16 位拼成 32 位整数。
 *
 * \param[in]   upper       高 16 位值
 * \param[in]   lower       低 16 位值
 *
 * \return      32 位整数。
 */
inline constexpr int make_int(int upper, int lower)
{
    return static_cast<int>((static_cast<uint32_t>(upper & 0xffff) << 16) | (lower & 0xffff));
}


/*!
 * \brief       连接字符串。
 *
 * \tparam      Range           字符串集合类型，例如 `std::vector<std::string>`。
 * \param[in]   elements        要连接的字符串集合
 * \param[in]   delimiter       用于连接字符串的分隔符
 *
 * \return      连接后的字符串。
 */
template <typename Range, typename Value = typename Range::value_type>
std::string join(Range const &elements, const char *const delimiter)
{
    std::ostringstream os;
    auto b = std::begin(elements), e = end(elements);

    if (b != e)
    {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }

    if (b != e)
    {
        os << *b;
    }

    return os.str();
}


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
