#include <dlfcn.h>
#include <Block.h>
#include <spawn.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <os/lock.h>
#include <sys/attr.h>
#include <sys/wait.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <mach/mach_vm.h>
#include <sys/fsgetpath.h>
#include <mach-o/getsect.h>
#include <mach/vm_region.h>
#include <dispatch/private.h>
#include <dispatch/dispatch.h>


#include <atomic>
#include <utility>
#include <algorithm>

extern "C" {
#include "execserverServer.h"

union catch_mach_exc_request_reply {
    union __RequestUnion__catch_mach_exc_subsystem request;
    union __ReplyUnion__catch_mach_exc_subsystem reply;
};
};

#include "test_support.h"

extern const int    NXArgc;
extern const char**  NXArgv;
extern const char**  environ;
extern char*        __progname;
#if __x86_64__
static const cpu_type_t currentArch = CPU_TYPE_X86_64;
#elif __i386__
static const cpu_type_t currentArch = CPU_TYPE_I386;
#elif __arm64__
static const cpu_type_t currentArch = CPU_TYPE_ARM64;
#elif __arm__
static const cpu_type_t currentArch = CPU_TYPE_ARM;
#endif

namespace {
struct ScopedLock {
    ScopedLock() : _lock(OS_UNFAIR_LOCK_INIT) {}
    template<typename F>
    void withLock(F f) {
        os_unfair_lock_lock(&_lock);
        f();
        os_unfair_lock_unlock(&_lock);
    }
private:
    os_unfair_lock _lock;
};

template <typename T, int QUANT=4, int INIT=1>
class GrowableArray
{
public:

    T&              operator[](size_t idx)       { assert(idx < _usedCount); return _elements[idx]; }
    const T&        operator[](size_t idx) const { assert(idx < _usedCount); return _elements[idx]; }
    T&              back()                       { assert(_usedCount > 0); return _elements[_usedCount-1]; }
    uintptr_t       count() const                { return _usedCount; }
    uintptr_t       maxCount() const             { return _allocCount; }
    bool            empty() const                { return (_usedCount == 0); }
    uintptr_t       index(const T& element)      { return &element - _elements; }
    void            push_back(const T& t)        { verifySpace(1); _elements[_usedCount++] = t; }
    void            pop_back()                   { assert(_usedCount > 0); _usedCount--; }
    T*              begin()                      { return &_elements[0]; }
    T*              end()                        { return &_elements[_usedCount]; }
    const T*        begin() const                { return &_elements[0]; }
    const T*        end() const                  { return &_elements[_usedCount]; }
    bool            contains(const T& targ) const { for (const T& a : *this) { if ( a == targ ) return true; } return false; }
    void            erase(T& targ);

protected:
    void            growTo(uintptr_t n);
    void            verifySpace(uintptr_t n)     { if (this->_usedCount+n > this->_allocCount) growTo(this->_usedCount + n); }

private:
    T*              _elements               = _initialAlloc;
    uintptr_t       _allocCount             = INIT;
    uintptr_t       _usedCount              = 0;
    T               _initialAlloc[INIT]     = { };
};


template <typename T, int QUANT, int INIT>
inline void GrowableArray<T,QUANT,INIT>::growTo(uintptr_t n)
{
    uintptr_t newCount = (n + QUANT - 1) & (-QUANT);
    T* newArray = (T*)::malloc(sizeof(T)*newCount);
    T* oldArray = this->_elements;
    if ( this->_usedCount != 0 )
        ::memcpy(newArray, oldArray, sizeof(T)*this->_usedCount);
    this->_elements   = newArray;
    this->_allocCount = newCount;
    if ( oldArray != this->_initialAlloc )
        ::free(oldArray);
}

template <typename T, int QUANT, int INIT>
inline void GrowableArray<T,QUANT,INIT>::erase(T& targ)
{
    intptr_t index = &targ - _elements;
    assert(index >= 0);
    assert(index < (intptr_t)_usedCount);
    intptr_t moveCount = _usedCount-index-1;
    if ( moveCount > 0 )
        ::memcpy(&_elements[index], &_elements[index+1], moveCount*sizeof(T));
    _usedCount -= 1;
}

struct TestState {
    TestState();
    static TestState* getState();
    void _PASSV(const char* file, unsigned line, const char* format, va_list args) __attribute__ ((noreturn));
    void _FAILV(const char* file, unsigned line, const char* format, va_list args) __attribute__ ((noreturn));
    void _LOGV(const char* file, unsigned line, const char* format, va_list args);
    GrowableArray<std::pair<mach_port_t, _dyld_test_crash_handler_t>>& getCrashHandlers();
private:
    enum OutputStyle {
        None,
        BATS,
        Console,
        XCTest
    };
    void runLeaks();
    void dumpLogs();
    static uint8_t hexCharToUInt(const char hexByte, uint8_t* value);
    static uint64_t hexToUInt64(const char* startHexByte, const char** endHexByte);
    
    ScopedLock _IOlock;
    GrowableArray<const char *> logs;
    const char *testName;
    bool logImmediate;
    bool logOnSuccess;
    bool checkForLeaks;
    OutputStyle output;
    GrowableArray<std::pair<mach_port_t, _dyld_test_crash_handler_t>> crashHandlers;
};

// Okay, this is tricky. We need something with roughly he semantics of a weak def, but without using weak defs as their presence
// m ay impact certain tests. Instead we do the following:
//
// 1. Embed a stuct containing a lock and a pointer to our global state object in eahc binary
// 2. Once per binary we walk the entire image list looking for the first entry that also has state data
// 3. If it has state we lock its initializaion lock, and if it is not initialized we initialize it
// 4. We then copy the initalized pointer into our own state, and unlock the initializer lock
//
// This should work because the image list forms a stable ordering. The one loose end is if an executable is  running where logging
// is only used in dylibs that are all being dlopned() and dlclosed. Since many dylibs cannot be dlclosed that should be a non-issue
// in practice.
};

__attribute__((section("__DATA,__dyld_test")))
static std::atomic<TestState*> sState;

kern_return_t
catch_mach_exception_raise(mach_port_t exception_port,
                           mach_port_t thread,
                           mach_port_t task,
                           exception_type_t exception,
                           mach_exception_data_t code,
                           mach_msg_type_number_t codeCnt)
{
    _dyld_test_crash_handler_t crashHandler = NULL;
    for (const auto& handler : TestState::getState()->getCrashHandlers()) {
        if (handler.first == exception_port) {
            crashHandler = handler.second;
        }
    }
    if (crashHandler) {
        if (exception == EXC_CORPSE_NOTIFY) {
            crashHandler(task);
        } else {
            return KERN_FAILURE;
        }
    }
    return KERN_SUCCESS;
}

kern_return_t
catch_mach_exception_raise_state(mach_port_t exception_port,
                                 exception_type_t exception,
                                 const mach_exception_data_t code,
                                 mach_msg_type_number_t codeCnt,
                                 int * flavor,
                                 const thread_state_t old_state,
                                 mach_msg_type_number_t old_stateCnt,
                                 thread_state_t new_state,
                                 mach_msg_type_number_t * new_stateCnt)
{
    return KERN_NOT_SUPPORTED;
}

kern_return_t
catch_mach_exception_raise_state_identity(mach_port_t exception_port,
                                          mach_port_t thread,
                                          mach_port_t task,
                                          exception_type_t exception,
                                          mach_exception_data_t code,
                                          mach_msg_type_number_t codeCnt,
                                          int * flavor,
                                          thread_state_t old_state,
                                          mach_msg_type_number_t old_stateCnt,
                                          thread_state_t new_state,
                                          mach_msg_type_number_t * new_stateCnt)
{
    return KERN_NOT_SUPPORTED;
}

_process::_process() :  executablePath(nullptr), args(nullptr), env(nullptr), stdoutHandler(nullptr), stderrHandler(nullptr),
                        crashHandler(nullptr), exitHandler(nullptr), pid(0), arch(currentArch), suspended(false), async(false) {}
_process::~_process() {
    if (stdoutHandler) { Block_release(stdoutHandler);}
    if (stderrHandler) { Block_release(stderrHandler);}
    if (crashHandler) { Block_release(crashHandler);}
    if (exitHandler) { Block_release(exitHandler);}
}

void _process::set_executable_path(const char* EP) { executablePath = EP; }
void _process::set_args(const char** A) { args = A; }
void _process::set_env(const char** E) { env = E; }
void _process::set_stdout_handler(_dyld_test_reader_t SOH) { stdoutHandler = Block_copy(SOH); };
void _process::set_stderr_handler(_dyld_test_reader_t SEH) { stderrHandler = Block_copy(SEH); }
void _process::set_exit_handler(_dyld_test_exit_handler_t EH) { exitHandler = Block_copy(EH); }
void _process::set_crash_handler(_dyld_test_crash_handler_t CH) { crashHandler = Block_copy(CH); }
void _process::set_launch_suspended(bool S) { suspended = S; }
void _process::set_launch_async(bool S) { async = S; }
void _process::set_launch_arch(cpu_type_t A) { arch = A; }

pid_t _process::launch() {
    dispatch_queue_t queue = dispatch_queue_create("com.apple.dyld.test.launch", NULL);
    dispatch_block_t oneShotSemaphoreBlock = dispatch_block_create(DISPATCH_BLOCK_INHERIT_QOS_CLASS, ^{});
    posix_spawn_file_actions_t fileActions = NULL;
    posix_spawnattr_t attr = NULL;
    dispatch_source_t stdoutSource = NULL;
    dispatch_source_t stderrSource = NULL;
    int stdoutPipe[2];
    int stderrPipe[2];

    if (posix_spawn_file_actions_init(&fileActions) != 0) {
        FAIL("Setting up spawn filea actions");
    }
    if (posix_spawnattr_init(&attr) != 0) { FAIL("Setting up spawn attr"); }
    if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED) != 0) {
        FAIL("Setting up spawn attr: POSIX_SPAWN_START_SUSPENDED");
    }

    if (pipe(stdoutPipe) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[0]) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_adddup2(&fileActions, stdoutPipe[1], STDOUT_FILENO) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[1]) != 0) { FAIL("Setting up pipe"); }
    fcntl((int)stdoutPipe[0], F_SETFL, O_NONBLOCK);
    stdoutSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)stdoutPipe[0], 0, queue);
    dispatch_source_set_event_handler(stdoutSource, ^{
        int fd = (int)dispatch_source_get_handle(stdoutSource);
        if (stdoutHandler) {
            stdoutHandler(fd);
        } else {
            char buffer[16384];
            ssize_t size = 0;
            do {
                size = read(fd, &buffer[0], 16384);
            } while (size > 0);
        }
    });
    dispatch_source_set_cancel_handler(stdoutSource, ^{
        dispatch_release(stdoutSource);
    });
    dispatch_resume(stdoutSource);

    if (pipe(stderrPipe) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_addclose(&fileActions, stderrPipe[0]) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_adddup2(&fileActions, stderrPipe[1], STDERR_FILENO) != 0) { FAIL("Setting up pipe"); }
    if (posix_spawn_file_actions_addclose(&fileActions, stderrPipe[1]) != 0) { FAIL("Setting up pipe"); }
    fcntl((int)stderrPipe[0], F_SETFL, O_NONBLOCK);
    stderrSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)stderrPipe[0], 0, queue);
    dispatch_source_set_event_handler(stderrSource, ^{
        int fd = (int)dispatch_source_get_handle(stderrSource);
        if (stderrHandler) {
            stderrHandler(fd);
        } else {
            char buffer[16384];
            ssize_t size = 0;
            do {
                size = read(fd, &buffer[0], 16384);
            } while (size > 0);
        }
    });
    dispatch_source_set_cancel_handler(stderrSource, ^{
        dispatch_release(stderrSource);
    });
    dispatch_resume(stderrSource);
    
    if (crashHandler) {
        auto& crashHandlers = TestState::getState()->getCrashHandlers();
        mach_port_t exceptionPort = MACH_PORT_NULL;
        mach_port_options_t options = { .flags = MPO_CONTEXT_AS_GUARD | MPO_STRICT | MPO_INSERT_SEND_RIGHT, .mpl = { 1 }};
        if ( mach_port_construct(mach_task_self(), &options, (mach_port_context_t)exceptionPort, &exceptionPort) != KERN_SUCCESS ) {
            FAIL("Could not construct port");
        }
        if (posix_spawnattr_setexceptionports_np(&attr,  EXC_MASK_CRASH | EXC_MASK_CORPSE_NOTIFY, exceptionPort,
                                                 EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES, 0) != 0) {
            FAIL("posix_spawnattr_setexceptionports_np failed");
        }
        crashHandlers.push_back(std::make_pair(exceptionPort, crashHandler));
        dispatch_source_t crashSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, exceptionPort, 0, queue);
        dispatch_source_set_event_handler(crashSource, ^{
            dispatch_mig_server(crashSource, sizeof(union catch_mach_exc_request_reply), ::mach_exc_server);
        });
        dispatch_source_set_cancel_handler(crashSource, ^{
            mach_port_destruct(mach_task_self(), exceptionPort, 0, (mach_port_context_t)exceptionPort);
        });
        dispatch_resume(crashSource);
    }

    pid_t pid;
    uint32_t argc = 0;
    if (args) {
        for (argc = 0; args[argc] != NULL; ++argc) {}
    }
    ++argc;
    const char *argv[argc+1];
    argv[0] = executablePath;
    for (uint32_t i = 1; i < argc; ++i) {
        argv[i] = args[i-1];
    }
    argv[argc] = NULL;
    
    int result = posix_spawn(&pid, executablePath, &fileActions, &attr, (char **)argv, (char **)env);
    if ( result != 0 ) {
        FAIL("posix_spawn(%s) failed, err=%d", executablePath, result);
    }
    dispatch_source_t exitSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, (pid_t)pid,
                                                            DISPATCH_PROC_EXIT, queue);
    dispatch_source_set_event_handler(exitSource, ^{
        if (exitHandler) {
            exitHandler((pid_t)dispatch_source_get_handle(exitSource));
        }
        dispatch_source_cancel(exitSource);
        if (stdoutSource) {
            dispatch_source_cancel(stdoutSource);
        }
        if (stderrSource) {
            dispatch_source_cancel(stderrSource);
        }
        oneShotSemaphoreBlock();
        dispatch_source_cancel(exitSource);
    });
    dispatch_resume(exitSource);

    if (stdoutHandler) {
        close(stdoutPipe[1]);
    }
    if (stderrHandler) {
        close(stderrPipe[1]);
    }
    if (fileActions) {
        posix_spawn_file_actions_destroy(&fileActions);
    }
    posix_spawnattr_destroy(&attr);
    if (!suspended) {
        kill(pid, SIGCONT);
    }
    if (!async) {
        dispatch_block_wait(oneShotSemaphoreBlock, DISPATCH_TIME_FOREVER);
    }
    Block_release(oneShotSemaphoreBlock);
    dispatch_release(queue);
    return pid;
}

void *_process::operator new(size_t size) {
    return malloc(size);
}

void _process::operator delete(void *ptr) {
    free(ptr);
}

// MARK: Private implementation details

template<typename F>
static
void forEachEnvVar(const char* envp[], F&& f) {
    for (uint32_t i = 0; envp[i] != nullptr; ++i) {
        const char* envBegin = envp[i];
        const char* envEnd = strchr(envp[i], '=');
        if (!envEnd) { continue; }
        size_t envSize = (envEnd-envBegin)+1;
        const char* valBegin = envEnd+1;
        const char* valEnd = strchr(envp[i], '\0');
        if (!valEnd) { continue; }
        size_t valSize = (valEnd-valBegin)+1;
        char env[envSize];
        char val[valSize];
        strlcpy(&env[0], envBegin, envSize);
        strlcpy(&val[0], valBegin, valSize);
        f(&env[0], &val[0]);
    }
}

uint8_t TestState::hexCharToUInt(const char hexByte, uint8_t* value) {
    if (hexByte >= '0' && hexByte <= '9') {
        *value = hexByte - '0';
        return true;
    } else if (hexByte >= 'A' && hexByte <= 'F') {
        *value = hexByte - 'A' + 10;
        return true;
    } else if (hexByte >= 'a' && hexByte <= 'f') {
        *value = hexByte - 'a' + 10;
        return true;
    }

    return false;
}

uint64_t TestState::hexToUInt64(const char* startHexByte, const char** endHexByte) {
    const char* scratch;
    if (endHexByte == NULL) {
        endHexByte = &scratch;
    }
    if (startHexByte == NULL)
        return 0;
    uint64_t retval = 0;
    if (startHexByte[0] == '0' &&  startHexByte[1] == 'x') {
        startHexByte +=2;
    }
    *endHexByte = startHexByte + 16;

    //FIXME overrun?
    for (uint32_t i = 0; i < 16; ++i) {
        uint8_t value;
        if (!hexCharToUInt(startHexByte[i], &value)) {
            *endHexByte = &startHexByte[i];
            break;
        }
        retval = (retval << 4) + value;
    }
    return retval;
}

TestState::TestState() : testName(__progname), logImmediate(false), logOnSuccess(false),  checkForLeaks(false), output(Console) {
    forEachEnvVar(environ, [this](const char* env, const char* val) {
        if (strcmp(env, "TEST_LOG_IMMEDIATE") == 0) {
            logImmediate = true;
        }
        if (strcmp(env, "TEST_LOG_ON_SUCCESS") == 0) {
            logOnSuccess = true;
        }
        if (strcmp(env, "MallocStackLogging") == 0) {
            checkForLeaks = true;
        }
        if (strcmp(env, "TEST_OUTPUT") == 0) {
            if (strcmp(val, "BATS") == 0) {
                output = BATS;
            } else if (strcmp(val, "XCTest") == 0) {
                output = XCTest;
            }
        }
    });
    if (output == BATS) {
        printf("[BEGIN]");
        if (checkForLeaks) {
            printf(" MallocStackLogging=1 MallocDebugReport=none");
        }
        forEachEnvVar(environ, [this](const char* env, const char* val) {
            if ((strncmp(env, "DYLD_", 5) == 0) || (strncmp(env, "TEST_", 5) == 0)) {
                printf(" %s=%s", env, val);
            }
        });
        printf(" %s", testName);
        for (uint32_t i = 1; i < NXArgc; ++i) {
            printf(" %s", NXArgv[i]);
        }
        printf("\n");
    }
}

static std::atomic<TestState*>& getExecutableImageState() {
    uint32_t imageCnt = _dyld_image_count();
    for (uint32_t i = 0; i < imageCnt; ++i) {
        #if __LP64__
            const struct mach_header_64* mh = (const struct mach_header_64*)_dyld_get_image_header(i);
        #else
            const struct mach_header* mh = _dyld_get_image_header(i);
        #endif
        if (mh->filetype != MH_EXECUTE) {
            continue;
        }
        size_t size = 0;
        auto state = (std::atomic<TestState*>*)getsectiondata(mh, "__DATA", "__dyld_test", &size);
        if (!state) {
            fprintf(stderr, "Could not find test state in main executable TestState\n");
            exit(0);
        }
        return *state;
    }
    fprintf(stderr, "Could not find test state in main executable\n");
    exit(0);
}

GrowableArray<std::pair<mach_port_t, _dyld_test_crash_handler_t>>& TestState::getCrashHandlers() {
    return crashHandlers;
}

TestState* TestState::getState() {
    if (!sState) {
        auto& state = getExecutableImageState();
        if (state == nullptr) {
            void *temp = malloc(sizeof(TestState));
            auto newState = new (temp) TestState();
            TestState* expected = nullptr;
            if(!state.compare_exchange_strong(expected, newState)) {
                newState->~TestState();
                free(temp);
            }
        }
        sState.store(state);
    }
    assert(sState != nullptr);
    return sState;
}

__attribute__((noreturn))
void TestState::runLeaks(void) {
    auto testState = TestState::getState();
    pid_t pid = getpid();
    char pidString[32];
    sprintf(&pidString[0], "%d", pid);
    if (getuid() != 0) {
        printf("Insufficient priviledges, skipping Leak check: %s\n", testState->testName);
        exit(0);
    }
    const char *args[] = { pidString, NULL };
    // We do this instead of using a dispatch_semaphore to prevent priority inversions
    __block dispatch_data_t leaksOutput = NULL;
    _process process;
    process.set_executable_path("/usr/bin/leaks");
    process.set_args(args);
    process.set_stdout_handler(^(int fd) {
        ssize_t size = 0;
        do {
            char buffer[16384];
            size = read(fd, &buffer[0], 16384);
            if (size == -1) { break; }
            dispatch_data_t data = dispatch_data_create(&buffer[0], size, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
            if (!leaksOutput) {
                leaksOutput = data;
            } else {
                leaksOutput = dispatch_data_create_concat(leaksOutput, data);
            }
        } while (size > 0);
    });
    process.set_exit_handler(^(pid_t pid) {
        int status = 0;
        (void)waitpid(pid, &status, 0);

        int exitStatus = WEXITSTATUS(status);
        if (exitStatus == 0) {
            PASS("No leaks");
        } else {
            if (leaksOutput) {
                const void * buffer;
                size_t size;
                __unused dispatch_data_t map = dispatch_data_create_map(leaksOutput, &buffer, &size);
                FAIL("Found Leaks:\n\n%s", buffer);
            }
        }
    });

    testState->checkForLeaks = false;
    (void)process.launch();
    exit(0);
}

void TestState::_PASSV(const char* file, unsigned line, const char* format, va_list args) {
    if (output == None) {
        exit(0);
    }
    if (checkForLeaks) {
        runLeaks();
    } else {
        _IOlock.withLock([this,&format,&args,&file,&line](){
            if (output == Console) {
                printf("[\033[0;32mPASS\033[0m] %s: ", testName);
                vprintf(format, args);
                printf("\n");
                if (logOnSuccess && logs.count()) {
                    printf("[\033[0;33mLOG\033[0m]\n");
                    for (const auto& log : logs) {
                        printf("\t%s\n", log);
                    }
                }
            } else if (output == BATS)  {
                printf("[PASS] %s: ", testName);
                vprintf(format, args);
                printf("\n");
                if (logOnSuccess && logs.count()) {
                    printf("[LOG]\n");
                    for (const auto& log : logs) {
                        printf("\t%s\n", log);
                    }
                }
            } else if (output == XCTest) {
                printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
                printf("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
                printf("<plist version=\"1.0\">");
                printf("<dict>");
                printf("<key>PASS</key><true />");
                printf("</dict>");
                printf("</plist>");
            }
        });
        exit(0);
    }
}

void _PASS(const char* file, unsigned line, const char* format, ...)  {
    va_list args;
    va_start (args, format);
    TestState::getState()->_PASSV(file, line, format, args);
    va_end (args);
}

void TestState::_FAILV(const char* file, unsigned line, const char* format, va_list args) {
    if (output == None) {
        exit(0);
    }
    _IOlock.withLock([this,&format,&args,&file,&line](){
        if (output == Console) {
            printf("[\033[0;31mFAIL\033[0m] %s: ", testName);
            vprintf(format, args);
            printf("\n");
            printf("[\033[0;33mLOG\033[0m]\n");
            if (logs.count()) {
                for (const auto& log : logs) {
                    printf("\t%s\n", log);
                }
            }
        } else if (output == BATS)  {
            printf("[FAIL] %s: ", testName);
            vprintf(format, args);
            printf("\n");
            if (logs.count()) {
                printf("[LOG]\n");
                for (const auto& log : logs) {
                    printf("\t%s\n", log);
                }
            }
        } else if (output == XCTest) {
            char *buffer;
            printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            printf("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");
            printf("<plist version=\"1.0\">");
            printf("<dict>");
            printf("<key>PASS</key><false />");
            printf("<key>FILE</key><string>%s</string>", file);
            printf("<key>LINE</key><integer>%u</integer>", line);
            vasprintf(&buffer, format, args);
            printf("<key>INFO</key><string>%s</string>", buffer);
            free(buffer);
            printf("</dict>");
            printf("</plist>");
        }
    });
    exit(0);
}

void _FAIL(const char* file, unsigned line, const char* format, ...)  {
    va_list args;
    va_start (args, format);
    TestState::getState()->_FAILV(file, line, format, args);
    va_end (args);
}

void TestState::_LOGV(const char* file, unsigned line, const char* format, va_list args) {
    _IOlock.withLock([this,&format,&args](){
        if (logImmediate) {
            vprintf(format, args);
            printf("\n");
        } else {
            char *str;
            vasprintf(&str, format, args);
            logs.push_back(str);
        }
    });
}

void _LOG(const char* file, unsigned line, const char* format, ...)  {
    va_list args;
    va_start (args, format);
    TestState::getState()->_LOGV(file, line, format, args);
    va_end (args);
}

void _TIMEOUT(const char* file, unsigned line, uint64_t seconds) {
    _LOG(file, line, "Registering %llu second test timeout", seconds);
    dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, DISPATCH_TARGET_QUEUE_DEFAULT);
    dispatch_time_t milestone = dispatch_time(DISPATCH_WALLTIME_NOW, seconds * NSEC_PER_SEC);
    dispatch_source_set_timer(source, milestone, 0, 0);
    dispatch_source_set_event_handler(source, ^{
        FAIL("Test timed out");
    });
    dispatch_resume(source);
}
