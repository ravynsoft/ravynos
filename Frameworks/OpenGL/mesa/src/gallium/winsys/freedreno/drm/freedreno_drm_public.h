
#ifndef __FREEDRENO_DRM_PUBLIC_H__
#define __FREEDRENO_DRM_PUBLIC_H__

struct pipe_screen;
struct renderonly;

struct pipe_screen *fd_drm_screen_create_renderonly(int drmFD,
                                                    struct renderonly *ro,
                                                    const struct pipe_screen_config *config);

#endif
