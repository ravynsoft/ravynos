#ifndef XLIB_SW_WINSYS
#define XLIB_SW_WINSYS

#include <X11/Xlib.h>

struct sw_winsys;

struct sw_winsys *xlib_create_sw_winsys(Display *display);

#endif
