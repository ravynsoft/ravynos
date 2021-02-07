#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSFileManager		*mgr;
  NSString		*helpers;
  NSString		*command;
  NSTask		*task;
  NSPipe		*ePipe;
  NSFileHandle		*hdl;
  NSData		*data;
  NSString		*string;
  NSLock 		*lock = nil;
  unsigned		count;

  mgr = [NSFileManager defaultManager];
  helpers = [mgr currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];

  command = [helpers stringByAppendingPathComponent: @"doubleNSLock"];
  task = [[NSTask alloc] init];
  ePipe = [[NSPipe pipe] retain];
  [task setLaunchPath: command];
  [task setStandardError: ePipe]; 
  hdl = [ePipe fileHandleForReading];
  [task launch];
  for (count = 0; count < 10 && [task isRunning]; count++)
    {
      [NSThread sleepForTimeInterval: 1.0];
    }
  data = [hdl availableData];
  NSLog(@"Data was %*.*s", [data length], [data length], [data bytes]);
  string = [NSString alloc];
  string = [string initWithData: data encoding: NSISOLatin1StringEncoding];
  PASS([string rangeOfString: @"deadlock"].length > 0,
    "NSLock reported deadlock as expected");
  if (NO == testPassed)
    {
      PASS(count == 10, "NSLock seems to have deadlocked as expected")
     [task terminate];
    }
  [task waitUntilExit];

  command = [helpers stringByAppendingPathComponent: @"doubleNSConditionLock"];
  task = [[NSTask alloc] init];
  ePipe = [[NSPipe pipe] retain];
  [task setLaunchPath: command];
  [task setStandardError: ePipe]; 
  hdl = [ePipe fileHandleForReading];
  [task launch];
  for (count = 0; count < 10 && [task isRunning]; count++)
    {
      [NSThread sleepForTimeInterval: 1.0];
    }
  data = [hdl availableData];
  NSLog(@"Data was %*.*s", [data length], [data length], [data bytes]);
  string = [NSString alloc];
  string = [string initWithData: data encoding: NSISOLatin1StringEncoding];
  PASS([string rangeOfString: @"deadlock"].length > 0,
    "NSConditionLock reported deadlock as expected");
  if (NO == testPassed)
    {
      PASS(count == 10, "NSConditionLock seems to have deadlocked as expected")
      [task terminate];
    }
  [task waitUntilExit];

  ASSIGN(lock,[NSRecursiveLock new]);
  [lock lock];
  [lock lock];
  [lock unlock];
  [lock unlock];

  ASSIGN(lock,[NSLock new]);
  PASS([lock tryLock] == YES, "NSLock can tryLock");
  PASS([lock tryLock] == NO, "NSLock says NO for recursive tryLock");
  [lock unlock];

  ASSIGN(lock,[NSConditionLock new]);
  PASS([lock tryLock] == YES, "NSConditionLock can tryLock");
  PASS([lock tryLock] == NO, "NSConditionLock says NO for recursive tryLock");
  [lock unlock];

  ASSIGN(lock,[NSRecursiveLock new]);
  PASS([lock tryLock] == YES, "NSRecursiveLock can tryLock");
  PASS([lock tryLock] == YES, "NSRecursiveLock says YES for recursive tryLock");
  [lock unlock];

  [arp release];
  return 0;
}
