#include <internal/platform.h>
#include <internal/unixcon.h>
#include <internal/linuxcon.h>
#include <internal/win32con.h>
#include <internal/sighandl.h>
#include <internal/codepage.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

namespace tvision
{

Platform *Platform::instance;

// This is used by TText. It is a global function pointer (instead of an
// instance method) so that it can be used regardless of whether the global
// Platform instance has been created/destroyed or not.
int (*Platform::charWidth)(uint32_t) noexcept = &Platform::initAndGetCharWidth;

int Platform::initAndGetCharWidth(uint32_t wc) noexcept
{
    initEncodingStuff();
    return charWidth(wc);
}

void Platform::initEncodingStuff() noexcept
{
    static int init = [] ()
    {
        CpTranslator::init();
#ifdef _WIN32
        setlocale(LC_ALL, ".utf8");
        charWidth = &Win32ConsoleStrategy::charWidth;
#else
        setlocale(LC_ALL, "");
#ifdef __linux__
        auto &io = StdioCtl::getInstance();
        if (io.isLinuxConsole())
            charWidth = &LinuxConsoleStrategy::charWidth;
        else
#endif // __linux__
            charWidth = &UnixConsoleStrategy::charWidth;
#endif // _WIN32

        (void) init;
        return 0;
    }();
}

Platform::Platform() noexcept
{
    instance = this;
    initEncodingStuff();
}

Platform::~Platform()
{
    restoreConsole();
#ifndef _WIN32
    StdioCtl::destroyInstance();
#endif
    instance = nullptr;
}

void Platform::restoreConsole(ConsoleStrategy *&c) noexcept
{
    if (c != &dummyConsole)
    {
        displayBuf.flushScreen(c->display);
        for (auto *source : c->sources)
            if (source)
                waiter.removeSource(*source);
        SignalHandler::disable();
        delete c;
        c = &dummyConsole;
    }
}

// The remaining methods are in platfcon.cpp.

} // namespace tvision
