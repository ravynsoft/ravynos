#include <internal/events.h>
#include <chrono>
using time_point = std::chrono::steady_clock::time_point;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::steady_clock;

#ifdef _TV_UNIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#endif

namespace tvision
{

/////////////////////////////////////////////////////////////////////////
// SysManualEvent

#ifdef _TV_UNIX

bool SysManualEvent::createHandle(int (&fds)[2]) noexcept
{
    if (pipe(fds) != -1)
    {
        for (int fd : fds)
            fcntl(fd, F_SETFD, FD_CLOEXEC);
        return true;
    }
    return false;
}

SysManualEvent::~SysManualEvent()
{
    close(fds[0]);
    close(fds[1]);
}

void SysManualEvent::signal() noexcept
{
    char c = 0;
    while (write(fds[1], &c, 1) == 0);
}

void SysManualEvent::clear() noexcept
{
    char c;
    while (read(fds[0], &c, sizeof(char)) == 0);
}

#else

bool SysManualEvent::createHandle(HANDLE &hEvent) noexcept
{
    return (hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr)) != NULL;
}

SysManualEvent::~SysManualEvent()
{
    CloseHandle(hEvent);
}

void SysManualEvent::signal() noexcept
{
    SetEvent(hEvent);
}

void SysManualEvent::clear() noexcept
{
    ResetEvent(hEvent);
}

#endif

/////////////////////////////////////////////////////////////////////////
// EventSource

bool EventSource::hasPendingEvents() noexcept
{
    return false;
}

bool EventSource::getEvent(TEvent &) noexcept
{
    return false;
}

/////////////////////////////////////////////////////////////////////////
// WakeUpEventSource

void WakeUpEventSource::signal() noexcept
{
    if (signaled.exchange(true) == false)
    {
        sys.signal();
    }
}

bool WakeUpEventSource::clear() noexcept
{
    if (signaled)
    {
        sys.clear();
        signaled = false;
        return true;
    }
    return false;
}

bool WakeUpEventSource::getEvent(TEvent &ev) noexcept
{
    if (clear())
    {
        if (callback)
            return callback(callbackArgs, ev);
        ev.what = evNothing;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////
// EventWaiter

#ifdef _TV_UNIX

static bool fdEmpty(int fd) noexcept
{
    int nbytes;
    return ioctl(fd, FIONREAD, &nbytes) == -1 || !nbytes;
}

static void pollHandles(PollData &pd, int ms) noexcept
{
    auto &fds = pd.handles;
    auto &states = pd.states;
    // We use 'select' instead of 'poll' because it is more portable, especially
    // on macOS. However, 'select' only supports file descriptors smaller than
    // 'FD_SETSIZE'. But this should not be an issue, since we just open a few
    // of them at program startup and when suspending/resuming the application.
    fd_set readFds;
    FD_ZERO(&readFds);
    int maxFd = -1;
    for (size_t i = 0; i < fds.size(); ++i)
        if (fds[i] < FD_SETSIZE)
        {
            FD_SET(fds[i], &readFds);
            if (fds[i] > maxFd)
                maxFd = fds[i];
        }

    if (maxFd >= 0)
    {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = ms*1000;
        if ( select( maxFd + 1, &readFds, nullptr, nullptr,
                     (ms < 0 ? nullptr : &timeout) ) >= 0 )
            for (size_t i = 0; i < fds.size(); ++i)
                if (fds[i] < FD_SETSIZE && FD_ISSET(fds[i], &readFds))
                {
                    if (fdEmpty(fds[i]))
                        states[i] = psDisconnect;
                    else
                        states[i] = psReady;
                }
    }
}

#else

static void pollHandles(PollData &pd, int ms) noexcept
{
    auto &handles = pd.handles;
    auto &states = pd.states;
    if (handles.size() == 0)
        Sleep(ms);
    else
    {
        DWORD res = WaitForMultipleObjects(
            handles.size(), &handles[0], FALSE, (ms < 0 ? INFINITE : ms));
        size_t i = 0;
        while (WAIT_OBJECT_0 <= res && res <= WAIT_OBJECT_0 + handles.size() - i - 1)
        {
            i += res - WAIT_OBJECT_0;
            states[i] = psReady;
            if (++i < handles.size())
                // Isn't this awful?
                res = WaitForMultipleObjects(
                    handles.size() - i, &handles[i], FALSE, 0);
            else
                break;
        }
    }
}

#endif // _TV_UNIX

EventWaiter::EventWaiter() noexcept
{
    SysManualEvent::Handle handle;
    if (SysManualEvent::createHandle(handle))
    {
        wakeUp.reset(new WakeUpEventSource(handle, nullptr, nullptr));
        addSource(*wakeUp);
    }
}

void EventWaiter::addSource(EventSource &src) noexcept
{
    sources.push_back(&src);
    pd.push_back(src.handle);
}

void EventWaiter::removeSource(EventSource &src) noexcept
{
    for (size_t i = 0; i < sources.size(); ++i)
        if (sources[i] == &src)
        {
            removeSource(i);
            break;
        }
}

inline void EventWaiter::removeSource(size_t i) noexcept
{
    sources.erase(sources.begin() + i);
    pd.erase(i);
}

void EventWaiter::pollSources(int timeoutMs) noexcept
{
    bool hasEvents = false;
    for (size_t i = 0; i < pd.size(); ++i)
        if ((pd.states[i] = sources[i]->hasPendingEvents() ? psReady : psNothing))
            hasEvents = true;
    if (!hasEvents)
    {
        pollHandles(pd, timeoutMs);
        for (size_t i = 0; i < pd.size(); ++i)
            if (pd.states[i] == psDisconnect)
                removeSource(i--);
    }
}

bool EventWaiter::hasReadyEvent() noexcept
{
    if (!readyEventPresent)
        for (size_t i = 0; i < pd.size(); ++i)
            if (pd.states[i] == psReady)
            {
                pd.states[i] = psNothing;
                readyEvent = {};
                if (sources[i]->getEvent(readyEvent))
                {
                    readyEventPresent = true;
                    break;
                }
            }
    return readyEventPresent;
}

inline void EventWaiter::getReadyEvent(TEvent &ev) noexcept
{
    ev = readyEvent;
    readyEventPresent = false;
}

static int pollDelayMs(time_point now, time_point end) noexcept
{
    return max(duration_cast<milliseconds>(nanoseconds(999999) + end - now).count(), 0);
}

bool EventWaiter::getEvent(TEvent &ev) noexcept
{
    if (hasReadyEvent() || (pollSources(0), hasReadyEvent()))
    {
        getReadyEvent(ev);
        return true;
    }
    return false;
}

void EventWaiter::waitForEvent(int ms) noexcept
{
    auto now = steady_clock::now();
    const auto end = ms < 0 ? time_point::max() : now + milliseconds(ms);
    while (!hasReadyEvent() && now <= end)
    {
        pollSources(ms < 0 ? -1 : pollDelayMs(now, end));
        now = steady_clock::now();
    }
}

void EventWaiter::interruptEventWait() noexcept
{
    if (wakeUp)
        wakeUp->signal();
}

} // namespace tvision
