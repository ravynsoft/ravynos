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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include <GL/gl.h> /* dri_interface needs GL types */
#include <GL/internal/dri_interface.h>

#include "drm-uapi/drm_fourcc.h"
#include "loader_dri_helper.h"
#include "util/driconf.h"

__DRIimage *loader_dri_create_image(__DRIscreen *screen,
                                    const __DRIimageExtension *image,
                                    uint32_t width, uint32_t height,
                                    uint32_t dri_format, uint32_t dri_usage,
                                    const uint64_t *modifiers,
                                    unsigned int modifiers_count,
                                    void *loaderPrivate)
{
   if (modifiers && modifiers_count > 0 &&
       image->base.version > 14 && image->createImageWithModifiers) {
      bool has_valid_modifier = false;
      int i;

      /* It's acceptable to create an image with INVALID modifier in the list,
       * but it cannot be on the only modifier (since it will certainly fail
       * later). While we could easily catch this after modifier creation, doing
       * the check here is a convenient debug check likely pointing at whatever
       * interface the client is using to build its modifier list.
       */
      for (i = 0; i < modifiers_count; i++) {
         if (modifiers[i] != DRM_FORMAT_MOD_INVALID) {
            has_valid_modifier = true;
            break;
         }
      }
      if (!has_valid_modifier)
         return NULL;

      if (image->base.version >= 19 && image->createImageWithModifiers2)
         return image->createImageWithModifiers2(screen, width, height,
                                                 dri_format, modifiers,
                                                 modifiers_count, dri_usage,
                                                 loaderPrivate);
      else
         return image->createImageWithModifiers(screen, width, height,
                                                dri_format, modifiers,
                                                modifiers_count, loaderPrivate);
   }

   /* No modifier given or fallback to the legacy createImage allowed */
   return image->createImage(screen, width, height, dri_format, dri_usage,
                             loaderPrivate);
}

static int dri_vblank_mode(__DRIscreen *driScreen, const __DRI2configQueryExtension *config)
{
   GLint vblank_mode = DRI_CONF_VBLANK_DEF_INTERVAL_1;

   if (config)
      config->configQueryi(driScreen, "vblank_mode", &vblank_mode);

   return vblank_mode;
}

int dri_get_initial_swap_interval(__DRIscreen *driScreen,
                                  const __DRI2configQueryExtension *config)
{
   int vblank_mode = dri_vblank_mode(driScreen, config);

   switch (vblank_mode) {
   case DRI_CONF_VBLANK_NEVER:
   case DRI_CONF_VBLANK_DEF_INTERVAL_0:
      return 0;
   case DRI_CONF_VBLANK_DEF_INTERVAL_1:
   case DRI_CONF_VBLANK_ALWAYS_SYNC:
   default:
      return 1;
   }
}

bool dri_valid_swap_interval(__DRIscreen *driScreen,
                             const __DRI2configQueryExtension *config, int interval)
{
   int vblank_mode = dri_vblank_mode(driScreen, config);

   switch (vblank_mode) {
   case DRI_CONF_VBLANK_NEVER:
      if (interval != 0)
         return false;
      break;
   case DRI_CONF_VBLANK_ALWAYS_SYNC:
      if (interval <= 0)
         return false;
      break;
   default:
      break;
   }

   return true;
}

/* the DRIimage createImage function takes __DRI_IMAGE_FORMAT codes, while
 * the createImageFromFds call takes DRM_FORMAT codes. To avoid
 * complete confusion, just deal in __DRI_IMAGE_FORMAT codes for now and
 * translate to DRM_FORMAT codes in the call to createImageFromFds
 */
int
loader_image_format_to_fourcc(int format)
{

   /* Convert from __DRI_IMAGE_FORMAT to DRM_FORMAT (sigh) */
   switch (format) {
   case __DRI_IMAGE_FORMAT_SARGB8: return __DRI_IMAGE_FOURCC_SARGB8888;
   case __DRI_IMAGE_FORMAT_SABGR8: return __DRI_IMAGE_FOURCC_SABGR8888;
   case __DRI_IMAGE_FORMAT_SXRGB8: return __DRI_IMAGE_FOURCC_SXRGB8888;
   case __DRI_IMAGE_FORMAT_RGB565: return DRM_FORMAT_RGB565;
   case __DRI_IMAGE_FORMAT_XRGB8888: return DRM_FORMAT_XRGB8888;
   case __DRI_IMAGE_FORMAT_ARGB8888: return DRM_FORMAT_ARGB8888;
   case __DRI_IMAGE_FORMAT_ABGR8888: return DRM_FORMAT_ABGR8888;
   case __DRI_IMAGE_FORMAT_XBGR8888: return DRM_FORMAT_XBGR8888;
   case __DRI_IMAGE_FORMAT_XRGB2101010: return DRM_FORMAT_XRGB2101010;
   case __DRI_IMAGE_FORMAT_ARGB2101010: return DRM_FORMAT_ARGB2101010;
   case __DRI_IMAGE_FORMAT_XBGR2101010: return DRM_FORMAT_XBGR2101010;
   case __DRI_IMAGE_FORMAT_ABGR2101010: return DRM_FORMAT_ABGR2101010;
   case __DRI_IMAGE_FORMAT_ABGR16161616: return DRM_FORMAT_ABGR16161616;
   case __DRI_IMAGE_FORMAT_XBGR16161616: return DRM_FORMAT_XBGR16161616;
   case __DRI_IMAGE_FORMAT_XBGR16161616F: return DRM_FORMAT_XBGR16161616F;
   case __DRI_IMAGE_FORMAT_ABGR16161616F: return DRM_FORMAT_ABGR16161616F;
   }
   return 0;
}

#ifdef HAVE_X11_PLATFORM
void
loader_init_screen_resources(struct loader_screen_resources *res,
                             xcb_connection_t *conn,
                             xcb_screen_t *screen)
{
   res->conn = conn;
   res->screen = screen;
   res->crtcs = NULL;

   mtx_init(&res->mtx, mtx_plain);
}

void
loader_destroy_screen_resources(struct loader_screen_resources *res)
{
   mtx_destroy(&res->mtx);
}

static unsigned
gcd_u32(unsigned a, unsigned b)
{
   assert(a > 0 || b > 0);

   while (b != 0) {
      unsigned remainder = a % b;
      a = b;
      b = remainder;
   }

   return a;
}

static void
calculate_refresh_rate(const xcb_randr_mode_info_t *mode,
                       unsigned *numerator, unsigned *denominator)
{
   unsigned vtotal = mode->vtotal;

   /* Double-scan doubles the number of lines */
   if (mode->mode_flags & XCB_RANDR_MODE_FLAG_DOUBLE_SCAN)
      vtotal *= 2;

   /* Interlace splits the frame into two fields; typically the monitor
    * reports field rate.
    */
   if (mode->mode_flags & XCB_RANDR_MODE_FLAG_INTERLACE)
      vtotal /= 2;

   uint32_t dots = mode->htotal * vtotal;

   if (dots == 0) {
      *numerator = 0;
      *denominator = 1;
   } else {
      uint32_t gcd = gcd_u32(mode->dot_clock, dots);

      *numerator = mode->dot_clock / gcd;
      *denominator = dots / gcd;
   }
}

bool
loader_update_screen_resources(struct loader_screen_resources *res)
{
   xcb_randr_get_crtc_info_cookie_t *crtc_cookies;

   /* If we have cached screen resources information, check each CRTC to
    * see if it's up to date.  Ideally, we'd watch PresentConfigureNotify
    * events on the root window to see if something changed, but those only
    * fire if the geometry changes.  It misses CRTC changes which only
    * alter the refresh rate.  We also can't watch RandR events internally
    * because they aren't XGE events.  So, we just check every CRTC for now.
    */
   bool config_unchanged = res->crtcs != NULL;

   crtc_cookies = malloc(res->num_crtcs * sizeof(*crtc_cookies));

   for (unsigned c = 0; c < res->num_crtcs; c++) {
      crtc_cookies[c] =
         xcb_randr_get_crtc_info_unchecked(res->conn, res->crtcs[c].id,
                                           res->config_timestamp);
   }

   for (unsigned c = 0; c < res->num_crtcs; c++) {
      xcb_randr_get_crtc_info_reply_t *reply =
         xcb_randr_get_crtc_info_reply(res->conn, crtc_cookies[c], NULL);

      /* Although randrproto 1.4.0 says that RRGetCrtcInfo is supposed to
       * return InvalidConfigTime if config_timestamp is out of date, the
       * implementation in xserver as of 21.x doesn't actually do so.  To
       * detect changes in refresh rate, we check the returned timestamp
       * on each tracked CRTC.
       */
      if (!reply ||
          reply->status == XCB_RANDR_SET_CONFIG_INVALID_CONFIG_TIME ||
          reply->timestamp != res->crtcs[c].timestamp) {
         config_unchanged = false;
         /* continue to consume all replies */
      }

      free(reply);
   }

   free(crtc_cookies);

   if (config_unchanged)
      return false;

   /* Do RRGetScreenResourcesCurrent to query the list of CRTCs and modes,
    * then RRGetCrtcInfo on each CRTC to determine what mode each uses, and
    * use the mode to calculate the refresh rate.
    */
   mtx_lock(&res->mtx);

   xcb_randr_get_screen_resources_current_cookie_t cookie =
      xcb_randr_get_screen_resources_current_unchecked(res->conn,
                                                       res->screen->root);
   xcb_randr_get_screen_resources_current_reply_t *reply =
      xcb_randr_get_screen_resources_current_reply(res->conn, cookie, NULL);

   xcb_randr_crtc_t *new_crtcs =
      xcb_randr_get_screen_resources_current_crtcs(reply);

   xcb_randr_mode_info_t *new_modes =
      xcb_randr_get_screen_resources_current_modes(reply);

   res->config_timestamp = reply->config_timestamp;

   free(res->crtcs);
   res->crtcs = calloc(reply->num_crtcs, sizeof(*res->crtcs));

   crtc_cookies = malloc(reply->num_crtcs * sizeof(*crtc_cookies));

   for (unsigned c = 0; c < reply->num_crtcs; c++) {
      crtc_cookies[c] =
         xcb_randr_get_crtc_info_unchecked(res->conn, new_crtcs[c],
                                           res->config_timestamp);
   }

   unsigned i = 0;
   for (unsigned c = 0; c < reply->num_crtcs; c++) {
      xcb_randr_get_crtc_info_reply_t *crtc_info =
         xcb_randr_get_crtc_info_reply(res->conn, crtc_cookies[c], NULL);

      if (!crtc_info || crtc_info->mode == XCB_NONE)
         continue;

      res->crtcs[i].id = new_crtcs[c];
      res->crtcs[i].timestamp = crtc_info->timestamp;
      res->crtcs[i].x = crtc_info->x;
      res->crtcs[i].y = crtc_info->y;
      res->crtcs[i].width = crtc_info->width;
      res->crtcs[i].height = crtc_info->height;

      for (int m = 0; m < reply->num_modes; m++) {
         if (new_modes[m].id == crtc_info->mode) {
            calculate_refresh_rate(&new_modes[m],
                                   &res->crtcs[i].refresh_numerator,
                                   &res->crtcs[i].refresh_denominator);
            break;
         }
      }

      i++;
      free(crtc_info);
   }

   res->num_crtcs = i;

   free(crtc_cookies);
   free(reply);

   mtx_unlock(&res->mtx);
   return true;
}
#endif
