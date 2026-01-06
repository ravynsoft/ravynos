/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <process/DefaultUser.h>

#include <mutex>
#include <sstream>
#include <string>
#include <cstring>

#if _WIN32
#include <windows.h>
#include <userenv.h>
#include <sddl.h>
#else
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#endif

using process::DefaultUser;

#if _WIN32
using WideString = std::basic_string<std::remove_const<std::remove_pointer<LPCWSTR>::type>::type>;

static std::string
WideStringToString(WideString const &str)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0, NULL, NULL);
    std::string multi = std::string();
    multi.resize(size);
    WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &multi[0], size, NULL, NULL);
    return multi;
}
#endif

DefaultUser::
DefaultUser() :
    User()
{
}

DefaultUser::
~DefaultUser()
{
}

#if _WIN32
template<typename T>
static T *
CreateToken(TOKEN_INFORMATION_CLASS type)
{
    HANDLE process = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process) == 0) {
        return nullptr;
    }

    DWORD size = 0;
    if (!GetTokenInformation(process, type, nullptr, 0, &size) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return nullptr;
    }

    T *token = static_cast<T *>(malloc(size));
    memset(token, 0, size);
    if (!GetTokenInformation(process, type, token, size, &size)) {
        return nullptr;
    }

    if (!CloseHandle(process)) {
        return nullptr;
    }

    return token;
}
#endif

std::string const &DefaultUser::
userID() const
{
    static std::string const *userID = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if _WIN32
        TOKEN_USER *token = CreateToken<TOKEN_USER>(TokenUser);
        if (token == nullptr) {
            abort();
        }

        LPWSTR string = nullptr;
        if (!ConvertSidToStringSidW(token->User.Sid, &string)) {
            abort();
        }

        WideString wide = WideString(string);
        userID = new std::string(WideStringToString(wide));

        LocalFree(string);
        free(token);
#else
        std::ostringstream os;
        os << ::getuid();
        userID = new std::string(os.str());
#endif
    });

    return *userID;
}

std::string const &DefaultUser::
groupID() const
{
    static std::string const *groupID = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if _WIN32
        TOKEN_PRIMARY_GROUP *token = CreateToken<TOKEN_PRIMARY_GROUP>(TokenPrimaryGroup);
        if (token == nullptr) {
            abort();
        }

        LPWSTR string = nullptr;
        if (!ConvertSidToStringSidW(token->PrimaryGroup, &string)) {
            abort();
        }

        WideString wide = WideString(string);
        groupID = new std::string(WideStringToString(wide));

        LocalFree(string);
        free(token);
#else
        std::ostringstream os;
        os << ::getgid();
        groupID = new std::string(os.str());
#endif
    });

    return *groupID;
}

std::string const &DefaultUser::
userName() const
{
    static std::string const *userName = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if _WIN32
        TOKEN_USER *token = CreateToken<TOKEN_USER>(TokenUser);
        if (token == nullptr) {
            abort();
        }

        DWORD size = 0;
        DWORD dsize = 0;
        SID_NAME_USE use;
        if (!LookupAccountSidW(nullptr, token->User.Sid, nullptr, &size, nullptr, &dsize, &use) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            abort();
        }

        WideString name;
        name.resize(size);
        WideString domain;
        name.resize(dsize);
        if (!LookupAccountSidW(nullptr, token->User.Sid, &name[0], &size, &domain[0], &dsize, &use)) {
            abort();
        }

        userName = new std::string(WideStringToString(name));

        free(token);
#else
        if (struct passwd const *pw = ::getpwuid(::getuid())) {
            if (pw->pw_name != nullptr) {
                userName = new std::string(pw->pw_name);
            }
        }

        if (userName == nullptr) {
            std::ostringstream os;
            os << ::getuid();
            userName = new std::string(os.str());
        }
#endif
    });

    return *userName;
}

std::string const &DefaultUser::
groupName() const
{
    static std::string const *groupName = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if _WIN32
        TOKEN_PRIMARY_GROUP *token = CreateToken<TOKEN_PRIMARY_GROUP>(TokenPrimaryGroup);
        if (token == nullptr) {
            abort();
        }

        DWORD size = 0;
        DWORD dsize = 0;
        SID_NAME_USE use;
        if (!LookupAccountSidW(nullptr, token->PrimaryGroup, nullptr, &size, nullptr, &dsize, &use) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            abort();
        }

        WideString name;
        name.resize(size);
        WideString domain;
        name.resize(dsize);
        if (!LookupAccountSidW(nullptr, token->PrimaryGroup, &name[0], &size, &domain[0], &dsize, &use)) {
            abort();
        }

        groupName = new std::string(WideStringToString(name));

        free(token);
#else
        if (struct group const *gr = ::getgrgid(::getgid())) {
            if (gr->gr_name != nullptr) {
                groupName = new std::string(gr->gr_name);
            }
        }

        if (groupName == nullptr) {
            std::ostringstream os;
            os << ::getgid();
            groupName = new std::string(os.str());
        }
#endif
    });

    return *groupName;
}

ext::optional<std::string> DefaultUser::
userHomeDirectory() const
{
#if _WIN32
    HANDLE process = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process) == 0) {
        return ext::nullopt;
    }

    /* Size includes NUL terminator. */
    DWORD size = 0;
    if (!GetUserProfileDirectoryW(process, nullptr, &size) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseHandle(process);
        return ext::nullopt;
    }

    auto buffer = WideString();
    buffer.resize(size - 1);
    if (!GetUserProfileDirectoryW(process, &buffer[0], &size)) {
        CloseHandle(process);
        return ext::nullopt;
    }

    CloseHandle(process);
    return WideStringToString(buffer);
#else
    char *home = ::getpwuid(::getuid())->pw_dir;
    if (home != nullptr) {
        return std::string(home);
    } else {
        return ext::nullopt;
    }
#endif
}
