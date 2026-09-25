// Copyright (c) 2025 Project Nova LLC

#include "Url.h"
#include "Library.h"
#include "Util.h"
#include "Dobby/dobby.h"
#include "Includes/curl.h"
#include <dlfcn.h>
#include <string>

#define URL_PROTOCOL_HTTP _("http")
#define URL_HOST _("127.0.0.1:3552")
#define URL_PORT std::string()
install_hook_name(curl_easy_setopt_unreal, void*, void* curl, int option, void* arg)
{
    if (!Util::IsPointerBad(arg) && option == CURLOPT_URL)
    {
        std::string url = reinterpret_cast<char*>(arg);
        Uri uri = Uri::Parse(url);

        if (uri.Host.ends_with(_("ol.epicgames.com"))
            || uri.Host.ends_with(_(".akamaized.net"))
            || uri.Host.ends_with(_("on.epicgames.com")))
        {
            // استخدام thread_local لضمان بقاء السلسلة النصية قائمة في الذاكرة أثناء تنفيذ الطلب
            static thread_local std::string redirected_url_unreal;
            redirected_url_unreal = Uri::CreateUri(URL_PROTOCOL_HTTP, URL_HOST, URL_PORT, uri.Path, uri.QueryString);
            
            return orig_curl_easy_setopt_unreal(curl, option, (void*)redirected_url_unreal.c_str());
        }

        return orig_curl_easy_setopt_unreal(curl, option, (void*)url.c_str());
    }
    else if (option == CURLOPT_SSL_VERIFYPEER)
    {
        return orig_curl_easy_setopt_unreal(curl, option, (void*)0);
    }

    return orig_curl_easy_setopt_unreal(curl, option, arg);
}
install_hook_name(curl_easy_setopt_eos, void*, void* curl, int option, void* arg)
{
    if (!Util::IsPointerBad(arg) && option == CURLOPT_URL)
    {
        std::string url = reinterpret_cast<char*>(arg);
        Uri uri = Uri::Parse(url);

        if (uri.Host.ends_with(_("ol.epicgames.com"))
            || uri.Host.ends_with(_(".akamaized.net"))
            || uri.Host.ends_with(_("on.epicgames.com")))
        {
            static thread_local std::string redirected_url_eos;
            redirected_url_eos = Uri::CreateUri(URL_PROTOCOL_HTTP, URL_HOST, URL_PORT, uri.Path, uri.QueryString);
            
            return orig_curl_easy_setopt_eos(curl, option, (void*)redirected_url_eos.c_str());
        }

        return orig_curl_easy_setopt_eos(curl, option, (void*)url.c_str());
    }
    else if (option == CURLOPT_SSL_VERIFYPEER)
    {
        return orig_curl_easy_setopt_eos(curl, option, (void*)0);
    }

    return orig_curl_easy_setopt_eos(curl, option, arg);
}
void* Main(void*)
{
    Library::WaitFor(_("libUnreal.so"));
    Library::WaitFor(_("libEOSSDK.so"));

    uintptr_t baseAddressUnreal = reinterpret_cast<uintptr_t>(Library::FindByName(_("libUnreal.so")));
    if (baseAddressUnreal)
    {
        uintptr_t offsetUnreal = 0x0EA16E54;
        void* curl_unreal_addr = reinterpret_cast<void*>(baseAddressUnreal + offsetUnreal);
        install_hook_curl_easy_setopt_unreal(curl_unreal_addr);
    }

    uintptr_t baseAddressEOS = reinterpret_cast<uintptr_t>(Library::FindByName(_("libEOSSDK.so")));
    if (baseAddressEOS)
    {
        uintptr_t offsetEOS = 0x01AF9A98;
        void* curl_eos_addr = reinterpret_cast<void*>(baseAddressEOS + offsetEOS);
        install_hook_curl_easy_setopt_eos(curl_eos_addr);
    }

    return nullptr;
}

__attribute__((constructor)) void libsinum_main()
{
    pthread_t ptid;
    pthread_create(&ptid, NULL, Main, NULL);
}
