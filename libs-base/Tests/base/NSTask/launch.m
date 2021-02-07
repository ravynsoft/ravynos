#import <Foundation/NSTask.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSData.h>
#import <Foundation/NSAutoreleasePool.h>

#import "ObjectTesting.h" 

int main()
{
  NSTask *task;
  NSPipe *outPipe;
  NSFileManager *mgr;
  NSString      *helpers;
  NSFileHandle  *outHandle;
  NSAutoreleasePool *arp;
  NSData *data = nil;

  arp = [[NSAutoreleasePool alloc] init];

  mgr = [NSFileManager defaultManager];
  helpers = [mgr currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];

  task = [[NSTask alloc] init];
  outPipe = [[NSPipe pipe] retain];
  [task setLaunchPath: [helpers stringByAppendingPathComponent: @"testcat"]];
  [task setArguments: [NSArray arrayWithObjects: nil]];
  [task setStandardOutput: outPipe]; 
  outHandle = [outPipe fileHandleForReading];

  [task launch];
  PASS([task standardOutput] == outPipe, "standardOutput returns pipe");
  data = [outHandle readDataToEndOfFile];
  PASS([data length] > 0, "was able to read data from subtask");
  NSLog(@"Data was %*.*s", [data length], [data length], [data bytes]);
  [task terminate];

  task = [[NSTask alloc] init];
  outPipe = [[NSPipe pipe] retain];
  [task setLaunchPath: [helpers stringByAppendingPathComponent: @"testecho"]];
  [task setArguments: [NSArray arrayWithObjects: @"Hello", @"there", nil]];
  [task setStandardOutput: outPipe]; 
  outHandle = [outPipe fileHandleForReading];

  [task launch];
  data = [outHandle readDataToEndOfFile];
  PASS([data length] > 0, "was able to read data from subtask");
  NSLog(@"Data was %*.*s", [data length], [data length], [data bytes]);
  [task terminate];


  PASS_EXCEPTION([task launch];, @"NSInvalidArgumentException",
    "raised exception on failed launch") 
  [outPipe release];
  [task release];

#if	!defined(_WIN32)
  task = [[NSTask alloc] init];
  [task setLaunchPath:
    [helpers stringByAppendingPathComponent: @"processgroup"]];
  [task setArguments: [NSArray arrayWithObjects:
    [NSString stringWithFormat: @"%d", getpgrp()],
    nil]];
  [task launch];
  [task waitUntilExit];
  PASS([task terminationStatus] == 0, "subtask changes process group");
  [task release];
#endif

  [arp release];

  return 0;
}
