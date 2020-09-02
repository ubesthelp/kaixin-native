/*! ***********************************************************************************************
 *
 * \file        logger.cpp
 * \brief       logger 类源文件。
 *
 * \version     0.1
 * \date        2020-08-21
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "logger.h"


namespace logger {


static kaixin_log_output_t g_output = nullptr;


kaixin_log_output_t set_output(kaixin_log_output_t output)
{
    auto old = g_output;
    g_output = output;
    return old;
}


void log(const char *msg, kaixin_log_severity_t severity)
{
    if (g_output != nullptr)
    {
        g_output(msg, severity);
    }
}


}       // namespace logger
