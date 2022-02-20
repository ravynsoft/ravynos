/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_RENDER_EGL_H
#define WLR_RENDER_EGL_H

#ifndef MESA_EGL_NO_X11_HEADERS
#define MESA_EGL_NO_X11_HEADERS
#endif
#ifndef EGL_NO_X11
#define EGL_NO_X11
#endif
#ifndef EGL_NO_PLATFORM_SPECIFIC_TYPES
#define EGL_NO_PLATFORM_SPECIFIC_TYPES
#endif

#include <wlr/config.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <pixman.h>
#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/render/dmabuf.h>
#include <wlr/render/drm_format_set.h>

struct wlr_egl {
	EGLDisplay display;
	EGLContext context;
	EGLDeviceEXT device; // may be EGL_NO_DEVICE_EXT
	struct gbm_device *gbm_device;

	struct {
		// Display extensions
		bool KHR_image_base;
		bool EXT_image_dma_buf_import;
		bool EXT_image_dma_buf_import_modifiers;
		bool IMG_context_priority;

		// Device extensions
		bool EXT_device_drm;
		bool EXT_device_drm_render_node;

		// Client extensions
		bool EXT_device_query;
		bool KHR_platform_gbm;
		bool EXT_platform_device;
	} exts;

	struct {
		PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
		PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
		PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
		PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
		PFNEGLQUERYDMABUFFORMATSEXTPROC eglQueryDmaBufFormatsEXT;
		PFNEGLQUERYDMABUFMODIFIERSEXTPROC eglQueryDmaBufModifiersEXT;
		PFNEGLDEBUGMESSAGECONTROLKHRPROC eglDebugMessageControlKHR;
		PFNEGLQUERYDISPLAYATTRIBEXTPROC eglQueryDisplayAttribEXT;
		PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT;
		PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT;
	} procs;

	bool has_modifiers;
	struct wlr_drm_format_set dmabuf_texture_formats;
	struct wlr_drm_format_set dmabuf_render_formats;
};

struct wlr_egl *wlr_egl_create_with_context(EGLDisplay display,
	EGLContext context);

/**
 * Make the EGL context current.
 *
 * Callers are expected to clear the current context when they are done by
 * calling wlr_egl_unset_current.
 */
bool wlr_egl_make_current(struct wlr_egl *egl);

bool wlr_egl_unset_current(struct wlr_egl *egl);

bool wlr_egl_is_current(struct wlr_egl *egl);

#endif
