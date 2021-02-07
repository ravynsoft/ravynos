#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSException.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id obj;
  NSMutableData     *data1;

  obj = [NSKeyedArchiver alloc];
  data1 = [NSMutableData dataWithLength: 0];
  obj = [obj initForWritingWithMutableData: data1];
  PASS((obj != nil && [obj isKindOfClass:[NSKeyedArchiver class]]), "-initForWritingWithMutableData seems ok");

  PASS_EXCEPTION([[NSUnarchiver alloc] initForReadingWithData:nil];, 
                 @"NSInvalidArgumentException",
		 "Creating an NSUnarchiver with nil data throws an exception");
  
  [arp release]; arp = nil;
  return 0; 
}
