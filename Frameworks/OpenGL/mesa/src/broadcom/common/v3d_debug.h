/*
 * Copyright 2003 VMware, Inc.
 * Copyright © 2007 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MESA_V3D_DEBUG_H
#define MESA_V3D_DEBUG_H

#include <stdint.h>
#include "compiler/shader_enums.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \file v3d_debug.h
 *
 * Basic V3D_DEBUG environment variable handling.  This file defines the
 * list of debugging flags, as well as some macros for handling them.
 */

extern uint32_t v3d_mesa_debug;

#define V3D_DBG(flag) unlikely(v3d_mesa_debug & V3D_DEBUG_ ## flag)

#define V3D_DEBUG_SHADERDB          (1 << 0)
#define V3D_DEBUG_TGSI              (1 << 1)
#define V3D_DEBUG_NIR               (1 << 2)
#define V3D_DEBUG_VIR               (1 << 3)
#define V3D_DEBUG_QPU               (1 << 4)
#define V3D_DEBUG_FS                (1 << 5)
#define V3D_DEBUG_GS                (1 << 6)
#define V3D_DEBUG_VS                (1 << 7)
#define V3D_DEBUG_CS                (1 << 8)
#define V3D_DEBUG_CL                (1 << 9)
#define V3D_DEBUG_SURFACE           (1 << 10)
#define V3D_DEBUG_PERF              (1 << 11)
#define V3D_DEBUG_NORAST            (1 << 12)
#define V3D_DEBUG_ALWAYS_FLUSH      (1 << 13)
#define V3D_DEBUG_CLIF              (1 << 14)
#define V3D_DEBUG_PRECOMPILE        (1 << 15)
#define V3D_DEBUG_RA                (1 << 16)
#define V3D_DEBUG_DUMP_SPIRV        (1 << 17)
#define V3D_DEBUG_TMU_32BIT         (1 << 18)
#define V3D_DEBUG_TMU_16BIT         (1 << 19)
#define V3D_DEBUG_NO_LOOP_UNROLL    (1 << 20)
#define V3D_DEBUG_CL_NO_BIN         (1 << 21)
#define V3D_DEBUG_DOUBLE_BUFFER     (1 << 22)
#define V3D_DEBUG_CACHE             (1 << 23)
#define V3D_DEBUG_NO_MERGE_JOBS     (1 << 24)
#define V3D_DEBUG_OPT_COMPILE_TIME  (1 << 25)
#define V3D_DEBUG_DISABLE_TFU       (1 << 26)

#define V3D_DEBUG_SHADERS           (V3D_DEBUG_TGSI | V3D_DEBUG_NIR | \
                                     V3D_DEBUG_VIR | V3D_DEBUG_QPU | \
                                     V3D_DEBUG_FS | V3D_DEBUG_GS | \
                                     V3D_DEBUG_VS | V3D_DEBUG_CS | \
                                     V3D_DEBUG_RA)

#ifdef HAVE_ANDROID_PLATFORM
#define LOG_TAG "BROADCOM-MESA"
#if ANDROID_API_LEVEL >= 26
#include <log/log.h>
#else
#include <cutils/log.h>
#endif /* use log/log.h start from android 8 major version */
#ifndef ALOGW
#define ALOGW LOGW
#endif
#define dbg_printf(...)	ALOGW(__VA_ARGS__)
#else
#define dbg_printf(...)	fprintf(stderr, __VA_ARGS__)
#endif /* HAVE_ANDROID_PLATFORM */

extern bool v3d_debug_flag_for_shader_stage(gl_shader_stage stage);

extern void v3d_process_debug_variable(void);

#ifdef __cplusplus
}
#endif

#endif /* V3D_DEBUG_H */
