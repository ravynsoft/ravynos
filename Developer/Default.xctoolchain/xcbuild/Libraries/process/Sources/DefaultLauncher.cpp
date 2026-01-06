/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <process/DefaultLauncher.h>
#include <process/Context.h>
#include <libutil/Filesystem.h>

#if _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

// In most cases, size of pipe will be greater than one page,
#define PIPE_BUFFER_SIZE 4096

using process::DefaultLauncher;
using process::Context;
using libutil::Filesystem;

#if _WIN32
using WideString = std::basic_string<std::remove_const<std::remove_pointer<LPCWSTR>::type>::type>;

static WideString
StringToWideString(std::string const &str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    WideString wide = WideString();
    wide.resize(size);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wide[0], size);
    return wide;
}

static WideString
EscapedToken(const WideString &token)
{
    if (token.find_first_of(StringToWideString(" \t\n\v\"")) == std::string::npos) {
        return token;
    }

    WideString escapedToken = StringToWideString("\"");
    WideString special = StringToWideString("\"\\");
    for (auto it = token.begin(); it != token.end(); ++it) {
        /* Find the first backslash or quote. */
        size_t first_ = token.find_first_of(special, it - token.begin());
        auto first = (first_ == std::string::npos ? token.end() : token.begin() + first_);

        /* Copy up to the first special character. */
        escapedToken.insert(escapedToken.end(), it, first);

        if (first == token.end()) {
            /* Nothing special; jump to the end. */
            it = std::prev(first);
        } else if (*first == L'"') {
            /* Escape and append the quote itself. */
            escapedToken += L'\\';
            escapedToken += *first;

            /* Move past the quote. */
            it = first;
        } else if (*first == L'\\') {
            /* Find next non-backslash character after the first backslash. */
            size_t after_ = token.find_first_not_of(L'\\', first - token.begin());
            auto after = (after_ == std::string::npos ? token.end() : token.begin() + after_);

            if (after == token.end() || *after == L'"') {
                /* Duplicate (escape) all backslashes before (possibly ending) quote. */
                escapedToken.insert(escapedToken.end(), first, after);
                escapedToken.insert(escapedToken.end(), first, after);
            } else {
                /* Not before quote, no need to escape backslashes. */
                escapedToken.insert(escapedToken.end(), first, after);
            }

            /* Jump to the next non-backslash character. */
            it = std::prev(after);
        } else {
            abort();
        }
    }

    escapedToken += L'"';
    return escapedToken;
}
#endif

DefaultLauncher::
DefaultLauncher() :
    Launcher()
{
}

DefaultLauncher::
~DefaultLauncher()
{
}

ext::optional<int> DefaultLauncher::
launch(Filesystem *filesystem, Context const *context)
{
#if _WIN32
    WideString executablePath = StringToWideString(context->executablePath());

    WideString arguments = EscapedToken(executablePath);
    for (std::string const &argument : context->commandLineArguments()) {
        arguments += L' ';
        arguments += EscapedToken(StringToWideString(argument));
    }

    WideString environment;
    for (auto const &entry : context->environmentVariables()) {
        environment += StringToWideString(entry.first);
        environment += StringToWideString("=");
        environment += StringToWideString(entry.second);
        environment += L'\0';
    }

    WideString currentDirectory = StringToWideString(context->currentDirectory());

    STARTUPINFOW startup;
    memset(&startup, 0, sizeof(startup));
    startup.cb = sizeof(startup);

    PROCESS_INFORMATION process;
    if (!CreateProcessW(
        executablePath.c_str(),
        &arguments[0],
        nullptr,
        nullptr,
        FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        static_cast<LPVOID>(&environment[0]),
        currentDirectory.c_str(),
        &startup,
        &process)) {
        return ext::nullopt;
    }

    /* Wait until the spawned process finishes */
    WaitForSingleObject(process.hProcess, INFINITE);

    /* Get the exit code of the process */
    DWORD status;
    bool getExitCodeProcessSuccess = GetExitCodeProcess(process.hProcess, &status);

    /* Close process handles */
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    if (getExitCodeProcessSuccess) {
        return static_cast<int>(status);
    } else {
        return ext::nullopt;
    }
#else
    /*
     * Extract input data for exec, so no C++ is required after fork.
     */
    std::string path = context->executablePath();
    if (!filesystem->isExecutable(path)) {
        return ext::nullopt;
    }
    char const *cPath = path.c_str();

    std::string directory = context->currentDirectory();
    char const *cDirectory = directory.c_str();

    /* Compute command-line arguments. */
    std::vector<char const *> execArgs;
    execArgs.push_back(cPath);

    std::vector<std::string> arguments = context->commandLineArguments();
    for (std::string const &argument : arguments) {
        execArgs.push_back(argument.c_str());
    }

    execArgs.push_back(nullptr);
    char *const *cExecArgs = const_cast<char *const *>(execArgs.data());

    /* Compute environment variables. */
    std::vector<std::string> envValues;
    for (auto const &value : context->environmentVariables()) {
        envValues.push_back(value.first + "=" + value.second);
    }

    std::vector<char const *> execEnv;
    for (auto const &value : envValues) {
        execEnv.push_back(value.c_str());
    }
    execEnv.push_back(nullptr);
    char *const *cExecEnv = const_cast<char *const *>(execEnv.data());

    /* Setup parent-child stdout/stderr pipe. */
    int pfd[2];
    bool pipe_setup_success = true;
    if (pipe(pfd) == -1) {
        ::perror("pipe");
        pipe_setup_success = false;
    }

    /*
     * Fork new process.
     */
    pid_t pid = fork();
    if (pid < 0) {
        /* Fork failed. */
        return ext::nullopt;
    } else if (pid == 0) {
        /* Fork succeeded, new process. */
        if (pipe_setup_success) {
            /* Setup pipe to parent, redirecting both stdout and stderr */
            dup2(pfd[1], STDOUT_FILENO);
            dup2(pfd[1], STDERR_FILENO);
            close(pfd[0]);
            close(pfd[1]);
        } else {
            /* No parent-child pipe setup, just ignore outputs from child */
            int nullfd = open("/dev/null", O_WRONLY);
            if (nullfd == -1) {
                ::perror("open");
                ::_exit(1);
            }
            dup2(nullfd, STDOUT_FILENO);
            dup2(nullfd, STDERR_FILENO);
            close(nullfd);
        }

        if (::chdir(cDirectory) == -1) {
            ::perror("chdir");
            ::_exit(1);
        }

        ::execve(cPath, cExecArgs, cExecEnv);
        ::_exit(-1);

        return ext::nullopt;
    } else {
        /* Fork succeeded, existing process. */
        if (pipe_setup_success) {
            close(pfd[1]);
            /* Read child's stdout/stderr through pipe, and output stdout */
            while (true) {
                char pin[PIPE_BUFFER_SIZE];
                int readlen = read(pfd[0], &pin, sizeof(pin));
                if (readlen > 0) {
                    fwrite(pin, readlen, 1, stdout);
                } else {
                    if (readlen != 0) {
                        ::perror("read");
                    }
                    break;
                }
            }
            close(pfd[0]);
        }

        int status;
        ::waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
#endif
}
