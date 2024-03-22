/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#ifndef SP_SCREEN_H
#define SP_SCREEN_H

#include "pipe/p_screen.h"
#include "pipe/p_defines.h"


struct sw_winsys;

struct softpipe_screen {
   struct pipe_screen base;

   struct sw_winsys *winsys;

   /* Increments whenever textures are modified.  Contexts can track
    * this.
    */
   unsigned timestamp;
   bool use_llvm;
};

static inline struct softpipe_screen *
softpipe_screen( struct pipe_screen *pipe )
{
   return (struct softpipe_screen *)pipe;
}

enum sp_debug_flag {
   SP_DBG_VS              = BITFIELD_BIT(0),
   /* SP_DBG_TCS             = BITFIELD_BIT(1), */
   /* SP_DBG_TES             = BITFIELD_BIT(2), */
   SP_DBG_GS              = BITFIELD_BIT(3),
   SP_DBG_FS              = BITFIELD_BIT(4),
   SP_DBG_CS              = BITFIELD_BIT(5),
   SP_DBG_USE_LLVM        = BITFIELD_BIT(6),
   SP_DBG_NO_RAST         = BITFIELD_BIT(7),
};

extern int sp_debug;

#endif /* SP_SCREEN_H */
