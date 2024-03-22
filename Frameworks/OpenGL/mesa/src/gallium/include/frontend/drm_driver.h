
#ifndef _DRM_DRIVER_H_
#define _DRM_DRIVER_H_

#include "util/compiler.h"

#include "winsys_handle.h"

struct pipe_screen;
struct pipe_screen_config;
struct pipe_context;
struct pipe_resource;

struct drm_driver_descriptor
{
   /**
    * Identifying prefix/suffix of the binary, used by the pipe-loader.
    */
   const char *driver_name;

   /**
    * Optional pointer to the array of driOptionDescription describing
    * driver-specific driconf options.
    */
   const struct driOptionDescription *driconf;

   /* Number of entries in the driconf array. */
   unsigned driconf_count;

   /**
    * Create a pipe srcreen.
    *
    * This function does any wrapping of the screen.
    * For example wrapping trace or rbug debugging drivers around it.
    */
   struct pipe_screen* (*create_screen)(int drm_fd,
                                        const struct pipe_screen_config *config);
};

extern const struct drm_driver_descriptor driver_descriptor;

#endif
