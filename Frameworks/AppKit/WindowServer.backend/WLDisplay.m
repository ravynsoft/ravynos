/* Copyright (c) 2008 Johannes Fortmann
   Copyright (C) 2022 Zoe Knox <zoe@pixin.net>. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE. */


#import "WLDisplay.h"
#import "WLWindow.h"
#import <AppKit/NSScreen.h>
#import <AppKit/NSApplication.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSocket_bsd.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/KTFont_FT.h>
#import <AppKit/O2Font_FT.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSFontTypeface.h>
#import <AppKit/NSWindow.h>
#import <stdio.h>
#import <stdlib.h>
#import <fcntl.h>
#import <errno.h>
#import <unistd.h>
#import <poll.h>
#import <sys/types.h>
#import <sys/mman.h>
#import <sys/stat.h>
#import <fontconfig.h>
#import <dev/evdev/input-event-codes.h>
#import <xkbcommon/xkbcommon.h>
#import <X11/keysym.h>

const NSString *WLOutputNameKey = @"WLOutputName";
const NSString *WLOutputDescriptionKey = @"WLOutputDescription";
const NSString *WLOutputDimensionsKey = @"WLOutputDimensions";
const NSString *WLOutputSizeKey = @"WLOutputSize";
const NSString *WLOutputModesKey = @"WLOutputModes";
const NSString *WLOutputCurrentModeKey = @"WLOutputCurrentMode";
const NSString *WLOutputPositionKey = @"WLOutputPosition";
const NSString *WLOutputTransformKey = @"WLOutputTransform";
const NSString *WLOutputScaleKey = @"WLOutputScale";
const NSString *WLOutputManufacturerKey = @"WLOutputManufacturer";
const NSString *WLOutputModelKey = @"WLOutputModel";
const NSString *WLOutputXDGOutputKey = @"WLOutputXDGOutput";

const NSString *WLModeSizeKey = @"WLModeSize";
const NSString *WLModeRefreshKey = @"WLModeRefresh";

NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";
NSString *WLOutputDidMoveNotification = @"WLOutputDidMoveNotification";

@implementation NSDisplay(WL)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([WLDisplay class],0,NULL);
}

@end

@implementation WLDisplay

static WLDisplay *__WLDisplay = nil;

static int errorHandler(struct wl_display *display,void *errorEvent) {
   return [(WLDisplay*)[WLDisplay currentDisplay] handleError:errorEvent];
}

static void handlePointerEnter(void *data, struct wl_pointer *ptr,
    uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) {
    WLDisplay *display = (WLDisplay *)data;
    [display enterSurface:surface device:WLPointerDevice];
}

static void handlePointerLeave(void *data, struct wl_pointer *ptr,
    uint32_t serial, struct wl_surface *surface) {
    WLDisplay *display = (WLDisplay *)data;
    [display leaveSurface:surface device:WLPointerDevice];
}

static void handlePointerMotion(void *data, struct wl_pointer *ptr,
    uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    WLDisplay *display = (WLDisplay *)data;
    [display pointerMotion:time at:NSMakePoint(wl_fixed_to_double(sx), wl_fixed_to_double(sy))];
}

- (void) pointerMotion:(uint32_t)time at:(NSPoint)point
{
    if(_pointerActiveSurface == NULL) {
        NSLog(@"ERROR: motion event without active surface");
        return;
    }
    WLWindow *window = [self windowForID:(unsigned long)_pointerActiveSurface];
    id delegate = [window delegate];

    NSRect bounds = CGOutsetRectForNativeWindowBorder([window frame], [window styleMask]);
    pointerPosition = NSMakePoint(point.x, bounds.size.height - point.y);
    NSPoint pos = [window transformPoint:pointerPosition];
    NSEventType type = NSMouseMoved;
         
    if([self pointerButtonState:WLPointerPrimaryButton] == YES)
        type = NSLeftMouseDragged;
    else if([self pointerButtonState:WLPointerSecondaryButton] == YES)
        type = NSRightMouseDragged;
         
    if(type == NSMouseMoved && ![delegate acceptsMouseMovedEvents])
       return; 

    NSEvent *event = [NSEvent mouseEventWithType:type
                                  location:pos
                             modifierFlags:[self modifierFlagsForState:xkb_state]
                                    window:delegate
                                clickCount:1 deltaX:0.0 deltaY:0.0];
    [self postEvent:event atStart:NO];
    [self discardEventsMatchingMask:NSLeftMouseDraggedMask beforeEvent:event];
}

static void handlePointerButton(void *data, struct wl_pointer *ptr,
    uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    WLDisplay *display = (WLDisplay *)data;
    [display pointerButton:button time:time state:state serial:serial];
}

- (void) pointerButton:(uint32_t)button time:(uint32_t)time state:(uint32_t)state
    serial:(uint32_t)serial
{
    NSEventType type;
    switch(button) {
        case BTN_LEFT: type = (state == WL_POINTER_BUTTON_STATE_RELEASED) ?
                NSLeftMouseUp : NSLeftMouseDown; break;
        case BTN_RIGHT: type = (state == WL_POINTER_BUTTON_STATE_RELEASED) ?
                NSRightMouseUp : NSRightMouseDown; break;
        default: NSLog(@"ignored button %d event", button); return;
    }
    
    NSTimeInterval now = [[NSDate date] timeIntervalSinceReferenceDate];
    if(now - _lastClickTimeStamp < [self doubleClickInterval])
        clickCount++;
    else
        clickCount = 1;
    _lastClickTimeStamp = now;

    // see if we are clicking a surface (window) or on the background
    WLWindow *window = [self windowForID:(unsigned long)_pointerActiveSurface];
    NSPoint pos = [window transformPoint:pointerPosition];
    id delegate = [window delegate];

    switch(type) {
        case NSLeftMouseUp:
            pointerButtonState &= ~WLPointerPrimaryButton;
            break;
        case NSLeftMouseDown:
            pointerButtonState |= WLPointerPrimaryButton;
            break;
        case NSRightMouseUp:
            pointerButtonState &= ~WLPointerSecondaryButton;
            break;
        case NSRightMouseDown:
            pointerButtonState |= WLPointerSecondaryButton;
            break;
    }

    NSEvent *event = [NSEvent mouseEventWithType:type
                                  location:pos
                             modifierFlags:[self modifierFlagsForState:xkb_state]
                                    window:delegate
                                clickCount:clickCount deltaX:0.0 deltaY:0.0];
    [event setSerialNumber:serial];
    [self postEvent:event atStart:NO];
}

static void handlePointerAxis(void *data, struct wl_pointer *ptr, uint32_t time,
    uint32_t axis, wl_fixed_t value) {
}

static void handlePointerAxisSource(void *data, struct wl_pointer *ptr, uint32_t source) {
}

static void handlePointerAxisStop(void *data, struct wl_pointer *ptr, uint32_t time, uint32_t axis) {
}

static void handlePointerAxisDiscrete(void *data, struct wl_pointer *ptr, uint32_t axis, int32_t discrete) {
}

static void handlePointerFrame(void *data, struct wl_pointer *ptr) {
}

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = handlePointerEnter,
    .leave = handlePointerLeave,
    .motion = handlePointerMotion,
    .button = handlePointerButton,
    .axis = handlePointerAxis,
    .frame = handlePointerFrame,
    .axis_source = handlePointerAxisSource,
    .axis_stop = handlePointerAxisStop,
    .axis_discrete = handlePointerAxisDiscrete,
};

static void handleKeyboardMap(void *data, struct wl_keyboard *kbd,
    uint32_t format, int32_t fd, uint32_t size) {
    WLDisplay *display = (WLDisplay *)data;
    if(format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        NSLog(@"ERROR: unsupported keymap format from WindowServer");
        return;
    }

    [display setKeymapWithFD:fd size:size];
    close(fd);
}

- (void)setKeymapWithFD:(int32_t)fd size:(uint32_t)size
{
    char *map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(xkb_context,
        map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map, size);

    struct xkb_state *state = xkb_state_new(xkb_keymap);
    struct xkb_state *unmodified = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(xkb_keymap);
    if(xkb_state_unmodified)
        xkb_state_unref(xkb_state_unmodified);
    if(xkb_state)
        xkb_state_unref(xkb_state);
    xkb_state = state;
    xkb_state_unmodified = unmodified;
}

- (struct xkb_state *)xkb_state
{
    return xkb_state;
}

static unichar translateKeySym(xkb_keysym_t keysym)
{
     switch(keysym) {
        case XK_Home:
        case XK_KP_Home: return NSHomeFunctionKey;
        case XK_Left:
        case XK_KP_Left: return NSLeftArrowFunctionKey;
        case XK_Up:
        case XK_KP_Up: return NSUpArrowFunctionKey;
        case XK_Right:
        case XK_KP_Right: return NSRightArrowFunctionKey;
        case XK_Down:
        case XK_KP_Down: return NSDownArrowFunctionKey;
        case XK_Page_Up:
        case XK_KP_Page_Up: return NSPageUpFunctionKey;
        case XK_Page_Down:
        case XK_KP_Page_Down: return NSPageDownFunctionKey;
        case XK_End:
        case XK_KP_End: return NSEndFunctionKey;
        case XK_Begin:
        case XK_KP_Begin: return NSHomeFunctionKey;
        case XK_Delete:
        case XK_KP_Delete: return NSDeleteFunctionKey;
        case XK_Insert:
        case XK_KP_Insert: return NSInsertFunctionKey;
        case XK_F1: return NSF1FunctionKey;
        case XK_F2: return NSF2FunctionKey;
        case XK_F3: return NSF3FunctionKey;
        case XK_F4: return NSF4FunctionKey;
        case XK_F5: return NSF5FunctionKey;
        case XK_F6: return NSF6FunctionKey;
        case XK_F7: return NSF7FunctionKey;
        case XK_F8: return NSF8FunctionKey;
        case XK_F9: return NSF9FunctionKey;
        case XK_F10: return NSF10FunctionKey;
        case XK_F11: return NSF11FunctionKey;
        case XK_F12: return NSF12FunctionKey;
        case XK_F13: return NSF13FunctionKey;
        case XK_F14: return NSF14FunctionKey;
        case XK_F15: return NSF15FunctionKey;
        case XK_F16: return NSF16FunctionKey;
        case XK_F17: return NSF17FunctionKey;
        case XK_F18: return NSF18FunctionKey;
        case XK_F19: return NSF19FunctionKey;
        case XK_F20: return NSF20FunctionKey;
        case XK_F21: return NSF21FunctionKey;
        case XK_F22: return NSF22FunctionKey;
        case XK_F23: return NSF23FunctionKey;
        case XK_F24: return NSF24FunctionKey;
        case XK_F25: return NSF25FunctionKey;
        case XK_F26: return NSF26FunctionKey;
        case XK_F27: return NSF27FunctionKey;
        case XK_F28: return NSF28FunctionKey;
        case XK_F29: return NSF29FunctionKey;
        case XK_F30: return NSF30FunctionKey;
        case XK_F31: return NSF31FunctionKey;
        case XK_F32: return NSF32FunctionKey;
        case XK_F33: return NSF33FunctionKey;
        case XK_F34: return NSF34FunctionKey;
        case XK_F35: return NSF35FunctionKey;
        default: return keysym;
    }
}

-(void)startKeyRepeat:(NSTimer *)timer {
    [self postEvent:repeatEvent atStart:NO];
}

- (void)keyboardInput:(uint32_t)keycode eventType:(NSEventType)type autoUp:(BOOL)autoUp
{
    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, keycode);
    if(sym == XKB_KEY_NoSymbol)
        return;

    unichar nskey = translateKeySym(sym);
    NSString *strChars, *strCharsIg;

    if(nskey == sym) { // we did not translate, look up the utf8
        char buf[128];
        xkb_state_key_get_utf8(xkb_state, keycode, buf, sizeof(buf));
        strChars = [NSString stringWithUTF8String:buf];
        xkb_state_key_get_utf8(xkb_state_unmodified, keycode, buf, sizeof(buf));
        strCharsIg = [NSString stringWithUTF8String:buf];
    } else {
        strChars = [NSString stringWithCharacters:&nskey length:1];
        strCharsIg = strChars;
    }

    WLWindow *window = [self windowForID:(unsigned long)_keyboardActiveSurface];
    id delegate = [window delegate];
    
    if(repeatEvent != nil) {
        [repeatEvent release];
        repeatEvent = nil;
    }
    NSEvent *event = [NSEvent keyEventWithType:type
                                      location:pointerPosition
                                 modifierFlags:[self modifierFlagsForState:xkb_state]
                                     timestamp:0.0
                                  windowNumber:(NSInteger)[delegate windowNumber]
                                       context:nil
                                    characters:strChars
                   charactersIgnoringModifiers:strCharsIg
                                     isARepeat:NO
                                       keyCode:keycode];
    [self postEvent:event atStart:NO];

    if(type == NSKeyDown) {
        if(autoUp == YES) {
            event = [NSEvent keyEventWithType:NSKeyUp
                                      location:pointerPosition
                                 modifierFlags:[self modifierFlagsForState:xkb_state]
                                     timestamp:0.0
                                  windowNumber:(NSInteger)[delegate windowNumber]
                                       context:nil
                                    characters:strChars
                   charactersIgnoringModifiers:strCharsIg
                                     isARepeat:NO
                                       keyCode:keycode];
            [self postEvent:event atStart:NO];
        } else {
            // set auto-repeat delay timer
            // FIXME: do not repeat modifier keys like Shift without a non-mod key
            repeatEvent = [[NSEvent keyEventWithType:NSKeyDown
                                      location:[event locationInWindow]
                                 modifierFlags:[event modifierFlags]
                                     timestamp:0.0
                                  windowNumber:[event windowNumber]
                                       context:nil
                                    characters:[event characters]
                   charactersIgnoringModifiers:[event charactersIgnoringModifiers]
                                     isARepeat:YES
                                       keyCode:[event keyCode]] retain];
            [NSTimer scheduledTimerWithInterval:(float)repeatDelay/1000 target:self
                selector:@selector(startKeyRepeat:) userInfo:nil repeats:NO];
        }
    }
}

static void handleKeyboardEnter(void *data, struct wl_keyboard *kbd,
    uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    WLDisplay *display = (WLDisplay *)data;
    [display enterSurface:surface device:WLKeyboardDevice];

    uint32_t *key;
    wl_array_for_each(key, keys) {
        [display keyboardInput:*key+8 eventType:NSKeyDown autoUp:YES];
    }
}

static void handleKeyboardLeave(void *data, struct wl_keyboard *kbd,
    uint32_t serial, struct wl_surface *surface) {
    WLDisplay *display = (WLDisplay *)data;
    [display leaveSurface:surface device:WLKeyboardDevice];
}

static void handleKeyboardInput(void *data, struct wl_keyboard *kbd,
    uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    WLDisplay *display = (WLDisplay *)data;
    NSEventType type = (state == WL_KEYBOARD_KEY_STATE_PRESSED) ? NSKeyDown : NSKeyUp;
    [display keyboardInput:key+8 eventType:type autoUp:NO];
}

static void handleKeyboardModifiers(void *data, struct wl_keyboard *kbd,
    uint32_t serial, uint32_t modsDown, uint32_t modsLatched, uint32_t modsLocked,
    uint32_t group) {
    WLDisplay *display = (WLDisplay *)data;
    xkb_state_update_mask([display xkb_state], 
        modsDown, modsLatched, modsLocked, 0, 0, group);
}

- (void)setKeyboardRepeatRate:(int)rate withDelay:(int)delay
{
    repeatRate = rate;
    repeatDelay = delay;
}

static void handleKeyboardRepeat(void *data, struct wl_keyboard *kbd, int32_t rate,
    int32_t delay) {
    WLDisplay *display = (WLDisplay *)data;
    [display setKeyboardRepeatRate:rate withDelay:delay];
}


static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = handleKeyboardMap,
    .enter = handleKeyboardEnter,
    .leave = handleKeyboardLeave,
    .key = handleKeyboardInput,
    .modifiers = handleKeyboardModifiers,
    .repeat_info = handleKeyboardRepeat,
};

static void handle_seat_capabilities(void *data, struct wl_seat *seat, uint32_t caps) {
    WLDisplay *display = (WLDisplay *)data;
    [display seatHasPointer:(caps & WL_SEAT_CAPABILITY_POINTER) ? YES: NO];
    [display seatHasKeyboard:(caps & WL_SEAT_CAPABILITY_KEYBOARD) ? YES : NO];
}

static void handle_seat_name(void *data, struct wl_seat *seat, const char *name) {
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = handle_seat_capabilities,
    .name = handle_seat_name,
};


static void handleWOMDone(void *data, struct wl_output *output) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:@"YES" forKey:@"WOMDone"];
}

static void handleWOMGeometry(void *data, struct wl_output *output, int32_t x, int32_t y,
    int32_t width, int32_t height, int32_t subpixel, const char *make, const char *model,
    int32_t transform) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:[NSString stringWithUTF8String:make] forKey:WLOutputManufacturerKey];
    [dict setObject:[NSString stringWithUTF8String:model] forKey:WLOutputModelKey];
    [dict setObject:[NSNumber numberWithInt:transform] forKey:WLOutputTransformKey];
    [dict setObject:NSStringFromPoint(NSMakePoint(x, y)) forKey:WLOutputPositionKey];
    [dict setObject:NSStringFromSize(NSMakeSize(width, height)) forKey:WLOutputDimensionsKey];
}

static void handleWOMScale(void *data, struct wl_output *output, int32_t scale) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:[NSNumber numberWithInt:scale] forKey:WLOutputScaleKey];
}

static void handleWOMMode(void *data, struct wl_output *output, uint32_t flags, int32_t width,
    int32_t height, int32_t refresh) {
    // refresh is in milliHertz...
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    NSMutableDictionary *mode = [NSMutableDictionary new];
    [mode setObject:NSStringFromSize(NSMakeSize(width, height)) forKey:WLModeSizeKey];
    [mode setObject:[NSNumber numberWithFloat:(float)refresh/1000] forKey:WLModeRefreshKey];
    NSMutableDictionary *modes = [dict objectForKey:WLOutputModesKey];
    if(modes == nil) {
        modes = [NSMutableDictionary new];
        [dict setObject:modes forKey:WLOutputModesKey];
    }
    [modes setObject:mode forKey:[NSString stringWithFormat:@"%dx%d@%d", width, height, refresh/1000]];
    if(flags & WL_OUTPUT_MODE_CURRENT)
        [dict setObject:mode forKey:WLOutputCurrentModeKey];
}

static const struct wl_output_listener wl_output_listener = {
    .done = handleWOMDone,
    .geometry = handleWOMGeometry,
    .mode = handleWOMMode,
    .scale = handleWOMScale,
};

static void handleOMName(void *data, struct zxdg_output_v1 *head, const char *s) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:[NSString stringWithUTF8String:s] forKey:WLOutputNameKey];
}

static void handleOMDescription(void *data, struct zxdg_output_v1 *head, const char *s) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:[NSString stringWithUTF8String:s] forKey:WLOutputDescriptionKey];
}

static void handleOMSize(void *data, struct zxdg_output_v1 *head, int32_t w, int32_t h) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:NSStringFromSize(NSMakeSize(w, h)) forKey:WLOutputSizeKey];
    [[NSNotificationCenter defaultCenter]
        postNotificationName:WLOutputDidResizeNotification object:nil userInfo:dict];
}

static void handleOMPosition(void *data, struct zxdg_output_v1 *head,
    int32_t x, int32_t y) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:NSStringFromPoint(NSMakePoint(x, y)) forKey:WLOutputPositionKey];
    [[NSNotificationCenter defaultCenter]
        postNotificationName:WLOutputDidMoveNotification object:nil userInfo:dict];
}

static void handleOMDone(void *data, struct zxdg_output_v1 *head) {
    NSMutableDictionary *dict = (NSMutableDictionary *)data;
    [dict setObject:@"YES" forKey:@"OMDone"];
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
    .name = handleOMName,
    .description = handleOMDescription,
    .logical_size = handleOMSize,
    .logical_position = handleOMPosition,
    .done = handleOMDone,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
    WLDisplay *display = (WLDisplay *)data;

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        [display set_compositor:wl_registry_bind(registry, name, &wl_compositor_interface, 1)];
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        struct wl_seat *seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
        [display setSeat:seat];
    } else if(strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
        struct zxdg_output_manager_v1 *manager = wl_registry_bind(registry, name,
            &zxdg_output_manager_v1_interface, 1);
        [display setOutputManager:manager];
    } else if(strcmp(interface, wl_output_interface.name) == 0) {
        [display addWaylandOutput:wl_registry_bind(registry, name, &wl_output_interface, 3)];
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// FIXME: handle output removal
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

-init {
    if(__WLDisplay != nil)
        return __WLDisplay;

    if(self = [super init]) {
        __WLDisplay = [self retain];

        if(getenv("XDG_RUNTIME_DIR") == NULL) {
            char *buf = 0;
            asprintf(&buf, "/tmp/runtime.%u", getuid());
            setenv("XDG_RUNTIME_DIR", buf, 0);
            if(access(buf, R_OK|W_OK|X_OK) != 0) {
                if(errno == ENOENT)
                    mkdir(buf, 0700);
            }
            free(buf);
        }

        _display = wl_display_connect(NULL);
        if(_display == NULL) {
            [self dealloc];
            __WLDisplay = nil;
            return nil;
        }
        _seat = NULL;
        _pointer = NULL;
        xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

        // make sure we have a default keymap in case compositor doesn't send one
        // FIXME: read this from system prefs
        struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_names(xkb_context,
            NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
        xkb_state = xkb_state_new(xkb_keymap);
        xkb_state_unmodified = xkb_state_new(xkb_keymap);
        xkb_keymap_unref(xkb_keymap);
        
        _fileDescriptor = wl_display_get_fd(_display);
        _inputSource = [[NSSelectInputSource socketInputSourceWithSocket:
            [NSSocket_bsd socketWithDescriptor:_fileDescriptor]] retain];
        [_inputSource setDelegate:self];
        [_inputSource setSelectEventMask:NSSelectReadEvent];
      
        _windowsByID = [NSMutableDictionary new];
        _outputs = [NSMutableDictionary new];
        _outputManager = NULL;

        lastFocusedWindow = nil;
        _pointerActiveSurface = NULL;
        _keyboardActiveSurface = NULL;
        _lastClickTimeStamp = 0.0;
        clickCount = 0;
        repeatEvent = nil;

        registry = wl_display_get_registry(_display);
        wl_registry_add_listener(registry, &registry_listener, (__bridge void *)self);
        wl_display_roundtrip(_display);

        if(compositor == NULL) {
            NSLog(@"WLDisplay: compositor not available");
            return nil;
        }
    }
    return self;
}

-(void)dealloc {
    if(_display)
        wl_display_disconnect(_display);
    if(_outputManager)
        zxdg_output_manager_v1_destroy(_outputManager);
    if(registry)
        wl_registry_destroy(registry);
    
    [_windowsByID release];
    [super dealloc];
}

-(struct wl_output *)wlOutputWithXDGKey:(NSNumber *)key {
    if(key == nil)
        return NULL;

    NSArray *keys = [_outputs allKeys];
    NSArray *vals = [_outputs allValues];

    for(int i = 0; i < [keys count]; ++i) {
        NSDictionary *d = [vals objectAtIndex:i];
        if([[d objectForKey:WLOutputXDGOutputKey] isEqualTo:key])
            return (struct wl_output *)[[keys objectAtIndex:i] longValue];
    }
    return NULL;
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType screen:(NSScreen *)screen {
        struct wl_output *output = [self wlOutputWithXDGKey:[screen key]]; 
	return [[[WLWindow alloc] initWithFrame:frame styleMask:styleMask isPanel:NO backingType:backingType output:output] autorelease];
}

-(CGWindow *)windowWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
	return [[[WLWindow alloc] initWithFrame:frame styleMask:styleMask isPanel:NO backingType:backingType output:NULL] autorelease];
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType screen:(NSScreen *)screen {
        struct wl_output *output = [self wlOutputWithXDGKey:[screen key]]; 
	return [[[WLWindow alloc] initWithFrame:frame styleMask:styleMask isPanel:YES backingType:backingType output:output] autorelease];
}

-(CGWindow *)panelWithFrame:(NSRect)frame styleMask:(unsigned)styleMask backingType:(unsigned)backingType {
	return [[[WLWindow alloc] initWithFrame:frame styleMask:styleMask isPanel:YES backingType:backingType output:NULL] autorelease];
}

-(struct wl_display *)display {
   return _display;
}

-(NSArray *)screens {
    NSArray *keys = [_outputs allKeys];
    int numHeads = [keys count];

    // if we haven't been told how many heads we have yet, fake it to avoid a crash
    if(numHeads <= 0) {
        NSRect frame = NSMakeRect(0, 0, 1024, 768);
        return [NSArray arrayWithObject:[[[NSScreen alloc]
            initWithFrame:frame visibleFrame:frame] autorelease]];
    }

    NSMutableArray *scr = [NSMutableArray arrayWithCapacity:numHeads];

    for(int i = 0; i < numHeads; ++i) {
        NSString *key = [keys objectAtIndex:i];
        NSDictionary *d = [_outputs objectForKey:key];
        NSDictionary *mode = [d objectForKey:WLOutputCurrentModeKey];
        NSRect frame = NSMakeRect(0, 0, 0, 0);
        frame.size = NSSizeFromString([mode objectForKey:WLModeSizeKey]);
        frame.origin = NSPointFromString([d objectForKey:WLOutputPositionKey]);
        [scr addObject:[[[NSScreen alloc] initWithFrame:frame visibleFrame:frame
            outputKey:[d objectForKey:WLOutputXDGOutputKey]] autorelease]];
    }
    return [NSArray arrayWithArray:scr];
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
   //FIXME beep XBell(_display, 100);
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
#if 0 // FIXME: get this from WindowServer
    int rootX, rootY;

    BOOL result = XQueryPointer(_display, DefaultRootWindow(_display),
        &window, &window, &rootX, &rootY, &winX, &winY, &mask);
    if(result == YES) {
        // invert the Y since Cocoa's origin is lower left
        int height = DisplayHeight(_display, DefaultScreen(_display));
        return NSMakePoint(rootX, height - rootY);
    }
#endif
    NSLog(@"-[WLDisplay mouseLocation] unable to locate mouse pointer");
    return NSMakePoint(0,0);
}

-(void)setWindow:(id)window forID:(unsigned long)i
{
   if(window)
      [_windowsByID setObject:window forKey:[NSNumber numberWithUnsignedLong:(unsigned long)i]];
   else
      [_windowsByID removeObjectForKey:[NSNumber numberWithUnsignedLong:(unsigned long)i]];
}

-(void) set_compositor:(struct wl_compositor *)comp
{
    compositor = comp;
}

- (void)addWaylandOutput:(struct wl_output *)output
{
    NSMutableDictionary *dict = [NSMutableDictionary new];

    wl_output_add_listener(output, &wl_output_listener, (void *)dict);
    if(_outputManager) {
        struct zxdg_output_v1 *xdgOut = zxdg_output_manager_v1_get_xdg_output(_outputManager, output);
        zxdg_output_v1_add_listener(xdgOut, &xdg_output_listener, (void *)dict);
        [dict setObject:[NSNumber numberWithUnsignedLong:(uintptr_t)xdgOut] forKey:WLOutputXDGOutputKey];
    }

    [_outputs setObject:dict forKey:[NSNumber numberWithUnsignedLong:(uintptr_t)output]];
}

- (void)setOutputManager:(struct zxdg_output_manager_v1 *)manager
{
    _outputManager = manager;
    NSArray *keys = [_outputs allKeys];
    for(int i = 0; i < [keys count]; ++i) {
        NSDictionary *d = [_outputs objectForKey:[keys objectAtIndex:i]];
        if([d objectForKey:WLOutputXDGOutputKey] == nil) {
            struct wl_output *output = (struct wl_output *)[[keys objectAtIndex:i] longValue];
            struct zxdg_output_v1 *xdgOut = zxdg_output_manager_v1_get_xdg_output(_outputManager, output);
            zxdg_output_v1_add_listener(xdgOut, &xdg_output_listener, (void *)d);
            [d setObject:[NSNumber numberWithUnsignedLong:(uintptr_t)xdgOut] forKey:WLOutputXDGOutputKey];
        }
    }
}

- (void)setSeat:(struct wl_seat *)seat
{
    _seat = seat;
    wl_seat_add_listener(seat, &wl_seat_listener, (void *)self);
}

- (void)seatHasPointer:(BOOL)hasPointer
{
    if(_pointer)
        wl_pointer_release(_pointer);

    if(hasPointer) {
        _pointer = wl_seat_get_pointer(_seat);
        wl_pointer_add_listener(_pointer, &wl_pointer_listener, (void *)self);
    } else
        _pointer = NULL;
}

- (void)seatHasKeyboard:(BOOL)hasKeyboard
{
    if(_keyboard)
        wl_keyboard_release(_keyboard);

    if(hasKeyboard) {
        _keyboard = wl_seat_get_keyboard(_seat);
        wl_keyboard_add_listener(_keyboard, &wl_keyboard_listener, (void *)self);
    } else
        _keyboard = NULL;
}

- (struct wl_seat *)seat
{
    return _seat;
}

- (void)enterSurface:(struct wl_surface *)surface device:(WLInputDevice)device
{
    switch(device) {
        case WLPointerDevice:
            _pointerActiveSurface = surface;
            break;
        case WLKeyboardDevice:
            _keyboardActiveSurface = surface;
            break;
        case WLTouchDevice:
            NSLog(@"FIXME: touch devices are not supported yet");
            break;
    }
}

- (void)leaveSurface:(struct wl_surface *)surface device:(WLInputDevice)device
{
    switch(device) {
        case WLPointerDevice:
            if(_pointerActiveSurface == surface)
                _pointerActiveSurface = NULL;
            break;
        case WLKeyboardDevice:
            if(_keyboardActiveSurface == surface)
                _keyboardActiveSurface = NULL;
            break;
        case WLTouchDevice:
            NSLog(@"FIXME: touch devices are not supported yet");
            break;
    }
}

- (struct wl_surface *)pointerActiveSurface
{
    return _pointerActiveSurface;
}

- (NSTimeInterval)lastClickTimeStamp
{
    return _lastClickTimeStamp;
}

- (void)setLastClickTimeStamp:(NSTimeInterval)now
{
    _lastClickTimeStamp = now;
}

- (BOOL)pointerButtonState:(WLPointerButtonMask)mask
{
    if((pointerButtonState & mask) == mask)
        return YES;
    return NO;
}

-(id)windowForID:(unsigned long)i
{
   return [_windowsByID objectForKey:[NSNumber numberWithUnsignedLong:i]];
}

-(NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue {
    NSEvent *result;
    NSRunLoop *loop = [NSRunLoop currentRunLoop];

    if(repeatEvent != nil) {
        int charDelay = ((float)1.0/repeatRate) * 1000000;
        [self postEvent:repeatEvent atStart:NO];
        usleep(charDelay);
    } 

    [loop addInputSource:_inputSource forMode:mode];
    result = [super nextEventMatchingMask:mask untilDate:untilDate inMode:mode dequeue:dequeue];
    [loop removeInputSource:_inputSource forMode:mode]; 
    return result;
}

-(unsigned int)modifierFlagsForState:(struct xkb_state *)state {
    unsigned int ret=0;
    if(xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE))
        ret |= NSShiftKeyMask;
    if(xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE))
        ret |= NSControlKeyMask;
    if(xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO, XKB_STATE_MODS_EFFECTIVE))
        ret |= NSCommandKeyMask;
    if(xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE))
        ret |= NSAlternateKeyMask;
    return ret;
}

NSArray *CGSOrderedWindowNumbers() {
    NSMutableArray *result = [NSMutableArray array];
    
    for (NSWindow* win in [NSApp windows]) [result addObject:[NSNumber numberWithInteger:[win windowNumber]]];
    
    NSUnimplementedFunction(); //(Window numbers not even remotely ordered)
    
    return result;
}

#if 0 // FIXME: this belongs in compositor?
-(void)postXEvent:(XEvent *)ev {
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
         
    case Expose:;
     O2Rect rect=NSMakeRect(ev->xexpose.x, ev->xexpose.y, ev->xexpose.width, ev->xexpose.height);
     
     rect.origin.y=[window frame].size.height-rect.origin.y-rect.size.height;
     // rect=NSInsetRect(rect, -10, -10);
     // [_backingContext addToDirtyRect:rect];
     if(ev->xexpose.count==0)
      [window flushBuffer];
     break;


    case DestroyNotify:;
     // we should never get this message before the WM_DELETE_WINDOW ClientNotify
     // so normally, window should be nil here.
     [window invalidate];
     break;

    case ConfigureNotify:
     [window frameChanged];
     [delegate platformWindow:window frameChanged:[window transformFrame:[window frame]] didSize:YES];
     break;


    case ClientMessage:
     if(ev->xclient.format=32 && ev->xclient.data.l[0]==XInternAtom(_display, "WM_DELETE_WINDOW", False))
      [delegate platformWindowWillClose:window];
     break;
#endif

-(void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent {
    while(wl_display_prepare_read(_display) != 0)
        wl_display_dispatch_pending(_display);
    wl_display_flush(_display);
    wl_display_read_events(_display);
    wl_display_dispatch_pending(_display);
    //[self postXEvent:&e];
}

-(int)handleError:(void*)errorEvent {
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
