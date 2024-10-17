/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <mach/mach.h>
#import "common.h"
#import "message.h"
#import "WSInput.h"

double clipTo(double val, double min, double max) {
    if(val < min) val = min;
    else if(val > max) val = max;
    return val;
}

static unichar translateKeySym(xkb_keysym_t keysym) {
     switch(keysym) {
        case XKB_KEY_Home:
        case XKB_KEY_KP_Home: return NSHomeFunctionKey;
        case XKB_KEY_Left:
        case XKB_KEY_KP_Left: return NSLeftArrowFunctionKey;
        case XKB_KEY_Up:
        case XKB_KEY_KP_Up: return NSUpArrowFunctionKey;
        case XKB_KEY_Right:
        case XKB_KEY_KP_Right: return NSRightArrowFunctionKey;
        case XKB_KEY_Down:
        case XKB_KEY_KP_Down: return NSDownArrowFunctionKey;
        case XKB_KEY_Page_Up:
        case XKB_KEY_KP_Page_Up: return NSPageUpFunctionKey;
        case XKB_KEY_Page_Down:
        case XKB_KEY_KP_Page_Down: return NSPageDownFunctionKey;
        case XKB_KEY_End:
        case XKB_KEY_KP_End: return NSEndFunctionKey;
        case XKB_KEY_Begin:
        case XKB_KEY_KP_Begin: return NSHomeFunctionKey;
        case XKB_KEY_Delete:
        case XKB_KEY_KP_Delete: return NSDeleteFunctionKey;
        case XKB_KEY_Insert:
        case XKB_KEY_KP_Insert: return NSInsertFunctionKey;
        case XKB_KEY_F1: return NSF1FunctionKey;
        case XKB_KEY_F2: return NSF2FunctionKey;
        case XKB_KEY_F3: return NSF3FunctionKey;
        case XKB_KEY_F4: return NSF4FunctionKey;
        case XKB_KEY_F5: return NSF5FunctionKey;
        case XKB_KEY_F6: return NSF6FunctionKey;
        case XKB_KEY_F7: return NSF7FunctionKey;
        case XKB_KEY_F8: return NSF8FunctionKey;
        case XKB_KEY_F9: return NSF9FunctionKey;
        case XKB_KEY_F10: return NSF10FunctionKey;
        case XKB_KEY_F11: return NSF11FunctionKey;
        case XKB_KEY_F12: return NSF12FunctionKey;
        case XKB_KEY_F13: return NSF13FunctionKey;
        case XKB_KEY_F14: return NSF14FunctionKey;
        case XKB_KEY_F15: return NSF15FunctionKey;
        case XKB_KEY_F16: return NSF16FunctionKey;
        case XKB_KEY_F17: return NSF17FunctionKey;
        case XKB_KEY_F18: return NSF18FunctionKey;
        case XKB_KEY_F19: return NSF19FunctionKey;
        case XKB_KEY_F20: return NSF20FunctionKey;
        case XKB_KEY_F21: return NSF21FunctionKey;
        case XKB_KEY_F22: return NSF22FunctionKey;
        case XKB_KEY_F23: return NSF23FunctionKey;
        case XKB_KEY_F24: return NSF24FunctionKey;
        case XKB_KEY_F25: return NSF25FunctionKey;
        case XKB_KEY_F26: return NSF26FunctionKey;
        case XKB_KEY_F27: return NSF27FunctionKey;
        case XKB_KEY_F28: return NSF28FunctionKey;
        case XKB_KEY_F29: return NSF29FunctionKey;
        case XKB_KEY_F30: return NSF30FunctionKey;
        case XKB_KEY_F31: return NSF31FunctionKey;
        case XKB_KEY_F32: return NSF32FunctionKey;
        case XKB_KEY_F33: return NSF33FunctionKey;
        case XKB_KEY_F34: return NSF34FunctionKey;
        case XKB_KEY_F35: return NSF35FunctionKey;
        default: return keysym;
    }
}

@implementation WSInput

-init {
    udev = udev_new();
    li = libinput_udev_create_context(&interface, NULL, udev);
    libinput_udev_assign_seat(li, "seat0");

    xkbCtx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkb_state = xkb_state_unmodified = NULL;
    xkb_keymap = NULL;
    [self setKeymap];

    return self;
}

-(void)dealloc {
    xkb_keymap_unref(xkb_keymap);
    xkb_state_unref(xkb_state);
    xkb_state_unref(xkb_state_unmodified);
    xkb_context_unref(xkbCtx);
    libinput_unref(li);
    udev_unref(udev);
}

-(void)run:(id)target {
    struct libinput_event *event = NULL;

    libinput_dispatch(li);
    while((event = libinput_get_event(li)) != NULL) {
        [self processEvent:event target:target];
        libinput_event_destroy(event);
        libinput_dispatch(li);
    }
}

-(void)setLogLevel:(int)level {
    logLevel = level;
    switch(level) {
        case WS_ERROR: level = LIBINPUT_LOG_PRIORITY_ERROR; break;
        case WS_WARNING: level = LIBINPUT_LOG_PRIORITY_INFO; break;
        case WS_INFO: level = LIBINPUT_LOG_PRIORITY_DEBUG; break;
    }
    libinput_log_set_priority(li, level);
}

// Call this whenever the cursor moves to a new screen
-(void)setGeometry:(NSRect)geom {
    geometry = geom;
}

/* event is destroyed after this function returns */
-(void)processEvent:(struct libinput_event *)event target:(NSObject *)target {
    enum libinput_event_type etype = libinput_event_get_type(event);
    if(logLevel >= WS_INFO)
        NSLog(@"input event: device %s type %d",
        libinput_device_get_name(libinput_event_get_device(event)), etype);
    
    struct mach_event me;
    memset(&me, 0, sizeof(struct mach_event));

    switch(etype) {
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            struct libinput_event_keyboard *ke = libinput_event_get_keyboard_event(event);
            uint32_t keycode = libinput_event_keyboard_get_key(ke);
            enum libinput_key_state state = libinput_event_keyboard_get_key_state(ke);
            if(logLevel >= WS_INFO)
                NSLog(@"Input event: type=KEY key=%u state=%u", keycode, state);

            keycode += 8; // evdev keycode offset
            xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, keycode);
            if(sym == XKB_KEY_NoSymbol)
                return;
            xkb_state_update_key(xkb_state, keycode, state == LIBINPUT_KEY_STATE_PRESSED
                    ? XKB_KEY_DOWN : XKB_KEY_UP);
            unichar nskey = translateKeySym(sym);

            me.code = (state == LIBINPUT_KEY_STATE_PRESSED ? NSKeyDown : NSKeyUp);
            me.keycode = keycode;
            me.mods = [self modifierFlagsForState:xkb_state];
            me.state = me.code;

            // Intercept keycodes reserved for WindowServer itself
            if((me.mods & NSShiftKeyMask) && nskey == 'Q') { // FIXME: remove
                [target signalQuit];
                return;
            }
            if((me.mods & NSCommandKeyMask) && sym == XKB_KEY_Tab) {
                if(me.code == NSKeyDown)
                    [target switchApp];
                return;
            }

            if(nskey == sym) { // we did not translate, look up the utf8
                char buf[8];
                xkb_state_key_get_utf8(xkb_state, keycode, buf, sizeof(buf));
                memcpy(me.chars, buf, sizeof(me.chars));
                xkb_state_key_get_utf8(xkb_state_unmodified, keycode, buf, sizeof(buf));
                memcpy(me.charsIg, buf, sizeof(me.charsIg));
            } else {
                me.chars[0] = nskey;
                me.charsIg[0] = nskey;
            }

            // FIXME: handle autorepeat
            me.repeat = 0;

            [target sendEventToApp:&me];
            break;
        }
        case LIBINPUT_EVENT_POINTER_MOTION: {
            if(buttonDown[0] == YES)
                me.code = NSLeftMouseDragged;
            else if(buttonDown[1] == YES)
                me.code = NSRightMouseDragged;
            else
                me.code = NSMouseMoved;
            struct libinput_event_pointer *pe = libinput_event_get_pointer_event(event);
            me.dx = libinput_event_pointer_get_dx(pe);
            me.dy = (-1) * libinput_event_pointer_get_dy(pe); // our origin is lower left
            me.mods = [self modifierFlagsForState:xkb_state];
            pointerX = clipTo(pointerX + me.dx, geometry.origin.x, geometry.size.width);
            pointerY = clipTo(pointerY + me.dy, geometry.origin.y, geometry.size.height);
            me.x = pointerX;
            me.y = pointerY;
            if(logLevel >= WS_INFO)
                NSLog(@"Input event: type=MOTION dx=%.2f dy=%.2f pos=%.2f,%.2f", me.dx, me.dy,
                        pointerX, pointerY);

            [target sendEventToApp:&me];
            break;
        }
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE: {
            if(buttonDown[0] == YES)
                me.code = NSLeftMouseDragged;
            else if(buttonDown[1] == YES)
                me.code = NSRightMouseDragged;
            else
                me.code = NSMouseMoved;
            struct libinput_event_pointer *pe = libinput_event_get_pointer_event(event);
            me.mods = [self modifierFlagsForState:xkb_state];
            me.x = libinput_event_pointer_get_absolute_x_transformed(pe, geometry.size.width);
            me.y = geometry.size.height -
                libinput_event_pointer_get_absolute_y_transformed(pe, geometry.size.height);
            pointerX = me.x;
            pointerY = me.y;
            if(logLevel >= WS_INFO)
                NSLog(@"Input event: type=MOTION x=%.2f y=%.2f", me.x, me.y);

            [target sendEventToApp:&me];
            break;
        }
        case LIBINPUT_EVENT_POINTER_BUTTON: {
            struct libinput_event_pointer *pe = libinput_event_get_pointer_event(event);
            uint32_t button = libinput_event_pointer_get_button(pe);
            uint32_t state = libinput_event_pointer_get_button_state(pe);
            if(logLevel >= WS_INFO)
                NSLog(@"Input event: type=BUTTON button=%u state=%u", button, state);
            if(button == BTN_LEFT)
                me.code = (state == 1) ? NSLeftMouseDown : NSLeftMouseUp;
            else if(button == BTN_RIGHT)
                me.code = (state == 1) ? NSRightMouseDown : NSRightMouseUp;
            else
                return;
            buttonDown[button == BTN_LEFT ? 0 : 1] = (BOOL)state;
            me.mods = [self modifierFlagsForState:xkb_state];
            me.x = pointerX;
            me.y = pointerY;

            [target sendEventToApp:&me];
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
        case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
        case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
            if(logLevel >= WS_INFO)
                NSLog(@"scroll event");
            break;
        case LIBINPUT_EVENT_NONE:
            return;

        case LIBINPUT_EVENT_DEVICE_ADDED:
        case LIBINPUT_EVENT_DEVICE_REMOVED:
        case LIBINPUT_EVENT_TOUCH_DOWN:
        case LIBINPUT_EVENT_TOUCH_UP:
        case LIBINPUT_EVENT_TOUCH_MOTION:
        case LIBINPUT_EVENT_TOUCH_CANCEL:
        case LIBINPUT_EVENT_TOUCH_FRAME:
            if(logLevel >= WS_INFO)
                NSLog(@"touch event");
            break;

        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
        case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
        case LIBINPUT_EVENT_GESTURE_HOLD_END:
            if(logLevel >= WS_INFO)
                NSLog(@"gesture event");
            return;

        default:
            if(logLevel >= WS_WARNING)
                NSLog(@"Unhandled input event type %u", etype);
            return;
    }
}

// this reads the default system keymap. Call it after changing the default from prefs.
-(void)setKeymap {
    xkb_keymap_unref(xkb_keymap);
    xkb_keymap = xkb_keymap_new_from_names(xkbCtx, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if(xkb_state)
        xkb_state_unref(xkb_state);
    if(xkb_state_unmodified)
        xkb_state_unref(xkb_state_unmodified);
    xkb_state = xkb_state_new(xkb_keymap);
    xkb_state_unmodified = xkb_state_new(xkb_keymap);
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

-(NSPoint)pointerPos {
    return NSMakePoint(pointerX, pointerY);
}

-(NSPoint)setPointerPos:(NSPoint)pos {
    pointerX = pos.x;
    pointerY = pos.y;
}

-(int)fileDescriptor {
    return libinput_get_fd(li);
}


@end
