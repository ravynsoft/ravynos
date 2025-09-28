#import <AppKit/NSWindowScripting.h>
#import <AppKit/NSApplication.h>

@implementation NSWindow(scripting)

-(NSInteger)orderedIndex {
   NSInteger result=[[NSApp orderedWindows] indexOfObjectIdenticalTo:self];
   
   /* Documentation says orderedIndex is zero based, but tests say it is 1 based
      Possible there is a window at 0 which is not in -[NSApp windows] ? Either way, available windows are 1 based.
    */
   if(result!=NSNotFound)
    result+=1;

   return result;
}

@end
