#ifndef TVISION_SIGWINCH_H
#define TVISION_SIGWINCH_H

#include <tvision/tv.h>

#ifdef _TV_UNIX
#include <internal/events.h>
#include <signal.h>

struct TEvent;

namespace tvision
{

class SigwinchHandler final : public WakeUpEventSource
{
    struct sigaction oldSa;

    static SigwinchHandler *instance;
    static void handleSignal(int) noexcept;
    static bool emitScreenChangedEvent(void *, TEvent &) noexcept;

    SigwinchHandler( SysManualEvent::Handle handle,
                     const struct sigaction &aOldSa ) noexcept;

public:

    static SigwinchHandler *create() noexcept;
    ~SigwinchHandler();
};

} // namespace tvision

#endif // _TV_UNIX

#endif // TVISION_SIGWINCH_H
