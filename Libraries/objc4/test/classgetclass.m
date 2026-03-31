// TEST_CONFIG

#include "test.h"
#include <objc/objc-runtime.h>
#include <objc/objc-gdb.h>
#import <Foundation/NSObject.h>

@interface Foo:NSObject
@end
@implementation Foo
@end

int main()
{
    testassert(gdb_class_getClass([Foo class]) == [Foo class]);
    succeed(__FILE__);
}
