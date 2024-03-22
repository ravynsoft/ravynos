/*
 * Copyright © 2013 Keith Packard
 * Copyright © 2015 Boyan Ding
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef LOADER_DRI3_HEADER_H
#define LOADER_DRI3_HEADER_H

#include <stdbool.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>

#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include <c11/threads.h>

enum loader_dri3_buffer_type {
   loader_dri3_buffer_back = 0,
   loader_dri3_buffer_front = 1
};

struct loader_dri3_buffer {
   __DRIimage   *image;
   uint32_t     pixmap;

   /* default case: linear buffer allocated in render gpu vram.
    * p2p case: linear buffer allocated in display gpu vram and imported
    *           to render gpu. p2p case is enabled when driver name matches
    *           while creating screen in dri3_create_screen() function.
    */
   __DRIimage   *linear_buffer;

   /* Synchronization between the client and X server is done using an
    * xshmfence that is mapped into an X server SyncFence. This lets the
    * client check whether the X server is done using a buffer with a simple
    * xshmfence call, rather than going to read X events from the wire.
    *
    * However, we can only wait for one xshmfence to be triggered at a time,
    * so we need to know *which* buffer is going to be idle next. We do that
    * by waiting for a PresentIdleNotify event. When that event arrives, the
    * 'busy' flag gets cleared and the client knows that the fence has been
    * triggered, and that the wait call will not block.
    */

   uint32_t     sync_fence;     /* XID of X SyncFence object */
   struct xshmfence *shm_fence; /* pointer to xshmfence object */
   bool         busy;           /* Set on swap, cleared on IdleNotify */
   bool         own_pixmap;     /* We allocated the pixmap ID, free on destroy */
   bool         reallocate;     /* Buffer should be reallocated and not reused */

   uint32_t     num_planes;
   uint32_t     size;
   int          strides[4];
   int          offsets[4];
   uint64_t     modifier;
   uint32_t     cpp;
   uint32_t     flags;
   uint32_t     width, height;
   uint64_t     last_swap;
};


#define LOADER_DRI3_MAX_BACK   4
#define LOADER_DRI3_BACK_ID(i) (i)
#define LOADER_DRI3_FRONT_ID   (LOADER_DRI3_MAX_BACK)

static inline int
loader_dri3_pixmap_buf_id(enum loader_dri3_buffer_type buffer_type)
{
   if (buffer_type == loader_dri3_buffer_back)
      return LOADER_DRI3_BACK_ID(0);
   else
      return LOADER_DRI3_FRONT_ID;
}

struct loader_dri3_extensions {
   const __DRIcoreExtension *core;
   const __DRIimageDriverExtension *image_driver;
   const __DRI2flushExtension *flush;
   const __DRI2configQueryExtension *config;
   const __DRItexBufferExtension *tex_buffer;
   const __DRIimageExtension *image;
};

struct loader_dri3_drawable;

struct loader_dri3_vtable {
   void (*set_drawable_size)(struct loader_dri3_drawable *, int, int);
   bool (*in_current_context)(struct loader_dri3_drawable *);
   __DRIcontext *(*get_dri_context)(struct loader_dri3_drawable *);
   __DRIscreen *(*get_dri_screen)(void);
   void (*flush_drawable)(struct loader_dri3_drawable *, unsigned);
};

#define LOADER_DRI3_NUM_BUFFERS (1 + LOADER_DRI3_MAX_BACK)

enum loader_dri3_drawable_type {
   LOADER_DRI3_DRAWABLE_UNKNOWN,
   LOADER_DRI3_DRAWABLE_WINDOW,
   LOADER_DRI3_DRAWABLE_PIXMAP,
   LOADER_DRI3_DRAWABLE_PBUFFER,
};

struct loader_dri3_drawable {
   xcb_connection_t *conn;
   xcb_screen_t *screen;
   __DRIdrawable *dri_drawable;
   xcb_drawable_t drawable;
   xcb_window_t window;
   xcb_xfixes_region_t region;
   int width;
   int height;
   int depth;
   uint8_t have_back;
   uint8_t have_fake_front;
   enum loader_dri3_drawable_type type;

   /* Information about the GPU owning the buffer */
   bool multiplanes_available;
   bool prefer_back_buffer_reuse;
   __DRIscreen *dri_screen_render_gpu;
   /* dri_screen_display_gpu holds display GPU in case of prime gpu offloading else
    * dri_screen_render_gpu and dri_screen_display_gpu is same.
    * In case of prime gpu offloading, if display and render driver names are different
    * (potentially not compatible), dri_screen_display_gpu will be NULL but fd_display_gpu
    * will still hold fd for display driver.
    */
   __DRIscreen *dri_screen_display_gpu;

   /* SBC numbers are tracked by using the serial numbers
    * in the present request and complete events
    */
   uint64_t send_sbc;
   uint64_t recv_sbc;

   /* Last received UST/MSC values for pixmap present complete */
   uint64_t ust, msc;

   /* Last received UST/MSC values from present notify msc event */
   uint64_t notify_ust, notify_msc;

   struct loader_dri3_buffer *buffers[LOADER_DRI3_NUM_BUFFERS];
   int cur_back;
   int cur_num_back;
   int max_num_back;
   int cur_blit_source;

   uint32_t *stamp;

   xcb_present_event_t eid;
   xcb_gcontext_t gc;
   xcb_special_event_t *special_event;

   bool first_init;
   bool adaptive_sync;
   bool adaptive_sync_active;
   bool block_on_depleted_buffers;
   bool queries_buffer_age;
   int swap_interval;

   struct loader_dri3_extensions *ext;
   const struct loader_dri3_vtable *vtable;

   unsigned int back_format;
   xcb_present_complete_mode_t last_present_mode;

   bool is_protected_content;

   /* Currently protects the following fields:
    * event_cnd, has_event_waiter,
    * recv_sbc, ust, msc, recv_msc_serial,
    * notify_ust, notify_msc
    */
   mtx_t mtx;
   cnd_t event_cnd;
   unsigned last_special_event_sequence;
   bool has_event_waiter;
};

void
loader_dri3_set_swap_interval(struct loader_dri3_drawable *draw,
                              int interval);

void
loader_dri3_drawable_fini(struct loader_dri3_drawable *draw);

int
loader_dri3_drawable_init(xcb_connection_t *conn,
                          xcb_drawable_t drawable,
                          enum loader_dri3_drawable_type type,
                          __DRIscreen *dri_screen_render_gpu,
                          __DRIscreen *dri_screen_display_gpu,
                          bool is_multiplanes_available,
                          bool prefer_back_buffer_reuse,
                          const __DRIconfig *dri_config,
                          struct loader_dri3_extensions *ext,
                          const struct loader_dri3_vtable *vtable,
                          struct loader_dri3_drawable*);

bool loader_dri3_wait_for_msc(struct loader_dri3_drawable *draw,
                              int64_t target_msc,
                              int64_t divisor, int64_t remainder,
                              int64_t *ust, int64_t *msc, int64_t *sbc);

int64_t
loader_dri3_swap_buffers_msc(struct loader_dri3_drawable *draw,
                             int64_t target_msc, int64_t divisor,
                             int64_t remainder, unsigned flush_flags,
                             const int *rects, int n_rects,
                             bool force_copy);

int
loader_dri3_wait_for_sbc(struct loader_dri3_drawable *draw,
                         int64_t target_sbc, int64_t *ust,
                         int64_t *msc, int64_t *sbc);

int loader_dri3_query_buffer_age(struct loader_dri3_drawable *draw);

void
loader_dri3_flush(struct loader_dri3_drawable *draw,
                  unsigned flags,
                  enum __DRI2throttleReason throttle_reason);

void
loader_dri3_copy_sub_buffer(struct loader_dri3_drawable *draw,
                            int x, int y,
                            int width, int height,
                            bool flush);

void
loader_dri3_copy_drawable(struct loader_dri3_drawable *draw,
                          xcb_drawable_t dest,
                          xcb_drawable_t src);

void
loader_dri3_wait_x(struct loader_dri3_drawable *draw);

void
loader_dri3_wait_gl(struct loader_dri3_drawable *draw);

int loader_dri3_open(xcb_connection_t *conn,
                     xcb_window_t root,
                     uint32_t provider);

__DRIimage *
loader_dri3_create_image(xcb_connection_t *c,
                         xcb_dri3_buffer_from_pixmap_reply_t *bp_reply,
                         unsigned int format,
                         __DRIscreen *dri_screen,
                         const __DRIimageExtension *image,
                         void *loaderPrivate);

#ifdef HAVE_DRI3_MODIFIERS
__DRIimage *
loader_dri3_create_image_from_buffers(xcb_connection_t *c,
                                      xcb_dri3_buffers_from_pixmap_reply_t *bp_reply,
                                      unsigned int format,
                                      __DRIscreen *dri_screen,
                                      const __DRIimageExtension *image,
                                      void *loaderPrivate);
#endif
int
loader_dri3_get_buffers(__DRIdrawable *driDrawable,
                        unsigned int format,
                        uint32_t *stamp,
                        void *loaderPrivate,
                        uint32_t buffer_mask,
                        struct __DRIimageList *buffers);

void
loader_dri3_update_drawable_geometry(struct loader_dri3_drawable *draw);

void
loader_dri3_swapbuffer_barrier(struct loader_dri3_drawable *draw);

void
loader_dri3_close_screen(__DRIscreen *dri_screen);

#endif
