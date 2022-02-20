#ifndef BACKEND_SESSION_SESSION_H
#define BACKEND_SESSION_SESSION_H

struct wlr_session;

struct wlr_session *libseat_session_create(struct wl_display *disp);
void libseat_session_destroy(struct wlr_session *base);
int libseat_session_open_device(struct wlr_session *base, const char *path);
void libseat_session_close_device(struct wlr_session *base, int fd);
bool libseat_change_vt(struct wlr_session *base, unsigned vt);

void session_init(struct wlr_session *session);

struct wlr_device *session_open_if_kms(struct wlr_session *restrict session,
	const char *restrict path);

#endif
