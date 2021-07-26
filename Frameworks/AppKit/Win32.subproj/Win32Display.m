/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/Win32Display.h>
#import <AppKit/Win32Event.h>
#import <AppKit/Win32Window.h>
#import <AppKit/Win32Cursor.h>
#import <Onyx2D/O2Context_gdi.h>
#import <AppKit/Win32DeviceContextWindow.h>
#import "O2Context_gdi+AppKit.h"
#import <AppKit/Win32EventInputSource.h>

#import <AppKit/NSScreen.h>
#import <AppKit/NSEvent_CoreGraphics.h>
#import <AppKit/NSGraphicsContext.h>

#import <AppKit/Win32GeneralPasteboard.h>
#import <AppKit/Win32Pasteboard.h>
#import <Foundation/NSSet.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSColor_CGColor.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Font.h>
#import <AppKit/NSPrintInfo.h>
#import <AppKit/NSSavePanel-Win32.h>
#import <AppKit/NSOpenPanel-Win32.h>
#import <windows.h>
#import <windowsx.h>
#import <winuser.h>
#import <commdlg.h>
#import <malloc.h>

#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSFontMetric.h>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif

@implementation NSDisplay(windows)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([Win32Display class],0,NULL);
}

@end


@implementation Win32Display

//#define WAITCURSOR

#ifdef WAITCURSOR

static DWORD   mainThreadId;
static HANDLE  waitCursorStop;
static HANDLE  waitCursorStart;
static HCURSOR waitCursorHandle;

static DWORD WINAPI runWaitCursor(LPVOID arg){
   waitCursorHandle=LoadCursor(NULL,IDC_WAIT);

   AttachThreadInput(GetCurrentThreadId(),mainThreadId,TRUE);

   while(YES){
    WaitForSingleObject(waitCursorStart,INFINITE);

    if(WaitForSingleObject(waitCursorStop,500)==WAIT_TIMEOUT){
     SetCursor(waitCursorHandle);

     WaitForSingleObject(waitCursorStop,INFINITE);
    }
   }
}
#endif

+(void)initialize {
   if(self==[Win32Display class]){
#ifdef WAITCURSOR
    mainThreadId=GetCurrentThreadId();
    waitCursorStop=CreateEvent(NULL,FALSE,FALSE,NULL);
    waitCursorStart=CreateEvent(NULL,FALSE,FALSE,NULL);
    CreateThread(NULL,0,runWaitCursor,0,0,&threadID);
#endif
   }
}

+(Win32Display *)currentDisplay {
   return (Win32Display *)[super currentDisplay];
}

-(void)loadPrivateFontPaths:(NSArray *)paths {

#ifndef FR_PRIVATE
#define FR_PRIVATE 0x10
#endif
    HANDLE library=LoadLibrary("GDI32");
    typedef WINGDIAPI int WINAPI (*ftype)(LPCWSTR,DWORD,PVOID);
    ftype  function=(ftype)GetProcAddress(library,"AddFontResourceExW");
    if(function==NULL) {
        NSLog(@"GetProcAddress(\"GDI32\",\"AddFontResourceExW\") failed");
        return;
    }

    for(NSString *path in paths){
        const uint16_t *rep=[path fileSystemRepresentationW];
        if(function(rep,FR_PRIVATE,0)==0){
            NSLog(@"AddFontResourceExW failed for %@",path);
        }
    }
}

-(void)loadPrivateFonts {
    // A special info plist key can specify a resource dir containing the bundled application fonts - returns nil if
    // the path is not specified so the original code path is followed
    NSString* fontsDir = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"ATSApplicationFontsPath"];
    
    NSArray*  ttfPaths=[[NSBundle mainBundle] pathsForResourcesOfType:@"ttf" inDirectory: fontsDir];
    NSArray*  TTFPaths=[[NSBundle mainBundle] pathsForResourcesOfType:@"TTF" inDirectory: fontsDir];
    NSArray*  otfPaths=[[NSBundle mainBundle] pathsForResourcesOfType:@"otf" inDirectory: fontsDir];
    NSArray*  OTFPaths=[[NSBundle mainBundle] pathsForResourcesOfType:@"OTF" inDirectory: fontsDir];
    
    NSMutableArray* allPaths = [NSMutableArray arrayWithCapacity: 100];
    [allPaths addObjectsFromArray: ttfPaths];
    [allPaths addObjectsFromArray: TTFPaths];
    [allPaths addObjectsFromArray: otfPaths];
    [allPaths addObjectsFromArray: OTFPaths];
    
    [self loadPrivateFontPaths: allPaths];
}

-(void)forceLoadOfFontsAtPaths:(NSArray *)paths {
    [self loadPrivateFontPaths: paths];
}

-(id)init {
   self=[super init];
   if (self!=nil){
    _eventInputSource=[Win32EventInputSource new];

    _generalPasteboard=nil;
    _pasteboards=[NSMutableDictionary new];

    _nameToColor=[NSMutableDictionary new];

    _cursorDisplayCount=1;
    _cursorCache=[NSMutableDictionary new];
	_pastLocation = [self mouseLocation];

    _ignoringModifiersString = [NSMutableString new];
       
    [self loadPrivateFonts];
   }
   return self;
}

static BOOL CALLBACK monitorEnumerator(HMONITOR hMonitor,HDC hdcMonitor,LPRECT rect,LPARAM dwData) {
	static FARPROC  getMonitorInfo = NULL;
	
	if (NULL == getMonitorInfo) {
		HANDLE library = LoadLibrary("USER32");
		getMonitorInfo = GetProcAddress(library,"GetMonitorInfoA");
	}

	NSMutableArray *array= (id)dwData;
  
	MONITORINFOEX info;
	info.cbSize = sizeof(info);
	getMonitorInfo(hMonitor,&info);
	
	NSRect frame = CGRectFromRECT( info.rcMonitor );
	NSRect visibleFrame = CGRectFromRECT( info.rcWork );
    
    // According to http://msdn.microsoft.com/en-us/library/windows/desktop/dd145066(v=vs.85).aspx
    // the rcMonitor and rcWork rects are already in virtual screen coords so there should be
    // no need for any further massaging.
    
    // But apparently it's not true and the visibleFrame.origin.y needs to be offset - i.e. guess
    // there's some coordinate system origin mismatch?
    visibleFrame.origin.y = NSHeight(frame)-NSHeight(visibleFrame);
    
	NSScreen *screen=[[[NSScreen alloc] initWithFrame:frame visibleFrame:visibleFrame] autorelease];

   if (info.dwFlags & MONITORINFOF_PRIMARY) [array insertObject:screen atIndex:0];
   else [array addObject:screen];

   return TRUE;
}

-(NSArray *)screens {
	static FARPROC enumDisplayMonitors = NULL;
	if (NULL == enumDisplayMonitors) {
		HANDLE  library = LoadLibrary( "USER32" );
		enumDisplayMonitors = GetProcAddress(library,"EnumDisplayMonitors");
	}
	
	if(enumDisplayMonitors != NULL){
		NSMutableArray *result = [NSMutableArray array];
		enumDisplayMonitors( NULL, NULL, monitorEnumerator, result );
		return result;
	} else {
		NSRect frame=NSMakeRect(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

		return [NSArray arrayWithObject:[[[NSScreen alloc] initWithFrame:frame visibleFrame:frame] autorelease]];
   }
}

-(NSPasteboard *)pasteboardWithName:(NSString *)name {
   if([name isEqualToString:NSGeneralPboard]){
    if(_generalPasteboard==nil)
     _generalPasteboard=[[Win32GeneralPasteboard alloc] init];

    return _generalPasteboard;
   }
   else if([name isEqualToString:NSDragPboard])
    return [[[Win32Pasteboard alloc] init] autorelease];
   else {
    NSPasteboard *result=[_pasteboards objectForKey:name];

    if(result==nil){
     result=[[[Win32Pasteboard alloc] init] autorelease];
     [_pasteboards setObject:result forKey:name];
    }

    return result;
   }
}

-(NSDraggingManager *)draggingManager {
   return NSThreadSharedInstance(@"Win32DraggingManager");
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
   return [[[Win32Window alloc] initWithFrame:frame styleMask:styleMask isPanel:NO backingType:backingType] autorelease];
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
   return [[[Win32Window alloc] initWithFrame:frame styleMask:styleMask isPanel:YES backingType:backingType] autorelease];
}

-(void)invalidateSystemColors {
   [_nameToColor removeAllObjects];
}

-(void)buildSystemColors {
   struct {
    NSString *name;
    int       value;
   } table[]={
    { @"controlBackgroundColor", COLOR_WINDOW },
    { @"controlColor", COLOR_3DFACE },
    { @"controlDarkShadowColor", COLOR_3DDKSHADOW },
    { @"controlHighlightColor", COLOR_3DLIGHT },
    { @"controlLightHighlightColor", COLOR_3DHILIGHT },
    { @"controlShadowColor", COLOR_3DSHADOW },
    { @"controlTextColor", COLOR_BTNTEXT },
  //  { @"disabledControlTextColor", COLOR_3DSHADOW },
    { @"disabledControlTextColor", COLOR_GRAYTEXT },
    { @"highlightColor", COLOR_3DHILIGHT },
    { @"knobColor", COLOR_3DFACE },
    { @"scrollBarColor", COLOR_SCROLLBAR },
    { @"selectedControlColor", COLOR_HIGHLIGHT },
    { @"selectedControlTextColor", COLOR_HIGHLIGHTTEXT },
    { @"selectedKnobColor", COLOR_HIGHLIGHT },
    { @"selectedTextBackgroundColor", COLOR_HIGHLIGHT },
    { @"selectedTextColor", COLOR_HIGHLIGHTTEXT },
    { @"shadowColor", COLOR_3DDKSHADOW },
    { @"textBackgroundColor", COLOR_WINDOW },
    { @"textColor", COLOR_WINDOWTEXT },
    { @"gridColor", COLOR_INACTIVEBORDER },		// what should this be?
    { @"headerColor", COLOR_3DFACE },		// these do not appear in the user-space System color list,
    { @"headerTextColor", COLOR_BTNTEXT },	// probably because Apple builds that off System.clr
   { @"alternateSelectedControlColor", COLOR_WINDOW }, // FIXME:
   { @"alternateSelectedControlTextColor", COLOR_WINDOWTEXT }, // FIXME:
   { @"secondarySelectedControlColor", COLOR_HIGHLIGHT }, // FIXME:
   { @"keyboardFocusIndicatorColor", COLOR_ACTIVEBORDER }, // FIXME:
   { @"windowFrameColor", COLOR_WINDOWFRAME }, // FIXME:
   { @"selectedMenuItemColor", 29 /* COLOR_MENUHILIGHT */ }, // FIXME:
   { @"selectedMenuItemTextColor", COLOR_HIGHLIGHTTEXT }, // FIXME:
// extensions
    { @"menuBackgroundColor", COLOR_MENU },
	{ @"mainMenuBarColor", 30 },
    { @"menuItemTextColor", COLOR_MENUTEXT },
     { @"_sourceListBackgroundColor", COLOR_WINDOW },
 
    { nil, 0 }
   };
   int i;
   CGColorSpaceRef colorSpace=[[O2ColorSpace alloc] initWithPlatformRGB];

   for(i=0;table[i].name!=nil;i++){
    LOGBRUSH   contents;
    CGColorRef colorRef;
    CGFloat    components[4];
    NSColor   *color;

    GetObject(GetSysColorBrush(table[i].value),sizeof(LOGBRUSH),&contents);

    components[0]=GetRValue(contents.lbColor)/255.0;
    components[1]=GetGValue(contents.lbColor)/255.0;
    components[2]=GetBValue(contents.lbColor)/255.0;
    components[3]=1;
    
    colorRef=CGColorCreate(colorSpace,components);

    color=[NSColor_CGColor colorWithColorRef:colorRef spaceName:NSDeviceRGBColorSpace];
    CGColorRelease(colorRef);
    [_nameToColor setObject:color forKey:table[i].name];
   }
   
   CGColorSpaceRelease(colorSpace);
}

-(NSColor *)colorWithName:(NSString *)colorName {
   if([_nameToColor count]==0)
    [self buildSystemColors];

   return [_nameToColor objectForKey:colorName];
}

-(void) _addSystemColor: (NSColor *) color forName: (NSString *) name {
   if([_nameToColor count]==0)
    [self buildSystemColors];
   [_nameToColor setObject: color forKey: name];
}

-(NSTimeInterval)textCaretBlinkInterval {
   return ((float)GetCaretBlinkTime())/1000.0;
}

-(void)hideCursor {
   _cursorDisplayCount=ShowCursor(FALSE);
}

-(void)unhideCursor {
   _cursorDisplayCount=ShowCursor(TRUE);
}

-(void)_unhideCursorForMouseMove {
   while(_cursorDisplayCount<=0)
    [self unhideCursor];
}

-cursorWithName:(NSString *)name {
   id result=[_cursorCache objectForKey:name];

   if(result==nil){
    result=[[[Win32Cursor alloc] initWithName:name] autorelease];
    [_cursorCache setObject:result forKey:name];
   }

   return result;
}

-(void)setCursor:(id)cursor {
   HCURSOR handle=[cursor cursorHandle];
   // HCURSOR current=GetCursor();

   [_cursor autorelease];
   _cursor=[cursor retain];

 //  if(current!=handle)
    SetCursor(_lastCursor=handle);
}

-(void)stopWaitCursor {
#ifdef WAITCURSOR
   SetEvent(waitCursorStop);

   if(_lastCursor!=NULL)
    SetCursor(_lastCursor);
   else {
    POINT pt;
    GetCursorPos(&pt);
    SetCursorPos(pt.x, pt.y);
  //  _lastCursor=GetCursor();
   }
#endif
}

-(void)startWaitCursor {
#ifdef WAITCURSOR
   ResetEvent(waitCursorStop);
   SetEvent(waitCursorStart);
#endif
}

-(NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue {
   NSEvent *result=nil;

   [[NSRunLoop currentRunLoop] addInputSource:_eventInputSource forMode:mode];
   [self stopWaitCursor];
   
   do{
    result=[super nextEventMatchingMask:mask|NSPlatformSpecificDisplayMask untilDate:untilDate inMode:mode dequeue:dequeue];
    
    if([result type]==NSPlatformSpecificDisplayEvent){
     Win32Event *win32Event=(Win32Event *)[(NSEvent_CoreGraphics *)result coreGraphicsEvent];
     MSG msg=[win32Event msg];
     
     DispatchMessageW(&msg);
     result=nil;
    }
    
    if(result!=nil)
     break;
   }while([untilDate timeIntervalSinceNow]>0 || [_eventQueue count]>0);
   [self startWaitCursor];
   
//   [[NSRunLoop currentRunLoop] removeInputSource:_eventInputSource forMode:mode];

   return result;
}

#define kVK_ANSI_A 0x00
#define kVK_ANSI_S 0x01
#define kVK_ANSI_D 0x02
#define kVK_ANSI_F 0x03
#define kVK_ANSI_H 0x04
#define kVK_ANSI_G 0x05
#define kVK_ANSI_Z 0x06
#define kVK_ANSI_X 0x07
#define kVK_ANSI_C 0x08
#define kVK_ANSI_V 0x09
#define kVK_ANSI_B 0x0B
#define kVK_ANSI_Q 0x0C
#define kVK_ANSI_W 0x0D
#define kVK_ANSI_E 0x0E
#define kVK_ANSI_R 0x0F
#define kVK_ANSI_Y 0x10
#define kVK_ANSI_T 0x11
#define kVK_ANSI_1 0x12
#define kVK_ANSI_2 0x13
#define kVK_ANSI_3 0x14
#define kVK_ANSI_4 0x15
#define kVK_ANSI_6 0x16
#define kVK_ANSI_5 0x17
#define kVK_ANSI_Equal 0x18
#define kVK_ANSI_9 0x19
#define kVK_ANSI_7 0x1A
#define kVK_ANSI_Minus 0x1B
#define kVK_ANSI_8 0x1C
#define kVK_ANSI_0 0x1D
#define kVK_ANSI_RightBracket 0x1E
#define kVK_ANSI_O 0x1F
#define kVK_ANSI_U 0x20
#define kVK_ANSI_LeftBracket 0x21
#define kVK_ANSI_I 0x22
#define kVK_ANSI_P 0x23
#define kVK_ANSI_L 0x25
#define kVK_ANSI_J 0x26
#define kVK_ANSI_Quote 0x27
#define kVK_ANSI_K 0x28
#define kVK_ANSI_Semicolon 0x29
#define kVK_ANSI_Backslash 0x2A
#define kVK_ANSI_Comma 0x2B
#define kVK_ANSI_Slash 0x2C
#define kVK_ANSI_N 0x2D
#define kVK_ANSI_M 0x2E
#define kVK_ANSI_Period 0x2F
#define kVK_ANSI_Grave 0x32
#define kVK_ANSI_KeypadDecimal 0x41
#define kVK_ANSI_KeypadMultiply 0x43
#define kVK_ANSI_KeypadPlus 0x45
#define kVK_ANSI_KeypadClear 0x47
#define kVK_ANSI_KeypadDivide 0x4B
#define kVK_ANSI_KeypadEnter 0x4C
#define kVK_ANSI_KeypadMinus 0x4E
#define kVK_ANSI_KeypadEquals 0x51
#define kVK_ANSI_Keypad0 0x52
#define kVK_ANSI_Keypad1 0x53
#define kVK_ANSI_Keypad2 0x54
#define kVK_ANSI_Keypad3 0x55
#define kVK_ANSI_Keypad4 0x56
#define kVK_ANSI_Keypad5 0x57
#define kVK_ANSI_Keypad6 0x58
#define kVK_ANSI_Keypad7 0x59
#define kVK_ANSI_Keypad8 0x5B
#define kVK_ANSI_Keypad9 0x5C

#define kVK_Return 0x24
#define kVK_Tab 0x30
#define kVK_Space 0x31
#define kVK_Delete 0x33
#define kVK_Escape 0x35
#define kVK_Command 0x37
#define kVK_Shift 0x38
#define kVK_CapsLock 0x39
#define kVK_Option 0x3A
#define kVK_Control 0x3B
#define kVK_RightShift 0x3C
#define kVK_RightOption 0x3D
#define kVK_RightControl 0x3E
#define kVK_Function 0x3F
#define kVK_F17 0x40
#define kVK_VolumeUp 0x48
#define kVK_VolumeDown 0x49
#define kVK_Mute 0x4A
#define kVK_F18 0x4F
#define kVK_F19 0x50
#define kVK_F20 0x5A
#define kVK_F5 0x60
#define kVK_F6 0x61
#define kVK_F7 0x62
#define kVK_F3 0x63
#define kVK_F8 0x64
#define kVK_F9 0x65
#define kVK_F11 0x67
#define kVK_F13 0x69
#define kVK_F16 0x6A
#define kVK_F14 0x6B
#define kVK_F10 0x6D
#define kVK_F12 0x6F
#define kVK_F15 0x71
#define kVK_Help 0x72
#define kVK_Home 0x73
#define kVK_PageUp 0x74
#define kVK_ForwardDelete 0x75
#define kVK_F4 0x76
#define kVK_End 0x77
#define kVK_F2 0x78
#define kVK_PageDown 0x79
#define kVK_F1 0x7A
#define kVK_LeftArrow 0x7B
#define kVK_RightArrow 0x7C
#define kVK_DownArrow 0x7D
#define kVK_UpArrow 0x7E
#define kVK_ISO_Section 0x0A
#define kVK_JIS_Yen 0x5D
#define kVK_JIS_Underscore 0x5E
#define kVK_JIS_KeypadComma 0x5F
#define kVK_JIS_Eisu 0x66
#define kVK_JIS_Kana 0x68

#define kVK_UNMAPPED 0xFFFF

unsigned appleKeyCodeForWindowsKeyCode(unsigned wParam,unsigned lParam,BOOL *isKeypad){
   unsigned scanCode=(lParam>>16)&0xFF;

   *isKeypad=NO;
// clang-format off
   if(lParam&0x01000000){
    *isKeypad=YES;

    switch(scanCode){
     case 0x35: return kVK_ANSI_KeypadDivide; // /
     case 0x1C: return kVK_ANSI_KeypadEnter; // Enter

     case 0x52: return kVK_Help;  // Insert
     case 0x47: return kVK_Home;  // Home
     case 0x49: return kVK_PageUp;  // PageUp

     case 0x53: return kVK_ForwardDelete;  // Delete
     case 0x4F: return kVK_End;  // End
     case 0x51: return kVK_PageDown;  // PageDown

     case 0x48: return kVK_UpArrow;  // Up
     case 0x4B: return kVK_LeftArrow;  // Left
     case 0x50: return kVK_DownArrow;  // Down
     case 0x4D: return kVK_RightArrow;  // Right

     case 0x38: return kVK_RightOption; // Right Alternate
     case 0x1D: return kVK_RightControl; // Right Control
    }
   }

   switch(wParam){
    case '0': return kVK_ANSI_0;
    case '1': return kVK_ANSI_1;
    case '2': return kVK_ANSI_2;
    case '3': return kVK_ANSI_3;
    case '4': return kVK_ANSI_4;
    case '5': return kVK_ANSI_5;
    case '6': return kVK_ANSI_6;
    case '7': return kVK_ANSI_7;
    case '8': return kVK_ANSI_8;
    case '9': return kVK_ANSI_9;

    case 'A': return kVK_ANSI_A;
    case 'B': return kVK_ANSI_B;
    case 'C': return kVK_ANSI_C;
    case 'D': return kVK_ANSI_D;
    case 'E': return kVK_ANSI_E;
    case 'F': return kVK_ANSI_F;
    case 'G': return kVK_ANSI_G;
    case 'H': return kVK_ANSI_H;
    case 'I': return kVK_ANSI_I;
    case 'J': return kVK_ANSI_J;
    case 'K': return kVK_ANSI_K;
    case 'L': return kVK_ANSI_L;
    case 'M': return kVK_ANSI_M;
    case 'N': return kVK_ANSI_N;
    case 'O': return kVK_ANSI_O;
    case 'P': return kVK_ANSI_P;
    case 'Q': return kVK_ANSI_Q;
    case 'R': return kVK_ANSI_R;
    case 'S': return kVK_ANSI_S;
    case 'T': return kVK_ANSI_T;
    case 'U': return kVK_ANSI_U;
    case 'V': return kVK_ANSI_V;
    case 'W': return kVK_ANSI_W;
    case 'X': return kVK_ANSI_X;
    case 'Y': return kVK_ANSI_Y;
    case 'Z': return kVK_ANSI_Z;

    case VK_LBUTTON: return kVK_UNMAPPED;
    case VK_RBUTTON: return kVK_UNMAPPED;
    case VK_CANCEL: return kVK_UNMAPPED;
    case VK_MBUTTON: return kVK_UNMAPPED;
    case VK_XBUTTON1: return kVK_UNMAPPED;
    case VK_XBUTTON2: return kVK_UNMAPPED;
    case VK_BACK: return kVK_Delete;
    case VK_TAB: return kVK_Tab;
    case VK_CLEAR: *isKeypad=YES; return kVK_ANSI_KeypadClear;
    case VK_RETURN: return kVK_Return;
    case VK_SHIFT: return kVK_Shift;
    case VK_CONTROL: return kVK_Control;
    case VK_MENU: return kVK_UNMAPPED;
    case VK_PAUSE: return kVK_UNMAPPED;
    case VK_CAPITAL: return kVK_CapsLock;
#if 0
    case VK_KANA: return kVK_UNMAPPED;
    case VK_HANGEUL: return kVK_UNMAPPED;
    case VK_HANGUL: return kVK_UNMAPPED;
    case VK_JUNJA: return kVK_UNMAPPED;
    case VK_FINAL: return kVK_UNMAPPED;
    case VK_HANJA: return kVK_UNMAPPED;
    case VK_KANJI: return kVK_UNMAPPED;
#endif
    case VK_ESCAPE: return kVK_Escape;
    case VK_CONVERT: return kVK_UNMAPPED;
    case VK_NONCONVERT: return kVK_UNMAPPED;
    case VK_ACCEPT: return kVK_UNMAPPED;
    case VK_MODECHANGE: return kVK_UNMAPPED;
    case VK_SPACE: return kVK_Space;
    case VK_PRIOR: return kVK_PageUp;
    case VK_NEXT: return kVK_PageDown;
    case VK_END: return kVK_End;
    case VK_HOME: return kVK_Home;
    case VK_LEFT: return kVK_LeftArrow;
    case VK_UP: return kVK_UpArrow;
    case VK_RIGHT: return kVK_RightArrow;
    case VK_DOWN: return kVK_DownArrow;
    case VK_SELECT: return kVK_UNMAPPED;
    case VK_PRINT: return kVK_UNMAPPED;
    case VK_EXECUTE: return kVK_UNMAPPED;
    case VK_SNAPSHOT: return kVK_UNMAPPED;
    case VK_INSERT: return kVK_UNMAPPED;
    case VK_DELETE: return kVK_ForwardDelete;
    case VK_HELP: return kVK_Help;
    case VK_LWIN: return kVK_UNMAPPED;
    case VK_RWIN: return kVK_UNMAPPED;
    case VK_APPS: return kVK_UNMAPPED;
    case VK_SLEEP: return kVK_UNMAPPED;
    case VK_NUMPAD0: *isKeypad=YES;return kVK_ANSI_Keypad0;
    case VK_NUMPAD1: *isKeypad=YES;return kVK_ANSI_Keypad1;
    case VK_NUMPAD2: *isKeypad=YES;return kVK_ANSI_Keypad2;
    case VK_NUMPAD3: *isKeypad=YES;return kVK_ANSI_Keypad3;
    case VK_NUMPAD4: *isKeypad=YES;return kVK_ANSI_Keypad4;
    case VK_NUMPAD5: *isKeypad=YES;return kVK_ANSI_Keypad5;
    case VK_NUMPAD6: *isKeypad=YES;return kVK_ANSI_Keypad6;
    case VK_NUMPAD7: *isKeypad=YES;return kVK_ANSI_Keypad7;
    case VK_NUMPAD8: *isKeypad=YES;return kVK_ANSI_Keypad8;
    case VK_NUMPAD9: *isKeypad=YES;return kVK_ANSI_Keypad9;
    case VK_MULTIPLY: *isKeypad=YES;return kVK_ANSI_KeypadMultiply;
    case VK_ADD: *isKeypad=YES;return kVK_ANSI_KeypadPlus;
    case VK_SEPARATOR: *isKeypad=YES;return kVK_ANSI_KeypadEnter;
    case VK_SUBTRACT: *isKeypad=YES;return kVK_ANSI_KeypadMinus;
    case VK_DECIMAL: *isKeypad=YES;return kVK_ANSI_KeypadDecimal;
    case VK_DIVIDE: *isKeypad=YES;return kVK_ANSI_KeypadDivide;
    case VK_F1: return kVK_F1;
    case VK_F2: return kVK_F2;
    case VK_F3: return kVK_F3;
    case VK_F4: return kVK_F4;
    case VK_F5: return kVK_F5;
    case VK_F6: return kVK_F6;
    case VK_F7: return kVK_F7;
    case VK_F8: return kVK_F8;
    case VK_F9: return kVK_F9;
    case VK_F10: return kVK_F10;
    case VK_F11: return kVK_F11;
    case VK_F12: return kVK_F12;
    case VK_F13: return kVK_F13;
    case VK_F14: return kVK_F14;
    case VK_F15: return kVK_F15;
    case VK_F16: return kVK_F16;
    case VK_F17: return kVK_F17;
    case VK_F18: return kVK_F18;
    case VK_F19: return kVK_F19;
    case VK_F20: return kVK_F20;
    case VK_F21: return kVK_UNMAPPED;
    case VK_F22: return kVK_UNMAPPED;
    case VK_F23: return kVK_UNMAPPED;
    case VK_F24: return kVK_UNMAPPED;
    case VK_NUMLOCK: return kVK_UNMAPPED;
    case VK_SCROLL: return kVK_UNMAPPED;
    case VK_LSHIFT: return kVK_UNMAPPED;
    case VK_RSHIFT: return kVK_UNMAPPED;
    case VK_LCONTROL: return kVK_UNMAPPED;
    case VK_RCONTROL: return kVK_UNMAPPED;
    case VK_LMENU: return kVK_UNMAPPED;
    case VK_RMENU: return kVK_UNMAPPED;
    case VK_BROWSER_BACK: return kVK_UNMAPPED;
    case VK_BROWSER_FORWARD: return kVK_UNMAPPED;
    case VK_BROWSER_REFRESH: return kVK_UNMAPPED;
    case VK_BROWSER_STOP: return kVK_UNMAPPED;
    case VK_BROWSER_SEARCH: return kVK_UNMAPPED;
    case VK_BROWSER_FAVORITES: return kVK_UNMAPPED;
    case VK_BROWSER_HOME: return kVK_UNMAPPED;
    case VK_VOLUME_MUTE: return kVK_Mute;
    case VK_VOLUME_DOWN: return kVK_VolumeDown;
    case VK_VOLUME_UP: return kVK_VolumeUp;
    case VK_MEDIA_NEXT_TRACK: return kVK_UNMAPPED;
    case VK_MEDIA_PREV_TRACK: return kVK_UNMAPPED;
    case VK_MEDIA_STOP: return kVK_UNMAPPED;
    case VK_MEDIA_PLAY_PAUSE: return kVK_UNMAPPED;
    case VK_LAUNCH_MAIL: return kVK_UNMAPPED;
    case VK_LAUNCH_MEDIA_SELECT: return kVK_UNMAPPED;
    case VK_LAUNCH_APP1: return kVK_UNMAPPED;
    case VK_LAUNCH_APP2: return kVK_UNMAPPED;
    
    case VK_OEM_1: return kVK_ANSI_Semicolon;
    case VK_OEM_PLUS: return kVK_ANSI_Equal;
    case VK_OEM_COMMA: return kVK_ANSI_Comma;
    case VK_OEM_MINUS: return kVK_ANSI_Minus;
    case VK_OEM_PERIOD: return kVK_ANSI_Period;
    case VK_OEM_2: return kVK_ANSI_Slash;
    case VK_OEM_3: return kVK_ANSI_Grave;
    case VK_OEM_4: return kVK_ANSI_LeftBracket;
    case VK_OEM_5: return kVK_ANSI_Backslash;
    case VK_OEM_6: return kVK_ANSI_RightBracket;
    case VK_OEM_7: return kVK_ANSI_Quote;
    case VK_OEM_8: return kVK_UNMAPPED;
    case VK_OEM_102: return kVK_UNMAPPED;
    case VK_PROCESSKEY: return kVK_UNMAPPED;
    case VK_PACKET: return kVK_UNMAPPED;
    case VK_ATTN: return kVK_UNMAPPED;
    case VK_CRSEL: return kVK_UNMAPPED;
    case VK_EXSEL: return kVK_UNMAPPED;
    case VK_EREOF: return kVK_UNMAPPED;
    case VK_PLAY: return kVK_UNMAPPED;
    case VK_ZOOM: return kVK_UNMAPPED;
    case VK_NONAME: return kVK_UNMAPPED;
    case VK_PA1: return kVK_UNMAPPED;
    case VK_OEM_CLEAR: return kVK_UNMAPPED;
   }
// clang-format on
    
   return kVK_UNMAPPED;
}

/* Windows does not use different scan codes for keypad keys, there is a seperate bit in lParam to distinguish this. YellowBox does not pass this extra bit of information on via NSEvent which is a real nuisance if you actually need it. This remaps the extended keys to the keyCode's used on NEXTSTEP/OPENSTEP.

The values should be upgraded to something which is more generic to implement, perhaps passing the windows values through.
 
 */
// FIX
-(unsigned)keyCodeForLParam:(LPARAM)lParam isKeypad:(BOOL *)isKeypad{
   unsigned keyCode=(lParam>>16)&0xFF;

   *isKeypad=NO;
// clang-format off
   if(lParam&0x01000000){
    *isKeypad=YES;

    switch(keyCode){
     case 0x35: keyCode=0x63; break; // /
     case 0x1C: keyCode=0x62; break; // Enter

     case 0x52: keyCode=0x68; break; // Insert
     case 0x47: keyCode=0x6C; break; // Home
     case 0x49: keyCode=0x6A; break; // PageUp

     case 0x53: keyCode=0x69; break; // Delete
     case 0x4F: keyCode=0x6D; break; // End
     case 0x51: keyCode=0x6b; break; // PageDown

     case 0x48: keyCode=0x64; break; // Up
     case 0x4B: keyCode=0x66; break; // Left
     case 0x50: keyCode=0x65; break; // Down
     case 0x4D: keyCode=0x67; break; // Right

     case 0x38: keyCode=0x61; break; // Right Alternate
     case 0x1D: keyCode=0x60; break; // Right Control
    }
   }
// clang-format on
    
   return keyCode;
}

-(BOOL)postKeyboardMSG:(MSG)msg type:(NSEventType)type location:(NSPoint)location modifierFlags:(unsigned)modifierFlags window:(NSWindow *)window keyboardState:(BYTE *)keyboardState {
    unichar        buffer[256],ignoringBuffer[256];
    BOOL           isARepeat=NO;
    int            bufferSize=0,ignoringBufferSize=0;
    BYTE           *keyState=keyboardState;
    
    // The recommended way to properly handle unicode is to handle WM_CHAR events, generated by using TranslateMessage
    // But to built the chars ignoring the modifiers, we need to use ToUnicode.
    // Unfortunately, ToUnicode is modifying the character layout internal state machine, used to deal with dead keys
    // So, for things that looks like shortcuts (ctrl is down, but not the alt key, used to compose some chars), we do all
    // the processing at WM_KEYDOWN events, using ToUnicode to get both the "normal" chars, and the unmodified ones
    // For other events, we call TranslateMessage, which is needed for proper IME support.
    // (that means shortcuts without a "ctrl" modifier are probably not properly handled, as well as the
    // charactersIgnoringModifiers property of NSEvent for non-shortcuts)
    if (msg.message == WM_CHAR || msg.message == WM_SYSCHAR) {
        *buffer = msg.wParam;
        bufferSize = 1;
    } else {
        BOOL isCtrlAlt = (modifierFlags & (NSCommandKeyMask | NSAlternateKeyMask)) == (NSCommandKeyMask | NSAlternateKeyMask);
        BOOL isCtrl = (modifierFlags & NSCommandKeyMask) == NSCommandKeyMask;
        if (isCtrl == NO || isCtrlAlt == YES) {
            // Not a shortcut - use the normal char composing path
            TranslateMessage(&msg);
        } else {
            // Get the char corresponding to the key down
            bufferSize = ToUnicode(msg.wParam,msg.lParam>>16,keyboardState,buffer,10,0);
            
            // Get the char without the modifiers
            BYTE kstate[256];
            memcpy(kstate, keyboardState, 256);
            kstate[VK_CONTROL]=0x00;
            kstate[VK_LCONTROL]=0x00;
            kstate[VK_RCONTROL]=0x00;
            kstate[VK_CAPITAL]=0x00;
            
            kstate[VK_MENU]=0x00;
            kstate[VK_LMENU]=0x00;
            kstate[VK_RMENU]=0x00;
            ignoringBufferSize = ToUnicode(msg.wParam,msg.lParam>>16,kstate,ignoringBuffer,10,0);
            // Empty the keyboard state machine until we eat all of the dead keys
            while(ignoringBufferSize < 0) {
                ignoringBufferSize = ToUnicode(msg.wParam,msg.lParam>>16,kstate,ignoringBuffer,10,0);
            }
            if (ignoringBufferSize > 0) {
                NSString *str = [[[NSString alloc] initWithCharacters:ignoringBuffer length:ignoringBufferSize] autorelease];
                [_ignoringModifiersString appendString:str];
            }
            
            // Calling to ToUnicode is changing the current logic state about dealing with dead keys so we need to put things
            // back to the state it was
            // see http://blogs.msdn.com/b/michkap/archive/2005/01/19/355870.aspx
            ToUnicode(msg.wParam,msg.lParam>>16,keyboardState,ignoringBuffer,10,0);
        }
    }
    
    // Let's save the current keyCode
    _keyCode=appleKeyCodeForWindowsKeyCode(msg.wParam,msg.lParam,&_isKeypad);
    
	if(bufferSize==0){
        // clang-format off
		// Handle the special keys - we won't receive any char message from them
        switch(msg.wParam){
            case VK_LBUTTON: break;
            case VK_RBUTTON: break;
            case VK_CANCEL:  break;
            case VK_MBUTTON: break;
                
            case VK_BACK:    break;
                
            case VK_CLEAR:   buffer[bufferSize++]=NSClearDisplayFunctionKey;break;
            case VK_RETURN:  break;
                
            case VK_SHIFT: break;
            case VK_CONTROL: break;
            case VK_MENU:
                buffer[bufferSize++]=' '; // lame
                type=NSFlagsChanged;
                break;
                
            case VK_PAUSE:    buffer[bufferSize++]=NSPauseFunctionKey;       break;
            case VK_CAPITAL:  break;
                
            case VK_PRIOR:    buffer[bufferSize++]=NSPageUpFunctionKey;      break;
            case VK_NEXT:     buffer[bufferSize++]=NSPageDownFunctionKey;    break;
            case VK_END:      buffer[bufferSize++]=NSEndFunctionKey;         break;
            case VK_HOME:     buffer[bufferSize++]=NSHomeFunctionKey;        break;
            case VK_LEFT:     buffer[bufferSize++]=NSLeftArrowFunctionKey;   break;
            case VK_UP:       buffer[bufferSize++]=NSUpArrowFunctionKey;     break;
            case VK_RIGHT:    buffer[bufferSize++]=NSRightArrowFunctionKey;  break;
            case VK_DOWN:     buffer[bufferSize++]=NSDownArrowFunctionKey;   break;
            case VK_SELECT:   buffer[bufferSize++]=NSSelectFunctionKey;      break;
            case VK_PRINT:    buffer[bufferSize++]=NSPrintFunctionKey;       break;
            case VK_EXECUTE:  buffer[bufferSize++]=NSExecuteFunctionKey;     break;
            case VK_SNAPSHOT: buffer[bufferSize++]=NSPrintScreenFunctionKey; break;
            case VK_INSERT:   buffer[bufferSize++]=NSInsertFunctionKey;      break;
            case VK_DELETE:   buffer[bufferSize++]=NSDeleteFunctionKey;      break;
            case VK_HELP:     buffer[bufferSize++]=NSHelpFunctionKey;        break;
                
            case VK_LWIN:     break;
            case VK_RWIN:     break;
            case VK_APPS:     break;
                
            case VK_SEPARATOR:break;
            case VK_SUBTRACT: break;
            case VK_DECIMAL:  break;
            case VK_DIVIDE:   break;
                
            case VK_F1:       buffer[bufferSize++]=NSF1FunctionKey; break;
            case VK_F2:       buffer[bufferSize++]=NSF2FunctionKey; break;
            case VK_F3:       buffer[bufferSize++]=NSF3FunctionKey; break;
            case VK_F4:       buffer[bufferSize++]=NSF4FunctionKey; break;
            case VK_F5:       buffer[bufferSize++]=NSF5FunctionKey; break;
            case VK_F6:       buffer[bufferSize++]=NSF6FunctionKey; break;
            case VK_F7:       buffer[bufferSize++]=NSF7FunctionKey; break;
            case VK_F8:       buffer[bufferSize++]=NSF8FunctionKey; break;
            case VK_F9:       buffer[bufferSize++]=NSF9FunctionKey; break;
            case VK_F10:      buffer[bufferSize++]=NSF10FunctionKey; break;
            case VK_F11:      buffer[bufferSize++]=NSF11FunctionKey; break;
            case VK_F12:      buffer[bufferSize++]=NSF12FunctionKey; break;
            case VK_F13:      buffer[bufferSize++]=NSF13FunctionKey; break;
            case VK_F14:      buffer[bufferSize++]=NSF14FunctionKey; break;
            case VK_F15:      buffer[bufferSize++]=NSF15FunctionKey; break;
            case VK_F16:      buffer[bufferSize++]=NSF16FunctionKey; break;
            case VK_F17:      buffer[bufferSize++]=NSF17FunctionKey; break;
            case VK_F18:      buffer[bufferSize++]=NSF18FunctionKey; break;
            case VK_F19:      buffer[bufferSize++]=NSF19FunctionKey; break;
            case VK_F20:      buffer[bufferSize++]=NSF20FunctionKey; break;
            case VK_F21:      buffer[bufferSize++]=NSF21FunctionKey; break;
            case VK_F22:      buffer[bufferSize++]=NSF22FunctionKey; break;
            case VK_F23:      buffer[bufferSize++]=NSF23FunctionKey; break;
            case VK_F24:      buffer[bufferSize++]=NSF24FunctionKey; break;
                
            case VK_NUMLOCK:  break;
            case VK_SCROLL:   buffer[bufferSize++]=NSScrollLockFunctionKey; break;
                
                /* these constants are only useful with GetKeyboardState
                 case VK_LSHIFT:   NSLog(@"VK_LSHIFT"); break;
                 case VK_RSHIFT:   NSLog(@"VK_RSHIFT"); break;
                 case VK_LCONTROL: NSLog(@"VK_LCONTROL"); break;
                 case VK_RCONTROL: NSLog(@"VK_RCONTROL"); break;
                 case VK_LMENU:    NSLog(@"VK_LMENU"); break;
                 case VK_RMENU:    NSLog(@"VK_RMENU"); break;
                 */
                
            case VK_ATTN: break;
            case VK_CRSEL: break;
            case VK_EXSEL: break;
            case VK_EREOF: break;
            case VK_PLAY: break;
            case VK_ZOOM: break;
            case VK_NONAME: break;
            case VK_PA1: break;
            case VK_OEM_CLEAR: break;
        }
        // clang-format on
    }
    if (bufferSize > 0 || [_ignoringModifiersString length] > 0) {
        NSEvent *event;
        BOOL     isKeypad;
        
        NSString *characters=(bufferSize>0)?[NSString stringWithCharacters:buffer length:bufferSize]:@"";
        NSString *charactersIgnoringModifiers=_ignoringModifiersString;
        // Only send the event if we have something to send
        if (characters.length > 0 || charactersIgnoringModifiers.length > 0) {
            if (_ignoringModifiersString.length == 0) {
                charactersIgnoringModifiers = characters;
            }
            if(_isKeypad) {
                modifierFlags|=NSNumericPadKeyMask;
            }
            event=[NSEvent keyEventWithType:type location:location modifierFlags:modifierFlags timestamp:[NSDate timeIntervalSinceReferenceDate] windowNumber:[window windowNumber] context:nil characters:characters charactersIgnoringModifiers:charactersIgnoringModifiers isARepeat:isARepeat keyCode:_keyCode];
            [self postEvent:event atStart:NO];

            [_ignoringModifiersString release];
            _ignoringModifiersString = [[NSMutableString alloc] init];

            return YES;
        }
    }
    return NO;
}

-(BOOL)postMouseMSG:(MSG)msg type:(NSEventType)type location:(NSPoint)location modifierFlags:(unsigned)modifierFlags window:(NSWindow *)window {
	NSEvent *event;
/* Use mouseLocation to compute deltas, message coordinates are window based, and if the window is moving
   with the mouse, things get messy
 */
    NSPoint currentLocation=[self mouseLocation];
    CGFloat deltaX=currentLocation.x-_pastLocation.x;
    CGFloat deltaY=-(currentLocation.y-_pastLocation.y);
    
	if (type == NSMouseMoved) {
		if (fabs(deltaX) < 1. && fabs(deltaY) < 1.) {
			return YES;
		}
	}
   event = [NSEvent mouseEventWithType:type location:location modifierFlags:modifierFlags window:window clickCount:_clickCount deltaX:deltaX deltaY:deltaY];

    _pastLocation = currentLocation;
    
	[self postEvent:event atStart:NO];
	
	return YES;
}

-(BOOL)postScrollWheelMSG:(MSG)msg type:(NSEventType)type location:(NSPoint)location modifierFlags:(unsigned)modifierFlags window:(NSWindow *)window {
   NSEvent *event;
    
    float deltaX=0;
    float deltaY=0;
    if(msg.message==WM_MOUSEWHEEL) {
        deltaY = ((float)GET_WHEEL_DELTA_WPARAM(msg.wParam));
        deltaY /= 120.f; // deltaY comes in units of 120 (for fractional rotations - when all you have is an int..)
    } else if(msg.message==WM_MOUSEHWHEEL) {
        deltaX = ((float)GET_WHEEL_DELTA_WPARAM(msg.wParam));
        deltaX /= 120.f; // deltaX comes in units of 120 (for fractional rotations - when all you have is an int..)
    }

    event=[NSEvent mouseEventWithType:type location:location modifierFlags:modifierFlags window:window clickCount:0 deltaX:deltaX deltaY:deltaY];
    [self postEvent:event atStart:NO];
    return YES;
}

-(unsigned)currentModifierFlagsWithKeyboardState:(BYTE *)keyboardState {
   unsigned result=0;
   BYTE    *keyState=keyboardState;

   if(keyState==NULL)
    return result;

   if(keyState[VK_LSHIFT]&0x80)
    result|=NSShiftKeyMask;
   if(keyState[VK_RSHIFT]&0x80)
    result|=NSShiftKeyMask;

   if(keyState[VK_CAPITAL]&0x80)
    result|=NSAlphaShiftKeyMask;

   if(keyState[VK_LCONTROL]&0x80)
    result|=[self modifierForDefault:@"LeftControl":NSControlKeyMask];
   if(keyState[VK_RCONTROL]&0x80)
    result|=[self modifierForDefault:@"RightControl":NSControlKeyMask];

   if(keyState[VK_LMENU]&0x80)
    result|=[self modifierForDefault:@"LeftAlt":NSAlternateKeyMask];
   if(keyState[VK_RMENU]&0x80)
    result|=[self modifierForDefault:@"RightAlt":NSAlternateKeyMask];


   if(keyState[VK_NUMPAD0]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD1]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD2]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD3]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD4]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD5]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD6]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD7]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD8]&0x80)
    result|=NSNumericPadKeyMask;
   if(keyState[VK_NUMPAD9]&0x80)
    result|=NSNumericPadKeyMask;

   return result;
}

-(NSUInteger)currentModifierFlags {
    BYTE keyState[256];
    BYTE *keyboardState=NULL;
    
    if(GetKeyboardState(keyState))
        keyboardState=keyState;

    return [self currentModifierFlagsWithKeyboardState:keyboardState];
}

NSArray *CGSOrderedWindowNumbers(){
   NSMutableArray *result=[NSMutableArray array];

   HWND check=GetTopWindow(NULL);
   
   while(check!=NULL){
    Win32Window *platformWindow=GetProp(check,"Win32Window");
    
    if(platformWindow!=nil)
     [result addObject:[NSNumber numberWithInteger:[platformWindow windowNumber]]];

    check=GetNextWindow(check,GW_HWNDNEXT);
   }

   return result;
}

static HWND findWindowForScrollWheel(POINT point){
   HWND check=GetTopWindow(NULL);
   
   while(check!=NULL){
    RECT checkRect={0};

    GetWindowRect(check,&checkRect);
    
    if(PtInRect(&checkRect,point)){
     if((id)GetProp(check,"self")!=nil)
      return check;
    }
    
    check=GetNextWindow(check,GW_HWNDNEXT);
   }
   
   return check;
}


-(BOOL)postMSG:(MSG)msg keyboardState:(BYTE *)keyboardState {
   NSEventType  type;
   HWND         windowHandle=msg.hwnd;
   id           platformWindow;
   NSWindow    *window=nil;
   POINT        deviceLocation;
   NSPoint      location;
   unsigned     modifierFlags;
   DWORD        tickCount=GetTickCount();
   int          lastClickCount=_clickCount;

   deviceLocation.x=GET_X_LPARAM(msg.lParam);
   deviceLocation.y=GET_Y_LPARAM(msg.lParam);

  if(msg.message==WM_MOUSEWHEEL || msg.message==WM_MOUSEHWHEEL) {
// Scroll wheel events go to the window under the mouse regardless of key. Win32 set hwnd to the active window
// So we look for the window under the mouse and use that for the event.
    POINT pt={GET_X_LPARAM(msg.lParam),GET_Y_LPARAM(msg.lParam)};
    RECT  r;
    
    GetWindowRect(windowHandle,&r);
    pt.x+=r.left;
    pt.y+=r.top;
    
    HWND scrollWheelWindow=findWindowForScrollWheel(pt);
    
    if(scrollWheelWindow!=NULL)
     windowHandle=scrollWheelWindow;
     
    platformWindow=(id)GetProp(windowHandle,"Win32Window");
   }
   else {
    platformWindow=(id)GetProp(msg.hwnd,"Win32Window");
   }
   
   if([platformWindow respondsToSelector:@selector(appkitWindow)])
    window=[platformWindow performSelector:@selector(appkitWindow)];

   if(![window isKindOfClass:[NSWindow class]])
    window=nil;

   if(window==nil) // not one of our events
    return NO;

   if(msg.message==WM_LBUTTONDBLCLK || msg.message==WM_RBUTTONDBLCLK){
    if(msg.lParam==_lastPosition && _lastTickCount+GetDoubleClickTime()>=tickCount)
      _clickCount=lastClickCount+1;
    else
      _clickCount=2;
    _lastTickCount=tickCount;
    _lastPosition=msg.lParam;
   }
   else if(msg.message==WM_LBUTTONDOWN || msg.message==WM_RBUTTONDOWN){
    if(msg.lParam==_lastPosition && _lastTickCount+GetDoubleClickTime()>=tickCount)
      _clickCount=lastClickCount+1;
    else
      _clickCount=1;
    _lastTickCount=tickCount;
    _lastPosition=msg.lParam;
   }
    switch(msg.message){

     case WM_CHAR:
     case WM_SYSCHAR:
     case WM_KEYDOWN:
     case WM_SYSKEYDOWN:
      type=NSKeyDown;
      break;

     case WM_KEYUP:
     case WM_SYSKEYUP:
      type=NSKeyUp;
      break;

     case WM_MOUSEMOVE:
      [self _unhideCursorForMouseMove];
      
      if(msg.wParam&MK_LBUTTON)
       type=NSLeftMouseDragged;
      else if(msg.wParam&MK_RBUTTON)
       type=NSRightMouseDragged;
      else {
       ReleaseCapture();
       if(window!=nil && [window acceptsMouseMovedEvents]){
        type=NSMouseMoved;
       }
       else {
        return YES;
       }
      }
      break;

     case WM_LBUTTONDOWN:
     case WM_LBUTTONDBLCLK:
      type=NSLeftMouseDown;
      SetCapture([platformWindow windowHandle]);
      break;

     case WM_LBUTTONUP:
      type=NSLeftMouseUp;
      ReleaseCapture();
      break;

     case WM_RBUTTONDOWN:
     case WM_RBUTTONDBLCLK:
      type=NSRightMouseDown;
      break;

     case WM_RBUTTONUP:
      type=NSRightMouseUp;
      break;

     case WM_MOUSEWHEEL:
     case WM_MOUSEHWHEEL:
      type=NSScrollWheel;
      break;

     case WM_NCMOUSEMOVE:
     case WM_NCLBUTTONDOWN:
     case WM_NCLBUTTONUP:
     case WM_NCLBUTTONDBLCLK:
     case WM_NCRBUTTONDOWN:
     case WM_NCRBUTTONUP:
     case WM_NCRBUTTONDBLCLK:
     case WM_NCMBUTTONDOWN:
     case WM_NCMBUTTONUP:
     case WM_NCMBUTTONDBLCLK:
      {
       Win32Event *cgEvent=[Win32Event eventWithMSG:msg];
       NSEvent    *event=[[[NSEvent_CoreGraphics alloc] initWithCoreGraphicsEvent:cgEvent window:window] autorelease];
       [self postEvent:event atStart:NO];
      }
      return YES;

     default:
      return NO;
    }


    location.x=deviceLocation.x;
    location.y=deviceLocation.y;

    BOOL childWindow=NO;
    
    if(msg.message==WM_MOUSEWHEEL || msg.message==WM_MOUSEHWHEEL) {
            // WM_MOUSEWHEEL coordinates are on screen coordinates, others are in window
            RECT frame={0};
            
            GetWindowRect([platformWindow windowHandle],&frame);
            
            location.x=location.x-frame.left;
            location.y=location.y-frame.top;
    }
    else {
        childWindow=(msg.hwnd!=[platformWindow windowHandle]);
        
        if(childWindow){
            RECT child={0},parent={0};
            
            // There is no way to get a child's frame inside the parent, you have to get
            // them both in screen coordinates and do a delta
            // GetClientRect always returns 0,0 for top,left which makes it useless     
            GetWindowRect(msg.hwnd,&child);
            GetWindowRect([platformWindow windowHandle],&parent);
            
            location.x+=child.left-parent.left;
            location.y+=child.top-parent.top;
        }
    }
    
    [platformWindow adjustEventLocation:&location childWindow:childWindow];
    
    modifierFlags=[self currentModifierFlagsWithKeyboardState:keyboardState];

    switch(type){
     case NSLeftMouseDown:
     case NSLeftMouseUp:
     case NSRightMouseDown:
     case NSRightMouseUp:
     case NSMouseMoved:
     case NSLeftMouseDragged:
     case NSRightMouseDragged:
     case NSMouseEntered:
     case NSMouseExited:
      return [self postMouseMSG:msg type:type location:location modifierFlags:modifierFlags window:window];

     case NSKeyDown:
     case NSKeyUp:
     case NSFlagsChanged:
      return [self postKeyboardMSG:msg type:type location:location modifierFlags:modifierFlags window:window keyboardState:keyboardState];

     case NSScrollWheel:
      return [self postScrollWheelMSG:msg type:type location:location modifierFlags:modifierFlags window:window];

     default:
      return NO;
    }

   return NO;
}

-(void)beep {
   MessageBeep(MB_OK);
}

static int CALLBACK buildFamily(const const EXTLOGFONTW* logFont,const TEXTMETRICW* metrics,DWORD fontType,LPARAM lParam){

    //   NEWTEXTMETRICEX *textMetric=(NEWTEXTMETRICEX *)textMetric_old;
   NSMutableSet *set=(NSMutableSet *)lParam;
//   NSString     *name=[NSString stringWithCString:logFont->elfFullName];
    if (logFont && logFont->elfLogFont.lfFaceName) {
        NSString    *name = [NSString stringWithFormat:@"%S", logFont->elfLogFont.lfFaceName];
        // Font name starting with "@" are rotated versions of the font, for vertical rendering
        // We don't want them - the are polluting our font list + they have the same PS name
        // as the normal ones, leading to confusion in our font picking algo
        if (name.length >= 1 && [name characterAtIndex:0] != '@') {
            [set addObject:name];
        }
    }
   return 1;
}

-(NSSet *)allFontFamilyNames {
   NSMutableSet *result=[[[NSMutableSet alloc] init] autorelease];
   HDC           dc=GetDC(NULL);
   LOGFONTW logFont = { 0 };
	
   logFont.lfCharSet=DEFAULT_CHARSET;
    
   if(!EnumFontFamiliesExW(dc,&logFont,buildFamily,(LPARAM)result,0))
        NSLog(@"EnumFontFamiliesExW failed %d",__LINE__);
    
   ReleaseDC(NULL,dc);
   return result;
}

static NSFontMetric *fontMetricWithLogicalAndMetric(const ENUMLOGFONTEX *logFont,
   const NEWTEXTMETRICEX *textMetric) {
   NSSize size=NSMakeSize(logFont->elfLogFont.lfWidth,logFont->elfLogFont.lfHeight);
   float  ascender=textMetric->ntmTm.tmAscent;
   float  descender=-((float)textMetric->ntmTm.tmDescent);

   return [[[NSFontMetric alloc]
       initWithSize:size 
           ascender:ascender
          descender:descender] autorelease];
}

static int CALLBACK buildTypeface(const LOGFONTA *lofFont_old,
   const TEXTMETRICA *textMetric_old,DWORD fontType,LPARAM lParam){
   NSMutableDictionary *result=(NSMutableDictionary *)lParam;
   LPENUMLOGFONTEX  logFont=(LPENUMLOGFONTEX)lofFont_old;
   NEWTEXTMETRICEX *textMetric=(NEWTEXTMETRICEX *)textMetric_old;
   NSString        *name=[NSString stringWithCString:(char *)(logFont->elfFullName)];
   NSString        *traitName=[NSString stringWithCString:(char *)logFont->elfStyle];
  // NSString       *encoding=[NSString stringWithCString:logFont->elfScript];
   NSFontTypeface  *typeface=[result objectForKey:name];

   if(typeface==nil){
    NSFontTraitMask traits=0;

    if(textMetric->ntmTm.ntmFlags&NTM_ITALIC)
     traits|=NSItalicFontMask;
    if(textMetric->ntmTm.ntmFlags&NTM_BOLD)
     traits|=NSBoldFontMask;

	   NSString *psName = [O2Font postscriptNameForNativeName:name];
	   NSString *displayName = [O2Font displayNameForPostscriptName:psName];
	   typeface=[[[NSFontTypeface alloc] initWithName:psName 
										  displayName:displayName 
											traitName:traitName 
											   traits:traits] autorelease];

    [result setObject:typeface forKey:name];
   }

   [typeface addMetric:fontMetricWithLogicalAndMetric(logFont,textMetric)];

   return 1;
}

-(NSArray *)fontTypefacesForFamilyName:(NSString *)name {
   NSMutableDictionary *result=[NSMutableDictionary dictionary];
   HDC             dc=GetDC(NULL);
   LOGFONT         logFont;

   logFont.lfCharSet=DEFAULT_CHARSET;
   logFont.lfPitchAndFamily=0;

   [name getCString:logFont.lfFaceName maxLength:LF_FACESIZE-1];

   if(!EnumFontFamiliesExA(dc,&logFont,buildTypeface,(LPARAM)result,0))
    NSLog(@"EnumFontFamiliesExA failed %d",__LINE__);

   ReleaseDC(NULL,dc);
   
   return [result allValues];
}

-(float)scrollerWidth {
   return GetSystemMetrics(SM_CXHTHUMB);
}

#define PTS2THOUSANDS(x) ((x/72.f) * 1000.f)
#define THOUSANDS2PTS(x) ((x / 1000.f) * 72.f)

-(int)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo {

    PAGESETUPDLG setup;

    setup.lStructSize=sizeof(PAGESETUPDLG);
    setup.hwndOwner=[(Win32Window *)[[NSApp mainWindow] platformWindow] windowHandle];
    setup.hDevMode=NULL;
    setup.hDevNames=NULL;
   setup.Flags=PSD_INTHOUSANDTHSOFINCHES;
   setup.ptPaperSize.x = PTS2THOUSANDS([printInfo paperSize].width);
   setup.ptPaperSize.y = PTS2THOUSANDS([printInfo paperSize].height);
	setup.rtMargin.top = PTS2THOUSANDS([printInfo topMargin]);
	setup.rtMargin.left = PTS2THOUSANDS([printInfo leftMargin]);
	setup.rtMargin.right = PTS2THOUSANDS([printInfo rightMargin]);
	setup.rtMargin.bottom = PTS2THOUSANDS([printInfo bottomMargin]);

   [self stopWaitCursor];
   int check = PageSetupDlg(&setup);
   [self startWaitCursor];
	if (check == 0) {
		return NSCancelButton;
	}
	else {
		NSSize size = NSMakeSize(THOUSANDS2PTS(setup.ptPaperSize.x),
								 THOUSANDS2PTS(setup.ptPaperSize.y));
		[printInfo setPaperSize: size];
		
		[printInfo setTopMargin: THOUSANDS2PTS(setup.rtMargin.top)];
		[printInfo setLeftMargin: THOUSANDS2PTS(setup.rtMargin.left)];
		[printInfo setRightMargin: THOUSANDS2PTS(setup.rtMargin.right)];
		[printInfo setBottomMargin: THOUSANDS2PTS(setup.rtMargin.bottom)];
        
        HANDLE devMode = setup.hDevMode;
        LPDEVMODE lpDevMode = (LPDEVMODE) GlobalLock(devMode);
        if (lpDevMode && lpDevMode->dmFields & DM_ORIENTATION) {
            if (lpDevMode->dmOrientation == DMORIENT_PORTRAIT) {
                [printInfo setOrientation:NSPortraitOrientation];
            } else {
                [printInfo setOrientation:NSLandscapeOrientation];
            }
        }
        GlobalUnlock(devMode);
	}
	return NSOKButton;
}

-(int)runModalPrintPanelWithPrintInfoDictionary:(NSMutableDictionary *)attributes {
   NSView             *view=[attributes objectForKey:@"_NSView"];
   PRINTDLG            printProperties;
   int                 check;

    PAGESETUPDLG setup;
    bzero(&setup, sizeof(setup));
    
    setup.lStructSize=sizeof(PAGESETUPDLG);
    
    // Get the printer defaults
    setup.Flags = PSD_RETURNDEFAULT;
    PageSetupDlg(&setup);
    
    // See http://support.microsoft.com/kb/193103/fr
    HANDLE devMode = setup.hDevMode;
    
    LPDEVMODE lpDevMode = (LPDEVMODE) GlobalLock(devMode);
    // Force the orientation if the printer supports it.
    if (lpDevMode->dmFields & DM_ORIENTATION) {
        if ([[attributes objectForKey:NSPrintOrientation] boolValue] == NSPortraitOrientation) {
            lpDevMode->dmOrientation = DMORIENT_PORTRAIT;
        } else {
            lpDevMode->dmOrientation = DMORIENT_LANDSCAPE;
        }
        lpDevMode->dmFields = DM_ORIENTATION;
    }

   printProperties.lStructSize=sizeof(PRINTDLG);
   printProperties.hwndOwner=[(Win32Window *)[[view window] platformWindow] windowHandle];
   printProperties.hDevMode = devMode;
   printProperties.hDevNames=NULL;
   printProperties.hDC=NULL;
   printProperties.Flags=PD_RETURNDC|PD_COLLATE;

   printProperties.nFromPage=[[attributes objectForKey:NSPrintFirstPage] intValue]; 
   printProperties.nToPage=[[attributes objectForKey:NSPrintLastPage] intValue]; 
   printProperties.nMinPage=[[attributes objectForKey:NSPrintFirstPage] intValue]; 
   printProperties.nMaxPage=[[attributes objectForKey:NSPrintLastPage] intValue];
   printProperties.nCopies=[[attributes objectForKey:NSPrintCopies] intValue]; 
   printProperties.hInstance=NULL; 
   printProperties.lCustData=0; 
   printProperties.lpfnPrintHook=NULL; 
   printProperties.lpfnSetupHook=NULL; 
   printProperties.lpPrintTemplateName=NULL; 
   printProperties.lpSetupTemplateName=NULL; 
   printProperties.hPrintTemplate=NULL; 
   printProperties.hSetupTemplate=NULL; 

   [self stopWaitCursor];
   check=PrintDlg(&printProperties);
   [self startWaitCursor];

    GlobalUnlock(devMode);

   if(check==0)
    return NSCancelButton;
   else {
    NSDictionary *auxiliaryInfo=[NSDictionary dictionaryWithObject:[attributes objectForKey:@"_title"] forKey:(id)kCGPDFContextTitle];
    O2Context_gdi *context=[[[O2Context_gdi alloc] initWithPrinterDC:printProperties.hDC auxiliaryInfo:auxiliaryInfo] autorelease];
    NSRect imageable;
    
    if([context getImageableRect:&imageable])
	   [attributes setObject:[NSValue valueWithRect:imageable] forKey:@"_imageableRect"];
     
	   [attributes setObject:context forKey:@"_KGContext"];
    
	   [attributes setObject:[NSValue valueWithSize:[context pointSize]] forKey:NSPrintPaperSize];
	   [attributes setObject:[NSNumber numberWithInt:printProperties.nFromPage] forKey:NSPrintFirstPage];
	   [attributes setObject:[NSNumber numberWithInt:printProperties.nToPage] forKey:NSPrintLastPage];

    // It seems Windows is drawing relatively to the imageable area, not the paper area, like Cocoa does - so translate the context
    // to make Cocotron happy
    O2AffineTransform translation = O2AffineTransformMakeTranslation(-imageable.origin.x, -imageable.origin.y);
    O2ContextConcatCTM(context, translation);
   }
     
   return NSOKButton;
}

-(int)savePanel:(NSSavePanel *)savePanel runModalForDirectory:(NSString *)directory file:(NSString *)file {
   return [savePanel _GetOpenFileName];
}

-(int)openPanel:(NSOpenPanel *)openPanel runModalForDirectory:(NSString *)directory file:(NSString *)file types:(NSArray *)types {
   if([openPanel canChooseDirectories])
    return [openPanel _SHBrowseForFolder:directory];
   else
    return [openPanel _GetOpenFileNameForTypes:types];
}

-(float)primaryScreenHeight {
   return GetSystemMetrics(SM_CYSCREEN);
}

-(NSPoint)mouseLocation {
   POINT   winPoint;
   NSPoint point;

   GetCursorPos(&winPoint);

   point.x=winPoint.x;
   point.y=winPoint.y;
   point.y=[self primaryScreenHeight]-point.y;

   return point;
}

@end
