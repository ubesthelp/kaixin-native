/*! ***********************************************************************************************
 *
 * \file        main.cpp
 * \brief       开心 C SDK 示例程序主源文件。
 *
 * \version     0.1
 * \date        2020-7-29
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "appkey.h"         // For APP_KEY, APP_SECRET
#include "kaixin.h"

using namespace std::chrono_literals;


static void kaixin_notification_callback(const kaixin_notification_arguments_t *args, void *)
{
    std::cout << "Notification action: " << args->action << std::endl;
}


static inline bool has_error(int r, const char *action)
{
    if (r != 0)
    {
        std::cerr << "Failed to " << action << ": 0x" << std::setfill('0') << std::setw(8)
            << std::right << std::hex << r << std::endl;
        return true;
    }

    return false;
}


static void log_output(const char *msg, kaixin_log_severity_t)
{
    std::cout << "[kaixin] " << msg << std::endl;
}


int main()
{
    do
    {
        kaixin_set_log_output(log_output);

        std::cout << "Initializing." << std::endl;
        auto r = kaixin_initialize(APP_ORG, APP_NAME, APP_KEY, APP_SECRET,
                                   KAIXIN_BASE_URL_FOR_TESTING);

        if (has_error(r, "initialize"))
        {
            break;
        }


        std::cout << "Getting materials." << std::endl;
        std::cout << "Bottem-left ad: " << kaixin_get_material(KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_LARGE) << std::endl;


        std::string username;
        std::string password;

        auto *profile = kaixin_get_profile();

        if (profile == nullptr || profile->username == nullptr)
        {
            std::cout << "Input username: ";
            std::cin >> username;

            std::cout << "Input password: ";
            std::cin >> password;

            std::cout << "Signing in." << std::endl;
            r = kaixin_sign_in(username.c_str(), password.c_str());

            if (has_error(r, "sign in"))
            {
                break;
            }

            std::cout << "Signed in." << std::endl;
        }
        else
        {
            std::cout << "Already signed in: " << profile->username << std::endl;
        }


        std::cout << "Getting lowest version." << std::endl;
        kaixin_version_t lowest = kaixin_get_lowest_version();
        std::cout << "Lowest version: " << lowest.major << "." << lowest.minor << "."
            << lowest.patch << std::endl;


        std::cout << "Getting device ID." << std::endl;
        std::cout << "Device ID: " << kaixin_get_device_id() << std::endl;


        std::cout << "Getting Shopee hosts." << std::endl;
        std::cout << "Shopee Taiwan: " << kaixin_get_shopee_hosts()->china.buyer.tw << std::endl;


        std::cout << "Getting buy URL." << std::endl;
        auto *url = kaixin_get_web_url(KAIXIN_WEB_PAGE_BUY_OR_ACTIVATE);
        std::cout << url << std::endl;
        kaixin_free_string(url);


        std::cout << "Getting auth." << std::endl;
        auto *auth = kaixin_get_auth();

        if (auth == nullptr)
        {
            std::cout << "No auth." << std::endl;
        }
        else
        {
            auto *p = auth;

            while (p != nullptr)
            {
                std::cout << p->module_name << ":" << std::endl;
                std::cout << "  edition: " << p->edition << std::endl;
                std::cout << "  count  : " << p->count << std::endl;
                std::cout << "  time   : " << ctime(&p->time) << std::endl;
                p = p->next;
            }

            kaixin_free_auth(auth);
        }


        // 设备授权应用在获取应用 ID 后设置通知回调
        kaixin_set_notification_callback(kaixin_notification_callback, nullptr);
        std::cout << "Enter anything to quit." << std::endl;
        std::cin >> username;


        //std::cout << "Signing out." << std::endl;
        //r = kaixin_sign_out();

        //if (r != 0)
        //{
        //    std::cerr << "Failed to sign out:" << r << std::endl;
        //}
    } while (false);

    std::cout << "Uninitializing." << std::endl;
    kaixin_uninitialize();

    return 0;
}
