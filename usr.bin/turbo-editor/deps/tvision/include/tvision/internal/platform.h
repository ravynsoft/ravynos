#ifndef TVISION_PLATFORM_H
#define TVISION_PLATFORM_H

#define Uses_TPoint
#define Uses_TColorAttr
#include <tvision/tv.h>
#include <internal/stdioctl.h>
#include <internal/dispbuff.h>
#include <internal/events.h>
#include <internal/mutex.h>
#include <vector>

struct TEvent;

namespace tvision
{

class DisplayStrategy
{
public:

    virtual ~DisplayStrategy() {}

    virtual TPoint getScreenSize() noexcept { return {}; }
    virtual int getCaretSize() noexcept { return 0; } // Range [0, 100].
    virtual void clearScreen() noexcept {}
    virtual ushort getScreenMode() noexcept { return 0; }
    virtual void reloadScreenInfo() noexcept {}
    virtual void lowlevelWriteChars(TStringView chars, TColorAttr attr) noexcept {}
    virtual void lowlevelMoveCursor(uint x, uint y) noexcept {};
    virtual void lowlevelMoveCursorX(uint x, uint y) noexcept { lowlevelMoveCursor(x, y); }
    virtual void lowlevelCursorSize(int size) noexcept {};
    virtual void lowlevelFlush() noexcept {};
    virtual bool screenChanged() noexcept { return false; }
};

class InputStrategy : public EventSource
{
public:

    InputStrategy(SysHandle aHandle) noexcept :
        EventSource(aHandle)
    {
    }

    virtual ~InputStrategy() {}

    virtual int getButtonCount() noexcept { return 0; }
    virtual void cursorOn() noexcept {}
    virtual void cursorOff() noexcept {}
};

struct ConsoleStrategy
{
    DisplayStrategy &display;
    InputStrategy &input;
    const std::vector<EventSource *> sources;

    ConsoleStrategy( DisplayStrategy &aDisplay, InputStrategy &aInput,
                     std::vector<EventSource *> &&aSources ) noexcept :
        display(aDisplay),
        input(aInput),
        sources(std::move(aSources))
    {
    }

    virtual ~ConsoleStrategy() {}

    virtual bool isAlive() noexcept { return true; }
    virtual bool setClipboardText(TStringView) noexcept { return false; }
    virtual bool requestClipboardText(void (&)(TStringView)) noexcept { return false; }
};

class Platform
{
    EventWaiter waiter;
    DisplayBuffer displayBuf;
    DisplayStrategy dummyDisplay;
    InputStrategy dummyInput {(SysHandle) 0};
    ConsoleStrategy dummyConsole {dummyDisplay, dummyInput, {}};
    // Invariant: 'console' contains either a non-owning reference to 'dummyConsole'
    // or an owning reference to a heap-allocated ConsoleStrategy object.
    SignalSafeReentrantMutex<ConsoleStrategy *> console {&dummyConsole};

    static Platform *instance;

    void setUpConsole(ConsoleStrategy *&) noexcept;
    void restoreConsole(ConsoleStrategy *&) noexcept;
    void checkConsole() noexcept;
    bool sizeChanged(TEvent &ev) noexcept;
    ConsoleStrategy &createConsole() noexcept;

    static int initAndGetCharWidth(uint32_t) noexcept;
    static void initEncodingStuff() noexcept;
    static void signalCallback(bool) noexcept;

    bool screenChanged() noexcept
        { return console.lock([] (auto *c) { return c->display.screenChanged(); }); }

public:

    static int (*charWidth)(uint32_t) noexcept;

    // Platform is a singleton. It gets created and destroyed by THardwareInfo.
    Platform() noexcept;
    ~Platform();

    // Note: explicit 'this' required by GCC 5.
    void setUpConsole() noexcept
        { console.lock([&] (auto *&c) { this->setUpConsole(c); }); }
    void restoreConsole() noexcept
        { console.lock([&] (auto *&c) { this->restoreConsole(c); }); }

    bool getEvent(TEvent &ev) noexcept;
    void waitForEvent(int ms) noexcept { checkConsole(); waiter.waitForEvent(ms); }
    void interruptEventWait() noexcept { waiter.interruptEventWait(); }

    int getButtonCount() noexcept
        { return console.lock([] (auto *c) { return c->input.getButtonCount(); }); }
    void cursorOn() noexcept
        { console.lock([] (auto *c) { c->input.cursorOn(); }); }
    void cursorOff() noexcept
        { console.lock([] (auto *c) { c->input.cursorOff(); }); }

    // Adjust the caret size to the range 1 to 100 because that's what the original
    // THardwareInfo::getCaretSize() does and what TScreen expects.
    int getCaretSize() noexcept { return min(max(displayBuf.caretSize, 1), 100); }
    bool isCaretVisible() noexcept { return displayBuf.caretSize > 0; }
    void clearScreen() noexcept
        { console.lock([&] (auto *c) { displayBuf.clearScreen(c->display); }); }
    int getScreenRows() noexcept { return displayBuf.size.y; }
    int getScreenCols() noexcept { return displayBuf.size.x; }
    void setCaretPosition(int x, int y) noexcept { displayBuf.setCaretPosition(x, y); }
    ushort getScreenMode() noexcept
        { return console.lock([] (auto *c) { return c->display.getScreenMode(); }); }
    void setCaretSize(int size) noexcept { displayBuf.setCaretSize(size); }
    void screenWrite(int x, int y, TScreenCell *b, int l) noexcept { displayBuf.screenWrite(x, y, b, l); }
    void flushScreen() noexcept
        { console.lock([&] (auto *c) { displayBuf.flushScreen(c->display); }); }
    TScreenCell *reloadScreenInfo() noexcept
        { return console.lock([&] (auto *c) { return displayBuf.reloadScreenInfo(c->display); }); }

    bool setClipboardText(TStringView text) noexcept
        { return console.lock([&] (auto *c) { return c->setClipboardText(text); }); }
    bool requestClipboardText(void (&accept)(TStringView)) noexcept
        { return console.lock([&] (auto *c) { return c->requestClipboardText(accept); }); }
};

} // namespace tvision

#endif // TVISION_PLATFORM_H
