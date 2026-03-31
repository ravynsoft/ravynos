// TEST_CONFIG

#include "test.h"
#include "testroot.i"
#include <simd/simd.h>
 
#if defined(__arm__) 
// rdar://8331406
#   define ALIGN_() 
#else
#   define ALIGN_() asm(".align 4");
#endif


@interface Super : TestRoot @end

@implementation Super

-(void)voidret_nop
{
    return;
}

-(void)voidret_nop2
{
    return;
}

-(id)idret_nop
{
    return nil;
}

-(long long)llret_nop
{
    return 0;
}

-(struct stret)stret_nop
{
    return STRET_RESULT;
}

-(double)fpret_nop
{
    return 0;
}

-(long double)lfpret_nop
{
    return 0;
}

-(vector_ulong2)vecret_nop
{
    return (vector_ulong2){0x1234567890abcdefULL, 0xfedcba0987654321ULL};
}

@end


@interface Sub : Super @end

@implementation Sub @end


int main()
{

    // cached message performance
    // catches failure to cache or (abi=2) failure to fixup (#5584187)
    // fixme unless they all fail

    uint64_t startTime;
    uint64_t totalTime;
    uint64_t targetTime;

    Sub *sub = [Sub new];

    // fill cache first

    [sub voidret_nop];
    [sub voidret_nop2];
    [sub llret_nop];
    [sub stret_nop];
    [sub fpret_nop];
    [sub lfpret_nop];
    [sub vecret_nop];
    [sub voidret_nop];
    [sub voidret_nop2];
    [sub llret_nop];
    [sub stret_nop];
    [sub fpret_nop];
    [sub lfpret_nop];
    [sub vecret_nop];
    [sub voidret_nop];
    [sub voidret_nop2];
    [sub llret_nop];
    [sub stret_nop];
    [sub fpret_nop];
    [sub lfpret_nop];
    [sub vecret_nop];

    // Some of these times have high variance on some compilers. 
    // The errors we're trying to catch should be catastrophically slow, 
    // so the margins here are generous to avoid false failures.

    // Use voidret because id return is too slow for perf test with ARC.

    // Pick smallest of voidret_nop and voidret_nop2 time
    // in the hopes that one of them didn't collide in the method cache.

    // ALIGN_ matches loop alignment to make -O0 work

#define COUNT 1000000

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub voidret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    testprintf("time: voidret  %llu\n", totalTime);
    targetTime = totalTime;

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub voidret_nop2];  
    }
    totalTime = mach_absolute_time() - startTime;
    testprintf("time: voidret2  %llu\n", totalTime);
    if (totalTime < targetTime) targetTime = totalTime;

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub llret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    timecheck("llret ", totalTime, targetTime * 0.65, targetTime * 2.0);

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub stret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    timecheck("stret ", totalTime, targetTime * 0.65, targetTime * 5.0);

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {        
        [sub fpret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    timecheck("fpret ", totalTime, targetTime * 0.65, targetTime * 4.0);

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub lfpret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    timecheck("lfpret", totalTime, targetTime * 0.65, targetTime * 4.0);

    startTime = mach_absolute_time();
    ALIGN_();
    for (int i = 0; i < COUNT; i++) {
        [sub vecret_nop];
    }
    totalTime = mach_absolute_time() - startTime;
    timecheck("vecret", totalTime, targetTime * 0.65, targetTime * 4.0);

    succeed(__FILE__);
}
