/*! ***********************************************************************************************
 *
 * \file        noncopyable.h
 * \brief       noncopyable 类头文件。
 *
 * \version     0.1
 * \date        2020-08-18
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once


/*!
 * \brief       不可复制类。
 */
class noncopyable
{
protected:
    constexpr noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};
