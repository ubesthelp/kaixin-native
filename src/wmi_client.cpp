/*! ***********************************************************************************************
 *
 * \file        wmi_client.cpp
 * \brief       wmi_client 类源文件。
 *
 * \version     1.0
 * \date        2020-08-08
 *
 * \author      Roy QIU <karoyqiu@gmail.com>
 * \copyright   © 2020 开心网络。
 *
 **************************************************************************************************/
#include "wmi_client.h"

#define _WIN32_DCOM
#include <comdef.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <codecvt>
#include <locale>

 // vaCI:skip+*:nullptr
#include "logger.h"
#include "utils.h"

#define BREAK_ON_FAILURE(hr)    if (FAILED(hr)) { \
    _com_error err(hr); \
    LE() << "Failed at" << __LINE__ << hr << err.ErrorMessage(); \
    break; \
}

_COM_SMARTPTR_TYPEDEF(IWbemLocator, __uuidof(IWbemLocator));
_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IEnumWbemClassObject, __uuidof(IEnumWbemClassObject));


class wmi_client_private
{
private:
    friend class wmi_client;

    IWbemLocatorPtr loc;
    IWbemServicesPtr svc;
    bool inited = false;
};


wmi_client::wmi_client()
    : d(new wmi_client_private)
{
    do
    {
        HRESULT hr = CoInitialize(NULL);
        BREAK_ON_FAILURE(hr);

        hr = d->loc.CreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER);
        BREAK_ON_FAILURE(hr);

        hr = d->loc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, NULL, 0, NULL, NULL, &d->svc);
        BREAK_ON_FAILURE(hr);

        hr = CoSetProxyBlanket(d->svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL,
                               RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        BREAK_ON_FAILURE(hr);

        d->inited = true;
    } while (false);
}


wmi_client::~wmi_client()
{
    auto inited = d->inited;
    delete d;

    if (inited)
    {
        CoUninitialize();
    }
}


std::vector<std::string> wmi_client::query(const wchar_t *cls, const wchar_t *field,
                                           const wchar_t *cond /*= nullptr*/) const
{
    assert(cls != nullptr && field != nullptr);
    std::vector<std::string> result;

    do
    {
        std::wstring q(L"SELECT * FROM Win32_");
        q += cls;

        if (cond != nullptr && wcslen(cond) > 0)
        {
            q += L" WHERE ";
            q += cond;
        }

        IEnumWbemClassObjectPtr spEnum;
        HRESULT hr = d->svc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(q.c_str()), WBEM_FLAG_FORWARD_ONLY,
                                       NULL, &spEnum);
        BREAK_ON_FAILURE(hr);

        for (;;)
        {
            IWbemClassObjectPtr spObj;
            ULONG nReturned = 0;
            hr = spEnum->Next(0, 1, &spObj, &nReturned);
            BREAK_ON_FAILURE(hr);

            if (nReturned == 0)
            {
                break;
            }

            _variant_t value;
            CIMTYPE type = 0;
            hr = spObj->Get(field, 0, &value, &type, NULL);
            BREAK_ON_FAILURE(hr);

            switch (value.vt)
            {
            case VT_BSTR:
                result.push_back(utils::to_narrow(value.bstrVal));
                break;
            case VT_I4:
                result.push_back(std::to_string(value.lVal));
                break;
            default:
                break;
            }
        }
    } while (false);

    if (result.empty())
    {
        result.emplace_back("0");
        LW() << "Empty result." << result.size();
    }

    return result;
}
