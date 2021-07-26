#import <CoreFoundation/CFRunLoop.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSValue.h>

/* For now CFRunLoop is NSRunLoop, the real organization should be NSRunLoop owns a CFRunLoop
 */
 
@interface __CFRunLoop : NSRunLoop
@end

const CFStringRef kCFRunLoopCommonModes=(CFStringRef)@"kCFRunLoopCommonModes";
const CFStringRef kCFRunLoopDefaultMode=(CFStringRef)@"kCFRunLoopDefaultMode";

@implementation __CFRunLoop

CFTypeID CFRunLoopGetTypeID(void) {
   NSUnimplementedFunction();
   return 0;
}

CFRunLoopRef CFRunLoopGetCurrent(void) {
   return (CFRunLoopRef)[NSRunLoop currentRunLoop];
}

CFRunLoopRef CFRunLoopGetMain(void) {
   return (CFRunLoopRef)[NSRunLoop mainRunLoop];
}

void CFRunLoopRun(void) {
   CFRunLoopRunInMode(kCFRunLoopDefaultMode,1000.0*365.0*24.0*60.0*60.0,NO);
}

void CFRunLoopAddCommonMode(CFRunLoopRef self,CFStringRef mode) {
   NSUnimplementedFunction();
}

void CFRunLoopAddObserver(CFRunLoopRef self,CFRunLoopObserverRef observer,CFStringRef mode) {
   NSUnimplementedFunction();
}

void CFRunLoopRemoveObserver(CFRunLoopRef self,CFRunLoopObserverRef observer,CFStringRef mode) {
   NSUnimplementedFunction();
}

Boolean CFRunLoopContainsObserver(CFRunLoopRef self,CFRunLoopObserverRef observer,CFStringRef mode) {
   NSUnimplementedFunction();
   return 0;
}

void CFRunLoopAddSource(CFRunLoopRef self,CFRunLoopSourceRef source,CFStringRef mode) {
   NSUnimplementedFunction();
}

void CFRunLoopRemoveSource(CFRunLoopRef self,CFRunLoopSourceRef source,CFStringRef mode) {
   NSUnimplementedFunction();
}

Boolean CFRunLoopContainsSource(CFRunLoopRef self,CFRunLoopSourceRef source,CFStringRef mode) {
   NSUnimplementedFunction();
   return 0;
}

void CFRunLoopAddTimer(CFRunLoopRef self,CFRunLoopTimerRef timer,CFStringRef mode) {
   NSUnimplementedFunction();
}

void CFRunLoopRemoveTimer(CFRunLoopRef self,CFRunLoopTimerRef timer,CFStringRef mode) {
   NSUnimplementedFunction();
}

Boolean CFRunLoopContainsTimer(CFRunLoopRef self,CFRunLoopTimerRef timer,CFStringRef mode) {
   NSUnimplementedFunction();
   return 0;
}

CFStringRef CFRunLoopCopyCurrentMode(CFRunLoopRef self) {
   NSUnimplementedFunction();
   return 0;
}
CFArrayRef CFRunLoopCopyAllModes(CFRunLoopRef self) {
   NSUnimplementedFunction();
   return 0;
}

Boolean CFRunLoopIsWaiting(CFRunLoopRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFAbsoluteTime CFRunLoopGetNextTimerFireDate(CFRunLoopRef self,CFStringRef mode) {
   NSUnimplementedFunction();
   return 0;
}

CFInteger CFRunLoopRunInMode(CFStringRef mode,CFTimeInterval seconds,Boolean returnAfterSourceHandled) {
   NSRunLoop    *runLoop=(NSRunLoop*)CFRunLoopGetCurrent();
   NSDate      *before=[NSDate dateWithTimeIntervalSinceNow:seconds];
   
   [runLoop->_continue addObject:[NSNumber numberWithBool:YES]];
   
   if(returnAfterSourceHandled){

    if(seconds==0)
     before=[NSDate distantFuture];
     
    [runLoop runMode:(NSString *)mode beforeDate:before];
   }
   else {
    while([runLoop runMode:(NSString *)mode beforeDate:before] && [[runLoop->_continue lastObject] boolValue])
     if([before timeIntervalSinceNow]<0)
      break;
   }
     
   [runLoop->_continue removeLastObject];
   
   return 0;
}

void CFRunLoopStop(CFRunLoopRef self) {
    NSRunLoop    *runLoop=(NSRunLoop*)self;
   [runLoop->_continue removeLastObject];
   [runLoop->_continue addObject:[NSNumber numberWithBool:NO]];
}

void CFRunLoopWakeUp(CFRunLoopRef self) {
   NSUnimplementedFunction();
}

// sources

CFTypeID CFRunLoopSourceGetTypeID(void) {
   NSUnimplementedFunction();
   return 0;
}

CFRunLoopSourceRef CFRunLoopSourceCreate(CFAllocatorRef allocator,CFIndex order,CFRunLoopSourceContext *context) {
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFRunLoopSourceGetOrder(CFRunLoopSourceRef self) {
   NSUnimplementedFunction();
   return 0;
}

void CFRunLoopSourceGetContext(CFRunLoopSourceRef self,CFRunLoopSourceContext *context) {
   NSUnimplementedFunction();
}

void CFRunLoopSourceInvalidate(CFRunLoopSourceRef self) {
   NSUnimplementedFunction();
}

Boolean CFRunLoopSourceIsValid(CFRunLoopSourceRef self) {
   NSUnimplementedFunction();
   return 0;
}

void CFRunLoopSourceSignal(CFRunLoopSourceRef self) {
   NSUnimplementedFunction();
}

// observers

CFTypeID CFRunLoopObserverGetTypeID(void) {
   NSUnimplementedFunction();
   return 0;
}

CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator,CFOptionFlags activities,Boolean repeats,CFIndex order,CFRunLoopObserverCallBack callback,CFRunLoopObserverContext *context) {
   NSUnimplementedFunction();
   return 0;
}

CFOptionFlags CFRunLoopObserverGetActivities(CFRunLoopObserverRef self) {
   NSUnimplementedFunction();
   return 0;
}

Boolean CFRunLoopObserverDoesRepeat(CFRunLoopObserverRef self) {
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFRunLoopObserverGetOrder(CFRunLoopObserverRef self) {
   NSUnimplementedFunction();
   return 0;
}

void CFRunLoopObserverGetContext(CFRunLoopObserverRef self,CFRunLoopObserverContext *context) {
   NSUnimplementedFunction();
}

void CFRunLoopObserverInvalidate(CFRunLoopObserverRef self) {
   NSUnimplementedFunction();
}
Boolean CFRunLoopObserverIsValid(CFRunLoopObserverRef self) {
   NSUnimplementedFunction();
   return 0;
}
@end

