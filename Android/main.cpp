// Copyright (c) 2025 Project Nova LLC
#include "Url.h"
#include "Library.h"
#include "Util.h"
#include "Dobby/dobby.h"
#include "Includes/curl.h"
#include <dlfcn.h>
#include <pthread.h>
#define URL_PROTOCOL_HTTP _("http")
#define URL_HOST _("127.0.0.1:3552")
#define URL_PORT std::string()
install_hook_name(curl_easy_setopt, void*, void* curl, int option, void* arg)
{
if (!Util::IsPointerBad(arg) && option == CURLOPT_URL)
{
std::string url = reinterpret_cast<char*>(arg);
Uri uri = Uri::Parse(url);
if (uri.Host.ends_with(("epicgames.com"))
|| uri.Host.ends_with((".akamaized.net"))
|| uri.Host.ends_with(_("epicgames.com")))
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
Library::WaitFor(("libUnreal.so"));
uintptr_t baseUnreal = reinterpret_cast<uintptr_t>(Library::FindByName(("libUnreal.so")));
if (baseUnreal)
{
uintptr_t offsetUnreal = 0x0EA16E54;
void* curl_easy_setopt_unreal = reinterpret_cast<void*>(baseUnreal + offsetUnreal);
install_hook_curl_easy_setopt(curl_easy_setopt_unreal);
}
Library::WaitFor(("libEOSSDK.so"));
uintptr_t baseEOS = reinterpret_cast<uintptr_t>(Library::FindByName(("libEOSSDK.so")));
if (baseEOS)
{
uintptr_t offsetEOS = 0x01AF9A98;
void* curl_easy_setopt_eos = reinterpret_cast<void*>(baseEOS + offsetEOS);
install_hook_curl_easy_setopt(curl_easy_setopt_eos);
}
return nullptr;
}
attribute((constructor)) void libsinum_main()
{
pthread_t ptid;
pthread_create(&ptid, NULL, Main, NULL);
}
