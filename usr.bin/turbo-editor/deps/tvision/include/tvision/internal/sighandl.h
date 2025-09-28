#ifndef TVISION_SIGHANDL_H
#define TVISION_SIGHANDL_H

#include <tvision/tv.h>

#ifdef _TV_UNIX
#include <signal.h>
#include <atomic>
#endif

namespace tvision
{

using SignalHandlerCallback = void(bool enter);

#ifdef _TV_UNIX

class SignalHandler
{
    static std::atomic<SignalHandlerCallback *> callback;

    enum HandledSignals
    {
        SigInt, SigQuit, SigIll, SigAbrt, SigFpe, SigSegv, SigTerm, SigTstp,
        HandledSignalCount
    };

public:

    static const int handledSignals[HandledSignalCount];

private:

    static constexpr struct sigaction makeDefaultAction() noexcept
    {
        struct sigaction sa = {};
        sa.sa_handler = SIG_DFL;
        return sa;
    }

    static struct sigaction makeHandlerAction() noexcept
    {
        struct sigaction sa = {};
        sa.sa_flags = SA_SIGINFO | SA_RESTART;
        sa.sa_sigaction = &handleSignal;
        return sa;
    }

    struct HandlerInfo
    {
        struct sigaction action {makeDefaultAction()};
        std::atomic<bool> running {false};
    };

    static void handleSignal(int, siginfo_t *, void *);
    static HandlerInfo &getHandlerInfo(int) noexcept;
    static bool invokeHandlerOrDefault(int, const struct sigaction &, siginfo_t *, void *) noexcept;
    static bool invokeDefault(int, siginfo_t *) noexcept;

public:

    // 'aCallback' must be noexcept and signal-safe.
    static void enable(SignalHandlerCallback &aCallback) noexcept;
    static void disable() noexcept;
};

#else

class SignalHandler
{
public:

    static void enable(SignalHandlerCallback &) noexcept {}
    static void disable() noexcept {}
};

#endif // _TV_UNIX

} // namespace tvision

#endif // TVISION_SIGHANDL_H
