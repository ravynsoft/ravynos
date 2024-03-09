#ifdef _WIN32

#include <internal/winwidth.h>
#include <internal/utf8.h>

namespace tvision
{

std::atomic<size_t> WinWidth::lastReset {0};
WinWidth thread_local WinWidth::localInstance;

WinWidth::~WinWidth()
{
    tearDown();
}

void WinWidth::setUp() noexcept
{
    if (cnHandle == INVALID_HANDLE_VALUE || currentReset != lastReset)
    {
        tearDown();
        currentReset = lastReset;
        cnHandle = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            0,
            0,
            CONSOLE_TEXTMODE_BUFFER,
            0);
        CONSOLE_CURSOR_INFO info = {1, FALSE};
        SetConsoleCursorInfo(cnHandle, &info);
    }
}

void WinWidth::tearDown() noexcept
{
    if (cnHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(cnHandle);
        cnHandle = INVALID_HANDLE_VALUE;
    }
    results.clear();
}

int WinWidth::calcWidth(uint32_t u32) noexcept
{
    setUp();
    auto it = results.find(u32);
    if (it == results.end())
    {
        char res = -1;
        if (cnHandle != INVALID_HANDLE_VALUE)
        {
            uint16_t u16[3]; int len;
            if ((len = utf32To16(u32, u16)) > 0)
            {
                // We print an additional character so that we can distinguish
                // actual double-width characters from the ones affected by
                // https://github.com/microsoft/terminal/issues/11756.
                u16[len] = '#';
                SetConsoleCursorPosition(cnHandle, {0, 0});
                WriteConsoleW(cnHandle, (wchar_t *) u16, len + 1, 0, 0);
                CONSOLE_SCREEN_BUFFER_INFO sbInfo;
                if ( GetConsoleScreenBufferInfo(cnHandle, &sbInfo) &&
                     (res = sbInfo.dwCursorPosition.X - 1) > 1 )
                {
                    COORD coord {1, sbInfo.dwCursorPosition.Y};
                    DWORD count = 0; wchar_t charAfter;
                    ReadConsoleOutputCharacterW(cnHandle, &charAfter, 1, coord, &count);
                    if (count == 1 && charAfter == '#')
                        res = -1;
                }
            }
            // Memoize the result.
            results.emplace(u32, res);
        }
        return res;
    }
    else
        return it->second;
    static_assert(sizeof(uint16_t) == sizeof(wchar_t), "");
}

} // namespace tvision

#endif // _WIN32
