#define Uses_TEvent
#define Uses_TPoint
#include <tvision/tv.h>

#ifdef _TV_UNIX

#include <internal/sigwinch.h>

namespace tvision
{

SigwinchHandler *SigwinchHandler::instance {nullptr};

void SigwinchHandler::handleSignal(int) noexcept
{
    if (instance)
        instance->signal();
    // Don't call the previous SIGWINCH handler. Unfortunately, Ncurses
    // clears the screen when doing so and causes blinking.
}

bool SigwinchHandler::emitScreenChangedEvent(void *, TEvent &ev) noexcept
{
    ev.what = evCommand;
    ev.message.command = cmScreenChanged;
    return true;
}

inline SigwinchHandler::SigwinchHandler( SysManualEvent::Handle handle,
                                         const struct sigaction &aOldSa ) noexcept :
    WakeUpEventSource(handle, &emitScreenChangedEvent, nullptr),
    oldSa(aOldSa)
{
    instance = this;
}

SigwinchHandler *SigwinchHandler::create() noexcept
{
    if (!instance)
    {
        struct sigaction sa, oldSa;
        sa.sa_handler = &handleSignal;
        sa.sa_flags = SA_RESTART;
        if (sigfillset(&sa.sa_mask) != -1 && sigaction(SIGWINCH, &sa, &oldSa) != -1)
        {
            SysManualEvent::Handle handle;
            if (SysManualEvent::createHandle(handle))
                return new SigwinchHandler(handle, oldSa);
            sigaction(SIGWINCH, &oldSa, nullptr);
        }
    }
    return nullptr;
}

SigwinchHandler::~SigwinchHandler()
{
    sigaction(SIGWINCH, &oldSa, nullptr);
    instance = nullptr;
}

} // namespace tvision

#endif // _TV_UNIX
