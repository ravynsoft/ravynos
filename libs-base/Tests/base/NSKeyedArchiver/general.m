#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSValue.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2, *val3, *s;
  NSNumber *val4;
  NSArray  *vals1, *vals2;
  NSData   *data1;
  NSMutableData *data2;
  NSArray *a;
  NSURL *u;
  NSMutableSet *ms;
  NSKeyedArchiver *archiver = nil;
  NSKeyedUnarchiver *unarchiver = nil;

  u = [NSURL URLWithString: @"http://www.w3.org/"];
  ms = [NSMutableSet set];
  [ms addObject: u];
  data2 = [NSMutableData new];
  archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData: data2];
  [archiver setOutputFormat: NSPropertyListXMLFormat_v1_0];
  [archiver encodeObject: ms forKey: @"root"];
  [archiver finishEncoding];
  NSLog(@"%*.*s",
    (unsigned)[data2 length], (unsigned)[data2 length], [data2 bytes]);
  ms = [NSKeyedUnarchiver unarchiveObjectWithData: data2];
  PASS([[[ms anyObject] absoluteString] isEqual: @"http://www.w3.org/"],
    "Can archive and restore a URL");
  
  [archiver release];
  [data2 release];


  PASS_RUNS(val1 = [NSString stringWithCString:"Archiver.dat"];
		 val2 = [NSString stringWithCString:"A Goodbye"];
		 val3 = [NSString stringWithCString:"Testing all strings"];
		 val4 = [NSNumber numberWithUnsignedInt: 100];
		 vals1 = [[[NSArray arrayWithObject:val1] 
			    arrayByAddingObject:val2] 
			   arrayByAddingObject: val4];		 
		 vals2 = [vals1 arrayByAddingObject: val2];,
		 "We can build basic strings and arrays for tests");
  
  PASS([NSKeyedArchiver archiveRootObject:vals2 toFile:val1],
    "archiveRootObject:toFile: seems ok"); 
  
  data1 = [NSKeyedArchiver archivedDataWithRootObject:vals2];
  PASS((data1 != nil && [data1 length] != 0),
    "archivedDataWithRootObject: seems ok");
  
  a = [NSKeyedUnarchiver unarchiveObjectWithData:data1];
  NSLog(@"From data: original array %@, decoded array %@",vals2, a);
  PASS((a != nil && [a isKindOfClass:[NSArray class]] && [a isEqual:vals2]),
       "unarchiveObjectWithData: seems ok");
  
  a = [NSKeyedUnarchiver unarchiveObjectWithFile:val1];
  NSLog(@"From file: original array %@, decoded array %@",vals2, a);
  PASS((a != nil && [a isKindOfClass:[NSArray class]] && [a isEqual:vals2]),
       "unarchiveObjectWithFile: seems ok");

  // encode
  data2 = [[NSMutableData alloc] initWithCapacity: 10240];
  archiver = [[NSKeyedArchiver alloc] initForWritingWithMutableData: data2];
  [archiver encodeObject: val3 forKey: @"string"];
  [archiver finishEncoding];

  // decode...
  unarchiver = [[NSKeyedUnarchiver alloc] initForReadingWithData: data2];
  s = [[unarchiver decodeObjectForKey: @"string"] retain];
  PASS((s != nil && [s isKindOfClass:[NSString class]] && [s isEqual: val3]),
    "encodeObject:forKey: seems okay");
  [data2 release];

  NSLog(@"Original string: %@, unarchived string: %@",val3, s);

  [[NSFileManager  defaultManager] removeFileAtPath: val1 handler: nil];
  [arp release]; arp = nil;
  return 0;
}
