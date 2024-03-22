#ifndef SP_PUBLIC_H
#define SP_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

struct pipe_screen;
struct sw_winsys;

struct pipe_screen *
softpipe_create_screen(struct sw_winsys *winsys);

#ifdef __cplusplus
}
#endif

#endif
