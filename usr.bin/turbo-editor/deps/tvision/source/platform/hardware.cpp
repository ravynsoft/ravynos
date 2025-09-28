#define Uses_TKeys
#define Uses_TEvent
#define Uses_TScreen
#define Uses_THardwareInfo
#include <tvision/tv.h>

#include <internal/platform.h>
#include <internal/getenv.h>
#include <iostream.h>
#include <chrono>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

TEvent THardwareInfo::eventQ[];
size_t THardwareInfo::eventCount = 0;

static bool alwaysFlush;
static tvision::Platform *platf;

THardwareInfo::THardwareInfo() noexcept
{
    using namespace tvision;
    pendingEvent = 0;
    alwaysFlush = getEnv<int>("TVISION_MAX_FPS", 0) < 0;
    platf = new Platform();
}

THardwareInfo::~THardwareInfo()
{
    delete platf;
    platf = nullptr;
}

void THardwareInfo::setCaretSize( ushort size ) noexcept { platf->setCaretSize(size); }
void THardwareInfo::setCaretPosition( ushort x, ushort y ) noexcept { platf->setCaretPosition(x, y); }
ushort THardwareInfo::getCaretSize() noexcept { return platf->getCaretSize(); }
BOOL THardwareInfo::isCaretVisible() noexcept { return platf->isCaretVisible(); }
ushort THardwareInfo::getScreenRows() noexcept { return platf->getScreenRows(); }
ushort THardwareInfo::getScreenCols() noexcept { return platf->getScreenCols(); }
ushort THardwareInfo::getScreenMode() noexcept { return platf->getScreenMode(); }
void THardwareInfo::setScreenMode( ushort mode ) noexcept {}
void THardwareInfo::clearScreen( ushort w, ushort h ) noexcept { platf->clearScreen(); }
void THardwareInfo::screenWrite( ushort x, ushort y, TScreenCell *buf, DWORD len ) noexcept
{
    platf->screenWrite(x, y, buf, len);
    if (alwaysFlush)
        flushScreen();
}
TScreenCell *THardwareInfo::allocateScreenBuffer() noexcept { return platf->reloadScreenInfo(); }
void THardwareInfo::freeScreenBuffer( TScreenCell *buffer ) noexcept {}
DWORD THardwareInfo::getButtonCount() noexcept { return platf->getButtonCount(); }
void THardwareInfo::cursorOn() noexcept { platf->cursorOn(); }
void THardwareInfo::cursorOff() noexcept { platf->cursorOff(); }
void THardwareInfo::flushScreen() noexcept { platf->flushScreen(); }
void THardwareInfo::setUpConsole() noexcept { platf->setUpConsole(); }
void THardwareInfo::restoreConsole() noexcept { platf->restoreConsole(); }

BOOL THardwareInfo::getPendingEvent(TEvent &event, Boolean mouse) noexcept
{
    for (size_t i = 0; i < eventCount; ++i)
        if (!!(eventQ[i].what & evMouse) == mouse)
        {
            event = eventQ[i];
            for (; i + 1 < eventCount; ++i)
                eventQ[i] = eventQ[i + 1];
            --eventCount;
            return True;
        }
    return False;
}

BOOL THardwareInfo::getMouseEvent( MouseEventType& event ) noexcept
{
    TEvent ev;
    if (getPendingEvent(ev, True))
    {
        event = ev.mouse;
        return True;
    }
    return False;
}

BOOL THardwareInfo::getKeyEvent( TEvent& event ) noexcept
{
    readEvents();
    if (getPendingEvent(event, False))
    {
        if (event.what & evKeyboard)
        {
            if (event.keyDown.keyCode == kbIns)
                insertState = !insertState;
            if (insertState)
                event.keyDown.controlKeyState |= kbInsState;
        }
        return event.what != evNothing;
    }
    return False;
}

void THardwareInfo::readEvents() noexcept
{
    // Do not read any more events until the queue is empty.
    if (!eventCount)
        while (eventCount < eventQSize && platf->getEvent(eventQ[eventCount]))
            ++eventCount;
}

void THardwareInfo::waitForEvent( int timeoutMs ) noexcept
{
    if (!eventCount)
    {
        // Flush the screen once for every time all events have been processed,
        // only when blocking for events.
        flushScreen();
        platf->waitForEvent(timeoutMs);
    }
}

void THardwareInfo::interruptEventWait() noexcept
{
    platf->interruptEventWait();
}

BOOL THardwareInfo::setClipboardText( TStringView text ) noexcept
{
    return platf->setClipboardText(text);
}

BOOL THardwareInfo::requestClipboardText( void (&accept)(TStringView) ) noexcept
{
    return platf->requestClipboardText(accept);
}
