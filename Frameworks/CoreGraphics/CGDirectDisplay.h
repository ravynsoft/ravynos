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


#import <CoreFoundation/CFRunLoop.h>
#import <CoreFoundation/CFMachPort.h>
#import <CoreGraphics/CoreGraphicsExport.h>
#import <CoreGraphics/CGError.h>
#import <CoreGraphics/CGGeometry.h>
#import <CoreGraphics/CGContext.h>
#import <CoreGraphics/CGWindow.h>
#import <CoreGraphics/CGColorSpace.h>
#import <CoreGraphics/CGImage.h>

#if __BLOCKS__
#import <dispatch/dispatch.h>
#endif

#define kCGSessionUserIDKey CFSTR("kCGSessionUserIDKey")
#define kCGSessionUserNameKey CFSTR("kCGSessionUserNameKey")
#define kCGSessionConsoleSetKey CFSTR("kCGSessionConsoleSetKey")
#define kCGSessionOnConsoleKey CFSTR("kCGSessionOnConsoleKey")
#define kCGSessionLoginDoneKey CFSTR("kCGSessionLoginDoneKey")

typedef CGError CGDisplayError; // deprecated, use CGError instead

#define kCGDirectMainDisplay CGMainDisplayID()
#define kCGNullDirectDisplay 0

typedef uint32_t CGDirectDisplayID;
typedef uint32_t CGDisplayCount; // deprecated, use uint32_t instead
typedef uint32_t CGRectCount;
typedef struct _CGDisplayConfig *CGDisplayConfigRef;
typedef struct CGDisplayMode *CGDisplayModeRef;
typedef float CGGammaValue;
typedef double CGRefreshRate;

typedef enum CGDisplayChangeSummaryFlags : uint32_t {
    kCGDisplayBeginConfigurationFlag = (1 << 0),
    kCGDisplayMovedFlag = (1 << 1),
    kCGDisplaySetMainFlag = (1 << 2),
    kCGDisplaySetModeFlag = (1 << 3),
    kCGDisplayAddFlag = (1 << 4),
    kCGDisplayRemoveFlag = (1 << 5),
    // nothing uses bits 6, 7
    kCGDisplayEnabledFlag = (1 << 8),
    kCGDisplayDisabledFlag = (1 << 9),
    kCGDisplayMirrorFlag = (1 << 10),
    kCGDisplayUnMirrorFlag = (1 << 11),
    kCGDisplayDesktopShapeChangedFlag = (1 << 12),
} CGDisplayChangeSummaryFlags;

typedef enum CGConfigureOption : uint32_t {
    kCGConfigureForAppOnly = 0,
    kCGConfigureForSession = 1,
    kCGConfigurePermanently = 2,
} CGConfigureOption;

typedef enum CGDisplayStreamFrameStatus : int32_t {
    kCGDisplayStreamFrameStatusFrameComplete,
    kCGDisplayStreamFrameStatusFrameIdle,
    kCGDisplayStreamFrameStatusFrameBlank,
    kCGDisplayStreamFrameStatusStopped,
} CGDisplayStreamFrameStatus;

// these are deprecated from 10.14+
// we are going for 10.15 compat, so we will support them for now
extern const CFStringRef kCGDisplayStreamSourceRect;
extern const CFStringRef kCGDisplayStreamDestinationRect;
extern const CFStringRef kCGDisplayStreamPreserveAspectRatio;
extern const CFStringRef kCGDisplayStreamColorSpace;
extern const CFStringRef kCGDisplayStreamMinimumFrameTime;
extern const CFStringRef kCGDisplayStreamShowCursor;
extern const CFStringRef kCGDisplayStreamQueueDepth;
extern const CFStringRef kCGDisplayStreamYCbCrMatrix;

extern const CFStringRef kCGDisplayStreamYCbCrMatrix_ITU_R_709_2;
extern const CFStringRef kCGDisplayStreamYCbCrMatrix_ITU_R_601_4;
extern const CFStringRef kCGDisplayStreamYCbCrMatrix_SMPTE_240M_1995;

typedef struct CGDisplayStream *CGDisplayStreamRef;
typedef struct CGDisplayStreamUpdate *CGDisplayStreamUpdateRef;
#if __BLOCKS__
// oh great, we're dragging in IOKit now... fake it for now
typedef void *IOSurfaceRef;
typedef void (^CGDisplayStreamFrameAvailableHandler)(CGDisplayStreamFrameStatus status,
        uint64_t displayTime, IOSurfaceRef frameSurface, CGDisplayStreamUpdateRef updateRef);
#endif
typedef enum CGDisplayStreamUpdateRectType : int32_t {
    kCGDisplayStreamUpdateRefreshedRects,
    kCGDisplayStreamUpdateMovedRects,
    kCGDisplayStreamUpdateDirtyRects,
    kCGDisplayStreamUpdateReducedDirtyRects,
} CGDisplayStreamUpdateRectType;

#define kCGDisplayBlendNormal 0.0
#define kCGDisplayBlendSolidColor 1.0

typedef float CGDisplayBlendFraction;

#define kCGMaxDisplayReservationInterval 15.0 // tested on Sonoma
#define kCGDisplayFadeReservationInvalidToken 0

typedef float CGDisplayFadeInterval;
typedef uint32_t CGDisplayFadeReservationToken;
typedef float CGDisplayReservationInterval;

typedef uint32_t CGOpenGLDisplayMask;

#define kCGNumReservedWindowLevels 16; // tested on Sonoma
typedef int32_t CGWindowLevel;
typedef enum CGWindowLevelKey : int32_t {
    kCGBaseWindowLevelKey = 0,
    kCGMinimumWindowLevelKey,
    kCGDesktopWindowLevelKey,
    kCGBackstopMenuLevelKey,
    kCGNormalWindowLevelKey,
    kCGFloatingWindowLevelKey,
    kCGTornOffMenuWindowLevelKey,
    kCGDockWindowLevelKey,
    kCGMainMenuWindowLevelKey,
    kCGStatusWindowLevelKey,
    kCGModalPanelWindowLevelKey,
    kCGPopUpMenuWindowLevelKey,
    kCGDraggingWindowLevelKey,
    kCGScreenSaverWindowLevelKey,
    kCGMaximumWindowLevelKey,
    kCGOverlayWindowLevelKey,
    kCGHelpWindowLevelKey,
    kCGUtilityWindowLevelKey,
    kCGDesktopIconWindowLevelKey,
    kCGCursorWindowLevelKey,
    kCGAssistiveTechHighWindowLevelKey,
    kCGNumberOfWindowLevelKeys,
} CGWindowLevelKey;


typedef enum CGCaptureOptions : uint32_t {
    kCGCaptureNoOptions = 0,
    kCGCaptureNoFill,   // deprecated
} CGCaptureOptions;


typedef enum CGScreenUpdateOperation : uint32_t {
    kCGScreenUpdateOperationRefresh = 0,
    kCGScreenUpdateOperationMove = (1u << 0),
    kCGScreenUpdateOperationReducedDirtyRectangleCount = (1u << 31),
} CGScreenUpdateOperation;

typedef struct CGScreenUpdateMoveDelta {
    int32_t dX;
    int32_t dY;
} CGScreenUpdateMoveDelta;

// Callback types
typedef void (*CGDisplayReconfigurationCallBack)(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *userInfo);
typedef void (*CGScreenRefreshCallback)(uint32_t count, const CGRect *rects, void *userInfo);
typedef void (*CGScreenUpdateMoveCallback)(CGScreenUpdateMoveDelta delta, size_t count, const CGRect *rects, void *userInfo);

// Finding displays
COREGRAPHICS_EXPORT CGDirectDisplayID CGMainDisplayID(void);
COREGRAPHICS_EXPORT CGError CGGetOnlineDisplayList(uint32_t maxDisplays, CGDirectDisplayID *onlineDisplays, uint32_t *displayCount);
COREGRAPHICS_EXPORT CGError CGGetActiveDisplayList(uint32_t maxDisplays, CGDirectDisplayID *activeDisplays, uint32_t *displayCount);
COREGRAPHICS_EXPORT CGError CGGetDisplaysWithOpenGLDisplayMask(CGOpenGLDisplayMask mask, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount);
COREGRAPHICS_EXPORT CGError CGGetDisplaysWithPoint(CGPoint point, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount);
COREGRAPHICS_EXPORT CGError CGGetDisplaysWithRect(CGRect rect, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount);
COREGRAPHICS_EXPORT CGDirectDisplayID CGOpenGLDisplayMaskToDisplayID(CGOpenGLDisplayMask mask);
COREGRAPHICS_EXPORT CGOpenGLDisplayMask CGDisplayIDToOpenGLDisplayMask(CGDirectDisplayID display);

// Capturing and releasing displays
COREGRAPHICS_EXPORT CGError CGDisplayCapture(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGError CGDisplayCaptureWithOptions(CGDirectDisplayID display, CGCaptureOptions options);
COREGRAPHICS_EXPORT CGError CGDisplayRelease(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGError CGCaptureAllDisplays(void);
COREGRAPHICS_EXPORT CGError CGCaptureAllDisplaysWithOptions(CGCaptureOptions options);

COREGRAPHICS_EXPORT CGError CGReleaseAllDisplays(void);
COREGRAPHICS_EXPORT CGWindowID CGShieldingWindowID(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGWindowLevel CGShieldingWindowLevel(void);
COREGRAPHICS_EXPORT CGContextRef CGDisplayGetDrawingContext(CGDirectDisplayID display);

// Creating images. These are deprecated in 14.4+ but not in 10.15!
COREGRAPHICS_EXPORT CGImageRef CGDisplayCreateImage(CGDirectDisplayID displayID); 
COREGRAPHICS_EXPORT CGImageRef CGDisplayCreateImageForRect(CGDirectDisplayID display, CGRect rect);

// Configuring displays
COREGRAPHICS_EXPORT CGError CGBeginDisplayConfiguration(CGDisplayConfigRef *config);
COREGRAPHICS_EXPORT CGError CGCancelDisplayConfiguration(CGDisplayConfigRef config);
COREGRAPHICS_EXPORT CGError CGCompleteDisplayConfiguration(CGDisplayConfigRef config, CGConfigureOption option);
COREGRAPHICS_EXPORT CGError CGConfigureDisplayMirrorOfDisplay(CGDisplayConfigRef config, CGDirectDisplayID display, CGDirectDisplayID master);
COREGRAPHICS_EXPORT CGError CGConfigureDisplayOrigin(CGDisplayConfigRef config, CGDirectDisplayID display, int32_t x, int32_t y);
COREGRAPHICS_EXPORT void CGRestorePermanentDisplayConfiguration(void);
COREGRAPHICS_EXPORT CGError CGConfigureDisplayStereoOperation(CGDisplayConfigRef config, CGDirectDisplayID display, boolean_t stereo, boolean_t forceBlueLine);
COREGRAPHICS_EXPORT CGError CGDisplaySetStereoOperation(CGDirectDisplayID display, boolean_t stereo, boolean_t forceBlueLine, CGConfigureOption option);
COREGRAPHICS_EXPORT CGError CGConfigureDisplayWithDisplayMode(CGDisplayConfigRef config, CGDirectDisplayID display, CGDisplayModeRef mode, CFDictionaryRef options);

// Getting display config
COREGRAPHICS_EXPORT CGColorSpaceRef CGDisplayCopyColorSpace(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsActive(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsAlwaysInMirrorSet(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsAsleep(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsBuiltin(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsInHWMirrorSet(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsInMirrorSet(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsMain(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsOnline(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayIsStereo(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGDirectDisplayID CGDisplayMirrorsDisplay(CGDirectDisplayID display);
COREGRAPHICS_EXPORT uint32_t CGDisplayModelNumber(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGDirectDisplayID CGDisplayPrimaryDisplay(CGDirectDisplayID display);
COREGRAPHICS_EXPORT double CGDisplayRotation(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGSize CGDisplayScreenSize(CGDirectDisplayID display);
COREGRAPHICS_EXPORT uint32_t CGDisplaySerialNumber(CGDirectDisplayID display);
COREGRAPHICS_EXPORT uint32_t CGDisplayUnitNumber(CGDirectDisplayID display);
COREGRAPHICS_EXPORT boolean_t CGDisplayUsesOpenGLAcceleration(CGDirectDisplayID display);
COREGRAPHICS_EXPORT uint32_t CGDisplayVendorNumber(CGDirectDisplayID display);

// Registering for config change notices
COREGRAPHICS_EXPORT CGError CGDisplayRegisterReconfigurationCallback(CGDisplayReconfigurationCallBack callback, void *userInfo);
COREGRAPHICS_EXPORT CGError CGDisplayRemoveReconfigurationCallback(CGDisplayReconfigurationCallBack callback, void *userInfo);

// Display mode information
COREGRAPHICS_EXPORT CGRect CGDisplayBounds(CGDirectDisplayID display);
COREGRAPHICS_EXPORT size_t CGDisplayPixelsHigh(CGDirectDisplayID display);
COREGRAPHICS_EXPORT size_t CGDisplayPixelsWide(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID display, CFDictionaryRef options);
COREGRAPHICS_EXPORT CGError CGDisplaySetDisplayMode(CGDirectDisplayID display, CGDisplayModeRef mode, CFDictionaryRef options);
COREGRAPHICS_EXPORT CGDisplayModeRef CGDisplayModeRetain(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT void CGDisplayModeRelease(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT size_t CGDisplayModeGetWidth(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT size_t CGDisplayModeGetHeight(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT uint32_t CGDisplayModeGetIOFlags(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT int32_t CGDisplayModeGetIODisplayModeID(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT bool CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef mode);
COREGRAPHICS_EXPORT CFTypeID CGDisplayModeGetTypeID(void);

// Gamma
COREGRAPHICS_EXPORT CGError CGSetDisplayTransferByFormula(CGDirectDisplayID display, CGGammaValue redMin, CGGammaValue redMax, CGGammaValue redGamma, CGGammaValue greenMin, CGGammaValue greenMax, CGGammaValue greenGamma, CGGammaValue blueMin, CGGammaValue blueMax, CGGammaValue blueGamma);
COREGRAPHICS_EXPORT CGError CGGetDisplayTransferByFormula(CGDirectDisplayID display, CGGammaValue *redMin, CGGammaValue *redMax, CGGammaValue *redGamma, CGGammaValue *greenMin, CGGammaValue *greenMax, CGGammaValue *greenGamma, CGGammaValue *blueMin, CGGammaValue *blueMax, CGGammaValue *blueGamma);
COREGRAPHICS_EXPORT CGError CGSetDisplayTransferByTable(CGDirectDisplayID display, uint32_t tableSize, const CGGammaValue *redTable, const CGGammaValue *greenTable, const CGGammaValue *blueTable);
COREGRAPHICS_EXPORT CGError CGGetDisplayTransferByTable(CGDirectDisplayID display, uint32_t capacity, CGGammaValue *redTable, CGGammaValue *greenTable, CGGammaValue *blueTable, uint32_t *sampleCount);
COREGRAPHICS_EXPORT CGError CGSetDisplayTransferByByteTable(CGDirectDisplayID display, uint32_t tableSize, const uint8_t *redTable, const uint8_t *greenTable, const uint8_t *blueTable);
COREGRAPHICS_EXPORT void CGDisplayRestoreColorSyncSettings(void);
COREGRAPHICS_EXPORT uint32_t CGDisplayGammaTableCapacity(CGDirectDisplayID display);

// Fade effects
COREGRAPHICS_EXPORT CGError CGConfigureDisplayFadeEffect(CGDisplayConfigRef config, CGDisplayFadeInterval fadeOutSeconds, CGDisplayFadeInterval fadeInSeconds, float fadeRed, float fadeGreen, float fadeBlue);
COREGRAPHICS_EXPORT CGError CGAcquireDisplayFadeReservation(CGDisplayReservationInterval seconds, CGDisplayFadeReservationToken *token);
COREGRAPHICS_EXPORT CGError CGDisplayFade(CGDisplayFadeReservationToken token, CGDisplayFadeInterval duration, CGDisplayBlendFraction startBlend, CGDisplayBlendFraction endBlend, float redBlend, float greenBlend, float blueBlend, boolean_t synchronous);
COREGRAPHICS_EXPORT CGError CGReleaseDisplayFadeReservation(CGDisplayFadeReservationToken token);

// Mouse cursor control
COREGRAPHICS_EXPORT CGError CGDisplayHideCursor(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGError CGDisplayShowCursor(CGDirectDisplayID display);
COREGRAPHICS_EXPORT CGError CGDisplayMoveCursorToPoint(CGDirectDisplayID display, CGPoint point);
COREGRAPHICS_EXPORT CGError CGAssociateMouseAndMouseCursorPosition(boolean_t connected);
COREGRAPHICS_EXPORT CGError CGWarpMouseCursorPosition(CGPoint newCursorPosition);
COREGRAPHICS_EXPORT void CGGetLastMouseDelta(int32_t *deltaX, int32_t *deltaY);

// WindowServer info
COREGRAPHICS_EXPORT CFDictionaryRef CGSessionCopyCurrentDictionary(void);
COREGRAPHICS_EXPORT CFMachPortRef CGWindowServerCFMachPort(void); // Deprecated in 10.8 but we use it internally
COREGRAPHICS_EXPORT CGWindowLevel CGWindowLevelForKey(CGWindowLevelKey key);

// Display streaming
#if __BLOCKS__
COREGRAPHICS_EXPORT CGDisplayStreamRef CGDisplayStreamCreate(CGDirectDisplayID display, size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef properties, CGDisplayStreamFrameAvailableHandler handler);
COREGRAPHICS_EXPORT CGDisplayStreamRef CGDisplayStreamCreateWithDispatchQueue(CGDirectDisplayID display, size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef properties, dispatch_queue_t queue, CGDisplayStreamFrameAvailableHandler handler);
#endif
COREGRAPHICS_EXPORT CGError CGDisplayStreamStart(CGDisplayStreamRef displayStream);
COREGRAPHICS_EXPORT CGError CGDisplayStreamStop(CGDisplayStreamRef displayStream);
COREGRAPHICS_EXPORT CFRunLoopSourceRef CGDisplayStreamGetRunLoopSource(CGDisplayStreamRef displayStream);
COREGRAPHICS_EXPORT const CGRect * CGDisplayStreamUpdateGetRects(CGDisplayStreamUpdateRef updateRef, CGDisplayStreamUpdateRectType rectType, size_t *rectCount);
COREGRAPHICS_EXPORT CGDisplayStreamUpdateRef CGDisplayStreamUpdateCreateMergedUpdate(CGDisplayStreamUpdateRef firstUpdate, CGDisplayStreamUpdateRef secondUpdate);
COREGRAPHICS_EXPORT void CGDisplayStreamUpdateGetMovedRectsDelta(CGDisplayStreamUpdateRef updateRef, CGFloat *dx, CGFloat *dy);
COREGRAPHICS_EXPORT size_t CGDisplayStreamUpdateGetDropCount(CGDisplayStreamUpdateRef updateRef);
COREGRAPHICS_EXPORT CFTypeID CGDisplayStreamGetTypeID(void);
COREGRAPHICS_EXPORT CFTypeID CGDisplayStreamUpdateGetTypeID(void);


