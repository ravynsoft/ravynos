/*
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

#ifndef LOADER_DRI_HELPER_H
#define LOADER_DRI_HELPER_H

#include <stdbool.h>
#include <sys/types.h>

#include <GL/gl.h> /* dri_interface needs GL types */
#include <GL/internal/dri_interface.h>
#include <c11/threads.h>

#ifdef HAVE_X11_PLATFORM
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>

struct loader_crtc_info {
   xcb_randr_crtc_t id;
   xcb_timestamp_t timestamp;

   int16_t x, y;
   uint16_t width, height;

   unsigned refresh_numerator;
   unsigned refresh_denominator;
};

struct loader_screen_resources {
   mtx_t mtx;

   xcb_connection_t *conn;
   xcb_screen_t *screen;

   xcb_timestamp_t config_timestamp;

   /* Number of CRTCs with an active mode set */
   unsigned num_crtcs;
   struct loader_crtc_info *crtcs;
};
#endif

__DRIimage *loader_dri_create_image(__DRIscreen *screen,
                                    const __DRIimageExtension *image,
                                    uint32_t width, uint32_t height,
                                    uint32_t dri_format, uint32_t dri_usage,
                                    const uint64_t *modifiers,
                                    unsigned int modifiers_count,
                                    void *loaderPrivate);

int dri_get_initial_swap_interval(__DRIscreen *driScreen,
                                  const __DRI2configQueryExtension *config);

bool dri_valid_swap_interval(__DRIscreen *driScreen,
                             const __DRI2configQueryExtension *config, int interval);

int
loader_image_format_to_fourcc(int format);

#ifdef HAVE_X11_PLATFORM
void
loader_init_screen_resources(struct loader_screen_resources *res,
                             xcb_connection_t *conn,
                             xcb_screen_t *screen);
bool
loader_update_screen_resources(struct loader_screen_resources *res);

void
loader_destroy_screen_resources(struct loader_screen_resources *res);
#endif

#endif /* LOADER_DRI_HELPER_H */
