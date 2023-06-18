/*
 * Copyright (C) 2022-2023 Zoe Knox <zoe@pixin.net>
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
#define MSG_ID_PORT     90210
#define MSG_ID_INLINE   90211

// Recent Items
#define CODE_ADD_RECENT_ITEM 1
#define CODE_ITEM_CLICKED 2

// Activation
#define CODE_APP_BECAME_ACTIVE 3
#define CODE_APP_BECAME_INACTIVE 4
#define CODE_APP_ACTIVATE 5
#define CODE_APP_HIDE 6

// Status Items
#define CODE_STATUS_ITEM_ADDED 7
#define CODE_ADD_STATUS_ITEM 8


typedef struct {
    mach_msg_header_t header;
    unsigned int code;
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
    unsigned int pid;
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
    unsigned int code;
    unsigned char data[64*1024];
    unsigned int len;
    mach_msg_trailer_t trailer;
} ReceiveMessage;
#endif

