#include <internal/unixclip.h>

#ifdef _TV_UNIX

#include <internal/getenv.h>
#include <internal/utf8.h>

#include <initializer_list>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <poll.h>

namespace tvision
{

#define PID_PLACEHOLDER "########"

enum { clipboardTimeoutMs = 1500 };

#ifdef __APPLE__
constexpr const char * pbcopyArgv[] = {"pbcopy", 0};
constexpr const char * pbpasteArgv[] = {"pbpaste", 0};
#else
constexpr const char * wlCopyArgv[] = {"wl-copy", 0};
constexpr const char * xselCopyArgv[] = {"xsel", "--input", "--clipboard", 0};
constexpr const char * xclipCopyArgv[] = {"xclip", "-in", "-selection", "clipboard", 0};
constexpr const char * wslCopyArgv[] = {"/mnt/c/Windows/System32/clip.exe", 0};
constexpr const char * wlPasteArgv[] = {"wl-paste", "--no-newline", 0};
constexpr const char * xselPasteArgv[] = {"xsel", "--output", "--clipboard", 0};
constexpr const char * xclipPasteArgv[] = {"xclip", "-out", "-selection", "clipboard", 0};

static char wslPasteCmd[] =
    // Both PowerShell and CScript can be used to read the clipboard.
    // Although PowerShell is more modern and can be run without a script file,
    // it is far slower and requires a very complicated workaround for
    // https://github.com/microsoft/terminal/issues/280. Therefore, we stick
    // to CScript.
    // The environment variable 'q' is used to insert quoutes in order
    // to work around https://github.com/microsoft/WSL/issues/2835.
    "(FOR"
        " %i IN (%q%%TMP%%q% %q%%TEMP%%q% %q%%USERPROFILE%%q% \\. .)"
    " DO"
        " set SCRIPT_PATH=%q%%~i\\" PID_PLACEHOLDER "%RANDOM%.vbs%q%"
        // Try to write the script file.
        " && cmd /C"
            " echo"
                // Print the clipboard contents in UTF-16.
                " WScript.StdOut.Write"
                    " CreateObject(%q%HTMLFile%q%^).ParentWindow.ClipboardData.GetData(%q%Text%q%^)"
            " ^> ^%SCRIPT_PATH^%"
        // If it worked, run it and exit.
        " && ("
            " cmd /C"
                " cscript //NoLogo //B //U ^%SCRIPT_PATH^%"
                // Delete the files and keep the error status.
                " ^&^& (del /Q ^%SCRIPT_PATH^% ^& exit ^)"
                " ^|^| (del /Q ^%SCRIPT_PATH^% ^& exit 1^)"
            " && exit"
            " || exit 1"
        ")"
        // Otherwise, try another path.
    ")"
    // We could not write anywhere.
    " & exit 1";

constexpr const char *wslPasteArgv[] = {"/mnt/c/Windows/System32/cmd.exe", "/D", "/Q", "/C", wslPasteCmd, 0};

constexpr const char *wslPasteEnv[] =
{
    "WSLENV", "q",
    "q", "\"",
    0,
};

static int initPlaceholders = []
{
    static_assert(sizeof(PID_PLACEHOLDER) - 1 == 8, "");

    // Use the PID to avoid name clashes between files created by different
    // processes. This is more effective than pseudo-random values which
    // depend on the current timestamp such as %RANDOM%.
    int pid = (int) getpid();
    char buf[sizeof(PID_PLACEHOLDER)];
    snprintf(buf, sizeof(buf), "%08X", pid);

    for (char *p : {wslPasteCmd})
        while ((p = strstr(p, PID_PLACEHOLDER)))
            memcpy(p, buf, 8);

    (void) initPlaceholders;
    return 0;
}();

static void wslCopyPrepare(TSpan<char> &text) noexcept
// Pre: 'text' is malloc-allocated.
// When writing text to clip.exe, append a null character at the end
// to work around https://github.com/microsoft/WSL/issues/4852.
{
    if (char *dstText = (char *) realloc(text.data(), text.size() + 1))
    {
        dstText[text.size()] = '\0';
        text = {dstText, text.size() + 1};
    }
}

static void wslPastePrepare(TSpan<char> &text) noexcept
// Pre: 'text' is malloc-allocated.
// We receive the clipboard contents from CScript in UTF-16, so we have to
// convert them to UTF-8.
{
    uint16_t *srcText = (uint16_t *) text.data();
    size_t srcLength = text.size()/2;
    char *dstText;
    size_t dstLength = 0;
    // Each UTF-16 code unit may produce up to 3 UTF-8 bytes.
    if ((dstText = (char *) malloc(srcLength * 3)))
        dstLength = utf16To8({srcText, srcLength}, dstText);
    free(srcText);
    text = {dstText, dstLength};
}
#endif

struct Command
{
    const char * const *argv;
    const char *requiredEnv;
    const char * const *customEnv;
    void (*prepare)(TSpan<char> &);
};

constexpr Command copyCommands[] =
{
#ifdef __APPLE__
    {pbcopyArgv},
#else
    {wslCopyArgv, nullptr, nullptr, wslCopyPrepare},
    {wlCopyArgv, "WAYLAND_DISPLAY"},
    {xselCopyArgv, "DISPLAY"},
    {xclipCopyArgv, "DISPLAY"},
#endif
};

constexpr Command pasteCommands[] =
{
#ifdef __APPLE__
    {pbpasteArgv},
#else
    {wlPasteArgv, "WAYLAND_DISPLAY"},
    {xselPasteArgv, "DISPLAY"},
    {xclipPasteArgv, "DISPLAY"},
    {wslPasteArgv, nullptr, wslPasteEnv, wslPastePrepare},
#endif
};

static bool executable_exists(const char *name);
static TSpan<char> read_subprocess(const char * const cmd[], const char * const env[], int timeoutMs);
static bool write_subprocess(const char * const cmd[], TStringView, int timeoutMs);

static bool commandIsAvailable(const Command &cmd)
{
    return (!cmd.requiredEnv || !getEnv<TStringView>(cmd.requiredEnv).empty())
        && executable_exists(cmd.argv[0]);
}

static TSpan<char> copyStr(TStringView str) noexcept
{
    if (char *buf = (char *) malloc(str.size()))
    {
        memcpy(buf, str.data(), str.size());
        return {buf, str.size()};
    }
    return {};
}

bool UnixClipboard::setClipboardText(TStringView aText) noexcept
{
    for (auto &cmd : copyCommands)
        if (commandIsAvailable(cmd))
        {
            TSpan<char> text = copyStr(aText);
            if (cmd.prepare)
                cmd.prepare(text);
            bool success = write_subprocess(cmd.argv, text, clipboardTimeoutMs);
            free(text.data());
            if (success)
                return true;
            break;
        }
    return false;
}

bool UnixClipboard::requestClipboardText(void (&accept)(TStringView)) noexcept
{
    for (auto &cmd : pasteCommands)
        if (commandIsAvailable(cmd))
        {
            TSpan<char> text = read_subprocess(cmd.argv, cmd.customEnv, clipboardTimeoutMs);
            if (cmd.prepare)
                cmd.prepare(text);
            if (text.data())
            {
                accept(text);
                free(text.data());
                return true;
            }
            break;
        }
    return false;
}

static bool executable_exists(const char *name)
{
    const char *path = "";
    if (name[0] != '/' && !(path = getenv("PATH")))
        path = "/usr/local/bin:/bin:/usr/bin";
    const char *end = path + strlen(path);
    size_t nameLen = strlen(name);
    const char *p = path;
    do
    {
        char buf[PATH_MAX];
        const char *q = strchr(p, ':');
        if (!q)
            q = end;
        if (q - p + nameLen + 2 <= PATH_MAX)
        {
            memcpy(&buf[0], p, q - p);
            buf[q - p] = '/';
            memcpy(&buf[q - p + 1], name, nameLen);
            buf[q - p + nameLen + 1] = '\0';

            struct stat st;
            if (stat(buf, &st) == 0 && !(~st.st_mode & (S_IFREG | S_IXOTH)))
                return true;
        }
        p = q;
    }
    while (p < end && *p++);
    return false;
}

enum class run_subprocess_mode
{
    read,
    write,
};

struct run_subprocess_t
{
    pid_t pid {-1};
    int fd {-1};
};

static run_subprocess_t run_subprocess(const char * const argv[], const char * const env[], run_subprocess_mode mode)
{
    int fds[2];
    if (pipe(fds) == -1)
        return {};
    pid_t pid = fork();
    if (pid == 0)
    {
        for (auto *p = env; p && *p; p += 2)
            setenv(p[0], p[1], true);

        int nul     = open("/dev/null", O_RDWR);
        int new_in  = mode == run_subprocess_mode::read ? nul    : fds[0];
        int new_out = mode == run_subprocess_mode::read ? fds[1] : nul;

        if ( nul != -1
             && dup2(new_in, STDIN_FILENO) != -1
             && dup2(new_out, STDOUT_FILENO) != -1
             && dup2(nul, STDERR_FILENO) != -1
             && close(fds[0]) != -1
             && close(fds[1]) != -1
             && close(nul) != -1 )
        {
            execvp(argv[0], (char * const *) argv);
        }
        _Exit(1);
    }
    else if (pid > 0)
    {
        int pipe_end = mode == run_subprocess_mode::read ? 0 : 1;
        close(fds[1 - pipe_end]);
        return {pid, fds[pipe_end]};
    }
    for (int fd : fds)
        close(fd);
    return {};
}

static bool close_subprocess(run_subprocess_t &process)
{
    close(process.fd);
    int status;
    int res = waitpid(process.pid, &status, 0);
    return res > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

struct read_pipe_t
{
    char *data {nullptr};
    size_t size {0};
    bool incomplete {false};
};

static read_pipe_t read_pipe(int fd, int timeoutMs)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return {};
    enum { minReadSize = 4096 };
    char *buf = (char *) malloc(minReadSize);
    if (!buf)
        return {};
    size_t bytesRead = 0;
    size_t capacity = minReadSize;
    int res = -1;
    while (true)
    {
        ssize_t r;
        while ((r = read(fd, buf + bytesRead, capacity - bytesRead)) > 0)
        {
            bytesRead += (size_t) r;
            if (capacity - bytesRead < minReadSize)
            {
                if (void *tmp = realloc(buf, capacity *= 2))
                    buf = (char *) tmp;
                else
                {
                    free(buf);
                    return {nullptr, 0, true};
                }
            }
        }
        if (r == 0 || (r == -1 && errno != EAGAIN))
            break;
        struct pollfd pfd {fd, POLLIN};
        if ((res = poll(&pfd, 1, timeoutMs)) <= 0 || !(pfd.revents & POLLIN))
            break;
    }
    return {buf, bytesRead, res == 0};
}

struct write_pipe_t
{
    bool success {false};
    bool incomplete {false};
};

static write_pipe_t write_pipe(int fd, const char *data, size_t size, int timeoutMs)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return {};
    size_t bytesWritten = 0;
    int res = -1;
    while (true)
    {
        ssize_t r;
        while ((r = write(fd, data + bytesWritten, size - bytesWritten)) > 0)
            bytesWritten += (size_t) r;
        if (r == 0 || (r == -1 && errno != EAGAIN))
            break;
        struct pollfd pfd {fd, POLLOUT};
        if ((res = poll(&pfd, 1, timeoutMs)) <= 0 || !(pfd.revents & POLLOUT))
            break;
    }
    return {bytesWritten == size, res == 0};
}

static TSpan<char> read_subprocess(const char * const cmd[], const char * const env[], int timeoutMs)
{
    auto process = run_subprocess(cmd, env, run_subprocess_mode::read);
    if (process.pid == -1)
        return {};
    auto res = read_pipe(process.fd, timeoutMs);
    if (res.incomplete)
        kill(process.pid, SIGKILL);
    bool processOk = close_subprocess(process);
    if (processOk || res.size > 0)
        return {res.data, res.size};
    free(res.data);
    return {};
}

static bool write_subprocess(const char * const cmd[], TStringView text, int timeoutMs)
{
    auto process = run_subprocess(cmd, nullptr, run_subprocess_mode::write);
    if (process.pid == -1)
        return false;
    auto res = write_pipe(process.fd, text.data(), text.size(), timeoutMs);
    if (res.incomplete)
        kill(process.pid, SIGKILL);
    bool processOk = close_subprocess(process);
    return processOk && res.success;
}

} // namespace tvision

#endif // _TV_UNIX
