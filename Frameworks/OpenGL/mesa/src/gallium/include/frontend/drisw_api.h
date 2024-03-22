#ifndef _DRISW_API_H_
#define _DRISW_API_H_

#include "util/compiler.h"
#include "sw_winsys.h"

struct pipe_screen;
struct dri_drawable;

/**
 * This callback struct is intended for the winsys to call the loader.
 */
struct drisw_loader_funcs
{
   void (*get_image) (struct dri_drawable *dri_drawable,
                      int x, int y, unsigned width, unsigned height, unsigned stride,
                      void *data);
   void (*put_image) (struct dri_drawable *dri_drawable,
                      void *data, unsigned width, unsigned height);
   void (*put_image2) (struct dri_drawable *dri_drawable,
                       void *data, int x, int y, unsigned width, unsigned height, unsigned stride);
   void (*put_image_shm) (struct dri_drawable *dri_drawable,
                          int shmid, char *shmaddr, unsigned offset, unsigned offset_x,
                          int x, int y, unsigned width, unsigned height, unsigned stride);
};

#endif
