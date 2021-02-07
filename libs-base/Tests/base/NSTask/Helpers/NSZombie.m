#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSObject.h>

int
main()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSObject *anObject = [NSObject new];

  [anObject release];
  [anObject release];
  [pool release];
  return 0;
}
