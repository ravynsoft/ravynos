// TEST_CONFIG

#include "test.h"
#include <Foundation/NSObject.h>
#include <objc/runtime.h>

static int values;
static int supers;
static int subs;

static const char *key = "key";


@interface Value : NSObject @end
@interface Super : NSObject @end
@interface Sub : NSObject @end

@interface Super2 : NSObject @end
@interface Sub2 : NSObject @end

@implementation Super 
-(id) init
{
    // rdar://8270243 don't lose associations after isa swizzling

    id value = [Value new];
    objc_setAssociatedObject(self, &key, value, OBJC_ASSOCIATION_RETAIN);
    RELEASE_VAR(value);

    object_setClass(self, [Sub class]);
    
    return self;
}

-(void) dealloc 
{
    supers++;
    SUPER_DEALLOC();
}

@end

@implementation Sub
-(void) dealloc 
{
    subs++;
    SUPER_DEALLOC();
}
@end

@implementation Super2
-(id) init
{
    // rdar://9617109 don't lose associations after isa swizzling

    id value = [Value new];
    object_setClass(self, [Sub2 class]);
    objc_setAssociatedObject(self, &key, value, OBJC_ASSOCIATION_RETAIN);
    RELEASE_VAR(value);
    object_setClass(self, [Super2 class]);
    
    return self;
}

-(void) dealloc 
{
    supers++;
    SUPER_DEALLOC();
}

@end

@implementation Sub2
-(void) dealloc 
{
    subs++;
    SUPER_DEALLOC();
}
@end

@implementation Value
-(void) dealloc {
    values++;
    SUPER_DEALLOC();
}
@end


int main()
{
    testonthread(^{
        int i;
        for (i = 0; i < 100; i++) {
            RELEASE_VALUE([[Super alloc] init]);
        }
    });
    testcollect();
            
    testassert(supers == 0);
    testassert(subs > 0);
    testassert(subs == values);


    supers = 0;
    subs = 0;
    values = 0;

    testonthread(^{
        int i;
        for (i = 0; i < 100; i++) {
            RELEASE_VALUE([[Super2 alloc] init]);
        }
    });
    testcollect();

    testassert(supers > 0);
    testassert(subs == 0);
    testassert(supers == values);
    
    // rdar://44094390 tolerate nil object and nil value
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonnull"
    objc_setAssociatedObject(nil, &key, nil, OBJC_ASSOCIATION_ASSIGN);
#pragma clang diagnostic pop

    succeed(__FILE__);
}
