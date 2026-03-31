// Note that test ARCLayoutsWithoutWeak uses the same files 
// with different build options.
/*
TEST_CONFIG MEM=arc
TEST_BUILD
    mkdir -p $T{OBJDIR}
    $C{COMPILE_NOLINK_NOMEM} -c $DIR/MRCBase.m -o $T{OBJDIR}/MRCBase.o
    $C{COMPILE_NOLINK_NOMEM} -c $DIR/MRCARC.m  -o $T{OBJDIR}/MRCARC.o
    $C{COMPILE_NOLINK}       -c $DIR/ARCBase.m -o $T{OBJDIR}/ARCBase.o
    $C{COMPILE_NOLINK}       -c $DIR/ARCMRC.m  -o $T{OBJDIR}/ARCMRC.o
    $C{COMPILE} '-DNAME=\"ARCLayouts.m\"' -fobjc-arc $DIR/ARCLayouts.m -x none $T{OBJDIR}/MRCBase.o $T{OBJDIR}/MRCARC.o $T{OBJDIR}/ARCBase.o $T{OBJDIR}/ARCMRC.o -framework Foundation -o ARCLayouts.exe
END
*/

#include "test.h"
#import <stdio.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

#import "ARCMRC.h"
#import "MRCARC.h"

@interface NSObject (Layouts)
+ (const char *)strongLayout;
+ (const char *)weakLayout;
@end

void printlayout(const char *name, const uint8_t *layout)
{
    if (!testverbose()) return;

    testprintf("%s: ", name);

    // these use fprintf() to avoid repeated VERBOSE: in the middle of the line
    if (!layout) {
        fprintf(stderr, "NULL\n");
        return;
    }

    const uint8_t *c;
    for (c = layout; *c; c++) {
        fprintf(stderr, "%02x ", *c);
    }

    fprintf(stderr, "00\n");
}

@implementation NSObject (Layouts)

+ (const char *)strongLayout {
    const uint8_t *layout = class_getIvarLayout(self);
    printlayout("strong", layout);
    return (const char *)layout;
}

+ (const char *)weakLayout {
    const uint8_t *weakLayout = class_getWeakIvarLayout(self);
    printlayout("weak", weakLayout);
    return (const char *)weakLayout;
}

+ (Ivar)instanceVariable:(const char *)name {
    return class_getInstanceVariable(self, name);
}

@end

void checkMM(Class cls, const char *ivarName, 
             objc_ivar_memory_management_t mmExpected)
{
    Ivar ivar = [cls instanceVariable:ivarName];
    objc_ivar_memory_management_t mm = _class_getIvarMemoryManagement(cls,ivar);
    testprintf("%s->%s want %d, got %d\n", 
               class_getName(cls), ivarName, mmExpected, mm);
    testassert(mmExpected == mm);
}

int main (int argc  __unused, const char * argv[] __unused) {

    testprintf("ARCMRC\n");
    testassert(strcmp([ARCMRC strongLayout], "\x01") == 0);
    testassert([ARCMRC weakLayout] == NULL);


    // Verify that ARCBase->misalign and MRCBase->alignment did their thing.
    testassert(ivar_getOffset(class_getInstanceVariable([ARCBase class], "misalign2")) & 1);
    testassert(ivar_getOffset(class_getInstanceVariable([MRCBase class], "alignment")) > (ptrdiff_t)sizeof(void*));

    testprintf("ARCMisalign\n");
    testassert([ARCMisalign strongLayout] == NULL);
    testassert([ARCMisalign weakLayout] == NULL);

    testprintf("ARCBase\n");
    if (strcmp([ARCBase strongLayout], "\x11\x30") == 0) {
        testwarn("1130 layout is a compiler flaw but doesn't fail");
    } else {
        testassert(strcmp([ARCBase strongLayout], "\x11") == 0);
    }
    testassert(strcmp([ARCBase weakLayout], "\x31") == 0);

    testprintf("MRCARC\n");
    testassert([MRCARC strongLayout] == NULL);
    testassert([MRCARC weakLayout] == NULL);

    testprintf("MRCBase\n");
    // MRC marks __weak only.
    testassert([MRCBase strongLayout] == NULL);
    if (supportsMRCWeak) {
        testassert(strcmp([MRCBase weakLayout], "\x71") == 0);
    } else {
        testassert([MRCBase weakLayout] == nil);
    }
    
    // now check consistency between dynamic accessors and KVC, etc.
    ARCMRC *am = [ARCMRC new];
    MRCARC *ma = [MRCARC new];

    NSString *amValue  = [[NSString alloc] initWithFormat:@"%s %p", "ARCMRC", am];
    NSString *amValue2 = [[NSString alloc] initWithFormat:@"%s %p", "ARCMRC", am];
    NSString *maValue  = [[NSString alloc] initWithFormat:@"%s %p", "MRCARC", ma];
    NSString *maValue2 = [[NSString alloc] initWithFormat:@"%s %p", "MRCARC", ma];

    am.number = M_PI;

    object_setIvar(am, [ARCMRC instanceVariable:"object"], amValue);
    testassert(CFGetRetainCount((__bridge CFTypeRef)amValue) == 1);
    testassert(am.object == amValue);

    object_setIvarWithStrongDefault(am, [ARCMRC instanceVariable:"object"], amValue2);
    testassert(CFGetRetainCount((__bridge CFTypeRef)amValue2) == 2);
    testassert(am.object == amValue2);

    am.pointer = @selector(ARCMRC);

    object_setIvar(am, [ARCMRC instanceVariable:"delegate"], ma);
    testassert(CFGetRetainCount((__bridge CFTypeRef)ma) == 1);
    testassert(am.delegate == ma);

    object_setIvarWithStrongDefault(am, [ARCMRC instanceVariable:"delegate"], ma);
    testassert(CFGetRetainCount((__bridge CFTypeRef)ma) == 1);
    testassert(am.delegate == ma);

    
    ma.number = M_E;

    object_setIvar(ma, [MRCARC instanceVariable:"object"], maValue);
    testassert(CFGetRetainCount((__bridge CFTypeRef)maValue) == 2);
    @autoreleasepool {
        testassert(ma.object == maValue);
    }

    object_setIvarWithStrongDefault(ma, [MRCARC instanceVariable:"object"], maValue2);
    testassert(CFGetRetainCount((__bridge CFTypeRef)maValue2) == 2);
    @autoreleasepool {
        testassert(ma.object == maValue2);
    }

    ma.pointer = @selector(MRCARC);

    ma.delegate = am;
    object_setIvar(ma, [MRCARC instanceVariable:"delegate"], am);
    testassert(CFGetRetainCount((__bridge CFTypeRef)am) == 1);
    @autoreleasepool {
        testassert(ma.delegate == am);
    }

    object_setIvarWithStrongDefault(ma, [MRCARC instanceVariable:"delegate"], am);
    testassert(CFGetRetainCount((__bridge CFTypeRef)am) == 1);
    @autoreleasepool {
        testassert(ma.delegate == am);
    }


    // Verify that object_copy() handles ARC variables correctly.

    MRCARC *ma2 = docopy(ma);
    testassert(ma2);
    testassert(ma2 != ma);
    testassert(CFGetRetainCount((__bridge CFTypeRef)maValue2) == 3);
    testassert(CFGetRetainCount((__bridge CFTypeRef)am) == 1);
    testassert(ma2.number == ma.number);
    testassert(ma2.object == ma.object);
    @autoreleasepool {
        testassert(ma2.delegate == ma.delegate);
    }
    testassert(ma2.pointer == ma.pointer);


    // Test _class_getIvarMemoryManagement() SPI

    objc_ivar_memory_management_t memoryMRCWeak = 
        supportsMRCWeak ? objc_ivar_memoryWeak : objc_ivar_memoryUnknown;
    checkMM([ARCMRC class], "number", objc_ivar_memoryUnknown);
    checkMM([ARCMRC class], "object", objc_ivar_memoryUnknown);
    checkMM([ARCMRC class], "pointer", objc_ivar_memoryUnknown);
    checkMM([ARCMRC class], "delegate", memoryMRCWeak);
    checkMM([ARCMRC class], "dataSource", objc_ivar_memoryStrong);

    checkMM([MRCARC class], "number", objc_ivar_memoryUnretained);
    checkMM([MRCARC class], "object", objc_ivar_memoryStrong);
    checkMM([MRCARC class], "pointer", objc_ivar_memoryUnretained);
    checkMM([MRCARC class], "delegate", objc_ivar_memoryWeak);
    checkMM([MRCARC class], "dataSource", objc_ivar_memoryUnknown);

    checkMM([ARCBase class], "number", objc_ivar_memoryUnretained);
    checkMM([ARCBase class], "object", objc_ivar_memoryStrong);
    checkMM([ARCBase class], "pointer", objc_ivar_memoryUnretained);
    checkMM([ARCBase class], "delegate", objc_ivar_memoryWeak);

    checkMM([MRCBase class], "number", objc_ivar_memoryUnknown);
    checkMM([MRCBase class], "object", objc_ivar_memoryUnknown);
    checkMM([MRCBase class], "pointer", objc_ivar_memoryUnknown);
    checkMM([MRCBase class], "delegate", memoryMRCWeak);
    
    succeed(NAME);
    return 0;
}
