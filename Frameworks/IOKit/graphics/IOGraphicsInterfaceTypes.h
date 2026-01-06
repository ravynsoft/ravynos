/*
 * Copyright (c) 1999-2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKIT_IOGRAPHICSINTERFACETYPES_H
#define _IOKIT_IOGRAPHICSINTERFACETYPES_H

#include <IOKit/graphics/IOAccelSurfaceConnect.h>

#define IO_FOUR_CHAR_CODE(x)    (x)

typedef UInt32 IOFourCharCode;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define kCurrentGraphicsInterfaceVersion        1
#define kCurrentGraphicsInterfaceRevision       2


#ifdef IOGA_COMPAT
typedef SInt32          IOBlitCompletionToken;
#endif

typedef UInt32          IOBlitType;
enum {
    kIOBlitTypeVerbMask                 = 0x000000ff,
    kIOBlitTypeRects                    = 0,
    kIOBlitTypeCopyRects,
    kIOBlitTypeLines,
    kIOBlitTypeScanlines,
    kIOBlitTypeCopyRegion,

    kIOBlitTypeMoveCursor,
    kIOBlitTypeShowCursor,
    kIOBlitTypeHideCursor,

    kIOBlitTypeMonoExpand               = 0x00000100,
    kIOBlitTypeColorSpaceConvert        = 0x00000200,
    kIOBlitTypeScale                    = 0x00000400,

    kIOBlitTypeSourceKeyColorModeMask   = 0x00003000,
    kIOBlitTypeDestKeyColorModeMask     = 0x0000c000,
    kIOBlitTypeSourceKeyColorEqual      = 0x00001000,
    kIOBlitTypeSourceKeyColorNotEqual   = 0x00002000,
    kIOBlitTypeDestKeyColorEqual        = 0x00004000,
    kIOBlitTypeDestKeyColorNotEqual     = 0x00008000,

    kIOBlitTypeOperationMask            = 0x0fff0000,
    kIOBlitTypeOperationShift           = 16,
    kIOBlitTypeOperationTypeMask        = 0x0f000000,

    kIOBlitTypeOperationType0           = 0x00000000,
    kIOBlitCopyOperation                = 0x00000000 | kIOBlitTypeOperationType0,
    kIOBlitOrOperation                  = 0x00010000 | kIOBlitTypeOperationType0,
    kIOBlitXorOperation                 = 0x00020000 | kIOBlitTypeOperationType0,
    kIOBlitBlendOperation               = 0x00030000 | kIOBlitTypeOperationType0,
    kIOBlitHighlightOperation           = 0x00040000 | kIOBlitTypeOperationType0
};

typedef UInt32          IOBlitSourceType;
enum {
    kIOBlitSourceDefault                = 0x00000000,
    kIOBlitSourceFramebuffer            = 0x00001000,
    kIOBlitSourceMemory                 = 0x00002000,
    kIOBlitSourceOOLMemory              = 0x00003000,
    kIOBlitSourcePattern                = 0x00004000,
    kIOBlitSourceOOLPattern             = 0x00005000,
    kIOBlitSourceSolid                  = 0x00006000,
    kIOBlitSourceCGSSurface             = 0x00007000,
    kIOBlitSourceIsSame                 = 0x80000000
};

#ifdef IOGA_COMPAT
typedef IOBlitSourceType        IOBlitSourceDestType;
enum {
    kIOBlitDestFramebuffer              = 0x00000001
};
#endif

typedef struct IOBlitOperationStruct {
    UInt32              color0;
    UInt32              color1;
    SInt32              offsetX;
    SInt32              offsetY;
    UInt32              sourceKeyColor;
    UInt32              destKeyColor;
    UInt32              specific[16];
} IOBlitOperation;

typedef struct IOBlitRectangleStruct {
    SInt32              x;
    SInt32              y;
    SInt32              width;
    SInt32              height;
} IOBlitRectangle;

typedef struct IOBlitRectanglesStruct {
    IOBlitOperation     operation;
    IOItemCount         count;
    IOBlitRectangle     rects[1];
} IOBlitRectangles;

typedef struct IOBlitCopyRectangleStruct {
    SInt32              sourceX;
    SInt32              sourceY;
    SInt32              x;
    SInt32              y;
    SInt32              width;
    SInt32              height;
} IOBlitCopyRectangle;

typedef struct IOBlitCopyRectanglesStruct {
    IOBlitOperation     operation;
    IOItemCount         count;
    IOBlitCopyRectangle rects[1];
} IOBlitCopyRectangles;


typedef struct IOBlitCopyRegionStruct {
    IOBlitOperation       operation;
    SInt32                deltaX;
    SInt32                deltaY;
    IOAccelDeviceRegion * region;
} IOBlitCopyRegion;


typedef struct IOBlitVertexStruct {
    SInt32              x;
    SInt32              y;
} IOBlitVertex;

typedef struct IOBlitVerticesStruct {
    IOBlitOperation     operation;
    IOItemCount         count;
    IOBlitVertex        vertices[2];
} IOBlitVertices;

typedef struct IOBlitScanlinesStruct {
    IOBlitOperation     operation;
    IOItemCount         count;
    SInt32              y;
    SInt32              height;
    SInt32              x[2];
} IOBlitScanlines;


typedef struct IOBlitCursorStruct {
    IOBlitOperation     operation;
    IOBlitRectangle     rect;
} IOBlitCursor;

typedef struct _IOBlitMemory * IOBlitMemoryRef;


/* Quickdraw.h pixel formats*/

enum {
        kIO1MonochromePixelFormat       = 0x00000001,           /* 1 bit indexed*/
        kIO2IndexedPixelFormat          = 0x00000002,           /* 2 bit indexed*/
        kIO4IndexedPixelFormat          = 0x00000004,           /* 4 bit indexed*/
        kIO8IndexedPixelFormat          = 0x00000008,           /* 8 bit indexed*/
        kIO16BE555PixelFormat           = 0x00000010,           /* 16 bit BE rgb 555 (Mac)*/
        kIO24RGBPixelFormat             = 0x00000018,           /* 24 bit rgb */
        kIO32ARGBPixelFormat            = 0x00000020,           /* 32 bit argb  (Mac)*/
        kIO1IndexedGrayPixelFormat      = 0x00000021,           /* 1 bit indexed gray*/
        kIO2IndexedGrayPixelFormat      = 0x00000022,           /* 2 bit indexed gray*/
        kIO4IndexedGrayPixelFormat      = 0x00000024,           /* 4 bit indexed gray*/
        kIO8IndexedGrayPixelFormat      = 0x00000028            /* 8 bit indexed gray*/
};

enum {
        kIO16LE555PixelFormat   = IO_FOUR_CHAR_CODE('L555'),    /* 16 bit LE rgb 555 (PC)*/
        kIO16LE5551PixelFormat  = IO_FOUR_CHAR_CODE('5551'),    /* 16 bit LE rgb 5551*/
        kIO16BE565PixelFormat   = IO_FOUR_CHAR_CODE('B565'),    /* 16 bit BE rgb 565*/
        kIO16LE565PixelFormat   = IO_FOUR_CHAR_CODE('L565'),    /* 16 bit LE rgb 565*/
        kIO24BGRPixelFormat     = IO_FOUR_CHAR_CODE('24BG'),    /* 24 bit bgr */
        kIO32BGRAPixelFormat    = IO_FOUR_CHAR_CODE('BGRA'),    /* 32 bit bgra  (Matrox)*/
        kIO32ABGRPixelFormat    = IO_FOUR_CHAR_CODE('ABGR'),    /* 32 bit abgr  */
        kIO32RGBAPixelFormat    = IO_FOUR_CHAR_CODE('RGBA'),    /* 32 bit rgba  */
        kIOYUVSPixelFormat      = IO_FOUR_CHAR_CODE('yuvs'),    /* YUV 4:2:2 byte ordering 16-unsigned = 'YUY2'*/
        kIOYUVUPixelFormat      = IO_FOUR_CHAR_CODE('yuvu'),    /* YUV 4:2:2 byte ordering 16-signed*/
        kIOYVU9PixelFormat      = IO_FOUR_CHAR_CODE('YVU9'),    /* YVU9 Planar  9*/
        kIOYUV411PixelFormat    = IO_FOUR_CHAR_CODE('Y411'),    /* YUV 4:1:1 Interleaved 16*/
        kIOYVYU422PixelFormat   = IO_FOUR_CHAR_CODE('YVYU'),    /* YVYU 4:2:2 byte ordering 16*/
        kIOUYVY422PixelFormat   = IO_FOUR_CHAR_CODE('UYVY'),    /* UYVY 4:2:2 byte ordering 16*/
        kIOYUV211PixelFormat    = IO_FOUR_CHAR_CODE('Y211'),    /* YUV 2:1:1 Packed     8*/
        kIO2vuyPixelFormat      = IO_FOUR_CHAR_CODE('2vuy') /* UYVY 4:2:2 byte ordering   16*/
};

/* Non Quickdraw.h pixel formats*/
enum {
        kIO16LE4444PixelFormat     = IO_FOUR_CHAR_CODE('L444'), /* 16 bit LE argb 4444*/
        kIO16BE4444PixelFormat     = IO_FOUR_CHAR_CODE('B444'), /* 16 bit BE argb 4444*/
        kIO64BGRAPixelFormat       = IO_FOUR_CHAR_CODE('B16I'), /* 64 bit bgra  */
        kIO64RGBAFloatPixelFormat  = IO_FOUR_CHAR_CODE('B16F'), /* 64 bit rgba  */
        kIO128RGBAFloatPixelFormat = IO_FOUR_CHAR_CODE('B32F')  /* 128 bit rgba float */
};

enum {
    kIOBlitMemoryRequiresHostFlush      = 0x00000001
};

typedef struct IOBlitSurfaceStruct {
    union {
        UInt8 *         bytes;
        IOBlitMemoryRef ref;
    }                   memory;
    IOFourCharCode      pixelFormat;
    IOBlitRectangle     size;
    UInt32              rowBytes;
    UInt32              byteOffset;
    UInt32 *            palette;
    IOOptionBits        accessFlags;
    IOBlitMemoryRef     interfaceRef;
    UInt32              more[14];
} IOBlitSurface;

typedef IOBlitSurface IOBlitMemory;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

enum {
    // options for Synchronize
    kIOBlitSynchronizeWaitBeamExit      = 0x00000001,
    kIOBlitSynchronizeFlushHostWrites   = 0x00000002
};

enum {
    // options for WaitComplete & Flush
    kIOBlitWaitContext                  = 0x00000000,
    kIOBlitWaitAll2D                    = 0x00000001,
    kIOBlitWaitGlobal                   = 0x00000001,
    kIOBlitWaitAll                      = 0x00000002,
    kIOBlitWaitCheck                    = 0x00000080,
    kIOBlitFlushWithSwap                = 0x00010000
};

enum {
    // options for AllocateSurface
    kIOBlitHasCGSSurface                = 0x00000001,
    kIOBlitFixedSource                  = 0x00000002,
    kIOBlitBeamSyncSwaps                = 0x00000004,
    kIOBlitReferenceSource              = 0x00000008
};

enum {
    // options for UnlockSurface
    kIOBlitUnlockWithSwap               = 0x80000000
};

enum {
    // options for SetDestination
    kIOBlitFramebufferDestination       = 0x00000000,
    kIOBlitSurfaceDestination           = 0x00000001
};



enum {
    // options for blit procs
    kIOBlitBeamSync                     = 0x00000001,
    kIOBlitBeamSyncAlways               = 0x00000002,
    kIOBlitBeamSyncSpin                 = 0x00000004,

    kIOBlitAllOptions                   = 0xffffffff
};

enum {
    // capabilities
    kIOBlitColorSpaceTypes              = IO_FOUR_CHAR_CODE('cspc')
};


// keys for IOAccelFindAccelerator()
#define kIOAccelTypesKey                        "IOAccelTypes"
#define kIOAccelIndexKey                        "IOAccelIndex"

#define kIOAccelRevisionKey                     "IOAccelRevision"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif /* !_IOKIT_IOGRAPHICSINTERFACETYPES_H */
