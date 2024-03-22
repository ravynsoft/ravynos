/*
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
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
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file dri_util.h
 * DRI utility functions definitions.
 *
 * \author Kevin E. Martin <kevin@precisioninsight.com>
 * \author Brian Paul <brian@precisioninsight.com>
 */

#ifndef _DRI_UTIL_H_
#define _DRI_UTIL_H_

#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include "kopper_interface.h"
#include "main/formats.h"
#include "main/glconfig.h"
#include "main/menums.h"
#include "util/xmlconfig.h"
#include <stdbool.h>

struct dri_screen;

#define __DRI_BACKEND_VTABLE "DRI_DriverVtable"

struct __DRIconfigRec {
    struct gl_config modes;
};

/**
 * Extensions.
 */
extern const __DRIcoreExtension driCoreExtension;
extern const __DRIswrastExtension driSWRastExtension;
extern const __DRIdri2Extension driDRI2Extension;
extern const __DRIdri2Extension swkmsDRI2Extension;
extern const __DRI2configQueryExtension dri2ConfigQueryExtension;
extern const __DRI2flushControlExtension dri2FlushControlExtension;

/**
 * Description of the attributes used to create a config.
 *
 * This is passed as the context_config parameter to CreateContext. The idea
 * with this struct is that it can be extended without having to modify all of
 * the drivers. The first three members (major/minor_version and flags) are
 * always valid, but the remaining members are only valid if the corresponding
 * flag is set for the attribute. If the flag is not set then the default
 * value should be assumed. That way the driver can quickly check if any
 * attributes were set that it doesn't understand and report an error.
 */
struct __DriverContextConfig {
    /* These members are always valid */
    unsigned major_version;
    unsigned minor_version;
    uint32_t flags;

    /* Flags describing which of the remaining members are valid */
    uint32_t attribute_mask;

    /* Only valid if __DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY is set */
    int reset_strategy;

    /* Only valid if __DRIVER_CONTEXT_PRIORITY is set */
    unsigned priority;

    /* Only valid if __DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR is set */
    int release_behavior;

    /* Only valid if __DRIVER_CONTEXT_ATTRIB_NO_ERROR is set */
    int no_error;

    /* Only valid if __DRIVER_CONTEXT_ATTRIB_PROTECTED is set */
    int protected_context;
};

#define __DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY   (1 << 0)
#define __DRIVER_CONTEXT_ATTRIB_PRIORITY         (1 << 1)
#define __DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR (1 << 2)
#define __DRIVER_CONTEXT_ATTRIB_NO_ERROR         (1 << 3)
#define __DRIVER_CONTEXT_ATTRIB_PROTECTED        (1 << 4)

__DRIscreen *
driCreateNewScreen2(int scrn, int fd,
                    const __DRIextension **loader_extensions,
                    const __DRIextension **driver_extensions,
                    const __DRIconfig ***driver_configs, void *data);
__DRIcontext *
driCreateContextAttribs(__DRIscreen *psp, int api,
                        const __DRIconfig *config,
                        __DRIcontext *shared,
                        unsigned num_attribs,
                        const uint32_t *attribs,
                        unsigned *error,
                        void *data);

extern uint32_t
driGLFormatToImageFormat(mesa_format format);

extern uint32_t
driGLFormatToSizedInternalGLFormat(mesa_format format);

extern mesa_format
driImageFormatToGLFormat(uint32_t image_format);

extern const __DRIimageDriverExtension driImageDriverExtension;

#endif /* _DRI_UTIL_H_ */
