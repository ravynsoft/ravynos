#ifdef WINDOWS
#import "NSRunLoopState_windows.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSSelectInputSourceSet.h>
#import <Foundation/NSHandleMonitorSet_win32.h>
#import "NSCancelInputSource_win32.h"

@implementation NSRunLoopState(windows)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSRunLoopState_windows class],0,NULL);
}

@end

@implementation NSRunLoopState_windows

-init {
   _inputSourceSet=[[NSHandleMonitorSet_win32 alloc] init];
   _asyncInputSourceSets=[[NSArray alloc] initWithObjects:[[[NSSelectInputSourceSet alloc] init] autorelease],nil];
   _timers=[NSMutableArray new];
   _cancelSource=[[NSCancelInputSource_win32 alloc] init];
   [self addInputSource:_cancelSource];
   return self;
}

@end
#endif
