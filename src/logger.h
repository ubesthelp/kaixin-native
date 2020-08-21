/*! ***********************************************************************************************
 *
 * \file        logger.h
 * \brief       logger 类头文件。
 *
 * \version     0.1
 * \date        2020-08-21
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#pragma once
#include <sstream>

#include "kaixin.h"

namespace logger {


kaixin_log_output_t set_output(kaixin_log_output_t output);

void log(const char *msg, kaixin_log_severity_t severity);

inline void debug(const char *msg) { log(msg, KAIXIN_SEVERITY_DEBUG); }
inline void info(const char *msg) { log(msg, KAIXIN_SEVERITY_INFO); }
inline void warning(const char *msg) { log(msg, KAIXIN_SEVERITY_WARNING); }
inline void error(const char *msg) { log(msg, KAIXIN_SEVERITY_ERROR); }
inline void critical(const char *msg) { log(msg, KAIXIN_SEVERITY_CRITICAL); }


class logger
{
public:
    explicit logger(kaixin_log_severity_t severity) : severity_(severity) { }
    
    ~logger()
    {
        auto msg = oss_.str();

        if (!msg.empty())
        {
            msg.resize(msg.size() - 1);
            log(msg.c_str(), severity_);
        }
    }

    template<typename T>
    logger & operator<<(T value)
    {
        oss_ << value << " ";
        return *this;
    }

private:
    kaixin_log_severity_t severity_;
    std::ostringstream oss_;
};


}       // namespace logger


#define LD()    ::logger::logger(KAIXIN_SEVERITY_DEBUG)
#define LI()    ::logger::logger(KAIXIN_SEVERITY_INFO)
#define LW()    ::logger::logger(KAIXIN_SEVERITY_WARNING)
#define LE()    ::logger::logger(KAIXIN_SEVERITY_ERROR)
#define LC()    ::logger::logger(KAIXIN_SEVERITY_CRITICAL)
