#ifndef RENDER_GLES2_H
#define RENDER_GLES2_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wlr/backend.h>
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#include <wlr/render/interface.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/addon.h>
#include <wlr/util/log.h>

struct wlr_gles2_pixel_format {
	uint32_t drm_format;
	GLint gl_format, gl_type;
	bool has_alpha;
};

struct wlr_gles2_tex_shader {
	GLuint program;
	GLint proj;
	GLint tex;
	GLint alpha;
	GLint pos_attrib;
	GLint tex_attrib;
};

struct wlr_gles2_renderer {
	struct wlr_renderer wlr_renderer;

	float projection[9];
	struct wlr_egl *egl;
	int drm_fd;

	const char *exts_str;
	struct {
		bool EXT_read_format_bgra;
		bool KHR_debug;
		bool OES_egl_image_external;
		bool OES_egl_image;
		bool EXT_texture_type_2_10_10_10_REV;
		bool OES_texture_half_float_linear;
	} exts;

	struct {
		PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
		PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
		PFNGLDEBUGMESSAGECONTROLKHRPROC glDebugMessageControlKHR;
		PFNGLPOPDEBUGGROUPKHRPROC glPopDebugGroupKHR;
		PFNGLPUSHDEBUGGROUPKHRPROC glPushDebugGroupKHR;
		PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES;
	} procs;

	struct {
		struct {
			GLuint program;
			GLint proj;
			GLint color;
			GLint pos_attrib;
		} quad;
		struct wlr_gles2_tex_shader tex_rgba;
		struct wlr_gles2_tex_shader tex_rgbx;
		struct wlr_gles2_tex_shader tex_ext;
	} shaders;

	struct wl_list buffers; // wlr_gles2_buffer.link
	struct wl_list textures; // wlr_gles2_texture.link

	struct wlr_gles2_buffer *current_buffer;
	uint32_t viewport_width, viewport_height;
};

struct wlr_gles2_buffer {
	struct wlr_buffer *buffer;
	struct wlr_gles2_renderer *renderer;
	struct wl_list link; // wlr_gles2_renderer.buffers

	EGLImageKHR image;
	GLuint rbo;
	GLuint fbo;

	struct wlr_addon addon;
};

struct wlr_gles2_texture {
	struct wlr_texture wlr_texture;
	struct wlr_gles2_renderer *renderer;
	struct wl_list link; // wlr_gles2_renderer.textures

	// Basically:
	//   GL_TEXTURE_2D == mutable
	//   GL_TEXTURE_EXTERNAL_OES == immutable
	GLenum target;
	GLuint tex;

	EGLImageKHR image;

	bool has_alpha;

	// Only affects target == GL_TEXTURE_2D
	uint32_t drm_format; // used to interpret upload data
	// If imported from a wlr_buffer
	struct wlr_buffer *buffer;
	struct wlr_addon buffer_addon;
};


bool is_gles2_pixel_format_supported(const struct wlr_gles2_renderer *renderer,
	const struct wlr_gles2_pixel_format *format);
const struct wlr_gles2_pixel_format *get_gles2_format_from_drm(uint32_t fmt);
const struct wlr_gles2_pixel_format *get_gles2_format_from_gl(
	GLint gl_format, GLint gl_type, bool alpha);
const uint32_t *get_gles2_shm_formats(const struct wlr_gles2_renderer *renderer,
	size_t *len);

struct wlr_gles2_renderer *gles2_get_renderer(
	struct wlr_renderer *wlr_renderer);
struct wlr_gles2_texture *gles2_get_texture(
	struct wlr_texture *wlr_texture);

struct wlr_texture *gles2_texture_from_wl_drm(struct wlr_renderer *wlr_renderer,
	struct wl_resource *data);
struct wlr_texture *gles2_texture_from_buffer(struct wlr_renderer *wlr_renderer,
	struct wlr_buffer *buffer);
void gles2_texture_destroy(struct wlr_gles2_texture *texture);

void push_gles2_debug_(struct wlr_gles2_renderer *renderer,
	const char *file, const char *func);
#define push_gles2_debug(renderer) push_gles2_debug_(renderer, _WLR_FILENAME, __func__)
void pop_gles2_debug(struct wlr_gles2_renderer *renderer);

#endif
