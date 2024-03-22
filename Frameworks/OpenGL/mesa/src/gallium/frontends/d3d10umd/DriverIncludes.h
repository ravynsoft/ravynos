/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * DriverIncludes.h --
 *    Basic DDK includes for building the client driver.
 */

#ifndef DRIVER_INCLUDES_H
#define DRIVER_INCLUDES_H

#ifdef __MINGW32__
#undef WIN32_LEAN_AND_MEAN /* for DEFINE_GUID macro */
#define _NO_OLDNAMES       /* avoid defining ssize_t */
#include <stdio.h>         /* for vsnprintf */
#undef fileno              /* we redefine this in vm_basic_defs.h */
#endif


#include <windows.h>

#include "winddk_compat.h"

//typedef LARGE_INTEGER PHYSICAL_ADDRESS;
//typedef __success(return >= 0) LONG NTSTATUS;

#define D3D10DDI_MINOR_HEADER_VERSION 2

/* Unfortunately WinDDK's d3d10umddi.h defines D3D10.x constants as global
 * const variables instead of preprocessor defines, causing LINK to fail due
 * to duplicate symbols.  Include d3d10_1.h to avoid the issue.
 */
#ifdef _MSC_VER
#include <d3d10_1.h>
#endif

#include <d3d10umddi.h>

#include "Debug.h"

#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "util/format/u_formats.h"
#include "pipe/p_defines.h"

#include "util/u_debug.h"
#include "util/u_inlines.h"


#endif   /* DRIVER_INCLUDES_H */
