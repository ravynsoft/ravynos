/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#ifndef STW_DEVICE_H_
#define STW_DEVICE_H_


#include "util/compiler.h"
#include "frontend/api.h"
#include "util/u_handle_table.h"
#include "util/u_dynarray.h"
#include "util/xmlconfig.h"
#include <GL/gl.h>
#include "stw_gdishim.h"
#include "gldrv.h"
#include "stw_pixelformat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_screen;
struct pipe_frontend_screen;
struct stw_framebuffer;

struct stw_device
{
   const struct stw_winsys *stw_winsys;

   CRITICAL_SECTION screen_mutex;
   bool screen_initialized;
   struct pipe_screen *screen;

   /* Cache some PIPE_CAP_* */
   unsigned max_2d_length;

   struct pipe_frontend_screen *fscreen;

   LUID AdapterLuid;

   struct util_dynarray pixelformats;
   unsigned pixelformat_count;

   struct WGLCALLBACKS callbacks;

   CRITICAL_SECTION ctx_mutex;
   struct handle_table *ctx_table;
   
   /* TODO: use an atomic counter to track the number of locked
    * stw_framebuffer objects.  Assert that the counter is zero when
    * trying to lock this mutex.
    */
   CRITICAL_SECTION fb_mutex;
   struct stw_framebuffer *fb_head;
   
#ifdef DEBUG
   unsigned long memdbg_no;
#endif

   /** WGL_EXT_swap_control */
   int refresh_rate;
   int swap_interval;

   driOptionCache option_cache;
   driOptionCache option_info;
   struct st_config_options st_options;

   bool initialized;
   bool zink;
};


extern struct stw_device *stw_dev;

bool
stw_init_screen(HDC hdc);

struct stw_device *
stw_get_device(void);

char *
stw_get_config_xml(void);

static inline struct stw_context *
stw_lookup_context_locked( DHGLRC dhglrc )
{
   if (dhglrc == 0 || stw_dev == NULL)
      return NULL;
   return (struct stw_context *) handle_table_get(stw_dev->ctx_table, dhglrc);
}


static inline void
stw_lock_contexts(struct stw_device *stw_dev)
{
   EnterCriticalSection(&stw_dev->ctx_mutex);
}


static inline void
stw_unlock_contexts(struct stw_device *stw_dev)
{
   LeaveCriticalSection(&stw_dev->ctx_mutex);
}

static inline struct stw_context *
stw_lookup_context( DHGLRC dhglrc )
{
   struct stw_context *ret;
   stw_lock_contexts(stw_dev);
   ret = stw_lookup_context_locked(dhglrc);
   stw_unlock_contexts(stw_dev);
   return ret;
}


static inline void
stw_lock_framebuffers(struct stw_device *stw_dev)
{
   EnterCriticalSection(&stw_dev->fb_mutex);
}


static inline void
stw_unlock_framebuffers(struct stw_device *stw_dev)
{
   LeaveCriticalSection(&stw_dev->fb_mutex);
}

#ifdef __cplusplus
}
#endif

#endif /* STW_DEVICE_H_ */
