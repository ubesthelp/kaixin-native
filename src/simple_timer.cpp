/*! ***********************************************************************************************
 *
 * \file        simple_timer.cpp
 * \brief       simple_timer 类源文件。
 *
 * \version     0.1
 * \date        2020-08-18
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "simple_timer.h"

#include <chrono>


simple_timer::simple_timer()
    : thread_(nullptr)
    , interval_(0)
    , interrupted_(false)
{
}


simple_timer::~simple_timer()
{
    stop();
}


void simple_timer::start(int ms)
{
    if (thread_ != nullptr)
    {
        return;
    }

    interval_ = ms;
    interrupted_ = false;
    thread_ = new std::thread(&simple_timer::timer_proc, this);
}


void simple_timer::stop()
{
    if (thread_ != nullptr)
    {
        interrupted_ = true;
        thread_->join();

        delete thread_;
        thread_ = nullptr;
    }
}


void simple_timer::timer_proc()
{
    std::chrono::milliseconds interval(interval_);
    std::chrono::milliseconds a_while(std::min(100, interval_));
    auto checkpoint = std::chrono::steady_clock::now() + interval;

    while (!interrupted_)
    {
        std::this_thread::sleep_for(a_while);

        if (std::chrono::steady_clock::now() >= checkpoint)
        {
            callback_();
            checkpoint = std::chrono::steady_clock::now() + interval;
        }
    }
}
