#ifndef _SEATD_DRM_H
#define _SEATD_DRM_H

int drm_set_master(int fd);
int drm_drop_master(int fd);
int path_is_drm(const char *path);

#endif
