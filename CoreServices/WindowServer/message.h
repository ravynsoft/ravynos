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

/* This header defines the Mach messages used between AppKit and
 * WindowServer components
 *
 * FIXME: refactor this to a .defs file and use mig. I suspect this
 * should really be implemented as remote notifications or something
 * but it is manageable for now.
 */

#define WINDOWSERVER_SVC_NAME "com.ravynos.WindowServer"

/* Message structure type. PORT conveys a mach_port_t, and INLINE is
 * everything else
 */
enum {
    MSG_ID_PORT = 90210,
    MSG_ID_INLINE = 90211,
    MSG_ID_RPC = 90222
};

/* INLINE message codes */
enum {
    CODE_NULL = 0,

    // Display management
    CODE_DISPLAY_INFO,          // Connect NSDisplay in AppKit to our display

    // Recent Items
    CODE_ADD_RECENT_ITEM,       // Add to Recent Items menu

    // Menus
    CODE_ITEM_CLICKED,          // Menu item was clicked

    // Activation
    CODE_APP_BECAME_ACTIVE,
    CODE_APP_BECAME_INACTIVE,
    CODE_ACTIVATION_STATE,      // Control active/inactive and active window
    CODE_APP_HIDE,

    // Status Items
    CODE_STATUS_ITEM_ADDED,
    CODE_ADD_STATUS_ITEM,       // Add item to status area

    // Input event
    CODE_INPUT_EVENT,           // Notify app of input

    // Window state change
    CODE_WINDOW_STATE,
};

enum {
    BTN_UP = 0,
    BTN_DOWN
};


/* this must be in sync with actual NSWindow state */
enum WindowState {
    NORMAL, CLOSED, MAXVERT, MAXHORIZ, MAXIMIZED, MINIMIZED, HIDDEN, WIN_STATE_MAX
};


/* Display info */
struct mach_display_info {
    uint32_t screens;           // how many screens we have
    double width;
    double height;
    uint32_t depth;
};

/* Efficient intermediate struct between raw input events and NSEvent */
struct mach_event {
    uint32_t code;
    uint32_t keycode;
    uint32_t mods;
    uint32_t state;
    uint32_t windowID;
    uint8_t chars[8];
    uint8_t charsIg[8];
    uint32_t repeat;
    double x;
    double y;
    double dx;
    double dy;
    uint8_t buttons[3]; // L R M
    double scroll;
    // FIXME: touch and gesture events
};

/* Intermediate struct for window management */
struct mach_win_data {
    uint32_t windowID;
    double x, y;
    double w, h;
    uint32_t style;
    uint32_t state;
    char title[128];
};

struct mach_activation_data {
    uint32_t windowID;
    uint32_t active;
};

typedef struct {
    mach_msg_header_t header;
    char bundleID[128];
    unsigned int code;
    unsigned int pid;
    unsigned char data[128*1024]; // 128 KB
    unsigned int len;
#ifdef WINDOWSERVER
    mach_msg_trailer_t trailer;
#endif
} Message;

typedef struct {
    mach_msg_header_t header;
    mach_msg_size_t msgh_descriptor_count;
    mach_msg_port_descriptor_t descriptor;
    char bundleID[128];
    unsigned int pid;
    unsigned char data[128*1024];
    unsigned int len;
#ifdef WINDOWSERVER
    mach_msg_trailer_t trailer;
#endif
} PortMessage;

#ifdef WINDOWSERVER
typedef union {
    PortMessage portMsg;
    Message msg;
} ReceiveMessage;
#else
typedef struct {
    mach_msg_header_t header;
    char bundleID[128];
    unsigned int code;
    unsigned int pid;
    unsigned char data[128*1024];
    unsigned int len;
    mach_msg_trailer_t trailer;
} ReceiveMessage;
#endif

