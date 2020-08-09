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
#include <iostream>
#include <string>

#include "appkey.h"         // For APP_KEY, APP_SECRET
#include "kaixin.h"


int main()
{
    do
    {
        std::cout << "Initializing." << std::endl;
        auto r = kaixin_initialize(APP_KEY, APP_SECRET, KAIXIN_BASE_URL_FOR_TESTING);

        if (r != 0)
        {
            std::cerr << "Failed to initialize:" << r << std::endl;
            break;
        }

        std::cout << "Input username: ";
        std::string username;
        std::cin >> username;

        std::cout << "Input password: ";
        std::string password;
        std::cin >> password;

        std::cout << "Signing in." << std::endl;
        r = kaixin_sign_in(username.c_str(), password.c_str());

        if (r != 0)
        {
            std::cerr << "Failed to sign in:" << r << std::endl;
            break;
        }

        std::cout << "Signed in." << std::endl;

        std::cout << "Getting device ID.";
        std::cout << "Device ID: " << kaixin_get_device_id() << std::endl;
    } while (false);

    std::cout << "Uninitializing." << std::endl;
    kaixin_uninitialize();

    return 0;
}
