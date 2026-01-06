/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_IOGRAPHICSTYPES_H
#define _IOKIT_IOGRAPHICSTYPES_H


#include <IOKit/IOTypes.h>
#include <IOKit/IOKitKeys.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOGRAPHICSTYPES_REV     71

typedef SInt32  IOIndex;
typedef UInt32  IOSelect;
typedef UInt32  IOFixed1616;
typedef UInt32  IODisplayVendorID;
typedef UInt32  IODisplayProductID;

typedef SInt32  IODisplayModeID;
enum {
    // This is the ID given to a programmable timing used at boot time
    kIODisplayModeIDBootProgrammable = (IODisplayModeID)0xFFFFFFFB,
    // Lowest (unsigned) DisplayModeID reserved by Apple
    kIODisplayModeIDReservedBase     = (IODisplayModeID)0x80000000
};

enum {
    kIOMaxPixelBits     = 64
};
typedef char IOPixelEncoding[ kIOMaxPixelBits ];

// Common Apple pixel formats

#define IO1BitIndexedPixels     "P"
#define IO2BitIndexedPixels     "PP"
#define IO4BitIndexedPixels     "PPPP"
#define IO8BitIndexedPixels     "PPPPPPPP"
#define IO16BitDirectPixels     "-RRRRRGGGGGBBBBB"
#define IO32BitDirectPixels     "--------RRRRRRRRGGGGGGGGBBBBBBBB"

#define kIO30BitDirectPixels    "--RRRRRRRRRRGGGGGGGGGGBBBBBBBBBB"
#define kIO64BitDirectPixels    "-16R16G16B16"

#define kIO16BitFloatPixels     "-16FR16FG16FB16"
#define kIO32BitFloatPixels     "-32FR32FG32FB32"

// other possible pixel formats

#define IOYUV422Pixels          "Y4U2V2"
#define IO8BitOverlayPixels     "O8"
// page flipping
#define IOPagedPixels           "Page1"

#define IO_SampleTypeAlpha      'A'
#define IO_SampleTypeSkip       '-'

// Info about a pixel format
enum {
    kIOCLUTPixels                   = 0,
    kIOFixedCLUTPixels              = 1,
    kIORGBDirectPixels              = 2,
    kIOMonoDirectPixels             = 3,
    kIOMonoInverseDirectPixels      = 4,
    kIORGBSignedDirectPixels        = 5,
    kIORGBSignedFloatingPointPixels = 6
};

/*!
 * @struct IOPixelInformation
 * @abstract A structure defining the format of a framebuffer.
 * @discussion This structure is used by IOFramebuffer to define the format of the pixels.
 * @field bytesPerRow The number of bytes per row.
 * @field bytesPerPlane Not used.
 * @field bitsPerPixel The number of bits per pixel, including unused bits and alpha.
 * @field pixelType One of kIOCLUTPixels (indexed pixels with changeable CLUT), kIORGBDirectPixels (direct pixels).
 * @field componentCount One for indexed pixels, three for direct pixel formats.
 * @field bitsPerComponent Number of bits per component in each pixel.
 * @field componentMasks Mask of the bits valid for each component of the pixel - in R, G, B order for direct pixels.
 * @field pixelFormat String description of the pixel format - IO32BitDirectPixels, IO16BitDirectPixels etc.
 * @field flags None defined - set to zero.
 * @field activeWidth Number of pixels visible per row.
 * @field activeHeight Number of visible pixel rows.
 * @field reserved Set to zero.
 */

struct IOPixelInformation {
    UInt32                      bytesPerRow;
    UInt32                      bytesPerPlane;
    UInt32                      bitsPerPixel;
    UInt32                      pixelType;
    UInt32                      componentCount;
    UInt32                      bitsPerComponent;
    UInt32                      componentMasks[ 8 * 2 ];
    IOPixelEncoding             pixelFormat;
    UInt32                      flags;
    UInt32                      activeWidth;
    UInt32                      activeHeight;
    UInt32                      reserved[ 2 ];
};
typedef struct IOPixelInformation IOPixelInformation;

// ID for industry standard display timings
typedef UInt32  IOAppleTimingID;

/*!
 * @struct IODisplayModeInformation
 * @abstract A structure defining the format of a framebuffer.
 * @discussion This structure is used by IOFramebuffer to define the format of the pixels.
 * @field nominalWidth Number of pixels visible per row.
 * @field nominalHeight Number of visible pixel rows.
 * @field refreshRate Refresh rate in fixed point 16.16.
 * @field maxDepthIndex Highest depth index available in this display mode.
 * @field flags Flags for the mode, including: <br>
 *   kDisplayModeInterlacedFlag mode is interlaced. <br>
 *   kDisplayModeSimulscanFlag mode is available on multiple display connections. <br>
 *   kDisplayModeNotPresetFlag mode is not a factory preset for the display (geometry may need correction). <br>
 *   kDisplayModeStretchedFlag mode is stretched/distorted to match the display aspect ratio. <br>
 * @field imageWidth Physical width of active image if known, in millimeters, otherwise zero. <br>
 * @field imageHeight Physical height of active image if known, in millimeters, otherwise zero. <br>
 * @field reserved Set to zero.
 */

struct IODisplayModeInformation {
    UInt32                      nominalWidth;
    UInt32                      nominalHeight;
    IOFixed1616                 refreshRate;
    IOIndex                     maxDepthIndex;
    UInt32                      flags;
    UInt16						imageWidth;
    UInt16						imageHeight;
    UInt32                      reserved[ 3 ];
};
typedef struct IODisplayModeInformation IODisplayModeInformation;

// flags
enum {
    kDisplayModeSafetyFlags             = 0x00000007,

    kDisplayModeAlwaysShowFlag          = 0x00000008,
    kDisplayModeNeverShowFlag           = 0x00000080,
    kDisplayModeNotResizeFlag           = 0x00000010,
    kDisplayModeRequiresPanFlag         = 0x00000020,

    kDisplayModeInterlacedFlag          = 0x00000040,

    kDisplayModeSimulscanFlag           = 0x00000100,
    kDisplayModeBuiltInFlag             = 0x00000400,
    kDisplayModeNotPresetFlag           = 0x00000200,
    kDisplayModeStretchedFlag           = 0x00000800,
    kDisplayModeNotGraphicsQualityFlag  = 0x00001000,
    kDisplayModeValidateAgainstDisplay  = 0x00002000,
    kDisplayModeTelevisionFlag          = 0x00100000,
    kDisplayModeValidForMirroringFlag   = 0x00200000,
    kDisplayModeAcceleratorBackedFlag   = 0x00400000,
    kDisplayModeValidForHiResFlag       = 0x00800000,
    kDisplayModeValidForAirPlayFlag     = 0x01000000,
    kDisplayModeNativeFlag              = 0x02000000
};
enum {
    kDisplayModeValidFlag               = 0x00000001,
    kDisplayModeSafeFlag                = 0x00000002,
    kDisplayModeDefaultFlag             = 0x00000004
};

#ifndef KERNEL
// Framebuffer info - obsolete

struct IOFramebufferInformation {
    IOPhysicalAddress           baseAddress;
    UInt32                      activeWidth;
    UInt32                      activeHeight;
    IOByteCount                 bytesPerRow;
    IOByteCount                 bytesPerPlane;
    UInt32                      bitsPerPixel;
    UInt32                      pixelType;
    UInt32                      flags;
    UInt32                      reserved[ 4 ];
};
typedef struct IOFramebufferInformation IOFramebufferInformation;
#endif

// flags
enum {
    kFramebufferSupportsCopybackCache   = 0x00010000,
    kFramebufferSupportsWritethruCache  = 0x00020000,
    kFramebufferSupportsGammaCorrection = 0x00040000,
    kFramebufferDisableAltivecAccess    = 0x00080000
};

// Aperture is an index into supported pixel formats for a mode & depth
typedef IOIndex IOPixelAperture;
enum {
    kIOFBSystemAperture = 0
};

//// CLUTs

// IOFBSetGamma Sync Types
#define kIOFBSetGammaSyncNotSpecified       -1
#define kIOFBSetGammaSyncNoSync             0
#define kIOFBSetGammaSyncVerticalBlankSync  1

typedef UInt16 IOColorComponent;

/*!
 * @struct IOColorEntry
 * @abstract A structure defining one entry of a color lookup table.
 * @discussion This structure is used by IOFramebuffer to define an entry of a color lookup table.
 * @field index Number of pixels visible per row.
 * @field red Value of red component 0-65535.
 * @field green Value of green component 0-65535.
 * @field blue Value of blue component 0-65535.
 */

struct IOColorEntry {
    UInt16                      index;
    IOColorComponent            red;
    IOColorComponent            green;
    IOColorComponent            blue;
};
typedef struct IOColorEntry IOColorEntry;

// options (masks)
enum {
    kSetCLUTByValue             = 0x00000001,           // else at index
    kSetCLUTImmediately         = 0x00000002,           // else at VBL
    kSetCLUTWithLuminance       = 0x00000004            // else RGB
};

//// Controller attributes

enum {
    kIOPowerStateAttribute              = 'pwrs',
    kIOPowerAttribute                   = 'powr',
    kIODriverPowerAttribute             = 'dpow',
    kIOHardwareCursorAttribute          = 'crsr',

    kIOMirrorAttribute                  = 'mirr',
    kIOMirrorDefaultAttribute           = 'mrdf',

    kIOCapturedAttribute                = 'capd',

    kIOCursorControlAttribute           = 'crsc',

    kIOSystemPowerAttribute             = 'spwr',
    kIOWindowServerActiveAttribute      = 'wsrv',
    kIOVRAMSaveAttribute                = 'vrsv',
    kIODeferCLUTSetAttribute            = 'vclt',

    kIOClamshellStateAttribute          = 'clam',

    kIOFBDisplayPortTrainingAttribute   = 'dpta',

    kIOFBDisplayState                   = 'dstt',

    kIOFBVariableRefreshRate            = 'vrr?',

    kIOFBLimitHDCPAttribute             = 'hdcp',
    kIOFBLimitHDCPStateAttribute        = 'sHDC',

    kIOFBStop                           = 'stop',

    kIOFBRedGammaScaleAttribute         = 'gslr',    // as of IOGRAPHICSTYPES_REV 54
    kIOFBGreenGammaScaleAttribute       = 'gslg',    // as of IOGRAPHICSTYPES_REV 54
    kIOFBBlueGammaScaleAttribute        = 'gslb',    // as of IOGRAPHICSTYPES_REV 54

    kIOFBHDRMetaDataAttribute           = 'hdrm',    // as of IOGRAPHICSTYPES_REV 64

    kIOBuiltinPanelPowerAttribute       = 'pnlp',    // as of IOGRAPHICSTYPES_REV 71
};

enum {
    kIOFBHDCPLimit_AllowAll             = 0,
    kIOFBHDCPLimit_NoHDCP1x             = 1 << 0,
    kIOFBHDCPLimit_NoHDCP20Type0        = 1 << 1,
    kIOFBHDCPLimit_NoHDCP20Type1        = 1 << 2,   // Default case
};

// <rdar://problem/34574357> kIOFBHDRMetaDataAttribute
struct IOFBHDRMetaDataV1
{
    uint16_t            displayPrimary_X0;      // X coordinate of red primary
    uint16_t            displayPrimary_Y0;      // Y coordinate of red primary
    uint16_t            displayPrimary_X1;      // X coordinate of green primary
    uint16_t            displayPrimary_Y1;      // Y coordinate of green primary
    uint16_t            displayPrimary_X2;      // X coordinate of blue primary
    uint16_t            displayPrimary_Y2;      // Y coordinate of blue primary
    uint16_t            displayPrimary_X;       // X coordinate of white primary
    uint16_t            displayPrimary_Y;       // Y coordinate of white primary
    uint16_t            desiredLuminance_Max;   // Desired max display luminance
    uint16_t            desiredLuminance_Min;   // Desired min display luminance
    uint16_t            desiredLightLevel_Avg;  // Desired max frame-average light level
    uint16_t            desiredLightLevel_Max;  // Desired max light level

    uint64_t            __reservedA[5];         // Reserved - set to zero
};
typedef struct IOFBHDRMetaDataV1 IOFBHDRMetaDataV1;
typedef union {
    IOFBHDRMetaDataV1   v1;
} IOFBHDRMetaData;

// <rdar://problem/29184178> IOGraphics: Implement display state attribute for deteriming display state post wake
// kIOFBDisplayState
enum {
    kIOFBDisplayState_AlreadyActive     = (1 << 0),
    kIOFBDisplayState_RestoredProfile   = (1 << 1),
    kIOFBDisplayState_PipelineBlack     = (1 << 2),
    kIOFBDisplayState_Mask              = (kIOFBDisplayState_AlreadyActive |
                                           kIOFBDisplayState_RestoredProfile |
                                           kIOFBDisplayState_PipelineBlack)
};

// values for kIOWindowServerActiveAttribute
enum {
    // States
    kIOWSAA_Unaccelerated       = 0,    // CPU rendering/access only, no GPU access
    kIOWSAA_Accelerated         = 1,    // GPU rendering/access only, no CPU mappings
    kIOWSAA_From_Accelerated    = 2,    // Transitioning from GPU to CPU
    kIOWSAA_To_Accelerated      = 3,    // Transitioning from CPU to GPU
    kIOWSAA_Sleep               = 4,
    kIOWSAA_Hibernate           = kIOWSAA_Sleep,
    kIOWSAA_DriverOpen          = 5,    // Reserved
    kIOWSAA_StateMask           = 0xF,
    // Bits
    kIOWSAA_Transactional       = 0x10,  // If this bit is present, transition is to/from transactional operation model.
    // These attributes are internal
    kIOWSAA_DeferStart          = 0x100,
    kIOWSAA_DeferEnd            = 0x200,
    kIOWSAA_NonConsoleDevice    = 0x400,    // If present, associated FB is non-console.  See ERS for further details.
    kIOWSAA_Reserved            = 0xF0000000
};

// IOFBNS prefix is IOFramebuffer notifyServer
enum {
    kIOFBNS_Rendezvous           = 0x87654321,  // Note sign-bit is 1 here.

    kIOFBNS_MessageMask         = 0x0000000f,
    kIOFBNS_Sleep               = 0x00,
    kIOFBNS_Wake                = 0x01,
    kIOFBNS_Doze                = 0x02,
#ifndef _OPEN_SOURCE_
    // <rdar://problem/39199290> IOGraphics: Add 10 second timeout to IODW DIM
    // policy.
    // Enum of Bitfields for Windows server notification msgh_id, which is a
    // integer_t signed type.
    kIOFBNS_Dim                 = 0x03,
    kIOFBNS_UnDim               = 0x04,
#endif // !_OPEN_SOURCE_

    // For Wake messages this field contains the current kIOFBDisplayState as
    // returned by attribute 'kIOFBDisplayState'
    kIOFBNS_DisplayStateMask    = 0x00000f00,
    kIOFBNS_DisplayStateShift   = 8,

    // Message Generation Count, top-bit i.e. sign-bit is always 0 for normal
    // messages, see kIOFBNS_Rendezvous is the only exception and it doesn't
    // encode a generation count.
    kIOFBNS_GenerationMask      = 0x7fff0000,
    kIOFBNS_GenerationShift     = 16,
};

// values for kIOMirrorAttribute
enum {
    kIOMirrorIsPrimary                  = 0x80000000,
    kIOMirrorHWClipped                  = 0x40000000,
    kIOMirrorIsMirrored                 = 0x20000000
};

// values for kIOMirrorDefaultAttribute
enum {
    kIOMirrorDefault                    = 0x00000001,
    kIOMirrorForced                     = 0x00000002
};

//// Display mode timing information

struct IODetailedTimingInformationV1 {
    // from EDID defn
    UInt32                      pixelClock;             // Hertz
    UInt32                      horizontalActive;       // pixels
    UInt32                      horizontalBlanking;     // pixels
    UInt32                      horizontalBorder;       // pixels
    UInt32                      horizontalSyncOffset;   // pixels
    UInt32                      horizontalSyncWidth;    // pixels
    UInt32                      verticalActive;         // lines
    UInt32                      verticalBlanking;       // lines
    UInt32                      verticalBorder;         // lines
    UInt32                      verticalSyncOffset;     // lines
    UInt32                      verticalSyncWidth;      // lines
};
typedef struct IODetailedTimingInformationV1 IODetailedTimingInformationV1;

/*!
 * @struct IODetailedTimingInformationV2
 * @abstract A structure defining the detailed timing information of a display mode.
 * @discussion This structure is used by IOFramebuffer to define detailed timing information for a display mode. The VESA EDID document has more information.
 * @field __reservedA Set to zero.
 * @field horizontalScaledInset If the mode is scaled, sets the number of active pixels to remove the left and right edges in order to display an underscanned image.
 * @field verticalScaledInset If the mode is scaled, sets the number of active lines to remove the top and bottom edges in order to display an underscanned image.
 * @field scalerFlags If the mode is scaled,
 *    kIOScaleStretchToFit may be set to allow stretching.
 *    kIOScaleRotateFlags is mask which may have the value given by kIOScaleRotate90, kIOScaleRotate180, kIOScaleRotate270 to display a rotated framebuffer.
 * @field horizontalScaled If the mode is scaled, sets the size of the image before scaling or rotation.
 * @field verticalScaled If the mode is scaled, sets the size of the image before scaling or rotation.
 * @field signalConfig 
 *    kIOAnalogSetupExpected set if display expects a blank-to-black setup or pedestal.  See VESA signal standards. <br>
 *    kIOInterlacedCEATiming set for a CEA style interlaced timing:<br>
 *      Field 1 vertical blanking = half specified vertical blanking lines. <br>
 *      Field 2 vertical blanking = (half vertical blanking lines) + 1 line. <br>
 *      Field 1 vertical offset = half specified vertical sync offset. <br>
 *      Field 2 vertical offset = (half specified vertical sync offset) + 0.5 lines. <br>
 * @field signalLevels One of:<br>
 *   kIOAnalogSignalLevel_0700_0300 0.700 - 0.300 V p-p.<br>
 *   kIOAnalogSignalLevel_0714_0286 0.714 - 0.286 V p-p.<br>
 *   kIOAnalogSignalLevel_1000_0400 1.000 - 0.400 V p-p.<br>
 *   kIOAnalogSignalLevel_0700_0000 0.700 - 0.000 V p-p.<br>
 * @field pixelClock Pixel clock frequency in Hz.
 * @field minPixelClock Minimum pixel clock frequency in Hz, with error.
 * @field maxPixelClock Maximum pixel clock frequency in Hz, with error.
 * @field horizontalActive Pixel clocks per line.
 * @field horizontalBlanking Blanking clocks per line.
 * @field horizontalSyncOffset First clock of horizontal sync.
 * @field horizontalSyncPulseWidth Width of horizontal sync.
 * @field verticalActive Number of lines per frame.
 * @field verticalBlanking Blanking lines per frame.
 * @field verticalSyncOffset First line of vertical sync.
 * @field verticalSyncPulseWidth Height of vertical sync.
 * @field horizontalBorderLeft Number of pixels in left horizontal border.
 * @field horizontalBorderRight Number of pixels in right horizontal border.
 * @field verticalBorderTop Number of lines in top vertical border.
 * @field verticalBorderBottom Number of lines in bottom vertical border.
 * @field horizontalSyncConfig kIOSyncPositivePolarity for positive polarity horizontal sync (0 for negative).
 * @field horizontalSyncLevel Zero.
 * @field verticalSyncConfig kIOSyncPositivePolarity for positive polarity vertical sync (0 for negative).
 * @field verticalSyncLevel Zero.
 * @field numLinks number of links to be used by a dual link timing, if zero, assume one link.
 * @field verticalBlankingExtension maximum number of blanking extension lines that is available. (0 for none).
 * @field pixelEncoding 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field bitsPerColorComponent 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field colorimetry 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field dynamicRange 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field dscCompressedBitsPerPixel 2018 Timing Features - ERS 2-63 (6.3.1)
 * @field dscSliceHeight 2018 Timing Features - ERS 2-63 (6.3.1)
 * @field dscSliceWidth 2018 Timing Features - ERS 2-63 (6.3.1)
 * @field __reservedB Reserved set to zero.
 */

struct IODetailedTimingInformationV2 {

    UInt32      __reservedA[3];                 // Init to 0
    UInt32      horizontalScaledInset;          // pixels
    UInt32      verticalScaledInset;            // lines

    UInt32      scalerFlags;
    UInt32      horizontalScaled;
    UInt32      verticalScaled;

    UInt32      signalConfig;
    UInt32      signalLevels;

    UInt64      pixelClock;                     // Hz

    UInt64      minPixelClock;                  // Hz - With error what is slowest actual clock
    UInt64      maxPixelClock;                  // Hz - With error what is fasted actual clock

    UInt32      horizontalActive;               // pixels
    UInt32      horizontalBlanking;             // pixels
    UInt32      horizontalSyncOffset;           // pixels
    UInt32      horizontalSyncPulseWidth;       // pixels

    UInt32      verticalActive;                 // lines
    UInt32      verticalBlanking;               // lines
    UInt32      verticalSyncOffset;             // lines
    UInt32      verticalSyncPulseWidth;         // lines

    UInt32      horizontalBorderLeft;           // pixels
    UInt32      horizontalBorderRight;          // pixels
    UInt32      verticalBorderTop;              // lines
    UInt32      verticalBorderBottom;           // lines

    UInt32      horizontalSyncConfig;
    UInt32      horizontalSyncLevel;            // Future use (init to 0)
    UInt32      verticalSyncConfig;
    UInt32      verticalSyncLevel;              // Future use (init to 0)
    UInt32      numLinks;

    UInt32      verticalBlankingExtension;      // lines (AdaptiveSync: 0 for non-AdaptiveSync support)

    UInt16      pixelEncoding;
    UInt16      bitsPerColorComponent;
    UInt16      colorimetry;
    UInt16      dynamicRange;
    
    UInt16      dscCompressedBitsPerPixel;
    UInt16      dscSliceHeight;
    UInt16      dscSliceWidth;
    UInt16      __reservedB[5];                 // Init to 0
};
typedef struct IODetailedTimingInformationV2 IODetailedTimingInformationV2;
typedef struct IODetailedTimingInformationV2 IODetailedTimingInformation;

struct IOTimingInformation {
    IOAppleTimingID             appleTimingID;  // kIOTimingIDXXX const
    UInt32                      flags;
    union {
      IODetailedTimingInformationV1     v1;
      IODetailedTimingInformationV2     v2;
    }                           detailedInfo;
};
typedef struct IOTimingInformation IOTimingInformation;

enum {
    // IOTimingInformation flags
    kIODetailedTimingValid      = 0x80000000,
    kIOScalingInfoValid         = 0x40000000
};

enum {
    // scalerFlags
    kIOScaleStretchToFit        = 0x00000001,

    kIOScaleRotateFlags         = 0x000000f0,

    kIOScaleSwapAxes            = 0x00000010,
    kIOScaleInvertX             = 0x00000020,
    kIOScaleInvertY             = 0x00000040,

    kIOScaleRotate0             = 0x00000000,
    kIOScaleRotate90            = kIOScaleSwapAxes | kIOScaleInvertX,
    kIOScaleRotate180           = kIOScaleInvertX  | kIOScaleInvertY,
    kIOScaleRotate270           = kIOScaleSwapAxes | kIOScaleInvertY
};

enum {
    kIOPixelEncodingNotSupported    = 0x0000,
    kIOPixelEncodingRGB444          = 0x0001,
    kIOPixelEncodingYCbCr444        = 0x0002,
    kIOPixelEncodingYCbCr422        = 0x0004,
    kIOPixelEncodingYCbCr420        = 0x0008
};

enum {
    kIOBitsPerColorComponentNotSupported    = 0x0000,
    kIOBitsPerColorComponent6               = 0x0001,
    kIOBitsPerColorComponent8               = 0x0002,
    kIOBitsPerColorComponent10              = 0x0004,
    kIOBitsPerColorComponent12              = 0x0008,
    kIOBitsPerColorComponent16              = 0x0010
};

enum {
    kIOColorimetryNotSupported  = 0x0000,
    kIOColorimetryNativeRGB     = 0x0001,
    kIOColorimetrysRGB          = 0x0002,
    kIOColorimetryDCIP3         = 0x0004,
    kIOColorimetryAdobeRGB      = 0x0008,
    kIOColorimetryxvYCC         = 0x0010,
    kIOColorimetryWGRGB         = 0x0020,
    kIOColorimetryBT601         = 0x0040,
    kIOColorimetryBT709         = 0x0080,
    kIOColorimetryBT2020        = 0x0100,
    kIOColorimetryBT2100        = 0x0200
};

enum {
    kIODynamicRangeNotSupported         = 0x0000,
    kIODynamicRangeSDR                  = 0x0001,
    kIODynamicRangeHDR10                = 0x0002,
    kIODynamicRangeDolbyNormalMode      = 0x0004,
    kIODynamicRangeDolbyTunnelMode      = 0x0008,
    kIODynamicRangeTraditionalGammaHDR  = 0x0010
};

#pragma pack(push, 4)
struct IOFBDisplayModeDescription {
    IODisplayModeInformation    info;
    IOTimingInformation         timingInfo;
};
typedef struct IOFBDisplayModeDescription IOFBDisplayModeDescription;
#pragma pack(pop)

/*!
 * @struct IODisplayTimingRangeV1
 * @abstract A structure defining the limits and attributes of a display or framebuffer.
 * @discussion This structure is used to define the limits for modes programmed as detailed timings by the OS. The VESA EDID is useful background information for many of these fields. A data property with this structure under the key kIOFBTimingRangeKey in a framebuffer will allow the OS to program detailed timings that fall within its range.
 * @field __reservedA Set to zero.
 * @field version Set to zero.
 * @field __reservedB Set to zero.
 * @field minPixelClock minimum pixel clock frequency in range, in Hz.
 * @field minPixelClock maximum pixel clock frequency in range, in Hz.
 * @field maxPixelError largest variation between specified and actual pixel clock frequency, in Hz.
 * @field supportedSyncFlags mask of supported sync attributes. The following are defined:<br>
 *    kIORangeSupportsSeparateSyncs - digital separate syncs.<br>
 *    kIORangeSupportsSyncOnGreen - sync on green.<br>
 *    kIORangeSupportsCompositeSync - composite sync.<br>
 *    kIORangeSupportsVSyncSerration - vertical sync has serration and equalization pulses.<br>
 * @field supportedSignalLevels mask of possible signal levels. The following are defined:<br>
 *    kIORangeSupportsSignal_0700_0300 0.700 - 0.300 V p-p.<br>
 *    kIORangeSupportsSignal_0714_0286 0.714 - 0.286 V p-p.<br>
 *    kIORangeSupportsSignal_1000_0400 1.000 - 0.400 V p-p.<br>
 *    kIORangeSupportsSignal_0700_0000 0.700 - 0.000 V p-p.<br>
 * @field supportedSignalConfigs mask of possible signal configurations. The following are defined:<br>
 *    kIORangeSupportsInterlacedCEATiming Supports CEA style interlaced timing:<br>
 *      Field 1 vertical blanking = specified vertical blanking lines. <br>
 *      Field 2 vertical blanking = vertical blanking lines + 1 line. <br> 
 *      Field 1 vertical offset = specified vertical sync offset. <br>
 *      Field 2 vertical offset = specified vertical sync offset + 0.5 lines. <br>
 *    kIORangeSupportsInterlacedCEATimingWithConfirm Supports CEA style interlaced timing, but require a confirm.
 * @field minFrameRate minimum frame rate (vertical refresh frequency) in range, in Hz.
 * @field maxFrameRate maximum frame rate (vertical refresh frequency) in range, in Hz.
 * @field minLineRate minimum line rate (horizontal refresh frequency) in range, in Hz.
 * @field maxLineRate maximum line rate (horizontal refresh frequency) in range, in Hz.
 * @field maxHorizontalTotal maximum clocks in horizontal line (active + blanking).
 * @field maxVerticalTotal maximum lines in vertical frame (active + blanking).
 * @field __reservedD Set to zero.
 * @field charSizeHorizontalActive horizontalActive must be a multiple of charSizeHorizontalActive.
 * @field charSizeHorizontalBlanking horizontalBlanking must be a multiple of charSizeHorizontalBlanking.
 * @field charSizeHorizontalSyncOffset horizontalSyncOffset must be a multiple of charSizeHorizontalSyncOffset.
 * @field charSizeHorizontalSyncPulse horizontalSyncPulse must be a multiple of charSizeHorizontalSyncPulse.
 * @field charSizeVerticalActive verticalActive must be a multiple of charSizeVerticalActive.
 * @field charSizeVerticalBlanking verticalBlanking must be a multiple of charSizeVerticalBlanking.
 * @field charSizeVerticalSyncOffset verticalSyncOffset must be a multiple of charSizeVerticalSyncOffset.
 * @field charSizeVerticalSyncPulse verticalSyncPulse must be a multiple of charSizeVerticalSyncPulse.
 * @field charSizeHorizontalBorderLeft horizontalBorderLeft must be a multiple of charSizeHorizontalBorderLeft.
 * @field charSizeHorizontalBorderRight horizontalBorderRight must be a multiple of charSizeHorizontalBorderRight.
 * @field charSizeVerticalBorderTop verticalBorderTop must be a multiple of charSizeVerticalBorderTop.
 * @field charSizeVerticalBorderBottom verticalBorderBottom must be a multiple of charSizeVerticalBorderBottom.
 * @field charSizeHorizontalTotal (horizontalActive + horizontalBlanking) must be a multiple of charSizeHorizontalTotal.
 * @field charSizeVerticalTotal (verticalActive + verticalBlanking) must be a multiple of charSizeVerticalTotal.
 * @field __reservedE Set to zero.
 * @field minHorizontalActiveClocks minimum value of horizontalActive.
 * @field maxHorizontalActiveClocks maximum value of horizontalActive.
 * @field minHorizontalBlankingClocks minimum value of horizontalBlanking.
 * @field maxHorizontalBlankingClocks maximum value of horizontalBlanking.
 * @field minHorizontalSyncOffsetClocks minimum value of horizontalSyncOffset.
 * @field maxHorizontalSyncOffsetClocks maximum value of horizontalSyncOffset.
 * @field minHorizontalPulseWidthClocks minimum value of horizontalPulseWidth.
 * @field maxHorizontalPulseWidthClocks maximum value of horizontalPulseWidth.
 * @field minVerticalActiveClocks minimum value of verticalActive.
 * @field maxVerticalActiveClocks maximum value of verticalActive.
 * @field minVerticalBlankingClocks minimum value of verticalBlanking.
 * @field maxVerticalBlankingClocks maximum value of verticalBlanking.
 * @field minVerticalSyncOffsetClocks minimum value of verticalSyncOffset.
 * @field maxVerticalSyncOffsetClocks maximum value of verticalSyncOffset.
 * @field minVerticalPulseWidthClocks minimum value of verticalPulseWidth.
 * @field maxVerticalPulseWidthClocks maximum value of verticalPulseWidth.
 * @field minHorizontalBorderLeft minimum value of horizontalBorderLeft.
 * @field maxHorizontalBorderLeft maximum value of horizontalBorderLeft.
 * @field minHorizontalBorderRight minimum value of horizontalBorderRight.
 * @field maxHorizontalBorderRight maximum value of horizontalBorderRight.
 * @field minVerticalBorderTop minimum value of verticalBorderTop.
 * @field maxVerticalBorderTop maximum value of verticalBorderTop.
 * @field minVerticalBorderBottom minimum value of verticalBorderBottom.
 * @field maxVerticalBorderBottom maximum value of verticalBorderBottom.
 * @field maxNumLinks number of links supported, if zero, 1 link is assumed.
 * @field minLink0PixelClock minimum pixel clock for link 0 (kHz).
 * @field maxLink0PixelClock maximum pixel clock for link 0 (kHz).
 * @field minLink1PixelClock minimum pixel clock for link 1 (kHz).
 * @field maxLink1PixelClock maximum pixel clock for link 1 (kHz).
 * @field supportedPixelEncoding 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field supportedBitsPerColorComponent 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field supportedColorimetry 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field supportedDynamicRange 2017 Timing Features - ERS 2-58 (6.3.1)
 * @field __reservedF Set to zero.
 */

struct IODisplayTimingRangeV1
{
    UInt32      __reservedA[2];                 // Init to 0
    UInt32      version;                        // Init to 0
    UInt32      __reservedB[5];                 // Init to 0

    UInt64      minPixelClock;                  // Min dot clock in Hz
    UInt64      maxPixelClock;                  // Max dot clock in Hz

    UInt32      maxPixelError;                  // Max dot clock error
    UInt32      supportedSyncFlags;
    UInt32      supportedSignalLevels;
    UInt32      supportedSignalConfigs;

    UInt32      minFrameRate;                   // Hz
    UInt32      maxFrameRate;                   // Hz
    UInt32      minLineRate;                    // Hz
    UInt32      maxLineRate;                    // Hz

    UInt32      maxHorizontalTotal;             // Clocks - Maximum total (active + blanking)
    UInt32      maxVerticalTotal;               // Clocks - Maximum total (active + blanking)
    UInt32      __reservedD[2];                 // Init to 0

    UInt8       charSizeHorizontalActive;
    UInt8       charSizeHorizontalBlanking;     
    UInt8       charSizeHorizontalSyncOffset; 
    UInt8       charSizeHorizontalSyncPulse; 

    UInt8       charSizeVerticalActive;         
    UInt8       charSizeVerticalBlanking;       
    UInt8       charSizeVerticalSyncOffset;     
    UInt8       charSizeVerticalSyncPulse;      

    UInt8       charSizeHorizontalBorderLeft; 
    UInt8       charSizeHorizontalBorderRight; 
    UInt8       charSizeVerticalBorderTop;      
    UInt8       charSizeVerticalBorderBottom; 

    UInt8       charSizeHorizontalTotal;                // Character size for active + blanking
    UInt8       charSizeVerticalTotal;                  // Character size for active + blanking
    UInt16      __reservedE;                            // Reserved (Init to 0)

    UInt32      minHorizontalActiveClocks;
    UInt32      maxHorizontalActiveClocks;
    UInt32      minHorizontalBlankingClocks;
    UInt32      maxHorizontalBlankingClocks;

    UInt32      minHorizontalSyncOffsetClocks;
    UInt32      maxHorizontalSyncOffsetClocks;
    UInt32      minHorizontalPulseWidthClocks;
    UInt32      maxHorizontalPulseWidthClocks;

    UInt32      minVerticalActiveClocks;
    UInt32      maxVerticalActiveClocks;
    UInt32      minVerticalBlankingClocks;
    UInt32      maxVerticalBlankingClocks;

    UInt32      minVerticalSyncOffsetClocks;
    UInt32      maxVerticalSyncOffsetClocks;
    UInt32      minVerticalPulseWidthClocks;
    UInt32      maxVerticalPulseWidthClocks;

    UInt32      minHorizontalBorderLeft;
    UInt32      maxHorizontalBorderLeft;
    UInt32      minHorizontalBorderRight;
    UInt32      maxHorizontalBorderRight;

    UInt32      minVerticalBorderTop;
    UInt32      maxVerticalBorderTop;
    UInt32      minVerticalBorderBottom;
    UInt32      maxVerticalBorderBottom;
    UInt32      maxNumLinks;                       // number of links, if zero, assume link 1
    UInt32      minLink0PixelClock;                // min pixel clock for link 0 (kHz)
    UInt32      maxLink0PixelClock;                // max pixel clock for link 0 (kHz)
    UInt32      minLink1PixelClock;                // min pixel clock for link 1 (kHz)
    UInt32      maxLink1PixelClock;                // max pixel clock for link 1 (kHz)

    UInt16      supportedPixelEncoding;
    UInt16      supportedBitsPerColorComponent;
    UInt16      supportedColorimetryModes;
    UInt16      supportedDynamicRangeModes;

    UInt32      __reservedF[1];                    // Init to 0
};
    
typedef struct IODisplayTimingRangeV1 IODisplayTimingRangeV1;
/*!
 * @struct IODisplayTimingRangeV2
 * @abstract A structure defining the limits and attributes of DSC capabilities in a  framebuffer.
 * @discussion This structure is used to define the limits for DSC enabled modes programmed as detailed timings by the OS. The VESA DSC spec is useful background information for many of these fields.
 * @field maxBandwidth Maximum permitted bandwidth of the given topology in bits per second.
 * @field dscMinSliceHeight Minimum slice Height, in units of line.
 * @field dscMaxSliceHeight Maximum slice Height, in units of line.
 * @field dscMinSliceWidth  Minimum slice width, in units of line.
 * @field dscMaxSliceWidth  Maximum slice width, in units of line.
 * @field dscMinSlicePerLine Minimum slice per Line.
 * @field dscMaxSlicePerLine Maximum slice per Line.
 * @field dscMinBPC  Minimum Bits per component, in units of bits.
 * @field dscMaxBPC  Maximum Bits per component, in units of bits.
 * @field dscMinBPP  Minimum target bits/pixel, in bpp.
 * @field dscMaxBPP  Maximum target bits/pixel, in bpp.
 * @field dscVBR     VBR mode, 0:disabled 1:enabled.
 * @field dscBlockPredEnable  DSC BP is user or not, 0: not used, 1: used.
 * @field __reservedF  Set to zero.
 */
struct IODisplayTimingRangeV2
{
    UInt32      __reservedA[2];                 // Init to 0
    UInt32      version;                        // Init to 0
    UInt32      __reservedB[5];                 // Init to 0
    
    UInt64      minPixelClock;                  // Min dot clock in Hz
    UInt64      maxPixelClock;                  // Max dot clock in Hz
    
    UInt32      maxPixelError;                  // Max dot clock error
    UInt32      supportedSyncFlags;
    UInt32      supportedSignalLevels;
    UInt32      supportedSignalConfigs;
    
    UInt32      minFrameRate;                   // Hz
    UInt32      maxFrameRate;                   // Hz
    UInt32      minLineRate;                    // Hz
    UInt32      maxLineRate;                    // Hz
    
    UInt32      maxHorizontalTotal;             // Clocks - Maximum total (active + blanking)
    UInt32      maxVerticalTotal;               // Clocks - Maximum total (active + blanking)
    UInt32      __reservedD[2];                 // Init to 0
    
    UInt8       charSizeHorizontalActive;
    UInt8       charSizeHorizontalBlanking;
    UInt8       charSizeHorizontalSyncOffset;
    UInt8       charSizeHorizontalSyncPulse;
    
    UInt8       charSizeVerticalActive;
    UInt8       charSizeVerticalBlanking;
    UInt8       charSizeVerticalSyncOffset;
    UInt8       charSizeVerticalSyncPulse;
    
    UInt8       charSizeHorizontalBorderLeft;
    UInt8       charSizeHorizontalBorderRight;
    UInt8       charSizeVerticalBorderTop;
    UInt8       charSizeVerticalBorderBottom;
    
    UInt8       charSizeHorizontalTotal;                // Character size for active + blanking
    UInt8       charSizeVerticalTotal;                  // Character size for active + blanking
    UInt16      __reservedE;                            // Reserved (Init to 0)
    
    UInt32      minHorizontalActiveClocks;
    UInt32      maxHorizontalActiveClocks;
    UInt32      minHorizontalBlankingClocks;
    UInt32      maxHorizontalBlankingClocks;
    
    UInt32      minHorizontalSyncOffsetClocks;
    UInt32      maxHorizontalSyncOffsetClocks;
    UInt32      minHorizontalPulseWidthClocks;
    UInt32      maxHorizontalPulseWidthClocks;
    
    UInt32      minVerticalActiveClocks;
    UInt32      maxVerticalActiveClocks;
    UInt32      minVerticalBlankingClocks;
    UInt32      maxVerticalBlankingClocks;
    
    UInt32      minVerticalSyncOffsetClocks;
    UInt32      maxVerticalSyncOffsetClocks;
    UInt32      minVerticalPulseWidthClocks;
    UInt32      maxVerticalPulseWidthClocks;
    
    UInt32      minHorizontalBorderLeft;
    UInt32      maxHorizontalBorderLeft;
    UInt32      minHorizontalBorderRight;
    UInt32      maxHorizontalBorderRight;
    
    UInt32      minVerticalBorderTop;
    UInt32      maxVerticalBorderTop;
    UInt32      minVerticalBorderBottom;
    UInt32      maxVerticalBorderBottom;
    UInt32      maxNumLinks;                       // number of links, if zero, assume link 1
    UInt32      minLink0PixelClock;                // min pixel clock for link 0 (kHz)
    UInt32      maxLink0PixelClock;                // max pixel clock for link 0 (kHz)
    UInt32      minLink1PixelClock;                // min pixel clock for link 1 (kHz)
    UInt32      maxLink1PixelClock;                // max pixel clock for link 1 (kHz)
    
    UInt16      supportedPixelEncoding;
    UInt16      supportedBitsPerColorComponent;
    UInt16      supportedColorimetryModes;
    UInt16      supportedDynamicRangeModes;
    
    UInt32      __reservedF[1];                    // Init to 0
    UInt64      maxBandwidth;
    UInt32      dscMinSliceHeight;
    UInt32      dscMaxSliceHeight;
    UInt32      dscMinSliceWidth;
    UInt32      dscMaxSliceWidth;
    UInt32      dscMinSlicePerLine;
    UInt32      dscMaxSlicePerLine;
    UInt16      dscMinBPC;
    UInt16      dscMaxBPC;
    UInt16      dscMinBPP;
    UInt16      dscMaxBPP;
    UInt8       dscVBR;
    UInt8       dscBlockPredEnable;
    UInt32      __reservedC[6];
};
    
typedef struct IODisplayTimingRangeV2 IODisplayTimingRangeV2;


typedef struct IODisplayTimingRangeV2 IODisplayTimingRange;

enum {
    // IOTimingRange version
    kIOTimingRangeV2      = 0x00000002,
    kIOTimingRangeV1      = 0x00000000
};

enum {
    // supportedPixelEncoding
    kIORangePixelEncodingNotSupported   = 0x0000,
    kIORangePixelEncodingRGB444         = 0x0001,
    kIORangePixelEncodingYCbCr444       = 0x0002,
    kIORangePixelEncodingYCbCr422       = 0x0004,
    kIORangePixelEncodingYCbCr420       = 0x0008,
};

enum {
    // supportedBitsPerColorComponent
    kIORangeBitsPerColorComponentNotSupported   = 0x0000,
    kIORangeBitsPerColorComponent6              = 0x0001,
    kIORangeBitsPerColorComponent8              = 0x0002,
    kIORangeBitsPerColorComponent10             = 0x0004,
    kIORangeBitsPerColorComponent12             = 0x0008,
    kIORangeBitsPerColorComponent16             = 0x0010,
};

enum {
    // supportedColorimetry
    kIORangeColorimetryNotSupported     = 0x0000,
    kIORangeColorimetryNativeRGB        = 0x0001,
    kIORangeColorimetrysRGB             = 0x0002,
    kIORangeColorimetryDCIP3            = 0x0004,
    kIORangeColorimetryAdobeRGB         = 0x0008,
    kIORangeColorimetryxvYCC            = 0x0010,
    kIORangeColorimetryWGRGB            = 0x0020,
    kIORangeColorimetryBT601            = 0x0040,
    kIORangeColorimetryBT709            = 0x0080,
    kIORangeColorimetryBT2020           = 0x0100,
    kIORangeColorimetryBT2100           = 0x0200,
};

enum {
    // supportedDynamicRange
    kIORangeDynamicRangeNotSupported        = 0x0000,
    kIORangeDynamicRangeSDR                 = 0x0001,
    kIORangeDynamicRangeHDR10               = 0x0002,
    kIORangeDynamicRangeDolbyNormalMode     = 0x0004,
    kIORangeDynamicRangeDolbyTunnelMode     = 0x0008,
    kIORangeDynamicRangeTraditionalGammaHDR = 0x0010
};

enum {
    // supportedSignalLevels
    kIORangeSupportsSignal_0700_0300    = 0x00000001,
    kIORangeSupportsSignal_0714_0286    = 0x00000002,
    kIORangeSupportsSignal_1000_0400    = 0x00000004,
    kIORangeSupportsSignal_0700_0000    = 0x00000008
};
enum {
    // supportedSyncFlags
    kIORangeSupportsSeparateSyncs        = 0x00000001,
    kIORangeSupportsSyncOnGreen          = 0x00000002,
    kIORangeSupportsCompositeSync        = 0x00000004,
    kIORangeSupportsVSyncSerration       = 0x00000008
};
enum {
    // supportedSignalConfigs
    kIORangeSupportsInterlacedCEATiming            = 0x00000004,
    kIORangeSupportsInterlacedCEATimingWithConfirm = 0x00000008
};

enum {
    // signalConfig
    kIODigitalSignal          = 0x00000001,
    kIOAnalogSetupExpected    = 0x00000002,
    kIOInterlacedCEATiming    = 0x00000004,
    kIONTSCTiming             = 0x00000008,
    kIOPALTiming              = 0x00000010,
    kIODSCBlockPredEnable     = 0x00000020,
};

enum {
    // signalLevels for analog
    kIOAnalogSignalLevel_0700_0300 = 0,
    kIOAnalogSignalLevel_0714_0286 = 1,
    kIOAnalogSignalLevel_1000_0400 = 2,
    kIOAnalogSignalLevel_0700_0000 = 3
};

enum {
    // horizontalSyncConfig and verticalSyncConfig
    kIOSyncPositivePolarity     = 0x00000001
};

/*!
 * @struct IODisplayScalerInformation
 * @abstract A structure defining the scaling capabilities of a framebuffer.
 * @discussion This structure is used to define the limits for modes programmed as detailed timings by the OS. A data property with this structure under the key kIOFBScalerInfoKey in a framebuffer will allow the OS to program detailed timings that are scaled to a displays native resolution.
 * @field __reservedA Set to zero.
 * @field version Set to zero.
 * @field __reservedB Set to zero.
 * @field scalerFeatures Mask of scaling features. The following are defined:<br>
 *   kIOScaleStretchOnly If set the framebuffer can only provide stretched scaling with non-square pixels, without borders.<br>
 *   kIOScaleCanUpSamplePixels If set framebuffer can scale up from a smaller number of source pixels to a larger native timing (eg. 640x480 pixels on a 1600x1200 timing).<br>
 *   kIOScaleCanDownSamplePixels If set framebuffer can scale down from a larger number of source pixels to a smaller native timing (eg. 1600x1200 pixels on a 640x480 timing).<br>
 *   kIOScaleCanScaleInterlaced If set framebuffer can scale an interlaced detailed timing.<br>
 *   kIOScaleCanSupportInset If set framebuffer can support scaled modes with non-zero horizontalScaledInset, verticalScaledInset fields.<br>
 *   kIOScaleCanRotate If set framebuffer can support some of the flags in the kIOScaleRotateFlags mask.<br>
 *   kIOScaleCanBorderInsetOnly If set framebuffer can support scaled modes with non-zero horizontalScaledInset, verticalScaledInset fields, but requires the active pixels to be equal in size to the inset area, ie. can do insets with a border versus scaling an image.<br>
 * @field maxHorizontalPixels Maximum number of horizontal source pixels (horizontalScaled).<br>
 * @field maxVerticalPixels Maximum number of vertical source pixels (verticalScaled).<br>
 * @field __reservedC Set to zero.
 */

struct IODisplayScalerInformation {
    UInt32              __reservedA[1];         // Init to 0
    UInt32              version;                // Init to 0
    UInt32              __reservedB[2];         // Init to 0
    
    IOOptionBits        scalerFeatures;
    UInt32              maxHorizontalPixels;
    UInt32              maxVerticalPixels;
    UInt32              __reservedC[5];         // Init to 0
};
typedef struct IODisplayScalerInformation IODisplayScalerInformation;

enum {
    /* scalerFeatures */
    kIOScaleStretchOnly           = 0x00000001,
    kIOScaleCanUpSamplePixels     = 0x00000002,
    kIOScaleCanDownSamplePixels   = 0x00000004,
    kIOScaleCanScaleInterlaced    = 0x00000008,
    kIOScaleCanSupportInset       = 0x00000010,
    kIOScaleCanRotate             = 0x00000020,
    kIOScaleCanBorderInsetOnly    = 0x00000040
};

//// Connections

enum {
    kOrConnections                      = 0xffffffe,
    kAndConnections                     = 0xffffffd
};

enum {
    kConnectionFlags                    = 'flgs',
    kConnectionSyncEnable               = 'sync',
    kConnectionSyncFlags                = 'sycf',
    kConnectionSupportsAppleSense       = 'asns',
    kConnectionSupportsLLDDCSense       = 'lddc',
    kConnectionSupportsHLDDCSense       = 'hddc',
    kConnectionEnable                   = 'enab',
    kConnectionCheckEnable              = 'cena',
    kConnectionProbe                    = 'prob',
    kConnectionIgnore                   = '\0igr',
    kConnectionChanged                  = 'chng',
    kConnectionPower                    = 'powr',
    kConnectionPostWake                 = 'pwak',
    kConnectionDisplayParameterCount    = 'pcnt',
    kConnectionDisplayParameters        = 'parm',
    kConnectionOverscan                 = 'oscn',
    kConnectionVideoBest                = 'vbst',

    kConnectionRedGammaScale            = 'rgsc',
    kConnectionGreenGammaScale          = 'ggsc',
    kConnectionBlueGammaScale           = 'bgsc',
    kConnectionGammaScale               = 'gsc ',
    kConnectionFlushParameters          = 'flus',

    kConnectionVBLMultiplier            = 'vblm',

    kConnectionHandleDisplayPortEvent   = 'dpir',

    kConnectionPanelTimingDisable       = 'pnlt',

    kConnectionColorMode                = 'cyuv',
    kConnectionColorModesSupported      = 'colr',
    kConnectionColorDepthsSupported     = ' bpc',

    kConnectionControllerDepthsSupported = '\0grd',
    kConnectionControllerColorDepth      = '\0dpd',
    kConnectionControllerDitherControl   = '\0gdc',

    kConnectionDisplayFlags              = 'dflg',

    kConnectionEnableAudio               = 'aud ',
    kConnectionAudioStreaming            = 'auds',

    kConnectionStartOfFrameTime          = 'soft',  // as of IOGRAPHICSTYPES_REV 65
};

// kConnectionFlags values
enum {
    kIOConnectionBuiltIn                = 0x00000800,
    kIOConnectionStereoSync             = 0x00008000
};

// kConnectionSyncControl values
enum {
    kIOHSyncDisable                     = 0x00000001,
    kIOVSyncDisable                     = 0x00000002,
    kIOCSyncDisable                     = 0x00000004,
    kIONoSeparateSyncControl            = 0x00000040,
    kIOTriStateSyncs                    = 0x00000080,
    kIOSyncOnBlue                       = 0x00000008,
    kIOSyncOnGreen                      = 0x00000010,
    kIOSyncOnRed                        = 0x00000020
};

// kConnectionHandleDisplayPortEvent values
enum {
    kIODPEventStart                             = 1,
    kIODPEventIdle                              = 2,

    kIODPEventForceRetrain                      = 3,

    kIODPEventRemoteControlCommandPending       = 256,
    kIODPEventAutomatedTestRequest              = 257,
    kIODPEventContentProtection                 = 258,
    kIODPEventMCCS                              = 259,
    kIODPEventSinkSpecific                      = 260
};

#define kIODisplayAttributesKey         "IODisplayAttributes"

#define kIODisplaySupportsUnderscanKey  "IODisplaySupportsUnderscan"
#define kIODisplaySupportsBasicAudioKey "IODisplaySupportsBasicAudio"
#define kIODisplaySupportsYCbCr444Key   "IODisplaySupportsYCbCr444"
#define kIODisplaySupportsYCbCr422Key   "IODisplaySupportsYCbCr422"
#define kIODisplaySelectedColorModeKey  "cmod"

enum
{ 
    kIODisplayColorMode         = kConnectionColorMode,
};

#if 0
enum
{
    // kConnectionColorMode attribute
    kIODisplayColorModeReserved   = 0x00000000,
    kIODisplayColorModeRGB        = 0x00000001,
    kIODisplayColorModeYCbCr422   = 0x00000010,
    kIODisplayColorModeYCbCr444   = 0x00000100,
    kIODisplayColorModeRGBLimited = 0x00001000,
    kIODisplayColorModeAuto       = 0x10000000,
};
#endif

enum
{ 
    // kConnectionColorDepthsSupported attribute
    kIODisplayRGBColorComponentBitsUnknown       = 0x00000000,
    kIODisplayRGBColorComponentBits6             = 0x00000001,
    kIODisplayRGBColorComponentBits8             = 0x00000002,
    kIODisplayRGBColorComponentBits10            = 0x00000004,
    kIODisplayRGBColorComponentBits12            = 0x00000008,
    kIODisplayRGBColorComponentBits14            = 0x00000010,
    kIODisplayRGBColorComponentBits16            = 0x00000020,

    kIODisplayYCbCr444ColorComponentBitsUnknown  = 0x00000000,
    kIODisplayYCbCr444ColorComponentBits6        = 0x00000100,
    kIODisplayYCbCr444ColorComponentBits8        = 0x00000200,
    kIODisplayYCbCr444ColorComponentBits10       = 0x00000400,
    kIODisplayYCbCr444ColorComponentBits12       = 0x00000800,
    kIODisplayYCbCr444ColorComponentBits14       = 0x00001000,
    kIODisplayYCbCr444ColorComponentBits16       = 0x00002000,

    kIODisplayYCbCr422ColorComponentBitsUnknown  = 0x00000000,
    kIODisplayYCbCr422ColorComponentBits6        = 0x00010000,
    kIODisplayYCbCr422ColorComponentBits8        = 0x00020000,
    kIODisplayYCbCr422ColorComponentBits10       = 0x00040000,
    kIODisplayYCbCr422ColorComponentBits12       = 0x00080000,
    kIODisplayYCbCr422ColorComponentBits14       = 0x00100000,
    kIODisplayYCbCr422ColorComponentBits16       = 0x00200000,
};

enum
{ 
    // kConnectionDitherControl attribute
    kIODisplayDitherDisable          = 0x00000000,
    kIODisplayDitherSpatial          = 0x00000001,
    kIODisplayDitherTemporal         = 0x00000002,
    kIODisplayDitherFrameRateControl = 0x00000004,
    kIODisplayDitherDefault          = 0x00000080,
    kIODisplayDitherAll              = 0x000000FF,
    kIODisplayDitherRGBShift         = 0,
    kIODisplayDitherYCbCr444Shift    = 8,
    kIODisplayDitherYCbCr422Shift    = 16,
};

enum
{ 
    // kConnectionDisplayFlags attribute
    kIODisplayNeedsCEAUnderscan      = 0x00000001,
};

enum
{
	kIODisplayPowerStateOff       =	0,
	kIODisplayPowerStateMinUsable =	1,
	kIODisplayPowerStateOn        = 2,
};

#define IO_DISPLAY_CAN_FILL             0x00000040
#define IO_DISPLAY_CAN_BLIT             0x00000020

#define IO_24BPP_TRANSFER_TABLE_SIZE    256
#define IO_15BPP_TRANSFER_TABLE_SIZE    256
#define IO_8BPP_TRANSFER_TABLE_SIZE     256
#define IO_12BPP_TRANSFER_TABLE_SIZE    256
#define IO_2BPP_TRANSFER_TABLE_SIZE     256

#define STDFB_BM256_TO_BM38_MAP_SIZE    256
#define STDFB_BM38_TO_BM256_MAP_SIZE    256
#define STDFB_BM38_TO_256_WITH_LOGICAL_SIZE     \
        (STDFB_BM38_TO_BM256_MAP_SIZE + (256/sizeof(int)))

#define STDFB_4BPS_TO_5BPS_MAP_SIZE     16
#define STDFB_5BPS_TO_4BPS_MAP_SIZE     32

enum {
    // connection types for IOServiceOpen
    kIOFBServerConnectType              = 0,
    kIOFBSharedConnectType              = 1,
    kIOGDiagnoseGTraceType              = 11452,  // On Display Wrangler
    kIOGDiagnoseConnectType             = 38744,
};

enum {
    // options for IOServiceRequestProbe()
    kIOFBUserRequestProbe               = 0x00000001
};

struct IOGPoint {
    SInt16      x;
    SInt16      y;
};
typedef struct IOGPoint IOGPoint;

struct IOGSize {
    SInt16      width;
    SInt16      height;
};
typedef struct IOGSize IOGSize;

struct IOGBounds {
    SInt16      minx;
    SInt16      maxx;
    SInt16      miny;
    SInt16      maxy;
};
typedef struct IOGBounds IOGBounds;

#ifndef kIODescriptionKey

#if !defined(__Point__) && !defined(BINTREE_H) && !defined(__MACTYPES__)
#define __Point__
typedef IOGPoint Point;
#endif

#if !defined(__Bounds__) && !defined(BINTREE_H) && !defined(__MACTYPES__)
#define __Bounds__
typedef IOGBounds Bounds;
#endif

#endif /* !kIODescriptionKey */

// cursor description

enum {
   kTransparentEncoding         = 0,
   kInvertingEncoding
};

enum {
   kTransparentEncodingShift    = (kTransparentEncoding << 1),
   kTransparentEncodedPixel     = (0x01 << kTransparentEncodingShift),

   kInvertingEncodingShift      = (kInvertingEncoding << 1),
   kInvertingEncodedPixel       = (0x01 << kInvertingEncodingShift)
};

enum {
   kHardwareCursorDescriptorMajorVersion        = 0x0001,
   kHardwareCursorDescriptorMinorVersion        = 0x0000
};

/*!
 * @struct IOHardwareCursorDescriptor
 * @abstract A structure defining the format of a hardware cursor.
 * @discussion This structure is used by IOFramebuffer to define the format of a hardware cursor.
 * @field majorVersion Set to kHardwareCursorDescriptorMajorVersion.
 * @field minorVersion Set to kHardwareCursorDescriptorMinorVersion.
 * @field height Maximum size of the cursor.
 * @field width Maximum size of the cursor.
 * @field bitDepth Number bits per pixel, or a QD/QT pixel type, for example kIO8IndexedPixelFormat, kIO32ARGBPixelFormat.
 * @field maskBitDepth Unused.
 * @field numColors Number of colors for indexed pixel types.
 * @field colorEncodings An array pointer specifying the pixel values corresponding to the indices into the color table, for indexed pixel types.
 * @field flags None defined, set to zero.
 * @field supportedSpecialEncodings Mask of supported special pixel values, eg. kTransparentEncodedPixel, kInvertingEncodedPixel.
 * @field specialEncodings Array of pixel values for each supported special encoding.
 */

struct IOHardwareCursorDescriptor {
   UInt16               majorVersion;
   UInt16               minorVersion;
   UInt32               height;
   UInt32               width;
   UInt32               bitDepth;                       // bits per pixel, or a QD/QT pixel type
   UInt32               maskBitDepth;                   // unused
   UInt32               numColors;                      // number of colors in the colorMap. ie. 
   UInt32 *             colorEncodings;
   UInt32               flags;
   UInt32               supportedSpecialEncodings;
   UInt32               specialEncodings[16];
};
typedef struct IOHardwareCursorDescriptor IOHardwareCursorDescriptor;

enum {
   kHardwareCursorInfoMajorVersion              = 0x0001,
   kHardwareCursorInfoMinorVersion              = 0x0000
};

/*!
 * @struct IOHardwareCursorInfo
 * @abstract A structure defining the converted data of a hardware cursor.
 * @discussion This structure is used by IOFramebuffer to return the data of a hardware cursor by convertCursorImage() after conversion based on the IOHardwareCursorDescriptor passed to that routine.
 * @field majorVersion Set to kHardwareCursorInfoMajorVersion.
 * @field minorVersion Set to kHardwareCursorInfoMinorVersion.
 * @field cursorHeight The actual size of the cursor is returned.
 * @field cursorWidth The actual size of the cursor is returned.
 * @field colorMap Pointer to array of IOColorEntry structures, with the number of elements set by the numColors field of the IOHardwareCursorDescriptor. Zero should be passed for direct pixel formats.
 * @field hardwareCursorData Buffer to receive the converted cursor data.
 * @field cursorHotSpotX Cursor's hotspot.
 * @field cursorHotSpotY Cursor's hotspot.
 * @field reserved Reserved, set to zero.
 */

struct IOHardwareCursorInfo {
   UInt16               majorVersion;
   UInt16               minorVersion;
   UInt32               cursorHeight;
   UInt32               cursorWidth;
   // nil or big enough for hardware's max colors
   IOColorEntry *       colorMap;
   UInt8 *              hardwareCursorData;
   UInt16               cursorHotSpotX;
   UInt16               cursorHotSpotY;
   UInt32               reserved[5];
};
typedef struct IOHardwareCursorInfo IOHardwareCursorInfo;

// interrupt types

enum {
    kIOFBVBLInterruptType               = 'vbl ',
    kIOFBHBLInterruptType               = 'hbl ',
    kIOFBFrameInterruptType             = 'fram',
    // Demand to check configuration (Hardware unchanged)
    kIOFBConnectInterruptType           = 'dci ',
    // Demand to rebuild (Hardware has reinitialized on dependent change)
    kIOFBChangedInterruptType           = 'chng',
    // Demand to remove framebuffer (Hardware not available on dependent change -- but must not buserror)
    kIOFBOfflineInterruptType           = 'remv',
    // Notice that hardware is available (after being removed)
    kIOFBOnlineInterruptType            = 'add ',
    // DisplayPort short pulse
    kIOFBDisplayPortInterruptType           = 'dpir',
    // DisplayPort link event
    kIOFBDisplayPortLinkChangeInterruptType = 'dplk',
    // MCCS
    kIOFBMCCSInterruptType                  = 'mccs',
    // early vram notification
    kIOFBWakeInterruptType                  = 'vwak',
};

// IOAppleTimingID's
enum {
    kIOTimingIDInvalid               = 0,       /*  Not a standard timing */
    kIOTimingIDApple_FixedRateLCD    = 42,      /*  Lump all fixed-rate LCDs into one category.*/
    kIOTimingIDApple_512x384_60hz    = 130,     /*  512x384  (60 Hz) Rubik timing. */
    kIOTimingIDApple_560x384_60hz    = 135,     /*  560x384  (60 Hz) Rubik-560 timing. */
    kIOTimingIDApple_640x480_67hz    = 140,     /*  640x480  (67 Hz) HR timing. */
    kIOTimingIDApple_640x400_67hz    = 145,     /*  640x400  (67 Hz) HR-400 timing. */
    kIOTimingIDVESA_640x480_60hz     = 150,     /*  640x480  (60 Hz) VGA timing. */
    kIOTimingIDVESA_640x480_72hz     = 152,     /*  640x480  (72 Hz) VGA timing. */
    kIOTimingIDVESA_640x480_75hz     = 154,     /*  640x480  (75 Hz) VGA timing. */
    kIOTimingIDVESA_640x480_85hz     = 158,     /*  640x480  (85 Hz) VGA timing. */
    kIOTimingIDGTF_640x480_120hz     = 159,     /*  640x480  (120 Hz) VESA Generalized Timing Formula */
    kIOTimingIDApple_640x870_75hz    = 160,     /*  640x870  (75 Hz) FPD timing.*/
    kIOTimingIDApple_640x818_75hz    = 165,     /*  640x818  (75 Hz) FPD-818 timing.*/
    kIOTimingIDApple_832x624_75hz    = 170,     /*  832x624  (75 Hz) GoldFish timing.*/
    kIOTimingIDVESA_800x600_56hz     = 180,     /*  800x600  (56 Hz) SVGA timing. */
    kIOTimingIDVESA_800x600_60hz     = 182,     /*  800x600  (60 Hz) SVGA timing. */
    kIOTimingIDVESA_800x600_72hz     = 184,     /*  800x600  (72 Hz) SVGA timing. */
    kIOTimingIDVESA_800x600_75hz     = 186,     /*  800x600  (75 Hz) SVGA timing. */
    kIOTimingIDVESA_800x600_85hz     = 188,     /*  800x600  (85 Hz) SVGA timing. */
    kIOTimingIDVESA_1024x768_60hz    = 190,     /* 1024x768  (60 Hz) VESA 1K-60Hz timing. */
    kIOTimingIDVESA_1024x768_70hz    = 200,     /* 1024x768  (70 Hz) VESA 1K-70Hz timing. */
    kIOTimingIDVESA_1024x768_75hz    = 204,     /* 1024x768  (75 Hz) VESA 1K-75Hz timing (very similar to kIOTimingIDApple_1024x768_75hz). */
    kIOTimingIDVESA_1024x768_85hz    = 208,     /* 1024x768  (85 Hz) VESA timing. */
    kIOTimingIDApple_1024x768_75hz   = 210,     /* 1024x768  (75 Hz) Apple 19" RGB. */
    kIOTimingIDVESA_1152x864_75hz    = 215,     /* 1152x864  (75 Hz) VESA timing. */
    kIOTimingIDApple_1152x870_75hz   = 220,     /* 1152x870  (75 Hz) Apple 21" RGB. */
    kIOTimingIDAppleNTSC_ST          = 230,     /*  512x384  (60 Hz, interlaced, non-convolved). */
    kIOTimingIDAppleNTSC_FF          = 232,     /*  640x480  (60 Hz, interlaced, non-convolved). */
    kIOTimingIDAppleNTSC_STconv      = 234,     /*  512x384  (60 Hz, interlaced, convolved). */
    kIOTimingIDAppleNTSC_FFconv      = 236,     /*  640x480  (60 Hz, interlaced, convolved). */
    kIOTimingIDApplePAL_ST           = 238,     /*  640x480  (50 Hz, interlaced, non-convolved). */
    kIOTimingIDApplePAL_FF           = 240,     /*  768x576  (50 Hz, interlaced, non-convolved). */
    kIOTimingIDApplePAL_STconv       = 242,     /*  640x480  (50 Hz, interlaced, convolved). */
    kIOTimingIDApplePAL_FFconv       = 244,     /*  768x576  (50 Hz, interlaced, convolved). */
    kIOTimingIDVESA_1280x960_75hz    = 250,     /* 1280x960  (75 Hz) */
    kIOTimingIDVESA_1280x960_60hz    = 252,     /* 1280x960  (60 Hz) */
    kIOTimingIDVESA_1280x960_85hz    = 254,     /* 1280x960  (85 Hz) */
    kIOTimingIDVESA_1280x1024_60hz   = 260,     /* 1280x1024 (60 Hz) */
    kIOTimingIDVESA_1280x1024_75hz   = 262,     /* 1280x1024 (75 Hz) */
    kIOTimingIDVESA_1280x1024_85hz   = 268,     /* 1280x1024 (85 Hz) */
    kIOTimingIDVESA_1600x1200_60hz   = 280,     /* 1600x1200 (60 Hz) VESA timing. */
    kIOTimingIDVESA_1600x1200_65hz   = 282,     /* 1600x1200 (65 Hz) VESA timing. */
    kIOTimingIDVESA_1600x1200_70hz   = 284,     /* 1600x1200 (70 Hz) VESA timing. */
    kIOTimingIDVESA_1600x1200_75hz   = 286,     /* 1600x1200 (75 Hz) VESA timing (pixel clock is 189.2 Mhz dot clock). */
    kIOTimingIDVESA_1600x1200_80hz   = 288,     /* 1600x1200 (80 Hz) VESA timing (pixel clock is 216>? Mhz dot clock) - proposed only. */
    kIOTimingIDVESA_1600x1200_85hz   = 289,     /* 1600x1200 (85 Hz) VESA timing (pixel clock is 229.5 Mhz dot clock). */
    kIOTimingIDVESA_1792x1344_60hz   = 296,     /* 1792x1344 (60 Hz) VESA timing (204.75 Mhz dot clock). */
    kIOTimingIDVESA_1792x1344_75hz   = 298,     /* 1792x1344 (75 Hz) VESA timing (261.75 Mhz dot clock). */
    kIOTimingIDVESA_1856x1392_60hz   = 300,     /* 1856x1392 (60 Hz) VESA timing (218.25 Mhz dot clock). */
    kIOTimingIDVESA_1856x1392_75hz   = 302,     /* 1856x1392 (75 Hz) VESA timing (288 Mhz dot clock). */
    kIOTimingIDVESA_1920x1440_60hz   = 304,     /* 1920x1440 (60 Hz) VESA timing (234 Mhz dot clock). */
    kIOTimingIDVESA_1920x1440_75hz   = 306,     /* 1920x1440 (75 Hz) VESA timing (297 Mhz dot clock). */
    kIOTimingIDSMPTE240M_60hz        = 400,     /* 60Hz V, 33.75KHz H, interlaced timing, 16:9 aspect, typical resolution of 1920x1035. */
    kIOTimingIDFilmRate_48hz         = 410,     /* 48Hz V, 25.20KHz H, non-interlaced timing, typical resolution of 640x480. */
    kIOTimingIDSony_1600x1024_76hz   = 500,     /* 1600x1024 (76 Hz) Sony timing (pixel clock is 170.447 Mhz dot clock). */
    kIOTimingIDSony_1920x1080_60hz   = 510,     /* 1920x1080 (60 Hz) Sony timing (pixel clock is 159.84 Mhz dot clock). */
    kIOTimingIDSony_1920x1080_72hz   = 520,     /* 1920x1080 (72 Hz) Sony timing (pixel clock is 216.023 Mhz dot clock). */
    kIOTimingIDSony_1920x1200_76hz   = 540,     /* 1900x1200 (76 Hz) Sony timing (pixel clock is 243.20 Mhz dot clock). */
    kIOTimingIDApple_0x0_0hz_Offline = 550,     /* Indicates that this timing will take the display off-line and remove it from the system. */
    kIOTimingIDVESA_848x480_60hz     = 570,     /*  848x480 (60 Hz)  VESA timing. */
    kIOTimingIDVESA_1360x768_60hz    = 590      /* 1360x768 (60 Hz)  VESA timing. */
};

// framebuffer property keys

#define kIOFramebufferInfoKey           "IOFramebufferInformation"

#define kIOFBWidthKey                   "IOFBWidth"
#define kIOFBHeightKey                  "IOFBHeight"
#define kIOFBRefreshRateKey             "IOFBRefreshRate"
#define kIOFBFlagsKey                   "IOFBFlags"
#define kIOFBBytesPerRowKey             "IOFBBytesPerRow"
#define kIOFBBytesPerPlaneKey           "IOFBBytesPerPlane"
#define kIOFBBitsPerPixelKey            "IOFBBitsPerPixel"
#define kIOFBComponentCountKey          "IOFBComponentCount"
#define kIOFBBitsPerComponentKey        "IOFBBitsPerComponent"

#define kIOFBDetailedTimingsKey         "IOFBDetailedTimings"
#define kIOFBTimingRangeKey             "IOFBTimingRange"
#define kIOFBScalerInfoKey              "IOFBScalerInfo"
#define kIOFBCursorInfoKey              "IOFBCursorInfo"
#define kIOFBHDMIDongleROMKey           "IOFBHDMIDongleROM"

#define kIOFBHostAccessFlagsKey         "IOFBHostAccessFlags"

#define kIOFBMemorySizeKey              "IOFBMemorySize"

#define kIOFBNeedsRefreshKey            "IOFBNeedsRefresh"

#define kIOFBProbeOptionsKey            "IOFBProbeOptions"

#define kIOFBGammaWidthKey              "IOFBGammaWidth"
#define kIOFBGammaCountKey              "IOFBGammaCount"
#define kIOFBCLUTDeferKey               "IOFBCLUTDefer"

#define kIOFBDisplayPortConfigurationDataKey    "dpcd-registers"
        
// exists on the hibernate progress display device
#ifndef kIOHibernatePreviewActiveKey
#define kIOHibernatePreviewActiveKey    "IOHibernatePreviewActive"
// values for kIOHibernatePreviewActiveKey set by driver
enum {
    kIOHibernatePreviewActive  = 0x00000001,
    kIOHibernatePreviewUpdates = 0x00000002
};
#endif

#define kIOHibernateEFIGfxStatusKey    "IOHibernateEFIGfxStatus"

// CFNumber/CFData
#define kIOFBAVSignalTypeKey            "av-signal-type"
enum {
    kIOFBAVSignalTypeUnknown = 0x00000000,
    kIOFBAVSignalTypeVGA     = 0x00000001,
    kIOFBAVSignalTypeDVI     = 0x00000002,
    kIOFBAVSignalTypeHDMI    = 0x00000008,
    kIOFBAVSignalTypeDP      = 0x00000010,
};

// kIOFBDisplayPortTrainingAttribute data

struct IOFBDPLinkConfig
{
    uint16_t version;		 // 8 bit high (major); 8 bit low (minor)
    uint8_t  bitRate;		 // same encoding as the spec
    uint8_t  __reservedA[1]; // reserved set to zero
	uint16_t t1Time;		 // minimum duration of the t1 pattern (microseconds)
	uint16_t t2Time;		 // minimum duration of the t2 pattern
	uint16_t t3Time;		 // minimum duration of the t3 pattern
	uint8_t  idlePatterns;   // minimum number of idle patterns
	uint8_t  laneCount;		 // number of lanes in the link
	uint8_t  voltage;
	uint8_t  preEmphasis;
	uint8_t  downspread;
	uint8_t  scrambler;
	uint8_t  maxBitRate;	 // same encoding as the bitRate field
	uint8_t  maxLaneCount;	 // an integer
	uint8_t  maxDownspread;	 // 0 = Off. 1 = 0.5
	uint8_t  __reservedB[9];	// reserved set to zero - fix align and provide 8 bytes of padding.
};
typedef struct IOFBDPLinkConfig IOFBDPLinkConfig;

enum
{
    kIOFBBitRateRBR		= 0x06,		// 1.62 Gbps per lane
    kIOFBBitRateHBR		= 0x0A,		// 2.70 Gbps per lane
    kIOFBBitRateHBR2	= 0x14,		// 5.40 Gbps per lane
};

enum {
    kIOFBLinkVoltageLevel0	= 0x00,
    kIOFBLinkVoltageLevel1	= 0x01,
    kIOFBLinkVoltageLevel2	= 0x02,
    kIOFBLinkVoltageLevel3	= 0x03
};

enum
{
    kIOFBLinkPreEmphasisLevel0 = 0x00,
    kIOFBLinkPreEmphasisLevel1 = 0x01,
    kIOFBLinkPreEmphasisLevel2 = 0x02,
    kIOFBLinkPreEmphasisLevel3 = 0x03
};

enum
{
    kIOFBLinkDownspreadNone  = 0x0,
    kIOFBLinkDownspreadMax   = 0x1
};

enum
{
    kIOFBLinkScramblerNormal    = 0x0, // for external displays
    kIOFBLinkScramblerAlternate = 0x1  // used for eDP
};

// diagnostic keys

#define kIOFBConfigKey                  "IOFBConfig"
#define kIOFBModesKey                   "IOFBModes"
#define kIOFBModeIDKey                  "ID"
#define kIOFBModeDMKey                  "DM"
#define kIOFBModeTMKey                  "TM"
#define kIOFBModeAIDKey                 "AID"
#define kIOFBModeDFKey                  "DF"
#define kIOFBModePIKey                  "PI"

// display property keys

#define kIODisplayEDIDKey               "IODisplayEDID"
#define kIODisplayEDIDOriginalKey       "IODisplayEDIDOriginal"
#define kIODisplayLocationKey           "IODisplayLocation"             // CFString
#define kIODisplayConnectFlagsKey       "IODisplayConnectFlags"         // CFNumber
#define kIODisplayHasBacklightKey       "IODisplayHasBacklight"         // CFBoolean
#define kIODisplayIsDigitalKey          "IODisplayIsDigital"            // CFBoolean
#define kDisplayBundleKey               "DisplayBundle"

#define kAppleDisplayTypeKey            "AppleDisplayType"
#define kAppleSenseKey                  "AppleSense"

#define kIODisplayMCCSVersionKey                "IODisplayMCCSVersion"
#define kIODisplayTechnologyTypeKey             "IODisplayTechnologyType"
#define kIODisplayUsageTimeKey                  "IODisplayUsageTime"
#define kIODisplayFirmwareLevelKey              "IODisplayFirmwareLevel"

enum {
    kDisplayVendorIDUnknown     = 'unkn',
    kDisplayProductIDGeneric    = 0x717
};

#define kDisplayVendorID                "DisplayVendorID"        // CFNumber
#define kDisplayProductID               "DisplayProductID"       // CFNumber
#define kDisplaySerialNumber            "DisplaySerialNumber"    // CFNumber
#define kDisplaySerialString            "DisplaySerialString"    // CFString
#define kDisplayWeekOfManufacture       "DisplayWeekManufacture" // CFNumber
#define kDisplayYearOfManufacture       "DisplayYearManufacture" // CFNumber

// CFDictionary of language-locale keys, name values
// eg. "en"="Color LCD", "en-GB"="Colour LCD"
#define kDisplayProductName             "DisplayProductName"

// all CFNumber or CFArray of CFNumber (floats)
#define kDisplayWhitePointX             "DisplayWhitePointX"
#define kDisplayWhitePointY             "DisplayWhitePointY"
#define kDisplayRedPointX               "DisplayRedPointX"
#define kDisplayRedPointY               "DisplayRedPointY"
#define kDisplayGreenPointX             "DisplayGreenPointX"
#define kDisplayGreenPointY             "DisplayGreenPointY"
#define kDisplayBluePointX              "DisplayBluePointX"
#define kDisplayBluePointY              "DisplayBluePointY"
#define kDisplayWhiteGamma              "DisplayWhiteGamma"
#define kDisplayRedGamma                "DisplayRedGamma"
#define kDisplayGreenGamma              "DisplayGreenGamma"
#define kDisplayBlueGamma               "DisplayBlueGamma"

// Display gamma
#define kDisplayGammaChannels           "DisplayGammaChannels"    // CFNumber 1 or 3 channel count
#define kDisplayGammaEntryCount         "DisplayGammaEntryCount"  // CFNumber 1-based count of entries per channel
#define kDisplayGammaEntrySize          "DisplayGammaEntrySize"   // CFNumber size in bytes of each table entry
#define kDisplayGammaTable              "DisplayGammaTable"       // CFData

// CFBoolean
#define kDisplayBrightnessAffectsGamma  "DisplayBrightnessAffectsGamma"
#define kDisplayViewAngleAffectsGamma   "DisplayViewAngleAffectsGamma"

// CFData
#define kDisplayCSProfile               "DisplayCSProfile"

// CFNumber
#define kDisplayHorizontalImageSize     "DisplayHorizontalImageSize"
#define kDisplayVerticalImageSize       "DisplayVerticalImageSize"

// Pixel description

// CFBoolean
#define kDisplayFixedPixelFormat        "DisplayFixedPixelFormat"

enum {
    kDisplaySubPixelLayoutUndefined     = 0x00000000,
    kDisplaySubPixelLayoutRGB           = 0x00000001,
    kDisplaySubPixelLayoutBGR           = 0x00000002,
    kDisplaySubPixelLayoutQuadGBL       = 0x00000003,
    kDisplaySubPixelLayoutQuadGBR       = 0x00000004,

    kDisplaySubPixelConfigurationUndefined    = 0x00000000,
    kDisplaySubPixelConfigurationDelta        = 0x00000001,
    kDisplaySubPixelConfigurationStripe       = 0x00000002,
    kDisplaySubPixelConfigurationStripeOffset = 0x00000003,
    kDisplaySubPixelConfigurationQuad         = 0x00000004,

    kDisplaySubPixelShapeUndefined      = 0x00000000,
    kDisplaySubPixelShapeRound          = 0x00000001,
    kDisplaySubPixelShapeSquare         = 0x00000002,
    kDisplaySubPixelShapeRectangular    = 0x00000003,
    kDisplaySubPixelShapeOval           = 0x00000004,
    kDisplaySubPixelShapeElliptical     = 0x00000005
};

// CFNumbers
#define kDisplaySubPixelLayout          "DisplaySubPixelLayout"
#define kDisplaySubPixelConfiguration   "DisplaySubPixelConfiguration"
#define kDisplaySubPixelShape           "DisplaySubPixelShape"

#define kIODisplayOverrideMatchingKey   "IODisplayOverrideMatching"

// Display parameters

#define kIODisplayParametersKey         "IODisplayParameters"
#define kIODisplayGUIDKey               "IODisplayGUID"

#define kIODisplayValueKey              "value"
#define kIODisplayMinValueKey           "min"
#define kIODisplayMaxValueKey           "max"

#define kIODisplayBrightnessProbeKey        "brightness-probe"
#define kIODisplayLinearBrightnessProbeKey  "linear-brightness-probe"
#define kIODisplayBrightnessKey             "brightness"
#define kIODisplayLinearBrightnessKey       "linear-brightness"
#define kIODisplayUsableLinearBrightnessKey "usable-linear-brightness"
#define kIODisplayBrightnessFadeKey         "brightness-fade"
#define kIODisplayContrastKey               "contrast"
#define kIODisplayHorizontalPositionKey     "horizontal-position"
#define kIODisplayHorizontalSizeKey     	"horizontal-size"
#define kIODisplayVerticalPositionKey   	"vertical-position"
#define kIODisplayVerticalSizeKey           "vertical-size"
#define kIODisplayTrapezoidKey              "trapezoid"
#define kIODisplayPincushionKey             "pincushion"
#define kIODisplayParallelogramKey          "parallelogram"
#define kIODisplayRotationKey               "rotation"
#define kIODisplayTheatreModeKey            "theatre-mode"
#define kIODisplayTheatreModeWindowKey      "theatre-mode-window"
#define kIODisplayOverscanKey               "oscn"
#define kIODisplayVideoBestKey              "vbst"

#define kIODisplaySpeakerVolumeKey              "speaker-volume"
#define kIODisplaySpeakerSelectKey              "speaker-select"
#define kIODisplayMicrophoneVolumeKey           "microphone-volume"
#define kIODisplayAmbientLightSensorKey         "ambient-light-sensor"
#define kIODisplayAudioMuteAndScreenBlankKey    "audio-mute-and-screen-blank"
#define kIODisplayAudioTrebleKey                "audio-treble"
#define kIODisplayAudioBassKey                  "audio-bass"
#define kIODisplayAudioBalanceLRKey             "audio-balance-LR"
#define kIODisplayAudioProcessorModeKey         "audio-processor-mode"
#define kIODisplayPowerModeKey                  "power-mode"
#define kIODisplayManufacturerSpecificKey       "manufacturer-specific"

#define kIODisplayPowerStateKey       			"dsyp"

#define kIODisplayControllerIDKey				"IODisplayControllerID"
#define kIODisplayCapabilityStringKey       	"IODisplayCapabilityString"

#define kIODisplayRedGammaScaleKey      "rgsc"
#define kIODisplayGreenGammaScaleKey    "ggsc"
#define kIODisplayBlueGammaScaleKey     "bgsc"
#define kIODisplayGammaScaleKey         "gsc "

#define kIODisplayParametersCommitKey   "commit"
#define kIODisplayParametersDefaultKey  "defaults"
#define kIODisplayParametersFlushKey    "flush"

#ifdef __cplusplus
}
#endif

#endif /* ! _IOKIT_IOGRAPHICSTYPES_H */
