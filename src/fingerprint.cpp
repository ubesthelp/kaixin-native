/*! ***********************************************************************************************
 *
 * \file        fingerprint.cpp
 * \brief       fingerprint 类源文件。
 *
 * \version     1.0
 * \date        2020-08-08
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "fingerprint.h"

#include <locale>
#include <sstream>
#include <cppcodec/base64_url_unpadded.hpp>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#ifdef _WIN32
#include "wmi_client.h"
#else
#error "Unsupported platform."
#endif

namespace fp {

template<typename C>
static std::basic_string<C> &to_lower(std::basic_string<C> &s, const std::locale &loc = {})
{
    const auto &facet = std::use_facet<std::ctype<C>>(loc);
    facet.tolower(s.data(), s.data() + s.length());
    return s;
}


std::string generate_simple_fingerprint()
{
    std::ostringstream oss;
    rapidjson::OStreamWrapper buffer(oss);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(buffer);
    writer.StartObject();

#ifdef _WIN32
    wmi_client wmic;
    writer.Key("bi");
    writer.String(to_lower(wmic.query(L"BIOS", L"SerialNumber").front()));
    writer.Key("bb");
    writer.String(to_lower(wmic.query(L"BaseBoard", L"SerialNumber").front()));
    writer.Key("sm");
    writer.String(to_lower(wmic.query(L"ComputerSystemProduct", L"IdentifyingNumber").front()));
    writer.Key("ma");
    writer.String(to_lower(wmic.query(L"ComputerSystem", L"Manufacturer").front()));
    writer.Key("mo");
    writer.String(to_lower(wmic.query(L"ComputerSystem", L"Model").front()));
    writer.Key("ni");
    writer.StartArray();

    for (auto &ni : wmic.query(L"NetworkAdapter", L"MACAddress", L"PNPDeviceID like 'PCI\\\\%'"))
    {
        writer.String(to_lower(ni));
    }

    writer.EndArray();
    writer.Key("uu");
    writer.String(to_lower(wmic.query(L"ComputerSystemProduct", L"UUID").front()));
#endif

    writer.EndObject();

    return cppcodec::base64_url_unpadded::encode(oss.str());
}


}       // namespace fp
