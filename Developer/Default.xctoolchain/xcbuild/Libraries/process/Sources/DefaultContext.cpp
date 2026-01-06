/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <process/DefaultContext.h>
#include <libutil/FSUtil.h>

#include <mutex>
#include <sstream>
#include <unordered_set>
#include <cstring>
#include <cassert>

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <linux/limits.h>
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16
#include <sys/auxv.h>
#endif
#endif

#if !_WIN32
extern "C" char **environ;
#endif

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

static WideString
StringToWideString(std::string const &str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    WideString wide = WideString();
    wide.resize(size);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wide[0], size);
    return wide;
}
#endif

using process::DefaultContext;
using libutil::FSUtil;

DefaultContext::
DefaultContext() :
    Context()
{
}

DefaultContext::
~DefaultContext()
{
}

std::string const &DefaultContext::
currentDirectory() const
{
    static std::string const *directory = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::string path;

#if _WIN32
        /* Length includes NUL terminator. */
        DWORD length = GetCurrentDirectoryW(0, NULL);
        if (length == 0) {
            abort();
        }

        auto buffer = WideString();
        buffer.resize(length - 1);
        /* Size of the buffer should incremented to account for ending null byte */
        if (GetCurrentDirectoryW(buffer.size() + sizeof(decltype(buffer)::value_type), &buffer[0]) == 0) {
            abort();
        }

        path = WideStringToString(buffer);
#else
        for (size_t size = PATH_MAX; true; size *= 2) {
            std::string current = std::string();
            current.resize(size);

            char *ret = ::getcwd(&current[0], current.size());
            if (ret != nullptr) {
                /* Success. */
                path = std::string(current.c_str());
                break;
            } else if (errno == ERANGE) {
                /* Needs more space. */
            } else {
                abort();
            }
        }
#endif

        directory = new std::string(path);
    });

    return *directory;
}

#if defined(__linux__)
static char initialWorkingDirectory[PATH_MAX] = { 0 };
__attribute__((constructor))
static void InitializeInitialWorkingDirectory()
{
    if (getcwd(initialWorkingDirectory, sizeof(initialWorkingDirectory)) == NULL) {
        abort();
    }
}

#if !(__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16)
static char initialExecutablePath[PATH_MAX] = { 0 };
__attribute__((constructor))
static void InitialExecutablePathInitialize(int argc, char **argv)
{
    strncpy(initialExecutablePath, argv[0], sizeof(initialExecutablePath));
}
#endif
#endif

std::string const &DefaultContext::
executablePath() const
{
    static std::string const *executablePath = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::string absolutePath;

#if _WIN32
        for (size_t size = MAX_PATH; true; size *= 2) {
            auto buffer = WideString();
            buffer.resize(size);

            DWORD ret = GetModuleFileNameW(NULL, &buffer[0], buffer.size());
            if (ret == 0) {
                /* Failure. */
                abort();
            } else if (ret != size) {
                /* Success. */
                buffer = WideString(buffer.c_str());
                absolutePath = WideStringToString(buffer);
                break;
            } else {
                /* Needs more space. */
                assert(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
            }
        }
#elif defined(__APPLE__)
        uint32_t size = 0;
        if (_NSGetExecutablePath(NULL, &size) != -1) {
            abort();
        }

        absolutePath.resize(size - 1); /* Size includes terminator. */
        if (_NSGetExecutablePath(&absolutePath[0], &size) != 0) {
            abort();
        }
#elif defined(__linux__)
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16
        char const *path = reinterpret_cast<char const *>(getauxval(AT_EXECFN));
        if (path == NULL) {
            abort();
        }
#elif defined(__GLIBC__)
        char const *path = reinterpret_cast<char const *>(initialExecutablePath);
#else
#error Requires glibc on Linux.
#endif
        absolutePath = FSUtil::ResolveRelativePath(std::string(path), std::string(initialWorkingDirectory));
#else
#error Unsupported platform.
#endif

        executablePath = new std::string(FSUtil::NormalizePath(absolutePath));
    });

    return *executablePath;
}

#if defined(__APPLE__) || defined(__linux__)
static int commandLineArgumentCount = 0;
static char **commandLineArgumentValues = NULL;

#if defined(__APPLE__)
__attribute__((constructor))
#endif
static void CommandLineArgumentsInitialize(int argc, char **argv)
{
    commandLineArgumentCount = argc;
    commandLineArgumentValues = argv;
}

#if defined(__linux__)
__attribute__((section(".init_array"))) auto commandLineArgumentInitializer = &CommandLineArgumentsInitialize;
#endif
#endif

std::vector<std::string> const &DefaultContext::
commandLineArguments() const
{
    static std::vector<std::string> const *arguments = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if _WIN32
        LPCWSTR commandLine = GetCommandLineW();

        int commandLineArgumentCount;
        LPWSTR *commandLineArgumentValues = CommandLineToArgvW(commandLine, &commandLineArgumentCount);
        if (commandLineArgumentValues == nullptr) {
            abort();
        }

        std::vector<std::string> args;
        for (size_t i = 1; i < static_cast<size_t>(commandLineArgumentCount); i++) {
            auto buffer = WideString(commandLineArgumentValues[i]);
            args.push_back(WideStringToString(buffer));
        }
        arguments = new std::vector<std::string>(args);

        LocalFree(commandLineArgumentValues);
#elif defined(__APPLE__) || defined(__linux__)
        arguments = new std::vector<std::string>(commandLineArgumentValues + 1, commandLineArgumentValues + commandLineArgumentCount);
#else
#error Unsupported platform.
#endif
    });

    return *arguments;
}

ext::optional<std::string> DefaultContext::
environmentVariable(std::string const &variable) const
{
#if _WIN32
    auto name = StringToWideString(variable);

    auto buffer = WideString();
    buffer.resize(32768);
    if (GetEnvironmentVariableW(name.data(), &buffer[0], buffer.size()) == 0) {
        assert(GetLastError() == ERROR_ENVVAR_NOT_FOUND);
        return ext::nullopt;
    }

    buffer = WideString(buffer.c_str());
    return WideStringToString(buffer);
#else
    if (char *value = ::getenv(variable.c_str())) {
        return std::string(value);
    } else {
        return ext::nullopt;
    }
#endif
}

std::unordered_map<std::string, std::string> const &DefaultContext::
environmentVariables() const
{
    static std::unordered_map<std::string, std::string> const *environment = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::unordered_map<std::string, std::string> values;

#if _WIN32
        LPWCH variables = GetEnvironmentStringsW();
        if (variables == NULL) {
            abort();
        }

        LPCWSTR current = variables;
        size_t length = wcslen(current);
        while (length != 0) {
            auto buffer = WideString(current, current + length);
            std::string variable = WideStringToString(buffer);

            std::string::size_type offset = variable.find('=');
            std::string name = variable.substr(0, offset);
            std::string value = variable.substr(offset + 1);
            values.insert({ name, value });

            current += length + 1;
            length = wcslen(current);
        }

        if (FreeEnvironmentStringsW(variables) == 0) {
            abort();
        }
#else
        for (char **current = environ; *current; current++) {
            std::string variable = *current;

            std::string::size_type offset = variable.find('=');
            std::string name = variable.substr(0, offset);
            std::string value = variable.substr(offset + 1);
            values.insert({ name, value });
        }
#endif

        environment = new std::unordered_map<std::string, std::string>(values);
    });

    return *environment;
}
