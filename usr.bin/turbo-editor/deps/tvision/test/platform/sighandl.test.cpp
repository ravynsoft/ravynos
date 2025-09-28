#include <internal/sighandl.h>

#ifdef __linux__

#include <test.h>

#include <sys/wait.h>
#include <sys/mman.h>

namespace tvision
{

struct SignalHandlerTest : public ::testing::Test
{
    struct Counters
    {
        size_t handlerInvocations;
        size_t callbackEnterInvocations;
        size_t callbackExitInvocations;
        size_t wrapperInvocations;
    };

    // Counters stored in shared memory.
    static Counters *counters;

    SignalHandlerTest() noexcept
    {
        counters = (Counters *) mmap(nullptr, sizeof(Counters), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        EXPECT_NE(counters, nullptr)
            << "Test bug: 'mmap' failed.";
        *counters = {};
    }

    ~SignalHandlerTest()
    {
        int ret = munmap(counters, sizeof(Counters));
        EXPECT_NE(ret, -1)
            << "Test bug: 'munmap' failed.";

        SignalHandler::disable();
        for (int signo : SignalHandler::handledSignals)
            signal(signo, SIG_DFL);
    }
};

SignalHandlerTest::Counters *SignalHandlerTest::counters;

static bool signalKillsSubprocess(int signo)
{
    auto pid = fork();
    if (pid == 0)
    {
        raise(signo);
        _Exit(0);
    }
    else if (pid > 0)
    {
        int status;
        while (waitpid(pid, &status, 0) != pid);
        if (WIFSIGNALED(status))
            return WTERMSIG(status) == signo;
        return false;
    }
    return false;
}

static void signalHandlerThatDoesNotKillTheProcess(int)
{
    ++SignalHandlerTest::counters->handlerInvocations;
}

static void extendedSignalHandlerThatDoesNotKillTheProcess(int signo, siginfo_t *info, void *context)
{
    ++SignalHandlerTest::counters->handlerInvocations;
}

static void signalCallbackThatTemporarilyDisablesSignalHandler(bool enter) noexcept
{
    // Disable and re-enable SignalHandler because this is also what is done in
    // practice by Platform::signalCallback.
    if (enter)
    {
        ++SignalHandlerTest::counters->callbackEnterInvocations;
        SignalHandler::disable();
    }
    else
    {
        ++SignalHandlerTest::counters->callbackExitInvocations;
        SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    }
}

static void signalCallbackThatAborts(bool enter) noexcept
{
    if (enter)
    {
        ++SignalHandlerTest::counters->callbackEnterInvocations;
        abort();
    }
    else
        ++SignalHandlerTest::counters->callbackExitInvocations;
}

static void signalCallbackThatSegfaults(bool enter) noexcept
{
    if (enter)
    {
        ++SignalHandlerTest::counters->callbackEnterInvocations;
        *(volatile int *) nullptr = 0;
    }
    else
        ++SignalHandlerTest::counters->callbackExitInvocations;
}

static void installSignalHandler(int signo, void (*handler)(int)) noexcept
{
    auto ret = signal(signo, handler);
    EXPECT_NE(ret, SIG_ERR)
        << "Test bug: failed to install signal handler.";
}

static void installExtendedSignalHandler(int signo, void (*handler)(int, siginfo_t *, void *)) noexcept
{
    struct sigaction sa {};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    int ret = sigaction(signo, &sa, nullptr);
    EXPECT_NE(ret, -1)
        << "Test bug: failed to install extended signal handler.";
}

static struct sigaction wrappedSa;

static void signalWrapper(int signo, siginfo_t *info, void *context)
{
    ++SignalHandlerTest::counters->wrapperInvocations;
    if (wrappedSa.sa_flags & SA_SIGINFO)
        wrappedSa.sa_sigaction(signo, info, context);
    else
        wrappedSa.sa_handler(signo);
}

static void installSignalWrapper(int signo)
{
    struct sigaction sa {};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &signalWrapper;
    int ret = sigaction(signo, &sa, &wrappedSa);
    EXPECT_NE(ret, -1)
        << "Test bug: failed to install signal wrapper.";
}

TEST_F(SignalHandlerTest, ShouldOnlyInvokeCallbackOnEnterWhenSignalKillsTheProcess)
{
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    ASSERT_TRUE(signalKillsSubprocess(SIGINT));
    ASSERT_EQ(counters->callbackEnterInvocations, 1);
    ASSERT_EQ(counters->callbackExitInvocations, 0);
}

TEST_F(SignalHandlerTest, ShouldInvokeCallbackOnEnterAndExitWhenSignalIsIgnored)
{
    installSignalHandler(SIGINT, SIG_IGN);
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE(raise(SIGINT), -1);
        ASSERT_EQ(counters->callbackEnterInvocations, i);
        ASSERT_EQ(counters->callbackExitInvocations, i);
    }
}

TEST_F(SignalHandlerTest, ShouldFallBackToDefaultHandlerWhenCallbackAborts)
{
    installSignalHandler(SIGABRT, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatAborts);
    ASSERT_TRUE(signalKillsSubprocess(SIGABRT));
    ASSERT_EQ(counters->handlerInvocations, 0);
    ASSERT_EQ(counters->callbackEnterInvocations, 1);
    ASSERT_EQ(counters->callbackExitInvocations, 0);
}

TEST_F(SignalHandlerTest, ShouldFallBackToDefaultHandlerWhenCallbackSegfaults)
{
    installSignalHandler(SIGSEGV, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatSegfaults);
    ASSERT_TRUE(signalKillsSubprocess(SIGSEGV));
    ASSERT_EQ(counters->handlerInvocations, 0);
    ASSERT_EQ(counters->callbackEnterInvocations, 1);
    ASSERT_EQ(counters->callbackExitInvocations, 0);
}

TEST_F(SignalHandlerTest, ShouldInvokeAPreviouslyInstalledHandler)
{
    installSignalHandler(SIGINT, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE(raise(SIGINT), -1);
        ASSERT_EQ(counters->handlerInvocations, i);
        ASSERT_EQ(counters->callbackEnterInvocations, i);
        ASSERT_EQ(counters->callbackExitInvocations, i);
    }
}

TEST_F(SignalHandlerTest, ShouldInvokeAPreviouslyInstalledExtendedHandler)
{
    installExtendedSignalHandler(SIGINT, extendedSignalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE(raise(SIGINT), -1);
        ASSERT_EQ(counters->handlerInvocations, i);
        ASSERT_EQ(counters->callbackEnterInvocations, i);
        ASSERT_EQ(counters->callbackExitInvocations, i);
    }
}

TEST_F(SignalHandlerTest, ShouldPreserveACustomHandlerInstalledBetween_enable_And_disable_)
{
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    installSignalHandler(SIGINT, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::disable();
    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE(raise(SIGINT), -1);
        ASSERT_EQ(counters->handlerInvocations, i);
        ASSERT_EQ(counters->callbackEnterInvocations, 0);
        ASSERT_EQ(counters->callbackExitInvocations, 0);
    }
}

TEST_F(SignalHandlerTest, ShouldPreserveAPreviouslyInstalledHandlerInstalledAfter_enable_)
{
    installSignalHandler(SIGINT, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    installSignalWrapper(SIGINT);
    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE(raise(SIGINT), -1);
        ASSERT_EQ(counters->handlerInvocations, i);
        ASSERT_EQ(counters->callbackEnterInvocations, i);
        ASSERT_EQ(counters->callbackExitInvocations, i);
        ASSERT_EQ(counters->wrapperInvocations, i);
    }
}

TEST_F(SignalHandlerTest, ShouldNotInvokeNeitherAPreviouslyInstalledHandlerNorTheCallbackIfAWrapperIsInstalledBetween_enable_And_disable_)
{
    installSignalHandler(SIGINT, signalHandlerThatDoesNotKillTheProcess);
    SignalHandler::enable(signalCallbackThatTemporarilyDisablesSignalHandler);
    installSignalWrapper(SIGINT);
    SignalHandler::disable();
    ASSERT_TRUE(signalKillsSubprocess(SIGINT));
    ASSERT_EQ(counters->handlerInvocations, 0);
    ASSERT_EQ(counters->callbackEnterInvocations, 0);
    ASSERT_EQ(counters->callbackExitInvocations, 0);
    ASSERT_EQ(counters->wrapperInvocations, 1);
}

} // namespace tvision

#endif // __linux__
