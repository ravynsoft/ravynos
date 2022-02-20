#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "egl_common.h"

EGLDisplay egl_display;
EGLConfig egl_config;
EGLContext egl_context;

PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC eglCreatePlatformWindowSurfaceEXT;

const EGLint config_attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_ALPHA_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_NONE,
};

const EGLint context_attribs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE,
};

bool egl_init(struct wl_display *display) {
	const char *client_exts_str =
		eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (client_exts_str == NULL) {
		if (eglGetError() == EGL_BAD_DISPLAY) {
			fprintf(stderr, "EGL_EXT_client_extensions not supported\n");
		} else {
			fprintf(stderr, "Failed to query EGL client extensions\n");
		}
		return false;
	}

	if (!strstr(client_exts_str, "EGL_EXT_platform_base")) {
		fprintf(stderr, "EGL_EXT_platform_base not supported\n");
		return false;
	}

	if (!strstr(client_exts_str, "EGL_EXT_platform_wayland")) {
		fprintf(stderr, "EGL_EXT_platform_wayland not supported\n");
		return false;
	}

	eglGetPlatformDisplayEXT =
		(void *)eglGetProcAddress("eglGetPlatformDisplayEXT");
	if (eglGetPlatformDisplayEXT == NULL) {
		fprintf(stderr, "Failed to get eglGetPlatformDisplayEXT\n");
		return false;
	}

	eglCreatePlatformWindowSurfaceEXT =
		(void *)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
	if (eglCreatePlatformWindowSurfaceEXT == NULL) {
		fprintf(stderr, "Failed to get eglCreatePlatformWindowSurfaceEXT\n");
		return false;
	}

	egl_display =
		eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_EXT,
			display, NULL);
	if (egl_display == EGL_NO_DISPLAY) {
		fprintf(stderr, "Failed to create EGL display\n");
		goto error;
	}

	if (eglInitialize(egl_display, NULL, NULL) == EGL_FALSE) {
		fprintf(stderr, "Failed to initialize EGL\n");
		goto error;
	}

	EGLint matched = 0;
	if (!eglChooseConfig(egl_display, config_attribs,
			&egl_config, 1, &matched)) {
		fprintf(stderr, "eglChooseConfig failed\n");
		goto error;
	}
	if (matched == 0) {
		fprintf(stderr, "Failed to match an EGL config\n");
		goto error;
	}

	egl_context =
		eglCreateContext(egl_display, egl_config,
			EGL_NO_CONTEXT, context_attribs);
	if (egl_context == EGL_NO_CONTEXT) {
		fprintf(stderr, "Failed to create EGL context\n");
		goto error;
	}

	return true;

error:
	eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE,
		EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (egl_display) {
		eglTerminate(egl_display);
	}
	eglReleaseThread();
	return false;
}

void egl_finish(void) {
	eglMakeCurrent(egl_display, EGL_NO_SURFACE,
		EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(egl_display, egl_context);
	eglTerminate(egl_display);
	eglReleaseThread();
}
