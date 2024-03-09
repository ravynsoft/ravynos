#ifndef _WIN32

#include <windows.h>
#include <chrono>

extern "C" DWORD GetTickCount() noexcept
{
    return (DWORD) GetTickCount64();
}

extern "C" ULONGLONG GetTickCount64() noexcept
{
    // This effectively gives a system time reference in milliseconds.
    // steady_clock is best suited for measuring intervals.
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

#endif // _WIN32
