// TEST_CONFIG

#include "test.h"
#include <objc/objc-exception.h>
#include <Foundation/NSObject.h>

static int state;

@interface Foo : NSObject @end
@interface Bar : NSObject @end

@interface Foo (Unimplemented)
+(void)method;
@end

@implementation Bar @end

@implementation Foo

-(void)check { state++; }
+(void)check { testassert(!"caught class object, not instance"); }

static id exc;

static void handler(id unused, void *ctx) __attribute__((used));
static void handler(id unused __unused, void *ctx __unused)
{
    testassert(state == 3); state++;
}

+(BOOL) resolveClassMethod:(SEL)__unused name
{
    testassert(state == 1); state++;
#if TARGET_OS_OSX
    objc_addExceptionHandler(&handler, 0);
    testassert(state == 2); 
#else
    state++;  // handler would have done this
#endif
    state++;
    exc = [Foo new];
    @throw exc;
}


@end

int main()
{
    // unwind exception and alt handler through objc_msgSend()

    PUSH_POOL {

        const int count = is_guardmalloc() ? 1000 : 100000;
        state = 0;
        for (int i = 0; i < count; i++) {
            @try {
                testassert(state == 0); state++;
                [Foo method];
                testassert(0);
            } @catch (Bar *e) {
                testassert(0);
            } @catch (Foo *e) {
                testassert(e == exc);
                testassert(state == 4); state++;
                testassert(state == 5); [e check];  // state++
                RELEASE_VAR(exc);
            } @catch (id e) {
                testassert(0);
            } @catch (...) {
                testassert(0);
            } @finally {
                testassert(state == 6); state++;
            }
            testassert(state == 7); state = 0;
        }
        
    } POP_POOL;

    succeed(__FILE__);
}
