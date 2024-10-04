/*
 * Quartz Display Services
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
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

/* Remote Procedure Call codes between CoreGraphics and WindowServer
 * Do not use these in applications! There is no guarantee of stability
 * between releases. They are for internal use only.
 */
typedef enum WSRPC {
    kCGWSRPCNull = 0,
    kCGMainDisplayID,
    kCGGetOnlineDisplayList,
    kCGGetActiveDisplayList,
    kCGGetDisplaysWithOpenGLDisplayMask,
    kCGGetDisplaysWithPoint,
    kCGGetDisplaysWithRect,
    kCGOpenGLDisplayMaskToDisplayID,
    kCGDisplayIDToOpenGLDisplayMask,
} WSRPC;

/* Data field header, followed by function-specific data struct */
struct wsRPCBase {
    uint32_t code;      // RPC function code (above)
    uint32_t len;       // Length of trailing data, if any
};

struct wsRPCSimple {
    struct wsRPCBase base;
    uint32_t val1;
    uint32_t val2;
    uint32_t val3;
    uint32_t val4;
};

