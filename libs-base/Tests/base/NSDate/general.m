#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSDate *cdate, *date1, *date2;
  NSComparisonResult comp;
  
  cdate = [NSDate date];
  
  comp = [cdate compare: [NSDate distantFuture]];
  PASS(comp == NSOrderedAscending, "+distantFuture is in the future");
  
  comp = [cdate compare: [NSDate distantPast]];
  PASS(comp == NSOrderedDescending, "+distantPast is in the past");
  
  date1 = [NSDate dateWithTimeIntervalSinceNow:-600];
  date2 = [cdate earlierDate: date1];
  PASS(date1 == date2, "-earlierDate works");
  
  date2 = [cdate laterDate: date1];
  PASS(cdate == date2, "-laterDate works");
  
  date2 = [date1 addTimeInterval:0];
  PASS ([date1 isEqualToDate:date2], "-isEqualToDate works");

  
  [arp release]; arp = nil;
  return 0;
}

