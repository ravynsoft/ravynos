#ifndef WINWIDTH_H
#define WINWIDTH_H

#ifdef _WIN32

#include <tvision/tv.h>
#include <tvision/compat/windows/windows.h>
#include <atomic>
#include <unordered_map>

namespace tvision
{

class WinWidth
{
    // Since there is no equivalent to wcwidth and the console API allows
    // having buffers out of sight, character widths are measured by printing
    // to a console buffer and taking the cursor position.

    // A separate state is stored for every thread so that mbcwidth() is both
    // thread-safe and lock-free.

    static std::atomic<size_t> lastReset;
    static thread_local WinWidth localInstance;

    std::unordered_map<uint32_t, short> results;
    HANDLE cnHandle {INVALID_HANDLE_VALUE};
    size_t currentReset {lastReset};

    int calcWidth(uint32_t) noexcept;
    void setUp() noexcept;
    void tearDown() noexcept;

    ~WinWidth();

public:

    static int width(uint32_t) noexcept;
    static void reset() noexcept;
};

inline int WinWidth::width(uint32_t wc) noexcept
{
    return localInstance.calcWidth(wc);
}

inline void WinWidth::reset() noexcept
{
    ++lastReset;
}

} // namespace tvision

#endif // _WIN32

#endif // WINWIDTH_H
