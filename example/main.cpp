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

#ifdef KAIXIN_BUILD_INTERNAL_API
#include "kaixin_internal.h"
#endif

using namespace std::chrono_literals;


static void kaixin_notification_callback(const kaixin_notification_arguments_t *args)
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


int main()
{
    do
    {
        std::cout << "Initializing." << std::endl;
        auto r = kaixin_initialize(APP_ORG, APP_NAME, APP_KEY, APP_SECRET,
                                   KAIXIN_BASE_URL_FOR_TESTING);

        if (has_error(r, "initialize"))
        {
            break;
        }


        //std::cout << "Getting materials." << std::endl;
        //std::cout << "Bottem-left ad: " << kaixin_get_material(KAIXIN_MATERIAL_AD_BOTTOMLEFT_IMAGE_LARGE) << std::endl;


        std::string username;
        std::string password;

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


        std::cout << "Getting lowest version." << std::endl;
        kaixin_version_t lowest = kaixin_get_lowest_version();
        std::cout << "Lowest version: " << lowest.major << "." << lowest.minor << "."
            << lowest.patch << std::endl;


        std::cout << "Getting device ID." << std::endl;
        std::cout << "Device ID: " << kaixin_get_device_id() << std::endl;


        // 设备授权应用在获取应用 ID 后设置通知回调
        kaixin_set_notification_callback(kaixin_notification_callback);
        std::this_thread::sleep_for(5s);


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
                std::cout << "  time   : " << p->time << std::endl;
                p = p->next;
            }

            kaixin_free_auth(auth);
        }


#ifdef KAIXIN_BUILD_INTERNAL_API
        //std::cout << "Change password." << std::endl;
        //std::cout << "Input new password: ";
        //std::cin >> username;

        //if (!username.empty())
        //{
        //    r = kaixin::itnl::change_password(password, username);

        //    if (r != 0)
        //    {
        //        std::cerr << "Failed to change password: " << r << std::endl;
        //        break;
        //    }
        //}


        //std::cout << "Set password: ";
        //std::cin >> password;

        //if (!password.empty())
        //{
        //    r = kaixin::itnl::set_password(password);

        //    if (r != 0)
        //    {
        //        std::cerr << "Failed to set password: " << r << std::endl;
        //        break;
        //    }
        //}


        //std::cout << "Send verification email." << std::endl;
        //std::cout << "To: ";
        //std::cin >> username;
        //std::cout << "Locale: ";
        //std::cin >> password;

        //if (!username.empty())
        //{
        //    r = kaixin::itnl::send_vcode_email(username, password);

        //    if (r != 0)
        //    {
        //        std::cerr << "Failed to send verification email: " << r << std::endl;
        //        break;
        //    }
        //}


        //std::cout << "Send reset password email." << std::endl;
        //std::string email, link;
        //std::cout << "To: ";
        //std::cin >> email;
        //std::cout << "Locale: ";
        //std::cin >> password;
        //std::cout << "Username: ";
        //std::cin >> username;
        //std::cout << "Link: ";
        //std::cin >> link;

        //if (!email.empty())
        //{
        //    r = kaixin::itnl::send_reset_password_email(email, { username, username }, { link, link }, password);

        //    if (r != 0)
        //    {
        //        std::cerr << "Failed to send reset password email: " << r << std::endl;
        //        break;
        //    }
        //}


        //std::cout << "Activate." << std::endl;
        //std::cout << "SN: ";
        //std::cin >> username;

        //if (!username.empty())
        //{
        //    r = kaixin::itnl::activate({ username });
        //    has_error(r, "activate");
        //}


        //std::cout << "Get payment URL." << std::endl;
        //std::cout << "Order number: ";
        //std::cin >> username;
        //std::cout << "Channel: ";
        //std::cin >> password;
        //std::cout << kaixin::itnl::get_payment_url(username, password) << std::endl;
#endif


        std::cout << "Signing out." << std::endl;
        r = kaixin_sign_out();

        if (r != 0)
        {
            std::cerr << "Failed to sign out:" << r << std::endl;
        }
    } while (false);

    std::cout << "Uninitializing." << std::endl;
    kaixin_uninitialize();

    return 0;
}
