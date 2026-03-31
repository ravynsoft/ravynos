// TEST_CONFIG MEM=mrc

#import "test.h"
#import "testroot.i"

#import <objc/objc-internal.h>

#import <stdio.h>

char dummy;

Class *seenClasses;
size_t seenClassesCount;

static void clear(void) {
    free(seenClasses);
    seenClasses = NULL;
    seenClassesCount = 0;
}

static void willInitializeClass(void *context, Class cls) {
    testprintf("Will initialize %s\n", class_getName(cls));
    seenClassesCount++;
    seenClasses = (Class *)realloc(seenClasses, seenClassesCount * sizeof(*seenClasses));
    seenClasses[seenClassesCount - 1] = cls;
    testassert(context == &dummy);
}

int initializedC;
@interface C: TestRoot @end
@implementation C
+ (void)initialize {
    testprintf("C initialize\n");
    initializedC = 1;
}
@end

int initializedD;
@interface D: TestRoot @end
@implementation D
+ (void)initialize {
    testprintf("D initialize\n");
    initializedD = 1;
}
@end

int initializedE;
@interface E: TestRoot @end
@implementation E
+ (void)initialize {
    testprintf("E initialize\n");
    initializedE = 1;
}
@end

int main()
{
    _objc_addWillInitializeClassFunc(willInitializeClass, &dummy);

    // Merely getting a class should not trigger the callback.
    clear();
    size_t oldCount = seenClassesCount;
    Class c = objc_getClass("C");
    testassert(seenClassesCount == oldCount);
    testassert(initializedC == 0);

    // Sending a message to C should trigger the callback and the superclass's callback.
    [c class];
    testassert(seenClassesCount == oldCount + 2);
    testassert(seenClasses[seenClassesCount - 2] == [TestRoot class]);
    testassert(seenClasses[seenClassesCount - 1] == [C class]);

    // Sending a message to D should trigger the callback only for D, since the
    // superclass is already initialized.
    oldCount = seenClassesCount;
    [D class];
    testassert(seenClassesCount == oldCount + 1);
    testassert(seenClasses[seenClassesCount - 1] == [D class]);

    // Registering a second callback should inform us of all three exactly once.
    clear();
    _objc_addWillInitializeClassFunc(willInitializeClass, &dummy);
    testassert(seenClassesCount == 3);

    int foundRoot = 0;
    int foundC = 0;
    int foundD = 0;
    for (size_t i = 0; i < seenClassesCount; i++) {
        if (seenClasses[i] == [TestRoot class])
            foundRoot++;
        if (seenClasses[i] == [C class])
            foundC++;
        if (seenClasses[i] == [D class])
            foundD++;
    }
    testassert(foundRoot == 1);
    testassert(foundC == 1);
    testassert(foundD == 1);
    
    // Both callbacks should fire when sending a message to E.
    clear();
    [E class];
    testassert(initializedE);
    testassert(seenClassesCount == 2);
    testassert(seenClasses[0] == [E class]);
    testassert(seenClasses[1] == [E class]);

    succeed(__FILE__);
}
