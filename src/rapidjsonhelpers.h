/*! ***********************************************************************************************
 *
 * \file        rapidjsonhelpers.h
 * \brief       rapidjson 辅助函数头文件。
 *
 * \version     0.1
 * \date        2020-7-28
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include <rapidjson/document.h>


RAPIDJSON_NAMESPACE_BEGIN


struct ValueTypeConverter
{
    ValueTypeConverter(const Value &v) : value(v) { }
    const Value &value;

    template<typename T>
    operator T() const
    {
        if (value.Is<T>())
        {
            return value.Get<T>();
        }

        return T();
    }

    operator std::string() const
    {
        if (value.IsString())
        {
            return std::string(value.GetString(), value.GetStringLength());
        }

        return {};
    }
};


#pragma region 按名称获取 JSON 对象的字段

template<typename T>
inline typename std::enable_if_t<!std::is_enum_v<T>, bool> get(T &t, const Value &j, const char *name)
{
    if (j.IsObject())
    {
        auto iter = j.FindMember(name);

        if (iter != j.MemberEnd() && iter->value.Is<T>())
        {
            t = iter->value.Get<T>();
            return true;
        }
    }

    return false;
}


template<typename T>
inline typename std::enable_if_t<std::is_enum_v<T>, bool> get(T &t, const Value &j, const char *name)
{
    return get(reinterpret_cast<std::underlying_type_t<T> &>(t), j, name);
}


template<>
inline bool get(double &t, const Value &j, const char *name)
{
    if (j.IsObject())
    {
        auto iter = j.FindMember(name);

        if (iter != j.MemberEnd())
        {
            if (iter->value.IsLosslessDouble())
            {
                t = iter->value.GetDouble();
                return true;
            }

            if (iter->value.IsString())
            {
                t = std::strtod(iter->value.GetString(), nullptr);
                return true;
            }
        }
    }

    return false;
}


template<>
inline bool get(float &t, const Value &j, const char *name)
{
    if (j.IsObject())
    {
        auto iter = j.FindMember(name);

        if (iter != j.MemberEnd())
        {
            if (iter->value.IsLosslessFloat())
            {
                t = iter->value.GetFloat();
                return true;
            }

            if (iter->value.IsString())
            {
                t = std::strtof(iter->value.GetString(), nullptr);
                return true;
            }
        }
    }

    return false;
}


template<typename T>
inline typename T get(const Value &j, const char *name)
{
    T value = {};
    get(value, j, name);
    return value;
}

inline ValueTypeConverter get(const Value &j, const char *name)
{
    return ValueTypeConverter(j[name]);
}

#pragma endregion


RAPIDJSON_NAMESPACE_END
