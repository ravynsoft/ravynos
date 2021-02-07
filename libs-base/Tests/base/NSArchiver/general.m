#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSArchiver.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileManager.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2, *val3;
  NSArray  *vals1, *vals2;
  NSData   *data1;
  NSArray *a;

  PASS_RUNS(val1 = [NSString stringWithCString:"Archiver.dat"];
    val2 = [NSString stringWithCString:"A Goodbye"];
    val3 = [NSString stringWithCString:"Testing all strings"];
    vals1 = [[NSArray arrayWithObject:val1] arrayByAddingObject:val2];
    vals2 = [vals1 arrayByAddingObject:val2];,
    "We can build basic strings and arrays for tests");
  
  data1 = [NSArchiver archivedDataWithRootObject:vals2];
  PASS((data1 != nil && [data1 length] != 0),
    "archivedDataWithRootObject: seems ok");
  
  PASS([NSArchiver archiveRootObject:vals2 toFile:val1],
    "archiveRootObject:toFile: seems ok"); 
  
  a = [NSUnarchiver unarchiveObjectWithData:data1];
  PASS((a != nil && [a isKindOfClass:[NSArray class]] && [a isEqual:vals2]),
       "unarchiveObjectWithData: seems ok");
  
  a = [NSUnarchiver unarchiveObjectWithFile:val1];
  PASS((a != nil && [a isKindOfClass:[NSArray class]] && [a isEqual:vals2]),
       "unarchiveObjectWithFile: seems ok");

  [[NSFileManager  defaultManager] removeFileAtPath: val1 handler: nil];

  [arp release]; arp = nil;
  return 0;
}
