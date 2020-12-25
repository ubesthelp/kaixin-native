/*! ***********************************************************************************************
 *
 * \file        simple_timer.h
 * \brief       simple_timer 类头文件。
 *
 * \version     0.1
 * \date        2020-08-18
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include "noncopyable.h"

#include <atomic>
#include <functional>

namespace std {
class thread;
}


/*!
 * \brief       使用线程实现的简单计时器类。
 */
class simple_timer : private noncopyable
{
public:
    using timeout_callback = std::function<void()>;

    simple_timer();
    ~simple_timer();

    void set_timeout_callback(timeout_callback callback) { callback_ = callback; }
    void set_single_shot(bool on) { singleshot_ = on; }

    void start(int ms);
    void stop();

private:
    void timer_proc();

private:
    std::thread *thread_;
    timeout_callback callback_;
    std::atomic_bool interrupted_;
    int interval_;
    bool singleshot_;
};
