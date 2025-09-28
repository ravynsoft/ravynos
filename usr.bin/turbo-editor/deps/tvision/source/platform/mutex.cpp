#include <internal/mutex.h>

namespace tvision
{

void SignalSafeMutexState::acquire(SignalSafeMutexState *self) noexcept
{
    ThreadId none {};
    ThreadId current {currentThreadId()};
    // Use a spin lock because regular mutexes are not signal-safe.
    if (self)
        while (self->lockingThread.compare_exchange_weak(none, current))
            ;
}

} // namespace tvision
