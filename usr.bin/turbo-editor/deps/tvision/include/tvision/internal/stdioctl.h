#ifndef TVISION_STDIOCTL_H
#define TVISION_STDIOCTL_H

#include <tvision/tv.h>
#include <stdio.h>

#ifdef _WIN32
#include <tvision/compat/windows/windows.h>
#endif

namespace tvision
{

class StdioCtl final
{
#ifdef _WIN32
    enum { input = 0, startupOutput = 1, activeOutput = 2 };
    struct
    {
        HANDLE handle {INVALID_HANDLE_VALUE};
        bool owning {false};
    } cn[3];
    bool ownsConsole {false};
#else
    int fds[2] {-1, -1};
    FILE *files[2] {nullptr, nullptr};
    bool ownsFiles {false};
#endif // _WIN32

    static StdioCtl *instance;

    StdioCtl() noexcept;
    ~StdioCtl();

public:

    // On Windows, the StdioCtl instance is created every time the alternate
    // screen buffer is enabled and it is destroyed when restoring the console.
    // On Unix, the StdioCtl instance is created just once at the beginning
    // of the program execution (in static initialization) and destroyed when
    // exiting the program.

    // Creates a global instance if none exists, and returns it.
    static StdioCtl &getInstance() noexcept;
    // Destroys the global instance if it exists.
    static void destroyInstance() noexcept;

    void write(const char *data, size_t bytes) const noexcept;
    TPoint getSize() const noexcept;
    TPoint getFontSize() const noexcept;

#ifdef _WIN32
    HANDLE in() const noexcept { return cn[input].handle; }
    HANDLE out() const noexcept { return cn[activeOutput].handle; }
#else
    int in() const noexcept { return fds[0]; }
    int out() const noexcept { return fds[1]; }
    FILE *fin() const noexcept { return files[0]; }
    FILE *fout() const noexcept { return files[1]; }
#ifdef __linux__
    bool isLinuxConsole() const noexcept;
#endif
#endif // _WIN32
};

} //namespace tvision

#endif // TVISION_STDIOCTL_H
