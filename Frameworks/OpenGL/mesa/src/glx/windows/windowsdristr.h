/*
 * Copyright © 2014 Jon Turney
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef WINDOWSDRISTR_H
#define WINDOWSDRISTR_H

#include "windowsdriconst.h"

#define WINDOWSDRINAME "Windows-DRI"

#define WINDOWS_DRI_MAJOR_VERSION	1       /* current version numbers */
#define WINDOWS_DRI_MINOR_VERSION	0
#define WINDOWS_DRI_PATCH_VERSION	0

typedef struct _WindowsDRIQueryVersion
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRIQueryVersion */
   CARD16 length B16;
} xWindowsDRIQueryVersionReq;
#define sz_xWindowsDRIQueryVersionReq	4

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
} xWindowsDRIQueryVersionReply;
#define sz_xWindowsDRIQueryVersionReply	32

typedef struct _WindowsDRIQueryDirectRenderingCapable
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* X_DRIQueryDirectRenderingCapable */
   CARD16 length B16;
   CARD32 screen B32;
} xWindowsDRIQueryDirectRenderingCapableReq;
#define sz_xWindowsDRIQueryDirectRenderingCapableReq	8

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
} xWindowsDRIQueryDirectRenderingCapableReply;
#define sz_xWindowsDRIQueryDirectRenderingCapableReply	32

typedef struct _WindowsDRINotify
{
   BYTE type;                   /* always eventBase + event type */
   BYTE kind;
   CARD16 sequenceNumber B16;
   CARD32 time B32;             /* time of change */
   CARD32 pad1 B32;
   CARD32 pad2 B32;
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xWindowsDRINotifyEvent;
#define sz_xWindowsDRINotifyEvent	32

typedef struct _WindowsDRIQueryDrawable
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRIQueryDrawable */
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 drawable B32;
} xWindowsDRIQueryDrawableReq;
#define sz_xWindowsDRIQueryDrawableReq 12

typedef struct
{
   BYTE type;                   /* X_Reply */
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD32 drawable_type B32;
   CARD32 handle B32;
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xWindowsDRIQueryDrawableReply;
#define sz_xWindowsDRIQueryDrawableReply 32

typedef struct _WindowsDRIFBConfigToPixelFormat
{
   CARD8 reqType;               /* always DRIReqCode */
   CARD8 driReqType;            /* always X_DRIFBConfigToPixelFormat */
   CARD16 length B16;
   CARD32 screen B32;
   CARD32 fbConfigID B32;
} xWindowsDRIFBConfigToPixelFormatReq;

#define sz_xWindowsDRIFBConfigToPixelFormatReq 12

typedef struct
{
   BYTE type;                   /* X_Reply */
   BOOL pad1;
   CARD16 sequenceNumber B16;
   CARD32 length B32;
   CARD32 pixelFormatIndex B32;
   CARD32 pad2 B32;
   CARD32 pad3 B32;
   CARD32 pad4 B32;
   CARD32 pad5 B32;
   CARD32 pad6 B32;
} xWindowsDRIFBConfigToPixelFormatReply;
#define sz_xWindowsDRIFBConfigToPixelFormatReply 32

#endif /* WINDOWSDRISTR_H */
