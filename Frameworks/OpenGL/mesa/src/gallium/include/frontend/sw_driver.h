
#ifndef _SW_DRIVER_H_
#define _SW_DRIVER_H_

#include "util/compiler.h"

struct pipe_screen;
struct sw_winsys;
struct drisw_loader_funcs;

struct sw_driver_descriptor
{
   struct pipe_screen *(*create_screen)(struct sw_winsys *ws, const struct pipe_screen_config *config, bool sw_vk);
   struct {
       const char * const name;
       union {
         struct sw_winsys *(*create_winsys)();
         struct sw_winsys *(*create_winsys_wrapped)(struct pipe_screen *screen);
         struct sw_winsys *(*create_winsys_dri)(const struct drisw_loader_funcs *lf);
         struct sw_winsys *(*create_winsys_kms_dri)(int fd);
       };
   } winsys[];
};

#endif
