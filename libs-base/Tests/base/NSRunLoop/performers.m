#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSDictionary.h>

#include <unistd.h>

int main()
{
  START_SET("NSRunLoop performers")
  NSRunLoop *run;
  NSDate *date;
  NSMutableString *str;
  NSFileHandle *fh = [NSFileHandle fileHandleWithStandardInput];
  NSTimeInterval delay = 0.1;

  [fh retain];
  [fh readInBackgroundAndNotify];
   
  run = [NSRunLoop currentRunLoop];

  /* A run loop with no input sources does nothing when you tell it to run.
     Thus, we need to provide at least one input source ... */

  str = [[NSMutableString alloc] init]; 
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"foo"
	order: 0
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  date = [NSDate dateWithTimeIntervalSinceNow: delay];
  [run runUntilDate: date];
  PASS([str isEqual: @"foo"], 
       "-performSelector:target:argument:order:modes: works for one performer");
  
  str = [[NSMutableString alloc] init]; 
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"foo"
	order: 0
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  date = [NSDate dateWithTimeIntervalSinceNow: delay];
  [run runUntilDate: date];
  [run runUntilDate: date];
  if ([str isEqual: @""])
    SKIP("it looks like stdin is redirected")
  PASS([str isEqual: @"foo"],
       "-performSelector:target:argument:order:modes: only sends the message once");
  
  str = [[NSMutableString alloc] init]; 
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"bar"
	order: 11
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"foo"
	order: 10
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  date = [NSDate dateWithTimeIntervalSinceNow: delay];
  [run runUntilDate: date];
  PASS([str isEqual: @"foobar"],
       "-performSelector:target:argument:order:modes: orders performers correctly");
  
  str = [[NSMutableString alloc] init]; 
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"foo"
	order: 10
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"bar"
	order: 11
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"zot"
	order: 11
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run cancelPerformSelector: @selector(appendString:)
  	target: str
	argument: @"bar"];
  date = [NSDate dateWithTimeIntervalSinceNow: delay];
  [run runUntilDate: date];
  PASS([str isEqual: @"foozot"],
       "-cancelPerformSelector:target:argument: works");
  
  str = [[NSMutableString alloc] init]; 
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"foo"
	order: 10
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run performSelector: @selector(appendString:)
  	target: str
	argument: @"zot"
	order: 11
	modes: [NSArray arrayWithObject: NSDefaultRunLoopMode]];
  [run cancelPerformSelectorsWithTarget: str];
  date = [NSDate dateWithTimeIntervalSinceNow: delay];
  [run runUntilDate: date];
  PASS([str isEqualToString: @""], "-cancelPerformSelectorsWithTarget: works %s",[str UTF8String]); 

  [fh closeFile];
  [fh release];

  END_SET("NSRunLoop performers")
  return 0;
}
