#import <Foundation/Foundation.h>
#import "Testing.h"

int main()
{ 
  NSAutoreleasePool   *pool = [NSAutoreleasePool new];
  NSZone *aZone;
  char *vp; 
  void *ovp;

  aZone = NSDefaultMallocZone();
  PASS((aZone != NULL), "NSDefaultMallocZone() returns something");

  aZone = NSCreateZone(1024,1024,0);
  PASS((aZone != NULL), "NSCreateZone() works for an unfreeable zone");
 
  aZone = NSCreateZone(1024,1024,1);
  PASS((aZone != NULL), "NSCreateZone() works for a normal zone");
 
  if ([NSGarbageCollector defaultCollector] == nil)
    {
      NSSetZoneName(aZone, @"My Zone");
      PASS(([NSZoneName(aZone) isEqual: @"My Zone"]),
	"NSZoneName() returns previously set string");
     
      vp = NSZoneCalloc(aZone,17,12);
      memset(vp,1,17*12);

      NS_DURING
	{
	  NSZoneFree(aZone,vp);
	  PASS(1, "NSZoneFree() calloc'd buffer"); 
	}
      NS_HANDLER
       PASS(0, "NSZoneFree() calloc'd buffer %s",
	[[localException name] UTF8String]); 
      NS_ENDHANDLER
      
      NS_DURING
	NSZoneFree(aZone,vp);
	PASS(0, "NSZoneFree() free'd buffer throws exception"); 
      NS_HANDLER
	PASS(1, "NSZoneFree() free'd buffer throws exception: %s",
	  [[localException name] UTF8String]); 
      NS_ENDHANDLER

      
      vp = NSZoneMalloc(aZone,2000);
      memset(vp,2,2000);

      NS_DURING
	{
	  NSZoneFree(aZone,vp);
	  PASS(1, "NSZoneFree() malloc'd buffer"); 
	}
      NS_HANDLER
       PASS(0, "NSZoneFree() malloc'd buffer %s",
	[[localException name] UTF8String]); 
      NS_ENDHANDLER

      ovp = NSZoneMalloc(aZone, 1000);
      vp = NSZoneRealloc(aZone, ovp, 2000); 
      memset(vp,3,2000); 
      
      NSZoneRealloc(aZone, vp, 1000);
      memset(vp,4,1000); 
      
      NS_DURING
	NSZoneFree(aZone,vp);
	PASS(1,"NSZoneFree() releases memory held after realloc");
      NS_HANDLER
	PASS(0,"NSZoneFree() releases memory held after realloc");
      NS_ENDHANDLER

      PASS((NSZoneFromPointer(vp) == aZone),
	"NSZoneFromPointer() returns zone where memory came from");
     
      NS_DURING
       [NSString allocWithZone:aZone];
       NSRecycleZone(aZone);
	PASS(1,"NSRecycleZone seems to operate");
      NS_HANDLER
	PASS(0,"NSRecycleZone seems to operate");
      NS_ENDHANDLER
    }

  [pool release]; pool = nil;
 
  return 0;
}
