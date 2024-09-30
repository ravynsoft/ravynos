# Quick Guide

## Introduction

This document contains a quick walk-through of the often-used parts of
the library. We will employ a few use-cases to lead the examples:

1. An evdev client. "evdev" is the Linux kernel's input subsystem; it
   only reports to the client which keys are pressed and released.

2. An X11 client, using the XCB library to communicate with the X
   server and the xcb-xkb library for using the XKB protocol.

3. A Wayland client, using the standard protocol.

The snippets are not complete, and some support code is omitted. You
can find complete and more complex examples in the source directory:

1. tools/interactive-evdev.c contains an interactive evdev client.

2. tools/interactive-x11.c contains an interactive X11 client.

3. tools/interactive-wayland.c contains an interactive Wayland client.

Also, the library contains many more functions for examining and using
the library context, the keymap and the keyboard state. See the
hyper-linked reference documentation or go through the header files in
xkbcommon/ for more details.

## Code

Before we can do anything interesting, we need a library context:

~~~{.c}
    #include <xkbcommon/xkbcommon.h>

    struct xkb_context *ctx;

    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) <error>
~~~

The `xkb_context` contains the keymap include paths, the log level and
functions, and other general customizable administrativia.

Next we need to create a keymap, `xkb_keymap`. This is an immutable object
which contains all of the information about the keys, layouts, etc. There
are different ways to do this.

If we are an evdev client, we have nothing to go by, so we need to ask
the user for his/her keymap preferences (for example, an Icelandic
keyboard with a Dvorak layout). The configuration format is commonly
called RMLVO (Rules+Model+Layout+Variant+Options), the same format used
by the X server. With it, we can fill a struct called `xkb_rule_names`;
passing `NULL` chooses the system's default.

~~~{.c}
    struct xkb_keymap *keymap;
    /* Example RMLVO for Icelandic Dvorak. */
    struct xkb_rule_names names = {
        .rules = NULL,
        .model = "pc105",
        .layout = "is",
        .variant = "dvorak",
        .options = "terminate:ctrl_alt_bksp"
    };

    keymap = xkb_keymap_new_from_names(ctx, &names,
                                       XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) <error>
~~~

If we are a Wayland client, the compositor gives us a string complete
with a keymap. In this case, we can create the keymap object like this:

~~~{.c}
    /* From the wl_keyboard::keymap event. */
    const char *keymap_string = <...>;
    struct xkb_keymap *keymap;

    keymap = xkb_keymap_new_from_string(ctx, keymap_string,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) <error>
~~~

If we are an X11 client, we are better off getting the keymap from the
X server directly. For this we need to choose the XInput device; here
we will use the core keyboard device:

~~~{.c}
    #include <xkbcommon/xkbcommon-x11.h>

    xcb_connection_t *conn = <...>;
    int32_t device_id;

    device_id = xkb_x11_get_core_keyboard_device_id(conn);
    if (device_id == -1) <error>

    keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) <error>
~~~

Now that we have the keymap, we are ready to handle the keyboard devices.
For each device, we create an `xkb_state`, which remembers things like which
keyboard modifiers and LEDs are active:

~~~{.c}
    struct xkb_state *state;

    state = xkb_state_new(keymap);
    if (!state) <error>
~~~

For X11/XCB clients, this is better:

~~~{.c}
    state = xkb_x11_state_new_from_device(keymap, conn, device_id);
    if (!state) <error>
~~~

When we have an `xkb_state` for a device, we can start handling key events
from it.  Given a keycode for a key, we can get its keysym:

~~~{.c}
    <key event structure> event;
    xkb_keycode_t keycode;
    xkb_keysym_t keysym;

    keycode = event->keycode;
    keysym = xkb_state_key_get_one_sym(state, keycode);
~~~

We can see which keysym we got, and get its name:

~~~{.c}
    char keysym_name[64];

    if (keysym == XKB_KEY_Space)
        <got a space>

    xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
~~~

libxkbcommon also supports an extension to the classic XKB, whereby a
single event can result in multiple keysyms. Here's how to use it:

~~~{.c}
    const xkb_keysym_t *keysyms;
    int num_keysyms;

    num_keysyms = xkb_state_key_get_syms(state, keycode, &keysyms);
~~~

We can also get a UTF-8 string representation for this key:

~~~{.c}
    char *buffer;
    int size;

    // First find the needed size; return value is the same as snprintf(3).
    size = xkb_state_key_get_utf8(state, keycode, NULL, 0) + 1;
    if (size <= 1) <nothing to do>
    buffer = <allocate size bytes>

    xkb_state_key_get_utf8(state, keycode, buffer, size);
~~~

Of course, we also need to keep the `xkb_state` up-to-date with the
keyboard device, if we want to get the correct keysyms in the future.

If we are an evdev client, we must let the library know whether a key
is pressed or released at any given time:

~~~{.c}
    enum xkb_state_component changed;

    if (<key press>)
        changed = xkb_state_update_key(state, keycode, XKB_KEY_DOWN);
    else if (<key release>)
        changed = xkb_state_update_key(state, keycode, XKB_KEY_UP);
~~~

The `changed` return value tells us exactly which parts of the state
have changed.

If it is a key-repeat event, we can ask the keymap what to do with it:

~~~{.c}
    if (<key repeat> && !xkb_keymap_key_repeats(keymap, keycode))
        <discard event>
~~~

On the other hand, if we are an X or Wayland client, the server already
does the hard work for us. It notifies us when the device's state
changes, and we can simply use what it tells us (the necessary
information usually comes in a form of some "state changed" event):

~~~{.c}
    changed = xkb_state_update_mask(state,
                                    event->depressed_mods,
                                    event->latched_mods,
                                    event->locked_mods,
                                    event->depressed_layout,
                                    event->latched_layout,
                                    event->locked_layout);
~~~

Now that we have an always-up-to-date `xkb_state`, we can examine it.
For example, we can check whether the Control modifier is active, or
whether the Num Lock LED is active:

~~~{.c}
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL,
                                     XKB_STATE_MODS_EFFECTIVE) > 0)
        <The Control modifier is active>

    if (xkb_state_led_name_is_active(state, XKB_LED_NAME_NUM) > 0)
        <The Num Lock LED is active>
~~~

And that's it! Eventually, we should free the objects we've created:

~~~{.c}
    xkb_state_unref(state);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
~~~
