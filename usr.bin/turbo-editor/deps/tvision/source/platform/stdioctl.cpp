#define Uses_TPoint
#include <tvision/tv.h>

#include <internal/stdioctl.h>
#include <internal/getenv.h>

namespace tvision
{

StdioCtl *StdioCtl::instance = nullptr;

StdioCtl &StdioCtl::getInstance() noexcept
{
    if (!instance)
        instance = new StdioCtl;
    return *instance;
}

void StdioCtl::destroyInstance() noexcept
{
    delete instance;
    instance = nullptr;
}

} // namespace tvision

#ifdef _TV_UNIX

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#if __has_include(<linux/kd.h>)
#   include <linux/kd.h>
#endif

namespace tvision
{

StdioCtl::StdioCtl() noexcept
{
    if ( getEnv<TStringView>("TVISION_USE_STDIO").empty()
         && (files[0] = fopen("/dev/tty", "r")) != nullptr
         && (files[1] = fopen("/dev/tty", "w")) != nullptr )
    {
        ownsFiles = true;
        fds[0] = fileno(files[0]);
        fds[1] = fileno(files[1]);
        // Subprocesses must not inherit these file descriptors.
        for (int fd : fds)
            fcntl(fd, F_SETFD, FD_CLOEXEC);
    }
    else
    {
        for (FILE *file : files)
            if (file != nullptr)
                fclose(file);
        fds[0] = STDIN_FILENO;
        fds[1] = STDOUT_FILENO;
        files[0] = stdin;
        files[1] = stdout;
    }
}

StdioCtl::~StdioCtl()
{
    if (ownsFiles)
        for (FILE *file : files)
            fclose(file);
}

void StdioCtl::write(const char *data, size_t bytes) const noexcept
{
    fflush(fout());
    size_t written = 0;
    int r;
    while ( written < bytes &&
            0 <= (r = ::write(out(), data + written, bytes - written)) )
        written += r;
}

TPoint StdioCtl::getSize() const noexcept
{
    struct winsize w;
    for (int fd : fds)
    {
        if (ioctl(fd, TIOCGWINSZ, &w) != -1)
        {
            int env_col = getEnv<int>("COLUMNS", INT_MAX);
            int env_row = getEnv<int>("LINES", INT_MAX);
            return {
                min(max(w.ws_col, 0), max(env_col, 0)),
                min(max(w.ws_row, 0), max(env_row, 0)),
            };
        }
    }
    return {0, 0};
}

TPoint StdioCtl::getFontSize() const noexcept
{
#ifdef KDFONTOP
    struct console_font_op cfo {};
    cfo.op = KD_FONT_OP_GET;
    cfo.width = cfo.height = 32;
    for (int fd : fds)
        if (ioctl(fd, KDFONTOP, &cfo) != -1)
            return {
                max(cfo.width, 0),
                max(cfo.height, 0),
            };
#endif
    struct winsize w;
    for (int fd : fds)
        if (ioctl(fd, TIOCGWINSZ, &w) != -1)
            return {
                w.ws_xpixel / max(w.ws_col, 1),
                w.ws_ypixel / max(w.ws_row, 1),
            };
    return {0, 0};
}

#ifdef __linux

bool StdioCtl::isLinuxConsole() const noexcept
{
    // This is the same function used to get the Shift/Ctrl/Alt modifiers
    // on the console. It only succeeds if a console file descriptor is used.
    for (int fd : fds)
    {
        char subcode = 6;
        if (ioctl(fd, TIOCLINUX, &subcode) != -1)
            return true;
    }
    return false;
}

#endif // __linux__

} // namespace tvision

#elif defined(_WIN32)

#include <stdio.h>

namespace tvision
{

namespace stdioctl
{

    static bool isValid(HANDLE h)
    {
        return h && h != INVALID_HANDLE_VALUE;
    }

    static bool isConsole(HANDLE h)
    {
        DWORD mode;
        return GetConsoleMode(h, &mode);
    }

    static COORD windowSize(SMALL_RECT srWindow)
    {
        return {
            short(srWindow.Right - srWindow.Left + 1),
            short(srWindow.Bottom - srWindow.Top + 1),
        };
    }

} // namespace stdioctl

StdioCtl::StdioCtl() noexcept
{
    // The console can be accessed in two ways: through GetStdHandle() or through
    // CreateFile(). GetStdHandle() will be unable to return a console handle
    // if standard handles have been redirected.
    //
    // Additionally, we want to spawn a new console when none is visible to the user.
    // This might happen under two circumstances:
    //
    // 1. The console crashed. This is easy to detect because all console operations
    //    fail on the console handles.
    // 2. The console exists somehow but cannot be made visible, not even by doing
    //    GetConsoleWindow() and then ShowWindow(SW_SHOW). This is what happens
    //    under Git Bash without pseudoconsole support. In this case, none of the
    //    standard handles is a console, yet the handles returned by CreateFile()
    //    still work.
    //
    // So, in order to find out if a console needs to be allocated, we
    // check whether at least of the standard handles is a console. If none
    // of them is, we allocate a new console. Yes, this will always spawn a
    // console if all three standard handles are redirected, but this is not
    // a common use case.
    //
    // Then comes the question of whether to access the console through GetStdHandle()
    // or through CreateFile(). CreateFile() has the advantage of not being affected
    // by standard handle redirection. However, I have found that some terminal
    // emulators (i.e. ConEmu) behave unexpectedly when using screen buffers
    // opened with CreateFile(). So we will use the standard handles whenever possible.
    //
    // It is worth mentioning that the handles returned by CreateFile() have to be
    // closed, but the ones returned by GetStdHandle() must not. So we have to remember
    // this information for each console handle.
    //
    // We also need to remember whether we allocated a console or not, so that
    // we can free it when tearing down. If we don't, weird things may happen.

    using namespace stdioctl;
    static constexpr struct { DWORD std; int index; } channels[] =
    {
        {STD_INPUT_HANDLE, input},
        {STD_OUTPUT_HANDLE, startupOutput},
        {STD_ERROR_HANDLE, startupOutput},
    };
    bool haveConsole = false;
    for (const auto &c : channels)
    {
        HANDLE h = GetStdHandle(c.std);
        if (isConsole(h))
        {
            haveConsole = true;
            if (!isValid(cn[c.index].handle))
                cn[c.index] = {h, false};
        }
    }
    if (!haveConsole)
    {
        FreeConsole();
        AllocConsole();
        ownsConsole = true;
    }
    if (!isValid(cn[input].handle))
    {
        cn[input].handle = CreateFileW(
            L"CONIN$",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            0,
            0);
        cn[input].owning = true;
    }
    if (!isValid(cn[startupOutput].handle))
    {
        cn[startupOutput].handle = CreateFileW(
            L"CONOUT$",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            0);
        cn[startupOutput].owning = true;
    }
    cn[activeOutput].handle = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CONSOLE_TEXTMODE_BUFFER,
        nullptr);
    cn[activeOutput].owning = true;
    {
        CONSOLE_SCREEN_BUFFER_INFO sbInfo {};
        GetConsoleScreenBufferInfo(cn[startupOutput].handle, &sbInfo);
        // Force the screen buffer size to match the window size.
        // The Console API guarantees this, but some implementations
        // are not compliant (e.g. Wine).
        sbInfo.dwSize = windowSize(sbInfo.srWindow);
        SetConsoleScreenBufferSize(cn[activeOutput].handle, sbInfo.dwSize);
    }
    SetConsoleActiveScreenBuffer(cn[activeOutput].handle);
    for (auto &c : cn)
        if (!isValid(c.handle))
        {
            fputs("Error: cannot get a console.\n", stderr);
            exit(1);
        }
}

StdioCtl::~StdioCtl()
{
    using namespace stdioctl;
    CONSOLE_SCREEN_BUFFER_INFO activeSbInfo {};
    GetConsoleScreenBufferInfo(cn[activeOutput].handle, &activeSbInfo);
    CONSOLE_SCREEN_BUFFER_INFO startupSbInfo {};
    GetConsoleScreenBufferInfo(cn[startupOutput].handle, &startupSbInfo);

    COORD activeWindowSize = windowSize(activeSbInfo.srWindow);
    COORD startupWindowSize = windowSize(startupSbInfo.srWindow);

    // Preserve the current window size.
    if ( activeWindowSize.X != startupWindowSize.X ||
         activeWindowSize.Y != startupWindowSize.Y )
    {
        // The buffer is not allowed to be smaller than the window, so enlarge
        // it if necessary. But do not shrink it in the opposite case, to avoid
        // loss of data.
        COORD dwSize = startupSbInfo.dwSize;
        if (dwSize.X < activeWindowSize.X)
            dwSize.X = activeWindowSize.X;
        if (dwSize.Y < activeWindowSize.Y)
            dwSize.Y = activeWindowSize.Y;
        SetConsoleScreenBufferSize(cn[startupOutput].handle, dwSize);
        // Get the updated cursor position, in case it changed after the resize.
        GetConsoleScreenBufferInfo(cn[startupOutput].handle, &startupSbInfo);
        // Make sure the cursor is visible. If possible, show it in the bottom row.
        SMALL_RECT srWindow = startupSbInfo.srWindow;
        COORD dwCursorPosition = startupSbInfo.dwCursorPosition;
        srWindow.Right = max(dwCursorPosition.X, activeWindowSize.X - 1);
        srWindow.Left = srWindow.Right - (activeWindowSize.X - 1);
        srWindow.Bottom = max(dwCursorPosition.Y, activeWindowSize.Y - 1);
        srWindow.Top = srWindow.Bottom - (activeWindowSize.Y - 1);
        SetConsoleWindowInfo(cn[startupOutput].handle, TRUE, &srWindow);
    }

    SetConsoleActiveScreenBuffer(cn[startupOutput].handle);
    for (auto &c : cn)
        if (c.owning)
            CloseHandle(c.handle);
    if (ownsConsole)
        FreeConsole();
}

void StdioCtl::write(const char *data, size_t bytes) const noexcept
{
    // Writing 0 bytes causes the cursor to become invisible for a short time
    // in old versions of the Windows console.
    if (bytes != 0)
        WriteConsoleA(out(), data, bytes, nullptr, nullptr);
}

TPoint StdioCtl::getSize() const noexcept
{
    CONSOLE_SCREEN_BUFFER_INFO sbInfo;
    auto &srWindow = sbInfo.srWindow;
    if (GetConsoleScreenBufferInfo(out(), &sbInfo))
        return {
            max(srWindow.Right - srWindow.Left + 1, 0),
            max(srWindow.Bottom - srWindow.Top + 1, 0),
        };
    return {0, 0};
}

TPoint StdioCtl::getFontSize() const noexcept
{
    CONSOLE_FONT_INFO fontInfo;
    if (GetCurrentConsoleFont(out(), FALSE, &fontInfo))
        return {
            fontInfo.dwFontSize.X,
            fontInfo.dwFontSize.Y,
        };
    return {0, 0};
}

} // namespace tvision

#endif // _TV_UNIX
