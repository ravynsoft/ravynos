// TEST_CONFIG OS=macosx,iphoneos,tvos,watchos

// This test checks that objc_msgSend's recovery path works correctly.
// It continuously runs msgSend on some background threads, then
// triggers the recovery path constantly as a stress test.

#include "test.h"
#include "testroot.i"
#include <dispatch/dispatch.h>

struct Big {
    uintptr_t a, b, c, d, e, f, g;
};

@interface C1: TestRoot
@end
@implementation C1
- (id)idret { return nil; }
- (double)fpret { return 0.0; }
- (long double)lfpret { return 0.0; }
- (struct Big)stret { return (struct Big){}; }
@end

@interface C2: C1
@end
@implementation C2
- (id)idret { return [super idret]; }
- (double)fpret { return [super fpret]; }
- (long double)lfpret { return [super lfpret]; }
- (struct Big)stret { return [super stret]; }
@end

EXTERN_C kern_return_t task_restartable_ranges_synchronize(task_t task);

EXTERN_C void sendWithMsgLookup(id self, SEL _cmd);

#if defined(__arm64__) && !__has_feature(ptrauth_calls)
asm(
"_sendWithMsgLookup:          \n"
"   stp  fp, lr, [sp, #-16]!  \n"
"   mov fp, sp                \n"
"   bl _objc_msgLookup        \n"    
"   mov sp, fp                \n"
"   ldp fp, lr, [sp], #16     \n"
"   br x17                    \n"
);
#elif defined(__x86_64__)
asm(
"_sendWithMsgLookup:      \n"
"   pushq %rbp            \n"
"   movq %rsp, %rbp       \n"
"   callq _objc_msgLookup \n"
"   popq %rbp             \n"
"   jmpq *%r11            \n"
);
#else
// Just skip it.
void sendWithMsgLookup(id self __unused, SEL _cmd __unused) {}
#endif

int main() {
    id obj = [C2 new];
    for(int i = 0; i < 2; i++) {
        dispatch_async(dispatch_get_global_queue(0, 0), ^{
            while(1) {
                [obj idret];
                [obj fpret];
                [obj lfpret];
                [obj stret];
                sendWithMsgLookup(obj, @selector(idret));
            }
        });
    }
    for(int i = 0; i < 1000000; i++) {
        task_restartable_ranges_synchronize(mach_task_self());;
    }
    succeed(__FILE__);
}
