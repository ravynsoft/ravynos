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
    // Finding Displays
    kCGMainDisplayID,
    kCGGetOnlineDisplayList,
    kCGGetActiveDisplayList,
    kCGGetDisplaysWithOpenGLDisplayMask,
    kCGGetDisplaysWithPoint,
    kCGGetDisplaysWithRect,
    kCGOpenGLDisplayMaskToDisplayID,
    kCGDisplayIDToOpenGLDisplayMask,
    // Capturing and Releasing Displays
    kCGDisplayCaptureWithOptions,
    kCGDisplayRelease,
    kCGCaptureAllDisplaysWithOptions,
    kCGReleaseAllDisplays,
    kCGDisplayGetDrawingContext,
    // Creating Images from Displays
    kCGDisplayCreateImageForRect,
    // Configuring Displays
    kCGCompleteDisplayConfiguration,
    kCGRestorePermanentDisplayConfiguration,
    // Getting the Display Configuration
    kCGDisplayStateFlags,
    kCGDisplayMirrorsDisplay,
    kCGDisplayModelNumber,
    kCGDisplayPrimaryDisplay,
    kCGDisplayRotation,
    kCGDisplayScreenSize,
    kCGDisplaySerialNumber,
    kCGDisplayUnitNumber,
    kCGDisplayUsesOpenGLAcceleration,
    kCGDisplayVendorNumber,
    // Retrieving Display Parameters
    kCGDisplayBounds,
    // Creating and Managing Display Modes
    kCGDisplayCopyDisplayMode,
    kCGDisplayCopyAllDisplayModes,
    kCGDisplaySetDisplayMode,
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

typedef enum WSDisplayFlags : uint32_t {
    kWSDisplayActive = (1 << 0),
    kWSDisplayOnline = (1 << 1),
    kWSDisplaySleeping = (1 << 2),
    kWSDisplayMirrored = (1 << 3),
    kWSDisplayPrimary = (1 << 4),
    kWSDisplayMain = (1 << 5),
    kWSDisplayBuiltin = (1 << 6),
    kWSDisplayStereo = (1 << 7),
    kWSDisplayHWMirror = (1 << 8)
} WSDisplayFlags;

struct CGDisplayMode {
    int refcount;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    float refresh;
    uint32_t flags;
};

struct _CGDispCfgMirror {
    uint32_t opcode;
    uint32_t display;
    uint32_t primary;
};

struct _CGDispCfgOrigin {
    uint32_t opcode;
    uint32_t display;
    int32_t x;
    int32_t y;
};

struct _CGDispCfgMode {
    uint32_t opcode;
    uint32_t display;
    struct CGDisplayMode mode; // copied in
};

struct _CGDisplayConfigInner {
    struct wsRPCBase rpc;
    uint32_t length;    // total length of object
    uint32_t option;    // scope of change
};

#define CGDISPCFG_MIRROR 100
#define CGDISPCFG_ORIGIN 101
#define CGDISPCFG_MODE 102

