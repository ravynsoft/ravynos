#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>

#if defined(GNUSTEP_BASE_LIBRARY)
@interface NSObject (ISCLASS)
// This method doesn't exist on Cocoa
- (BOOL) isClass;
@end

@interface MyEvilClass : NSObject
{
 Class class;
 const char *name;
 long version;
 unsigned long info;
 /* not sure which of these is correct */
 Class class_;
 const char *name_;
 long version_;
 unsigned long info_;
}
-(void)setInfo:(unsigned long)info;
@end

@implementation MyEvilClass 
-(void)setInfo:(unsigned long)theInfo
{
  info = theInfo;
}
@end
#endif

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id evilObject;

#if defined(GNUSTEP_BASE_LIBRARY)
  PASS([NSObject isClass] &&
       [NSString isClass] &&
       [NSArray isClass],
       "-isClass returns YES on a Class");
  
  PASS((![[[NSObject new] autorelease] isClass] &&
       ![[NSString stringWithCString:"foo"] isClass] &&
       ![[[NSArray new] autorelease] isClass]),
       "-isClass returns NO on an instance");
  
  evilObject = [MyEvilClass new];
  [evilObject setInfo:1];
  PASS(![evilObject isClass], 
       "-isClass returns NO on an instance (special test for broken libobjc)");
#endif
  
  PASS(([[[NSObject new] autorelease] isKindOfClass:[NSObject class]] &&
       [[[NSString new] autorelease] isKindOfClass:[NSString class]] &&
       ![[[NSObject new] autorelease] isKindOfClass:[NSString class]] &&
       [[[NSString new] autorelease] isKindOfClass:[NSObject class]] &&
       ![[[NSString new] autorelease] isKindOfClass:[NSArray class]] &&
       [[[NSMutableString new] autorelease] isKindOfClass:[NSString class]]),
       "-isKindOfClass: works"); 
  
     /* should return YES if receiver and argument are both NSObject */
  PASS([NSObject isKindOfClass:[NSObject class]] &&
       ![NSString isKindOfClass:[NSString class]] &&  
       ![NSObject isKindOfClass:[NSString class]] &&
       [NSString isKindOfClass:[NSObject class]],
       "+isKindOfClass: works");

  [arp release]; arp = nil;
  return 0;
}
