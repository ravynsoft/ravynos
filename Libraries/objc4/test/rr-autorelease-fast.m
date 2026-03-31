// TEST_CONFIG MEM=mrc
// TEST_CFLAGS -Os

#include "test.h"
#include "testroot.i"

#include <objc/objc-internal.h>
#include <objc/objc-abi.h>
#include <Foundation/Foundation.h>

@interface TestObject : TestRoot @end
@implementation TestObject @end


// MAGIC and NOT_MAGIC each call two functions 
// with or without the magic instruction sequence, respectively.
// 
// tmp = first(obj);
// magic, or not;
// tmp = second(tmp);

#if __arm__

#define NOT_MAGIC(first, second)                \
    tmp = first(obj);                           \
    asm volatile("mov r8, r8");                 \
    tmp = second(tmp);

#define MAGIC(first, second)                    \
    tmp = first(obj);                           \
    asm volatile("mov r7, r7");                 \
    tmp = second(tmp);

// arm
#elif __arm64__

#define NOT_MAGIC(first, second)                \
    tmp = first(obj);                           \
    asm volatile("mov x28, x28");               \
    tmp = second(tmp);

#define MAGIC(first, second)                    \
    tmp = first(obj);                           \
    asm volatile("mov x29, x29");               \
    tmp = second(tmp);

// arm64
#elif __x86_64__

#define NOT_MAGIC(first, second) \
    tmp = first(obj);            \
    asm volatile("nop");         \
    tmp = second(tmp);

#define MAGIC(first, second) \
    tmp = first(obj);        \
    tmp = second(tmp);

// x86_64
#elif __i386__

#define NOT_MAGIC(first, second) \
    tmp = first(obj);            \
    tmp = second(tmp);

#define MAGIC(first, second)                             \
    asm volatile("\n subl $16, %%esp"                    \
                 "\n movl %[obj], (%%esp)"               \
                 "\n call _" #first                      \
                 "\n"                                    \
                 "\n movl %%ebp, %%ebp"                  \
                 "\n"                                    \
                 "\n movl %%eax, (%%esp)"                \
                 "\n call _" #second                     \
                 "\n movl %%eax, %[tmp]"                 \
                 "\n addl $16, %%esp"                    \
                 : [tmp] "=r" (tmp)                      \
                 : [obj] "r" (obj)                       \
                 : "eax", "edx", "ecx", "cc", "memory")

// i386
#else

#error unknown architecture

#endif


int
main()
{
    TestObject *tmp, *obj;
    
#ifdef __x86_64__
    // need to get DYLD to resolve the stubs on x86
    PUSH_POOL {
        TestObject *warm_up = [[TestObject alloc] init];
        testassert(warm_up);
        warm_up = objc_retainAutoreleasedReturnValue(warm_up);
        warm_up = objc_unsafeClaimAutoreleasedReturnValue(warm_up);
        [warm_up release];
        warm_up = nil;
    } POP_POOL;
#endif
    
    testprintf("  Successful +1 -> +1 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        MAGIC(objc_autoreleaseReturnValue, 
              objc_retainAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 0);
        
        [tmp release];
        testassert(TestRootDealloc == 1);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 0);

    } POP_POOL;
    
    testprintf("Unsuccessful +1 -> +1 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        NOT_MAGIC(objc_autoreleaseReturnValue, 
                  objc_retainAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 1);
        
        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 1);

    } POP_POOL;
    testassert(TestRootDealloc == 1);
    testassert(TestRootRetain == 1);
    testassert(TestRootRelease == 2);
    testassert(TestRootAutorelease == 1);


    testprintf("  Successful +0 -> +1 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        MAGIC(objc_retainAutoreleaseReturnValue, 
              objc_retainAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 0);
        
        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 0);

        [tmp release];
        testassert(TestRootDealloc == 1);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 2);
        testassert(TestRootAutorelease == 0);

    } POP_POOL;
    
    testprintf("Unsuccessful +0 -> +1 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        NOT_MAGIC(objc_retainAutoreleaseReturnValue, 
                  objc_retainAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 2);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 1);
        
        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 2);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 1);

        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 2);
        testassert(TestRootRelease == 2);
        testassert(TestRootAutorelease == 1);

    } POP_POOL;
    testassert(TestRootDealloc == 1);
    testassert(TestRootRetain == 2);
    testassert(TestRootRelease == 3);
    testassert(TestRootAutorelease == 1);


    testprintf("  Successful +1 -> +0 handshake\n");
    
    PUSH_POOL {
        obj = [[[TestObject alloc] init] retain];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        MAGIC(objc_autoreleaseReturnValue, 
              objc_unsafeClaimAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 0);
        
        [tmp release];
        testassert(TestRootDealloc == 1);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 2);
        testassert(TestRootAutorelease == 0);

    } POP_POOL;

    testprintf("Unsuccessful +1 -> +0 handshake\n");
    
    PUSH_POOL {
        obj = [[[TestObject alloc] init] retain];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        NOT_MAGIC(objc_autoreleaseReturnValue, 
                  objc_unsafeClaimAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 1);
        
        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 1);

    } POP_POOL;
    testassert(TestRootDealloc == 1);
    testassert(TestRootRetain == 0);
    testassert(TestRootRelease == 2);
    testassert(TestRootAutorelease == 1);

    
    testprintf("  Successful +0 -> +0 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        MAGIC(objc_retainAutoreleaseReturnValue, 
              objc_unsafeClaimAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 0);
        
        [tmp release];
        testassert(TestRootDealloc == 1);
        testassert(TestRootRetain == 0);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 0);

    } POP_POOL;

    testprintf("Unsuccessful +0 -> +0 handshake\n");
    
    PUSH_POOL {
        obj = [[TestObject alloc] init];
        testassert(obj);
        
        TestRootRetain = 0;
        TestRootRelease = 0;
        TestRootAutorelease = 0;
        TestRootDealloc = 0;

        NOT_MAGIC(objc_retainAutoreleaseReturnValue, 
                  objc_unsafeClaimAutoreleasedReturnValue);

        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 0);
        testassert(TestRootAutorelease == 1);
        
        [tmp release];
        testassert(TestRootDealloc == 0);
        testassert(TestRootRetain == 1);
        testassert(TestRootRelease == 1);
        testassert(TestRootAutorelease == 1);

    } POP_POOL;
    testassert(TestRootDealloc == 1);
    testassert(TestRootRetain == 1);
    testassert(TestRootRelease == 2);
    testassert(TestRootAutorelease == 1);

    succeed(__FILE__);
    
    return 0;
}

