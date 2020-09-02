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

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
// Windows 平台
#define KAIXIN_OS_WINDOWS
#include <variant>
#else
// 其它平台没测试过，暂不支持。
#error "Unsupported platform."
#endif

#if defined(_MSC_VER)
#define KAIXIN_COMP_MSVC
#elif defined(__clang__)
#define KAIXIN_COMP_CLANG
#elif defined(__GNUC__)
#define KAIXIN_COMP_GNUC
#endif


namespace utils {


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
 * \brief       获取当时时间数。
 *
 * \tparam      U           时间单位，默认为秒
 *
 * \return      UNIX 时间。
 */
template<typename U = std::chrono::seconds>
inline constexpr int64_t get_timestamp()
{
    using namespace std::chrono;
    auto t = time_point_cast<U>(system_clock::now());
    return t.time_since_epoch().count();
}


/*!
 * \brief       获取当时时间毫秒数。
 *
 * \return      UNIX 时间，毫秒。
 */
inline constexpr int64_t get_timestamp_ms()
{
    return get_timestamp<std::chrono::milliseconds>();
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
 * \brief       获取当前区域语言。
 *
 * \return      BCP 47 格式区域语言，以半角下划线“_”分隔。
 */
const std::string &get_current_locale();


/*!
 * \brief       获取本地代理编号。
 *
 * \return      本地代理编号。
 */
std::string get_local_agent_code();


#ifdef KAIXIN_OS_WINDOWS
/*!
 * \brief       将宽字符串转换为 UTF-8 编码窄字符串。
 *
 * \param[in]   wide        要转换的宽字符串
 *
 * \return      转换后的 UTF-8 窄字符串。
 */
std::string to_narrow(const std::wstring &wide);


/*!
 * \brief       将 UTF-8 编码窄字符串转换为宽字符串。
 *
 * \param[in]   wide        要转换的 UTF-8 编码窄字符串
 *
 * \return      转换后宽字符串。
 */
std::wstring to_wide(const std::string &narrow);


// 注册表值
using reg_value = std::variant<std::string, std::vector<uint8_t>, uint32_t, int64_t>;


/*!
 * \brief       获取注册表值。
 *
 * \param[in]   value_name      值名称
 * \param[in]   default_value   当值不存在时要返回的默认值
 *
 * \return      值或默认值。
 */
reg_value get_reg_value(const std::string &value_name, const reg_value &default_value = {});


/*!
 * \brief       获取注册表值。
 *
 * \tparam      T               值类型
 * \param[in]   value_name      值名称
 * \param[in]   default_value   当值不存在时要返回的默认值
 *
 * \return      值或默认值。
 */
template<typename T>
inline T get_reg_type_value(const std::string &value_name, const T &default_value = {})
{
    auto value = get_reg_value(value_name, default_value);
    return std::get<T>(value);
}


/*!
 * \brief       设置注册表值。
 *
 * \param[in]   value_name      值名称
 * \param[in]   value           值
 *
 * \return      如果设置成功，则返回 `true`；否则返回 `false`。
 */
bool set_reg_value(const std::string &value_name, const reg_value &value);


/*!
 * \brief       设置注册表值。
 *
 * \param[in]   value_name      值名称
 *
 * \return      如果删除成功，则返回 `true`；否则返回 `false`。
 */
bool delete_reg_value(const std::string &value_name);


/*!
 * \brief       加密字符串。
 *
 * \param[in]   data            要加密的字符串
 *
 * \return      字符串加密后的二进制数据。
 */
std::vector<uint8_t> protect_data(const std::string &data);


/*!
 * \brief       解密字符串。
 *
 * \param[in]   data            要解密的二进制数据
 *
 * \return      解密后的字符串。
 */
std::string unprotect_data(const std::vector<uint8_t> &data);
#endif


}       // namespace utils
