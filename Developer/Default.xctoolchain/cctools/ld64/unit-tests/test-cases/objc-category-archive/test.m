
#include <Foundation/Foundation.h>

int some_global_to_stop_libtool_warning = 5;

@interface NSObject (stuff)
- (void) mycatmethod1;
@end

@implementation NSObject (stuff)
- (void) mycatmethod1 { }
@end

@interface NSObject (other)
- (void) mycatmethod2;
@end

@implementation NSObject (other)
- (void) mycatmethod2 { }
@end

