/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
Copyright (c) 2002, 2008, 2009 Apple Computer, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Jens Owen <jens@valinux.com>
 *   Rickard E. (Rik) Fiath <faith@valinux.com>
 *
 */

#ifndef _APPLEDRISTR_H_
#define _APPLEDRISTR_H_

#include "appledri.h"

#define APPLEDRINAME "Apple-DRI"

#define APPLE_DRI_MAJOR_VERSION	1       /* current version numbers */
#define APPLE_DRI_MINOR_VERSION	0
#define APPLE_DRI_PATCH_VERSION	0

typedef struct _AppleDRIQueryVersion
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRIQueryVersion */
   CARD16 length B16;
} xAppleDRIQueryVersionReq;
#define sz_xAppleDRIQueryVersionReq	4

typedef struct
{
   BYTE type;                   /* X_Reply */
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD16 majorVersion B16;     /* major version of DRI protocol */
   CARD16 minorVersion B16;     /* minor version of DRI protocol */
   CARD32 patchVersion B32;     /* patch version of DRI protocol */
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xAppleDRIQueryVersionReply;
#define sz_xAppleDRIQueryVersionReply	32

typedef struct _AppleDRIQueryDirectRenderingCapable
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* X_DRIQueryDirectRenderingCapable */
   CARD16 length B16;
   CARD32 screen B32;
} xAppleDRIQueryDirectRenderingCapableReq;
#define sz_xAppleDRIQueryDirectRenderingCapableReq	8

typedef struct
{
   BYTE type;                   /* X_Reply */
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   BOOL isCapable;
   BOOL pad2;
   BOOL pad3;
   BOOL pad4;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
   CARD32 pad7 B32;
   CARD32 pad8 B32;
   CARD32 pad9 B32;
} xAppleDRIQueryDirectRenderingCapableReply;
#define sz_xAppleDRIQueryDirectRenderingCapableReply	32

typedef struct _AppleDRIAuthConnection
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRICloseConnection */
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 magic B32;
} xAppleDRIAuthConnectionReq;
#define sz_xAppleDRIAuthConnectionReq	12

typedef struct
{
   BYTE type;
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD32 authenticated B32;
   CARD32 pad2 B32;
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xAppleDRIAuthConnectionReply;
#define zx_xAppleDRIAuthConnectionReply  32

typedef struct _AppleDRICreateSurface
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRICreateSurface */
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 drawable B32;
   CARD32 client_id B32;
} xAppleDRICreateSurfaceReq;
#define sz_xAppleDRICreateSurfaceReq	16

typedef struct
{
   BYTE type;                   /* X_Reply */
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD32 key_0 B32;
   CARD32 key_1 B32;
   CARD32 uid B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xAppleDRICreateSurfaceReply;
#define sz_xAppleDRICreateSurfaceReply	32

typedef struct _AppleDRIDestroySurface
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRIDestroySurface */
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 drawable B32;
} xAppleDRIDestroySurfaceReq;
#define sz_xAppleDRIDestroySurfaceReq	12

typedef struct _AppleDRINotify
{
   BYTE type;                   /* always eventBase + event type */
   BYTE kind;
   CARD16 sequenceNumber B16;
   CARD32 time B32;             /* time of change */
   CARD32 pad1 B32;
   CARD32 arg B32;
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xAppleDRINotifyEvent;
#define sz_xAppleDRINotifyEvent	32


typedef struct
{
   CARD8 reqType;
   CARD8 driReqType;
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 drawable B32;
   BOOL doubleSwap;
   CARD8 pad1, pad2, pad3;
} xAppleDRICreateSharedBufferReq;

#define sz_xAppleDRICreateSharedBufferReq 16

typedef struct
{
   BYTE type;
   BYTE data1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD32 stringLength B32;     /* 0 on error */
   CARD32 width B32;
   CARD32 height B32;
   CARD32 pad1 B32;
   CARD32 pad2 B32;
   CARD32 pad3 B32;
} xAppleDRICreateSharedBufferReply;

#define sz_xAppleDRICreateSharedBufferReply 32

typedef struct
{
   CARD8 reqType;
   CARD8 driReqType;
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 drawable B32;
} xAppleDRISwapBuffersReq;

#define sz_xAppleDRISwapBuffersReq 12

typedef struct
{
   CARD8 reqType;               /*1 */
   CARD8 driReqType;            /*2 */
   CARD16 length B16;           /*4 */
   CARD32 screen B32;           /*8 */
   CARD32 drawable B32;         /*12 */
} xAppleDRICreatePixmapReq;

#define sz_xAppleDRICreatePixmapReq 12

typedef struct
{
   BYTE type;                   /*1 */
   BOOL pad1;                   /*2 */
   CARD16 sequenceNumber B16;   /*4 */
   CARD32 length B32;           /*8 */
   CARD32 width B32;            /*12 */
   CARD32 height B32;           /*16 */
   CARD32 pitch B32;            /*20 */
   CARD32 bpp B32;              /*24 */
   CARD32 size B32;             /*28 */
   CARD32 stringLength B32;     /*32 */
} xAppleDRICreatePixmapReply;

#define sz_xAppleDRICreatePixmapReply 32

typedef struct
{
   CARD8 reqType;               /*1 */
   CARD8 driReqType;            /*2 */
   CARD16 length B16;           /*4 */
   CARD32 drawable B32;         /*8 */
} xAppleDRIDestroyPixmapReq;

#define sz_xAppleDRIDestroyPixmapReq 8

#ifdef _APPLEDRI_SERVER_

void AppleDRISendEvent(
#if NeedFunctionPrototypes
                         int /* type */ ,
                         unsigned int /* mask */ ,
                         int /* which */ ,
                         int    /* arg */
#endif
   );

#endif /* _APPLEDRI_SERVER_ */
#endif /* _APPLEDRISTR_H_ */
