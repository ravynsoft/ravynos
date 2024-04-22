/*
 * Copyright © 2013 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xkb.h>

#include "xkbcommon/xkbcommon-x11.h"
#include "tools-common.h"

/*
 * Note: This program only handles the core keyboard device for now.
 * It should be straigtforward to change struct keyboard to a list of
 * keyboards with device IDs, as in tools/interactive-evdev.c. This would
 * require:
 *
 * - Initially listing the keyboard devices.
 * - Listening to device changes.
 * - Matching events to their devices.
 *
 * XKB itself knows about xinput1 devices, and most requests and events are
 * device-specific.
 *
 * In order to list the devices and react to changes, you need xinput1/2.
 * You also need xinput for the key press/release event, since the core
 * protocol key press event does not carry a device ID to match on.
 */

struct keyboard {
    xcb_connection_t *conn;
    uint8_t first_xkb_event;
    struct xkb_context *ctx;

    struct xkb_keymap *keymap;
    struct xkb_state *state;
    int32_t device_id;
};

static bool terminate;

static int
select_xkb_events_for_device(xcb_connection_t *conn, int32_t device_id)
{
    enum {
        required_events =
            (XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
             XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
             XCB_XKB_EVENT_TYPE_STATE_NOTIFY),

        required_nkn_details =
            (XCB_XKB_NKN_DETAIL_KEYCODES),

        required_map_parts =
            (XCB_XKB_MAP_PART_KEY_TYPES |
             XCB_XKB_MAP_PART_KEY_SYMS |
             XCB_XKB_MAP_PART_MODIFIER_MAP |
             XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
             XCB_XKB_MAP_PART_KEY_ACTIONS |
             XCB_XKB_MAP_PART_VIRTUAL_MODS |
             XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP),

        required_state_details =
            (XCB_XKB_STATE_PART_MODIFIER_BASE |
             XCB_XKB_STATE_PART_MODIFIER_LATCH |
             XCB_XKB_STATE_PART_MODIFIER_LOCK |
             XCB_XKB_STATE_PART_GROUP_BASE |
             XCB_XKB_STATE_PART_GROUP_LATCH |
             XCB_XKB_STATE_PART_GROUP_LOCK),
    };

    static const xcb_xkb_select_events_details_t details = {
        .affectNewKeyboard = required_nkn_details,
        .newKeyboardDetails = required_nkn_details,
        .affectState = required_state_details,
        .stateDetails = required_state_details,
    };

    xcb_void_cookie_t cookie =
        xcb_xkb_select_events_aux_checked(conn,
                                          device_id,
                                          required_events,    /* affectWhich */
                                          0,                  /* clear */
                                          0,                  /* selectAll */
                                          required_map_parts, /* affectMap */
                                          required_map_parts, /* map */
                                          &details);          /* details */

    xcb_generic_error_t *error = xcb_request_check(conn, cookie);
    if (error) {
        free(error);
        return -1;
    }

    return 0;
}

static int
update_keymap(struct keyboard *kbd)
{
    struct xkb_keymap *new_keymap;
    struct xkb_state *new_state;

    new_keymap = xkb_x11_keymap_new_from_device(kbd->ctx, kbd->conn,
                                                kbd->device_id,
                                                XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!new_keymap)
        goto err_out;

    new_state = xkb_x11_state_new_from_device(new_keymap, kbd->conn,
                                              kbd->device_id);
    if (!new_state)
        goto err_keymap;

    if (kbd->keymap)
        printf("Keymap updated!\n");

    xkb_state_unref(kbd->state);
    xkb_keymap_unref(kbd->keymap);
    kbd->keymap = new_keymap;
    kbd->state = new_state;
    return 0;

err_keymap:
    xkb_keymap_unref(new_keymap);
err_out:
    return -1;
}

static int
init_kbd(struct keyboard *kbd, xcb_connection_t *conn, uint8_t first_xkb_event,
         int32_t device_id, struct xkb_context *ctx)
{
    int ret;

    kbd->conn = conn;
    kbd->first_xkb_event = first_xkb_event;
    kbd->ctx = ctx;
    kbd->keymap = NULL;
    kbd->state = NULL;
    kbd->device_id = device_id;

    ret = update_keymap(kbd);
    if (ret)
        goto err_out;

    ret = select_xkb_events_for_device(conn, device_id);
    if (ret)
        goto err_state;

    return 0;

err_state:
    xkb_state_unref(kbd->state);
    xkb_keymap_unref(kbd->keymap);
err_out:
    return -1;
}

static void
deinit_kbd(struct keyboard *kbd)
{
    xkb_state_unref(kbd->state);
    xkb_keymap_unref(kbd->keymap);
}

static void
process_xkb_event(xcb_generic_event_t *gevent, struct keyboard *kbd)
{
    union xkb_event {
        struct {
            uint8_t response_type;
            uint8_t xkbType;
            uint16_t sequence;
            xcb_timestamp_t time;
            uint8_t deviceID;
        } any;
        xcb_xkb_new_keyboard_notify_event_t new_keyboard_notify;
        xcb_xkb_map_notify_event_t map_notify;
        xcb_xkb_state_notify_event_t state_notify;
    } *event = (union xkb_event *) gevent;

    if (event->any.deviceID != kbd->device_id)
        return;

    /*
     * XkbNewKkdNotify and XkbMapNotify together capture all sorts of keymap
     * updates (e.g. xmodmap, xkbcomp, setxkbmap), with minimal redundent
     * recompilations.
     */
    switch (event->any.xkbType) {
    case XCB_XKB_NEW_KEYBOARD_NOTIFY:
        if (event->new_keyboard_notify.changed & XCB_XKB_NKN_DETAIL_KEYCODES)
            update_keymap(kbd);
        break;

    case XCB_XKB_MAP_NOTIFY:
        update_keymap(kbd);
        break;

    case XCB_XKB_STATE_NOTIFY:
        xkb_state_update_mask(kbd->state,
                              event->state_notify.baseMods,
                              event->state_notify.latchedMods,
                              event->state_notify.lockedMods,
                              event->state_notify.baseGroup,
                              event->state_notify.latchedGroup,
                              event->state_notify.lockedGroup);
        break;
    }
}

static void
process_event(xcb_generic_event_t *gevent, struct keyboard *kbd)
{
    switch (gevent->response_type) {
    case XCB_KEY_PRESS: {
        xcb_key_press_event_t *event = (xcb_key_press_event_t *) gevent;
        xkb_keycode_t keycode = event->detail;

        tools_print_keycode_state(kbd->state, NULL, keycode,
                                  XKB_CONSUMED_MODE_XKB,
                                  PRINT_ALL_FIELDS);

        /* Exit on ESC. */
        if (xkb_state_key_get_one_sym(kbd->state, keycode) == XKB_KEY_Escape)
            terminate = true;
        break;
    }
    default:
        if (gevent->response_type == kbd->first_xkb_event)
            process_xkb_event(gevent, kbd);
        break;
    }
}

static int
loop(xcb_connection_t *conn, struct keyboard *kbd)
{
    while (!terminate) {
        xcb_generic_event_t *event;

        switch (xcb_connection_has_error(conn)) {
        case 0:
            break;
        case XCB_CONN_ERROR:
            fprintf(stderr,
                    "Closed connection to X server: connection error\n");
            return -1;
        case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
            fprintf(stderr,
                    "Closed connection to X server: extension not supported\n");
            return -1;
        default:
            fprintf(stderr,
                    "Closed connection to X server: error code %d\n",
                    xcb_connection_has_error(conn));
            return -1;
        }

        event = xcb_wait_for_event(conn);
        if (!event) {
            continue;
        }

        process_event(event, kbd);
        free(event);
    }

    return 0;
}

static int
create_capture_window(xcb_connection_t *conn)
{
    xcb_generic_error_t *error;
    xcb_void_cookie_t cookie;
    xcb_screen_t *screen =
        xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    xcb_window_t window = xcb_generate_id(conn);
    uint32_t values[2] = {
        screen->white_pixel,
        XCB_EVENT_MASK_KEY_PRESS,
    };

    cookie = xcb_create_window_checked(conn, XCB_COPY_FROM_PARENT,
                                       window, screen->root,
                                       10, 10, 100, 100, 1,
                                       XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                       screen->root_visual,
                                       XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
                                       values);
    if ((error = xcb_request_check(conn, cookie)) != NULL) {
        free(error);
        return -1;
    }

    cookie = xcb_map_window_checked(conn, window);
    if ((error = xcb_request_check(conn, cookie)) != NULL) {
        free(error);
        return -1;
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    int ret;
    xcb_connection_t *conn;
    uint8_t first_xkb_event;
    int32_t core_kbd_device_id;
    struct xkb_context *ctx;
    struct keyboard core_kbd;

    if (argc != 1) {
        ret = strcmp(argv[1], "--help");
        fprintf(ret ? stderr : stdout, "Usage: %s [--help]\n", argv[0]);
        if (ret)
            fprintf(stderr, "unrecognized option: %s\n", argv[1]);
        return ret ? EXIT_INVALID_USAGE : EXIT_SUCCESS;
    }

    setlocale(LC_ALL, "");

    conn = xcb_connect(NULL, NULL);
    if (!conn || xcb_connection_has_error(conn)) {
        fprintf(stderr, "Couldn't connect to X server: error code %d\n",
                conn ? xcb_connection_has_error(conn) : -1);
        ret = -1;
        goto err_out;
    }

    ret = xkb_x11_setup_xkb_extension(conn,
                                      XKB_X11_MIN_MAJOR_XKB_VERSION,
                                      XKB_X11_MIN_MINOR_XKB_VERSION,
                                      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                      NULL, NULL, &first_xkb_event, NULL);
    if (!ret) {
        fprintf(stderr, "Couldn't setup XKB extension\n");
        goto err_conn;
    }

    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) {
        ret = -1;
        fprintf(stderr, "Couldn't create xkb context\n");
        goto err_conn;
    }

    core_kbd_device_id = xkb_x11_get_core_keyboard_device_id(conn);
    if (core_kbd_device_id == -1) {
        ret = -1;
        fprintf(stderr, "Couldn't find core keyboard device\n");
        goto err_ctx;
    }

    ret = init_kbd(&core_kbd, conn, first_xkb_event, core_kbd_device_id, ctx);
    if (ret) {
        fprintf(stderr, "Couldn't initialize core keyboard device\n");
        goto err_ctx;
    }

    ret = create_capture_window(conn);
    if (ret) {
        fprintf(stderr, "Couldn't create a capture window\n");
        goto err_core_kbd;
    }

    tools_disable_stdin_echo();
    ret = loop(conn, &core_kbd);
    tools_enable_stdin_echo();

err_core_kbd:
    deinit_kbd(&core_kbd);
err_ctx:
    xkb_context_unref(ctx);
err_conn:
    xcb_disconnect(conn);
err_out:
    exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
