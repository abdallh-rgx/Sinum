// Copyright (c) 2025 Project Nova LLC

#include "Url.h"
#include "Library.h"
#include "Util.h"
#include "Dobby/dobby.h"
#include "Includes/curl.h"
#include <dlfcn.h>

#define URL_PROTOCOL_HTTP _("http")
#define URL_HOST _("127.0.0.1:3551")
#define URL_PORT std::string()

install_hook_name(curl_easy_setopt, void*, void* curl, int option, void* arg)
{
    if (!Util::IsPointerBad(arg) && option == CURLOPT_URL)
    {
        std::string url = reinterpret_cast<char*>(arg);

        Uri uri = Uri::Parse(url);

        if (uri.Host.ends_with(_("ol.epicgames.com"))
            || uri.Host.ends_with(_(".akamaized.net"))
            || uri.Host.ends_with(_("on.epicgames.com")))
        {
            url = Uri::CreateUri(URL_PROTOCOL_HTTP, URL_HOST, URL_PORT, uri.Path, uri.QueryString);
        }

        return orig_curl_easy_setopt(curl, option, (void*)url.c_str());
    }
    else if (option == CURLOPT_SSL_VERIFYPEER)
    {
        return orig_curl_easy_setopt(curl, option, (void*)0);
    }

    return orig_curl_easy_setopt(curl, option, arg);
}

void* Main(void*)
{
    Library::WaitFor(_("libUnreal.so"));

    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(Library::FindByName(_("libUE4.so")));
    if (!baseAddress) return nullptr;

    uintptr_t offset = 0x0E9A36AC;
    void* curl_easy_setopt = reinterpret_cast<void*>(baseAddress + offset);

    install_hook_curl_easy_setopt(curl_easy_setopt);

    return nullptr;
}

__attribute__((constructor)) void libsinum_main()
{
    pthread_t ptid;
    pthread_create(&ptid, NULL, Main, NULL);
}
