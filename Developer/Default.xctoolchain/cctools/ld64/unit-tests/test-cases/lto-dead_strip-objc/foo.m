#include <Foundation/Foundation.h>

@interface Foo : NSObject
- (NSString*) foo;
@end


@implementation Foo
- (NSString*) foo
{
	return [NSString stringWithUTF8String:"hello"];
}
@end


@interface Bar : NSData
- (NSArray*) bar;
@end


@implementation Bar
- (NSArray*) bar
{
	return [NSArray array];
}
@end

