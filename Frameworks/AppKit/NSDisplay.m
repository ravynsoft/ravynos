/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSDisplay.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSColorList.h>

@implementation NSDisplay

+(void)initialize {
   if(self==[NSDisplay class]){
    NSDictionary *map=[NSDictionary dictionaryWithObjectsAndKeys:
     @"Command",@"LeftControl",
     @"Alt",@"LeftAlt",
     @"Control",@"RightControl",
     @"Alt",@"RightAlt",
     nil];
    NSDictionary *modifierMapping=[NSDictionary dictionaryWithObject:map forKey:@"NSModifierFlagMapping"];

    [[NSUserDefaults standardUserDefaults] registerDefaults:modifierMapping];
   }
}

+(NSDisplay *)currentDisplay {
   return NSThreadSharedInstance(@"NSDisplay");
}

-init {
   _eventQueue=[NSMutableArray new];
   return self;
}

-(NSArray *)screens {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSPasteboard *)pasteboardWithName:(NSString *)name {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDraggingManager *)draggingManager {
   NSInvalidAbstractInvocation();
   return nil;
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
   NSInvalidAbstractInvocation();
   return nil;
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSColor *)colorWithName:(NSString *)colorName {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)_addSystemColor:(NSColor *) result forName:(NSString *)colorName {
   NSInvalidAbstractInvocation();
 }


-(NSTimeInterval)textCaretBlinkInterval {
   NSInvalidAbstractInvocation();
   return 1;
}

-(void)hideCursor {
   NSInvalidAbstractInvocation();
}

-(void)unhideCursor {
   NSInvalidAbstractInvocation();
}

// Arrow, IBeam, HorizontalResize, VerticalResize
-(id)cursorWithName:(NSString *)name {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)setCursor:(id)cursor {
   NSInvalidAbstractInvocation();
}

-(NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue {
   NSEvent *result=nil;

   if([_eventQueue count])
      untilDate=[NSDate date];
   
   [[NSRunLoop currentRunLoop] runMode:mode beforeDate:untilDate];

   while(result==nil && [_eventQueue count]>0){
    NSEvent *check=[_eventQueue objectAtIndex:0];
    
    if(!(NSEventMaskFromType([check type])&mask))
     [_eventQueue removeObjectAtIndex:0];
   else {
     result=[[check retain] autorelease];

    if(dequeue)
     [_eventQueue removeObjectAtIndex:0];
   }
   }

   if(result==nil)
    result=[[[NSEvent alloc] initWithType:NSAppKitSystem location:NSMakePoint(0,0) modifierFlags:0 window:nil] autorelease];
   
   return result;
}

-(void)discardEventsMatchingMask:(unsigned)mask beforeEvent:(NSEvent *)event {
   int count=[_eventQueue count];

   while(--count>=0){
    NSEvent *check=[_eventQueue objectAtIndex:count];

    if(check==event)
     break;
   }

   while(--count>=0){
    if(NSEventMaskFromType([event type])&mask)
     [_eventQueue removeObjectAtIndex:count];
   }
}

-(void)postEvent:(NSEvent *)event atStart:(BOOL)atStart {
   if(atStart)
    [_eventQueue insertObject:event atIndex:0];
   else
    [_eventQueue addObject:event];
}

-(BOOL)containsAndRemovePeriodicEvents {
   BOOL result=NO;
   int  count=[_eventQueue count];

   while(--count>=0){
    if([(NSEvent *)[_eventQueue objectAtIndex:count] type]==NSPeriodic){
     result=YES;
     [_eventQueue removeObjectAtIndex:count];
    }
   }

   return result;
}

-(unsigned)modifierForDefault:(NSString *)key:(unsigned)standard {
   NSDictionary *modmap=[[NSUserDefaults standardUserDefaults] dictionaryForKey:@"NSModifierFlagMapping"];
   NSString     *remap=[modmap objectForKey:key];

   if([remap isEqualToString:@"Command"])
    return NSCommandKeyMask;
   if([remap isEqualToString:@"Alt"])
    return NSAlternateKeyMask;
   if([remap isEqualToString:@"Control"])
    return NSControlKeyMask;

   return standard;
}

-(void)beep {
   NSInvalidAbstractInvocation();
}

-(NSSet *)allFontFamilyNames {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSArray *)fontTypefacesForFamilyName:(NSString *)name {
   NSInvalidAbstractInvocation();
   return nil;
}

-(float)scrollerWidth {
   NSInvalidAbstractInvocation();
   return 0;
}

-(int)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo {
   NSInvalidAbstractInvocation();
	return 0;
}

-(int)runModalPrintPanelWithPrintInfoDictionary:(NSMutableDictionary *)attributes {
   NSInvalidAbstractInvocation();
   return 0;
}

-(CGContextRef)graphicsPortForPrintOperationWithView:(NSView *)view printInfo:(NSPrintInfo *)printInfo pageRange:(NSRange)pageRange {
   NSInvalidAbstractInvocation();
   return nil;
}

-(int)savePanel:(NSSavePanel *)savePanel runModalForDirectory:(NSString *)directory file:(NSString *)file {
   NSInvalidAbstractInvocation();
   return 0;
}

-(int)openPanel:(NSOpenPanel *)openPanel runModalForDirectory:(NSString *)directory file:(NSString *)file types:(NSArray *)types {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSPoint)mouseLocation {
   NSInvalidAbstractInvocation();
   return NSMakePoint(0,0);
}

-(NSUInteger)currentModifierFlags {
   NSInvalidAbstractInvocation();
   return 0;
}

@end

void NSColorSetCatalogColor(NSString *catalogName,NSString *colorName,NSColor *color){
    NSColorList *list = [NSColorList colorListNamed:catalogName];
    if(list)
        [list setColor:color forKey:colorName];
}

NSColor *NSColorGetCatalogColor(NSString *catalogName,NSString *colorName){
    return [NSColor colorWithCatalogName:catalogName colorName:colorName];
}
