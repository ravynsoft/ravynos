/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#import "X11Display.h"
#import "X11Window.h"
#import <AppKit/NSScreen.h>
#import <AppKit/NSApplication.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSocket_bsd.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/KTFont_FT.h>
#import <AppKit/NSRaise.h>
#import <AppKit/O2Font_FT.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSWindow.h>
#import <fcntl.h>
#import <fontconfig.h>
#import <X11/Xutil.h>
#import <X11/keysym.h>

@implementation NSDisplay(X11)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([X11Display class],0,NULL);
}

@end

@implementation X11Display

static int errorHandler(Display *display,XErrorEvent *errorEvent) {
   return [(X11Display*)[X11Display currentDisplay] handleError:errorEvent];
}

-init {
   if(self=[super init]){
   
    _display=XOpenDisplay(NULL);
    
    if(_display==NULL){
     _display=XOpenDisplay(":0");
    }
    
    if(_display==NULL) {
     [self dealloc];
     return nil;
    }
        
    if(NSDebugEnabled)
     XSynchronize(_display, True);
     
    XSetErrorHandler(errorHandler);
      
    _fileDescriptor=ConnectionNumber(_display);
    _inputSource=[[NSSelectInputSource socketInputSourceWithSocket:[NSSocket_bsd socketWithDescriptor:_fileDescriptor]] retain];
    [_inputSource setDelegate:self];
    [_inputSource setSelectEventMask:NSSelectReadEvent];
      
    _windowsByID=[NSMutableDictionary new];

    lastFocusedWindow=nil;
    lastClickTimeStamp=0.0;
    clickCount=0;
   }
   return self;
}

-(void)dealloc {
   if(_display)
    XCloseDisplay(_display);
    
   [_windowsByID release];
   [super dealloc];
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
	return [[[X11Window alloc] initWithFrame:frame styleMask:styleMask isPanel:NO backingType:backingType] autorelease];
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
	return [[[X11Window alloc] initWithFrame:frame styleMask:styleMask isPanel:YES backingType:backingType] autorelease];
}

-(Display *)display {
   return _display;
}

-(NSArray *)screens {
   NSRect frame=NSMakeRect(0, 0,
                           DisplayWidth(_display, DefaultScreen(_display)),
                           DisplayHeight(_display, DefaultScreen(_display)));
   return [NSArray arrayWithObject:[[[NSScreen alloc] initWithFrame:frame visibleFrame:frame] autorelease]];
}

-(NSPasteboard *)pasteboardWithName:(NSString *)name {
   NSUnimplementedMethod();
   return nil;
}

-(NSDraggingManager *)draggingManager {
//   NSUnimplementedMethod();
   return nil;
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
   
   return [NSColor redColor];
   
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

-(void)beep {
   XBell(_display, 100);
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
    Window window;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    BOOL result = XQueryPointer(_display, DefaultRootWindow(_display),
        &window, &window, &rootX, &rootY, &winX, &winY, &mask);
    if(result == YES) {
        // invert the Y since Cocoa's origin is lower left
        int height = DisplayHeight(_display, DefaultScreen(_display));
        return NSMakePoint(rootX, height - rootY);
    }
    NSLog(@"-[X11Display mouseLocation] unable to locate mouse pointer");
    return NSMakePoint(0,0);
}

-(void)setWindow:(id)window forID:(XID)i
{
   if(window)
      [_windowsByID setObject:window forKey:[NSNumber numberWithUnsignedLong:(unsigned long)i]];
   else
      [_windowsByID removeObjectForKey:[NSNumber numberWithUnsignedLong:(unsigned long)i]];
}

-(id)windowForID:(XID)i
{
   return [_windowsByID objectForKey:[NSNumber numberWithUnsignedLong:i]];
}

-(NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue {
   NSEvent *result;

   [[NSRunLoop currentRunLoop] addInputSource:_inputSource forMode:mode];
   result=[super nextEventMatchingMask:mask untilDate:untilDate inMode:mode dequeue:dequeue];
   [[NSRunLoop currentRunLoop] removeInputSource:_inputSource forMode:mode];
   
   return result;
}

-(unsigned int)modifierFlagsForState:(unsigned int)state {
   unsigned int ret=0;
   if(state & ShiftMask)
      ret|=NSShiftKeyMask;
   if(state & ControlMask)
      ret|=NSControlKeyMask;
   if(state & Mod2Mask)
      ret|=NSCommandKeyMask;
   // TODO: alt doesn't work; might want to track key presses/releases instead
   return ret;
}

NSArray *CGSOrderedWindowNumbers() {
    NSMutableArray *result = [NSMutableArray array];
    
    for (NSWindow* win in [NSApp windows]) [result addObject:[NSNumber numberWithInteger:[win windowNumber]]];
    
    NSUnimplementedFunction(); //(Window numbers not even remotely ordered)
    
    return result;
}

-(void)postXEvent:(XEvent *)ev {
   id event=nil;
   NSEventType type;
   id window=[self windowForID:ev->xany.window];
   if(window == nil) { // guard against unknown IDs
      //NSLog(@"postXEvent: unknown window ID %d",ev->xany.window);
      return;
   }
   id delegate=[window delegate];
   
   switch(ev->type) {
    case KeyPress:
    case KeyRelease:;
     unsigned int modifierFlags=[self modifierFlagsForState:ev->xkey.state];
     unichar keys[32]={0};
     size_t buflen = 0;
     char buf[4];
     
     KeySym ks;
     XLookupString((XKeyEvent*)ev, buf, 4, &ks, NULL);

     // translate special characters
     switch(ks) {
        case XK_Home:
        case XK_KP_Home: keys[buflen++] = NSHomeFunctionKey; break;
        case XK_Left:
        case XK_KP_Left: keys[buflen++] = NSLeftArrowFunctionKey; break;
        case XK_Up:
        case XK_KP_Up: keys[buflen++] = NSUpArrowFunctionKey; break;
        case XK_Right:
        case XK_KP_Right: keys[buflen++] = NSRightArrowFunctionKey; break;
        case XK_Down:
        case XK_KP_Down: keys[buflen++] = NSDownArrowFunctionKey; break;
        case XK_Page_Up:
        case XK_KP_Page_Up: keys[buflen++] = NSPageUpFunctionKey; break;
        case XK_Page_Down:
        case XK_KP_Page_Down: keys[buflen++] = NSPageDownFunctionKey; break;
        case XK_End:
        case XK_KP_End: keys[buflen++] = NSEndFunctionKey; break;
        case XK_Begin:
        case XK_KP_Begin: keys[buflen++] = NSHomeFunctionKey; break;
        case XK_Delete:
        case XK_KP_Delete: keys[buflen++] = NSDeleteFunctionKey; break;
        case XK_Insert:
        case XK_KP_Insert: keys[buflen++] = NSInsertFunctionKey; break;
        case XK_F1: keys[buflen++] = NSF1FunctionKey; break;
        case XK_F2: keys[buflen++] = NSF2FunctionKey; break;
        case XK_F3: keys[buflen++] = NSF3FunctionKey; break;
        case XK_F4: keys[buflen++] = NSF4FunctionKey; break;
        case XK_F5: keys[buflen++] = NSF5FunctionKey; break;
        case XK_F6: keys[buflen++] = NSF6FunctionKey; break;
        case XK_F7: keys[buflen++] = NSF7FunctionKey; break;
        case XK_F8: keys[buflen++] = NSF8FunctionKey; break;
        case XK_F9: keys[buflen++] = NSF9FunctionKey; break;
        case XK_F10: keys[buflen++] = NSF10FunctionKey; break;
        case XK_F11: keys[buflen++] = NSF11FunctionKey; break;
        case XK_F12: keys[buflen++] = NSF12FunctionKey; break;
        case XK_F13: keys[buflen++] = NSF13FunctionKey; break;
        case XK_F14: keys[buflen++] = NSF14FunctionKey; break;
        case XK_F15: keys[buflen++] = NSF15FunctionKey; break;
        case XK_F16: keys[buflen++] = NSF16FunctionKey; break;
        case XK_F17: keys[buflen++] = NSF17FunctionKey; break;
        case XK_F18: keys[buflen++] = NSF18FunctionKey; break;
        case XK_F19: keys[buflen++] = NSF19FunctionKey; break;
        case XK_F20: keys[buflen++] = NSF20FunctionKey; break;
        case XK_F21: keys[buflen++] = NSF21FunctionKey; break;
        case XK_F22: keys[buflen++] = NSF22FunctionKey; break;
        case XK_F23: keys[buflen++] = NSF23FunctionKey; break;
        case XK_F24: keys[buflen++] = NSF24FunctionKey; break;
        case XK_F25: keys[buflen++] = NSF25FunctionKey; break;
        case XK_F26: keys[buflen++] = NSF26FunctionKey; break;
        case XK_F27: keys[buflen++] = NSF27FunctionKey; break;
        case XK_F28: keys[buflen++] = NSF28FunctionKey; break;
        case XK_F29: keys[buflen++] = NSF29FunctionKey; break;
        case XK_F30: keys[buflen++] = NSF30FunctionKey; break;
        case XK_F31: keys[buflen++] = NSF31FunctionKey; break;
        case XK_F32: keys[buflen++] = NSF32FunctionKey; break;
        case XK_F33: keys[buflen++] = NSF33FunctionKey; break;
        case XK_F34: keys[buflen++] = NSF34FunctionKey; break;
        case XK_F35: keys[buflen++] = NSF35FunctionKey; break;
        default: keys[buflen++] = buf[0];
     }

     id str=[[NSString alloc] initWithCharacters:keys length:buflen];
     NSPoint pos=[window transformPoint:NSMakePoint(ev->xkey.x, ev->xkey.y)];

     id strIg=[str lowercaseString];
     if(ev->xkey.state) {
      ev->xkey.state=0;
      XLookupString((XKeyEvent*)ev, buf, 4, NULL, NULL);
      strIg=[[NSString alloc] initWithCharacters:buf length:1];
     }
      
     id event=[NSEvent keyEventWithType:ev->type == KeyPress ? NSKeyDown : NSKeyUp location:pos
                              modifierFlags:modifierFlags
                                  timestamp:0.0 
                               windowNumber:(NSInteger)[delegate windowNumber]
                                    context:nil
                                 characters:str 
                charactersIgnoringModifiers:strIg
                                  isARepeat:NO
                                    keyCode:ev->xkey.keycode]; // FIXME: translate these to Apple keycodes?
         
     [self postEvent:event atStart:NO];
         
     [str release];
     break;

    case ButtonPress:;
     NSTimeInterval now=[[NSDate date] timeIntervalSinceReferenceDate];
     
     if(now-lastClickTimeStamp<[self doubleClickInterval]) {
      clickCount++;
     }
     else {
      clickCount=1;  
     }
     lastClickTimeStamp=now;

     pos=[window transformPoint:NSMakePoint(ev->xbutton.x, ev->xbutton.y)];

     event=[NSEvent mouseEventWithType:NSLeftMouseDown
                                  location:pos
                             modifierFlags:[self modifierFlagsForState:ev->xbutton.state]
                                    window:delegate
                                clickCount:clickCount deltaX:0.0 deltaY:0.0];
     [self postEvent:event atStart:NO];
     break;

    case ButtonRelease:;
     pos=[window transformPoint:NSMakePoint(ev->xbutton.x, ev->xbutton.y)];

     event=[NSEvent mouseEventWithType:NSLeftMouseUp
                                  location:pos
                             modifierFlags:[self modifierFlagsForState:ev->xbutton.state]
                                    window:delegate
                                clickCount:clickCount deltaX:0.0 deltaY:0.0];
     [self postEvent:event atStart:NO];
     break;

    case MotionNotify:;
     pos=[window transformPoint:NSMakePoint(ev->xmotion.x, ev->xmotion.y)];
     type=NSMouseMoved;
         
     if(ev->xmotion.state&Button1Mask) {
      type=NSLeftMouseDragged;
     }
     else if (ev->xmotion.state&Button2Mask) {
      type=NSRightMouseDragged;
     }
         
     if(type==NSMouseMoved && ![delegate acceptsMouseMovedEvents])
      break;
         
     event=[NSEvent mouseEventWithType:type
                                  location:pos
                             modifierFlags:[self modifierFlagsForState:ev->xmotion.state]
                                    window:delegate
                                clickCount:1 deltaX:0.0 deltaY:0.0];
      [self postEvent:event atStart:NO];
      [self discardEventsMatchingMask:NSLeftMouseDraggedMask beforeEvent:event];
      break;

    case EnterNotify:
     NSLog(@"EnterNotify");
     break;
     
    case LeaveNotify:
     NSLog(@"LeaveNotify");
     break;

    case FocusIn:
     if([delegate attachedSheet]) {
      [[delegate attachedSheet] makeKeyAndOrderFront:delegate];
      break;
     }
     if(lastFocusedWindow) {
      [lastFocusedWindow platformWindowDeactivated:window checkForAppDeactivation:NO];
      lastFocusedWindow=nil;  
     }
     [delegate platformWindowActivated:window displayIfNeeded:YES];
     lastFocusedWindow=delegate;
     break;
     
    case FocusOut:
     [delegate platformWindowDeactivated:window checkForAppDeactivation:NO];
     lastFocusedWindow=nil;
     break;
         
    case KeymapNotify:
     NSLog(@"KeymapNotify");
     break;

    case Expose:;
     O2Rect rect=NSMakeRect(ev->xexpose.x, ev->xexpose.y, ev->xexpose.width, ev->xexpose.height);
     
     rect.origin.y=[window frame].size.height-rect.origin.y-rect.size.height;
     // rect=NSInsetRect(rect, -10, -10);
     // [_backingContext addToDirtyRect:rect];
     if(ev->xexpose.count==0)
      [window flushBuffer];
     break;

    case GraphicsExpose:
     NSLog(@"GraphicsExpose");
     break;
     
    case NoExpose:
     NSLog(@"NoExpose");
     break;
     
    case VisibilityNotify:
//      NSLog(@"VisibilityNotify");
     break;

    case CreateNotify:
     NSLog(@"CreateNotify");
     break;

    case DestroyNotify:;
     // we should never get this message before the WM_DELETE_WINDOW ClientNotify
     // so normally, window should be nil here.
     [window invalidate];
     break;

    case UnmapNotify:
     NSLog(@"UnmapNotify");
     break;

    case MapNotify:
     NSLog(@"MapNotify");
     break;

    case MapRequest:
     NSLog(@"MapRequest");
     break;

    case ReparentNotify:
     NSLog(@"ReparentNotify");
     break;

    case ConfigureNotify:
     [window frameChanged];
     [delegate platformWindow:window frameChanged:[window transformFrame:[window frame]] didSize:YES];
     break;

    case ConfigureRequest:
     NSLog(@"ConfigureRequest");
     break;

    case GravityNotify:
     NSLog(@"GravityNotify");
     break;

    case ResizeRequest:
     NSLog(@"ResizeRequest");
     break;

    case CirculateNotify:
     NSLog(@"CirculateNotify");
     break;

    case CirculateRequest:
     NSLog(@"CirculateRequest");
     break;

    case PropertyNotify:
     NSLog(@"PropertyNotify");
     break;

    case SelectionClear:
     NSLog(@"SelectionClear");
     break;

    case SelectionRequest:
     NSLog(@"SelectionRequest");
     break;

    case SelectionNotify:
     NSLog(@"SelectionNotify");
     break;

    case ColormapNotify:
     NSLog(@"ColormapNotify");
     break;

    case ClientMessage:
     if(ev->xclient.format=32 && ev->xclient.data.l[0]==XInternAtom(_display, "WM_DELETE_WINDOW", False))
      [delegate platformWindowWillClose:window];
     break;

    case MappingNotify:
     NSLog(@"MappingNotify");
     break;

    case GenericEvent:
     NSLog(@"GenericEvent");
     break;

    default:
     NSLog(@"Unknown X11 event type %i", ev->type);
     break;
   }

}

-(void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent {
   int numEvents;
   
   while((numEvents=XPending(_display))>0) {
    XEvent e;
    int    error;
    
    if((error=XNextEvent(_display, &e))!=0)
     NSLog(@"XNextEvent returned %d",error);
    else
     [self postXEvent:&e];
     
   }
}

-(int)handleError:(XErrorEvent*)errorEvent {
   NSLog(@"************** ERROR");
   return 0;
}
@end

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
