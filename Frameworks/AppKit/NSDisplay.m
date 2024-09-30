/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
   Copyright (C) 2022-2024 Zoe Knox <zoe@ravynsoft.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#import <AppKit/NSDisplay.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSColorList.h>
#import <AppKit/NSFontTypeface.h>
#import <AppKit/WSWindow.h>
#import <Onyx2D/O2Font.h>
#import <AppKit/O2Font_FT.h>
#import <fontconfig.h>

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
    _screens = [NSMutableArray new];
    _ready = NO;
    return self;
}

- (void)configureWithInfo:(struct mach_display_info *)info {
    NSRect frame = NSMakeRect(0, 0, info->width, info->height);
    NSScreen *screen = [[[NSScreen alloc] initWithFrame:frame visibleFrame:frame] retain];
    [_screens addObject:screen];
    _depth = info->depth;
    _ready = YES;
}

- (BOOL)isReady { return _ready; }

-(NSArray *)screens {
    return [NSArray arrayWithArray:_screens];
}

-(uint32_t)depth { return _depth; }

-(NSPasteboard *)pasteboardWithName:(NSString *)name {
   NSUnimplementedMethod();
   return nil;
}

-(NSDraggingManager *)draggingManager {
   NSUnimplementedMethod();
   return nil;
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType windowNumber:(int)number screen:(NSScreen *)screen { // FIXME: screen is currently ignored
	return [[[WSWindow alloc] initWithFrame:frame
                                      styleMask:styleMask
                                        isPanel:NO
                                    backingType:backingType
                                   windowNumber:(int)number] autorelease];
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType windowNumber:(int)number screen:(NSScreen *)screen { // FIXME: screen is currently ignored
	return [[[WSWindow alloc] initWithFrame:frame
                                      styleMask:styleMask
                                        isPanel:YES
                                    backingType:backingType
                                   windowNumber:(int)number] autorelease];
}

-(NSColor *)colorWithName:(NSString *)colorName {
   if([colorName isEqual:@"controlColor"])
      return [NSColor lightGrayColor];
   if([colorName isEqual:@"disabledControlTextColor"])
      return [NSColor grayColor];
   if([colorName isEqual:@"controlTextColor"])
      return [NSColor blackColor];
   if([colorName isEqual:@"menuBackgroundColor"])
      return [NSColor lightGrayColor];
   if([colorName isEqual:@"controlShadowColor"])
      return [NSColor darkGrayColor];
   if([colorName isEqual:@"selectedControlColor"])
      return [NSColor cyanColor];
   if([colorName isEqual:@"controlBackgroundColor"])
      return [NSColor whiteColor];
   if([colorName isEqual:@"controlLightHighlightColor"])
      return [NSColor lightGrayColor];

   if([colorName isEqual:@"textBackgroundColor"])
      return [NSColor whiteColor];
   if([colorName isEqual:@"textColor"])
      return [NSColor blackColor];
   if([colorName isEqual:@"menuItemTextColor"])
      return [NSColor blackColor];
   if([colorName isEqual:@"selectedMenuItemTextColor"])
      return [NSColor blackColor];
   if([colorName isEqual:@"selectedMenuItemColor"])
      return [NSColor cyanColor];
   if([colorName isEqual:@"selectedControlTextColor"])
      return [NSColor blackColor];
   
   return nil;
}

-(void)_addSystemColor:(NSColor *) result forName:(NSString *)colorName {
   NSUnimplementedMethod();
 }


-(NSTimeInterval)textCaretBlinkInterval {
   return 0.5;
}

-(void)hideCursor {
   NSUnimplementedMethod();
}

-(void)unhideCursor {
   NSUnimplementedMethod();
}

// Arrow, IBeam, HorizontalResize, VerticalResize
-(id)cursorWithName:(NSString *)name {
   NSUnimplementedMethod();
   return nil;
}

-(void)setCursor:(id)cursor {
   NSUnimplementedMethod();
}

-(NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue {
    NSEvent *result=nil;

    if([_eventQueue count])
        untilDate=[NSDate date];
   
    [[NSRunLoop currentRunLoop] addInputSource:[NSApp inputSource] forMode:mode];
    [[NSRunLoop currentRunLoop] runMode:mode beforeDate:untilDate];
    [[NSRunLoop currentRunLoop] removeInputSource:[NSApp inputSource] forMode:mode];

    while(result==nil && [_eventQueue count]>0) {
        NSEvent *check=[_eventQueue objectAtIndex:0];
    
    if(!(NSEventMaskFromType([check type])&mask))
         [_eventQueue removeObjectAtIndex:0];
    else {
         result=[[check retain] autorelease];

         NSEventType _type = [result type];
         if(_type == NSMouseMoved || _type == NSLeftMouseDragged || _type == NSRightMouseDragged)
             pointerPos = [result locationInWindow];

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
   NSUnimplementedMethod();
}

-(NSSet *)allFontFamilyNames {
   int i;
   FcPattern *pat=FcPatternCreate();
   FcObjectSet *props=FcObjectSetBuild(FC_FAMILY, NULL);
   
   FcFontSet *set = FcFontList (O2FontSharedFontConfig(), pat, props);
   NSMutableSet* ret=[NSMutableSet set];
   
   for(i = 0; i < set->nfont; i++)
   {
      FcChar8 *family;
      if (FcPatternGetString (set->fonts[i], FC_FAMILY, 0, &family) == FcResultMatch) {
         [ret addObject:[NSString stringWithUTF8String:(char*)family]];
      }
   }
   
   FcPatternDestroy(pat);
   FcObjectSetDestroy(props);
   FcFontSetDestroy(set);
   return ret;
}

-(NSArray *)fontTypefacesForFamilyName:(NSString *)familyName {
   int i;
   FcPattern *pat=FcPatternCreate();
   FcPatternAddString(pat, FC_FAMILY, (unsigned char*)[familyName UTF8String]);
   FcObjectSet *props=FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_SLANT, FC_WIDTH, FC_WEIGHT, NULL);

   FcFontSet *set = FcFontList (O2FontSharedFontConfig(), pat, props);
   NSMutableArray* ret=[NSMutableArray array];
   
   for(i = 0; i < set->nfont; i++)
   {
      FcChar8 *typeface;
      FcPattern *p=set->fonts[i];
      if (FcPatternGetString (p, FC_STYLE, 0, &typeface) == FcResultMatch) {
         NSString* traitName=[NSString stringWithUTF8String:(char*)typeface];
         FcChar8* pattern=FcNameUnparse(p);
         NSString* name=[NSString stringWithUTF8String:(char*)pattern];
         FcStrFree(pattern);
         
         NSFontTraitMask traits=0;
         int slant, width, weight;
         
         FcPatternGetInteger(p, FC_SLANT, FC_SLANT_ROMAN, &slant);
         FcPatternGetInteger(p, FC_WIDTH, FC_WIDTH_NORMAL, &width);
         FcPatternGetInteger(p, FC_WEIGHT, 0, &weight);

         switch(slant) {
            case FC_SLANT_OBLIQUE:
            case FC_SLANT_ITALIC:
               traits|=NSItalicFontMask;
               break;
         }
         
         if(weight>FC_WEIGHT_SEMIBOLD)
            traits|=NSBoldFontMask;

         if(width<=FC_WIDTH_SEMICONDENSED)
            traits|=NSNarrowFontMask;
         else if(width>=FC_WIDTH_SEMIEXPANDED)
            traits|=NSExpandedFontMask;

        // FIXME: we should set FixedPitch (monospace) and other attrs too (see NSFontManager.h)
         
         name = [NSString stringWithFormat:@"%@-%@",
            [[[[name componentsSeparatedByString:@":"] firstObject] // strip off any 'style=XXX' stuff
            componentsSeparatedByString:@","] firstObject],         // and any multiple names
            traitName]; // and append "-Traits"
         NSFontTypeface *face=[[NSFontTypeface alloc] initWithName:name traitName:traitName traits:traits];
         [ret addObject:face];
         [face release];
      }
   }
   
   FcPatternDestroy(pat);
   FcObjectSetDestroy(props);
   FcFontSetDestroy(set);
   return ret;
}

-(float)scrollerWidth {
   return 15.0;
}

-(float)doubleClickInterval {
   return 1.0;
}


-(int)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo {
   NSUnimplementedMethod();
	return 0;
}

-(int)runModalPrintPanelWithPrintInfoDictionary:(NSMutableDictionary *)attributes {
   NSUnimplementedMethod();
   return 0;
}

-(O2Context *)graphicsPortForPrintOperationWithView:(NSView *)view printInfo:(NSPrintInfo *)printInfo pageRange:(NSRange)pageRange {
   NSUnimplementedMethod();
   return nil;
}

-(int)savePanel:(NSSavePanel *)savePanel runModalForDirectory:(NSString *)directory file:(NSString *)file {
   NSUnimplementedMethod();
   return 0;
}

-(int)openPanel:(NSOpenPanel *)openPanel runModalForDirectory:(NSString *)directory file:(NSString *)file types:(NSArray *)types {
   NSUnimplementedMethod();
   return 0;
}

-(NSPoint)mouseLocation {
    return pointerPos;
}

-(NSUInteger)currentModifierFlags {
   NSUnimplementedMethod();
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

#import <AppKit/NSGraphicsStyle.h>

@implementation NSGraphicsStyle (Overrides) 
-(void)drawMenuBranchArrowInRect:(NSRect)rect selected:(BOOL)selected {
    NSImage* arrow=[NSImage imageNamed:@"NSMenuArrow"];
    // ??? magic numbers
    rect.origin.y+=5;
    rect.origin.x-=2;
    [arrow drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
}
@end
