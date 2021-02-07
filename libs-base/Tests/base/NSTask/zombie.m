#import <Foundation/NSTask.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSData.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>

#import "ObjectTesting.h" 

int main()
{
  NSTask *task;
  NSPipe *outPipe;
  NSFileManager *mgr;
  NSString      *helpers;
  NSFileHandle  *outHandle;
  NSAutoreleasePool *arp;
  NSMutableDictionary *env;
  NSData *data = nil;
  NSString *str;

  arp = [[NSAutoreleasePool alloc] init];

  mgr = [NSFileManager defaultManager];
  helpers = [mgr currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];

  env = [[[NSProcessInfo processInfo] environment] mutableCopy];
  [env setObject: @"YES" forKey: @"NSZombieEnabled"];
  task = [[NSTask alloc] init];
  outPipe = [[NSPipe pipe] retain];
  [task setLaunchPath: [helpers stringByAppendingPathComponent: @"NSZombie"]];
  [task setArguments: [NSArray arrayWithObjects: nil]];
  [task setStandardError: outPipe]; 
  [task setEnvironment: env];
  outHandle = [outPipe fileHandleForReading];

  [task launch];
  data = [outHandle readDataToEndOfFile];
  NSLog(@"Data was %*.*s", [data length], [data length], [data bytes]);
  str = [[NSString alloc] initWithData: data
			      encoding: NSISOLatin1StringEncoding];
  PASS(str != nil && [str rangeOfString: @"sent to deallocated"].length > 0,
    "was able to read zombie message from subtask");
  [task terminate];

  [arp release];

  return 0;
}
