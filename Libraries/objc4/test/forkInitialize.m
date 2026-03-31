/*
TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: \+\[BlockingSub initialize\] may have been in progress in another thread when fork\(\) was called\.
objc\[\d+\]: \+\[BlockingSub initialize\] may have been in progress in another thread when fork\(\) was called\. We cannot safely call it or ignore it in the fork\(\) child process\. Crashing instead\. Set a breakpoint on objc_initializeAfterForkError to debug\.
objc\[\d+\]: HALTED
OK: forkInitialize\.m
END
*/

#include "test.h"

static void *retain_fn(void *self, SEL _cmd __unused) { return self; }
static void release_fn(void *self __unused, SEL _cmd __unused) { }

OBJC_ROOT_CLASS
@interface BlockingRootClass @end
@implementation BlockingRootClass
+(id)self { return self; }
+(void)initialize {
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");

    if (self == [BlockingRootClass self]) {
        while (1) sleep(1);
    }
}
@end

@interface BlockingRootSub : BlockingRootClass @end
@implementation BlockingRootSub
@end

OBJC_ROOT_CLASS
@interface BlockingSubRoot @end
@implementation BlockingSubRoot
+(id)self { return self; }
+(void)initialize {
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");
}
@end

@interface BlockingSub : BlockingSubRoot @end
@implementation BlockingSub
+(void)initialize {
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");

    while (1) sleep(1);
}
@end

OBJC_ROOT_CLASS
@interface AnotherRootClass @end

@interface BoringSub : AnotherRootClass @end
@implementation BoringSub
// can't implement +initialize here
@end

@implementation AnotherRootClass

void doFork()
{
    testprintf("FORK\n");

    pid_t child;
    switch((child = fork())) {
      case -1:
        fail("fork failed");
      case 0:
        // child
        // This one succeeds even though we're nested inside it's
        // superclass's +initialize, because ordinary +initialize nesting
        // still works across fork().
        // This falls in the isInitializing() case in _class_initialize.
        [BoringSub self];

#if !SINGLETHREADED
        // This one succeeds even though another thread is in its
        // superclass's +initialize, because that superclass is a root class
        // so we assume that +initialize is empty and therefore this one
        // is safe to call.
        // This falls in the reallyInitialize case in _class_initialize.
        [BlockingRootSub self];

        // This one aborts without deadlocking because it was in progress
        // when fork() was called.
        // This falls in the isInitializing() case in _class_initialize.
        [BlockingSub self];
        
        fail("should have crashed");
#endif
        break;
      default: {
        // parent
        int result = 0;
        while (waitpid(child, &result, 0) < 0) {
            if (errno != EINTR) {
                fail("waitpid failed (errno %d %s)", 
                     errno, strerror(errno));
            }
        }
        if (!WIFEXITED(result)) {
            fail("child crashed (waitpid result %d)", result);
        }
        break;
      }
    }
}

+(void)initialize {
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");

    if (self == [AnotherRootClass self]) {
        static bool called = false;
        if (!called) {
            doFork();
            called = true;
        } else {
            fail("+[AnotherRootClass initialize] called again");
        }
    }
}

+(id)self {
    return self;
}
@end


void *blocker(void *arg __unused)
{
    [BlockingSub self];
    return nil;
}

void *blocker2(void *arg __unused)
{
    [BlockingRootClass self];
    return nil;
}

int main()
{
#if !SINGLETHREADED
    pthread_t th;
    pthread_create(&th, nil, blocker, nil);
    pthread_detach(th);
    pthread_create(&th, nil, blocker2, nil);
    pthread_detach(th);
    sleep(1);
#endif
    
    [AnotherRootClass self];
    succeed(__FILE__);
}
