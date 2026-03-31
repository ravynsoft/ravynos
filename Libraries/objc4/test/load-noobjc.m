/*
dyld3 calls the load callback with its own internal lock held, which causes
this test to deadlock. Disable the test in dyld3 mode. If
rdar://problem/53769512 is fixed then remove this.
TEST_CONFIG DYLD=2
TEST_BUILD
    $C{COMPILE} $DIR/load-noobjc.m -o load-noobjc.exe
    $C{COMPILE} $DIR/load-noobjc2.m -o libload-noobjc2.dylib -bundle -bundle_loader load-noobjc.exe
    $C{COMPILE} $DIR/load-noobjc3.m -o libload-noobjc3.dylib -bundle -bundle_loader load-noobjc.exe
END
*/

#include "test.h"
#include <dlfcn.h>

int state = 0;
semaphore_t go;

void *thread(void *arg __unused)
{
    dlopen("libload-noobjc2.dylib", RTLD_LAZY);
    fail("dlopen should not have returned");
}

int main()
{
    semaphore_create(mach_task_self(), &go, SYNC_POLICY_FIFO, 0);

    pthread_t th;
    pthread_create(&th, nil, &thread, nil);

    // Wait for thread to stop in libload-noobjc2's +load method.
    semaphore_wait(go);

    // run nooobjc3's constructor function.
    // There's no objc code here so it shouldn't require the +load lock.
    void *dlh = dlopen("libload-noobjc3.dylib", RTLD_LAZY);
    testassert(dlh);
    testassert(state == 1);

    succeed(__FILE__);
}
