//
//  X11Display.h
//  AppKit
//
//  Created by Johannes Fortmann on 13.10.08.
//  Copyright 2008 -. All rights reserved.
//  Copyright (C) 2022 Zoe Knox. All rights reserved.
//

#import <AppKit/NSDisplay.h>
#import <wayland-client.h>

typedef enum {
    WLPointerPrimaryButton = 0x1,
    WLPointerMiddleButton = 0x2,
    WLPointerSecondaryButton = 0x4
} WLPointerButtonMask;

typedef enum {
    WLLeftShiftKeyMask = 0x1,
    WLLeftControlKeyMask = 0x2,
    WLLeftAltKeyMask = 0x4,
    WLLeftCommandKeyMask = 0x8,
    WLRightShiftKeyMask = 0x10,
    WLRightControlKeyMask = 0x20,
    WLRightAltKeyMask = 0x40,
    WLRightCommandKeyMask = 0x80
} WLModifierKeyMask;

typedef enum {
    WLPointerDevice,
    WLKeyboardDevice,
    WLTouchDevice
} WLInputDevice;

@interface WLDisplay : NSDisplay {
    struct wl_display *_display;
    struct wl_seat *_seat;
    struct wl_pointer *_pointer;
    struct wl_keyboard *_keyboard;
    int _fileDescriptor;
    NSSelectInputSource *_inputSource;
    NSMutableDictionary *_windowsByID;

    id lastFocusedWindow;
    struct wl_surface *_pointerActiveSurface;
    struct wl_surface *_keyboardActiveSurface;
    NSPoint pointerPosition;
    WLPointerButtonMask pointerButtonState;
    WLModifierKeyMask modifierKeyState;
    NSTimeInterval _lastClickTimeStamp;
    int clickCount;

    struct xkb_state *xkb_state;
    struct xkb_state *xkb_state_unmodified;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
    int repeatRate;
    int repeatDelay;
    pthread_t repeatThread;
}

- (struct wl_display *)display;

- (void)setWindow:(id)window forID:(unsigned long)i;
- (void)setSeat:(struct wl_seat *)seat;
- (void)seatHasPointer:(BOOL)hasPointer;
- (void)setHasKeyboard:(BOOL)hasKeyboard;
- (void)enterSurface:(struct wl_surface *)surface device:(WLInputDevice)device;
- (void)leaveSurface:(struct wl_surface *)surface device:(WLInputDevice)device;
- (struct wl_surface *)pointerActiveSurface;
- (struct wl_surface *)keyboardActiveSurface;
- (BOOL)pointerButtonState:(WLPointerButtonMask)mask;

- (NSTimeInterval)lastClickTimeStamp;
- (void)setLastClickTimeStamp:(NSTimeInterval)now;
- (float)doubleClickInterval;
- (int)handleError:(void *)errorEvent;
@end

struct RepeatArgs {
    NSEvent *event;
    int rate;
    int delay;
    WLDisplay *display;
};

