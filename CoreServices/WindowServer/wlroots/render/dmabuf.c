#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <wlr/render/dmabuf.h>
#include <wlr/util/log.h>

void wlr_dmabuf_attributes_finish(struct wlr_dmabuf_attributes *attribs) {
	for (int i = 0; i < attribs->n_planes; ++i) {
		close(attribs->fd[i]);
		attribs->fd[i] = -1;
	}
	attribs->n_planes = 0;
}

bool wlr_dmabuf_attributes_copy(struct wlr_dmabuf_attributes *dst,
		const struct wlr_dmabuf_attributes *src) {
	memcpy(dst, src, sizeof(struct wlr_dmabuf_attributes));

	int i;
	for (i = 0; i < src->n_planes; ++i) {
		dst->fd[i] = fcntl(src->fd[i], F_DUPFD_CLOEXEC, 0);
		if (dst->fd[i] < 0) {
			wlr_log_errno(WLR_ERROR, "fcntl(F_DUPFD_CLOEXEC) failed");
			goto error;
		}
	}

	return true;

error:
	for (int j = 0; j < i; j++) {
		close(dst->fd[i]);
		dst->fd[j] = -1;
	}
	dst->n_planes = 0;
	return false;
}
