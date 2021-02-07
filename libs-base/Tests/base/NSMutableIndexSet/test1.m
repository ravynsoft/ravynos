#import <Foundation/Foundation.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSPropertyList.h>
#include <Testing.h>
int main()
{
  NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
  NSMutableData		*m = [NSMutableData dataWithCapacity: 1024];
  NSPropertyListFormat  aFormat;
  NSString		*aString;
  id			pl;
  NSKeyedArchiver	*a;
  NSKeyedUnarchiver	*u;
  NSMutableIndexSet	*original;
  NSIndexSet		*decoded;

  a = [[NSKeyedArchiver alloc] initForWritingWithMutableData: m];
  [a setOutputFormat: NSPropertyListXMLFormat_v1_0];

  original = [NSMutableIndexSet indexSetWithIndexesInRange: NSMakeRange(2,3)];
  [original addIndex: 7];
  [original addIndex: 557];
  [original addIndex: 947];
  [a encodeObject: original forKey: @"outer1"];
  [a finishEncoding];
  
  pl = [NSPropertyListSerialization propertyListFromData: m
                                        mutabilityOption: 0
                                                  format: &aFormat
                                        errorDescription: &aString];
  pl = [(NSDictionary*)[[(NSDictionary*)pl objectForKey: @"$objects"]
    objectAtIndex: 2] objectForKey: @"NS.data"];
  // FIXME ... maybe check encoded data format

  [a release];

  u = [[NSKeyedUnarchiver alloc] initForReadingWithData: m];
  decoded = [u decodeObjectForKey: @"outer1"];
  PASS([decoded isEqual: original], "decoded value equals encoded");
  [u release];
  [pool release];
  return 0;
}
