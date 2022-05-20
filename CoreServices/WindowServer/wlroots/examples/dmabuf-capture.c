#define _POSIX_C_SOURCE 199309L
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/display.h>
#include <libavutil/hwcontext_drm.h>
#include <libavutil/pixdesc.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <drm_fourcc.h>
#include "wlr-export-dmabuf-unstable-v1-client-protocol.h"

struct wayland_output {
	struct wl_list link;
	uint32_t id;
	struct wl_output *output;
	char *make;
	char *model;
	int width;
	int height;
	AVRational framerate;
};

struct fifo_buffer {
	AVFrame **queued_frames;
	int num_queued_frames;
	int max_queued_frames;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pthread_mutex_t cond_lock;
};

struct capture_context {
	AVClass *class; /* For pretty logging */
	struct wl_display *display;
	struct wl_registry *registry;
	struct zwlr_export_dmabuf_manager_v1 *export_manager;

	struct wl_list output_list;

	/* Target */
	struct wl_output *target_output;
	bool with_cursor;

	/* Main frame callback */
	struct zwlr_export_dmabuf_frame_v1 *frame_callback;

	/* If something happens during capture */
	int err;
	bool quit;

	/* FFmpeg specific parts */
	pthread_t vid_thread;
	AVFrame *current_frame;
	AVFormatContext *avf;
	AVCodecContext *avctx;
	AVBufferRef *drm_device_ref;
	AVBufferRef *drm_frames_ref;
	AVBufferRef *mapped_device_ref;
	AVBufferRef *mapped_frames_ref;

	/* Sync stuff */
	struct fifo_buffer vid_frames;

	int64_t start_pts;

	/* Config */
	enum AVPixelFormat software_format;
	enum AVHWDeviceType hw_device_type;
	AVDictionary *encoder_opts;
	int is_software_encoder;
	char *hardware_device;
	char *out_filename;
	char *encoder_name;
	float out_bitrate;
};

static int init_fifo(struct fifo_buffer *buf, int max_queued_frames) {
	pthread_mutex_init(&buf->lock, NULL);
	pthread_cond_init(&buf->cond, NULL);
	pthread_mutex_init(&buf->cond_lock, NULL);
	buf->num_queued_frames = 0;
	buf->max_queued_frames = max_queued_frames;
	buf->queued_frames = av_mallocz(buf->max_queued_frames * sizeof(AVFrame));
	return !buf->queued_frames ? AVERROR(ENOMEM) : 0;
}

static int get_fifo_size(struct fifo_buffer *buf) {
	pthread_mutex_lock(&buf->lock);
	int ret = buf->num_queued_frames;
	pthread_mutex_unlock(&buf->lock);
	return ret;
}

static int push_to_fifo(struct fifo_buffer *buf, AVFrame *f) {
	int ret;
	pthread_mutex_lock(&buf->lock);
	if ((buf->num_queued_frames + 1) > buf->max_queued_frames) {
		av_frame_free(&f);
		ret = 1;
	} else {
		buf->queued_frames[buf->num_queued_frames++] = f;
		ret = 0;
	}
	pthread_mutex_unlock(&buf->lock);
	pthread_cond_signal(&buf->cond);
	return ret;
}

static AVFrame *pop_from_fifo(struct fifo_buffer *buf) {
	pthread_mutex_lock(&buf->lock);

	if (!buf->num_queued_frames) {
		pthread_mutex_unlock(&buf->lock);
		pthread_cond_wait(&buf->cond, &buf->cond_lock);
		pthread_mutex_lock(&buf->lock);
	}

	AVFrame *rf = buf->queued_frames[0];
	for (int i = 1; i < buf->num_queued_frames; i++) {
		buf->queued_frames[i - 1] = buf->queued_frames[i];
	}
	buf->num_queued_frames--;
	buf->queued_frames[buf->num_queued_frames] = NULL;

	pthread_mutex_unlock(&buf->lock);
	return rf;
}

static void free_fifo(struct fifo_buffer *buf) {
	pthread_mutex_lock(&buf->lock);
	if (buf->num_queued_frames) {
		for (int i = 0; i < buf->num_queued_frames; i++) {
			av_frame_free(&buf->queued_frames[i]);
		}
	}
	av_freep(&buf->queued_frames);
	pthread_mutex_unlock(&buf->lock);
}

static void output_handle_geometry(void *data, struct wl_output *wl_output,
		int32_t x, int32_t y, int32_t phys_width, int32_t phys_height,
		int32_t subpixel, const char *make, const char *model,
		int32_t transform) {
	struct wayland_output *output = data;
	output->make = av_strdup(make);
	output->model = av_strdup(model);
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
		uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	if (flags & WL_OUTPUT_MODE_CURRENT) {
		struct wayland_output *output = data;
		output->width = width;
		output->height = height;
		output->framerate = (AVRational){ refresh, 1000 };
	}
}

static void output_handle_done(void* data, struct wl_output *wl_output) {
	/* Nothing to do */
}

static void output_handle_scale(void* data, struct wl_output *wl_output,
		int32_t factor) {
	/* Nothing to do */
}

static const struct wl_output_listener output_listener = {
	.geometry = output_handle_geometry,
	.mode = output_handle_mode,
	.done = output_handle_done,
	.scale = output_handle_scale,
};

static void registry_handle_add(void *data, struct wl_registry *reg,
		uint32_t id, const char *interface, uint32_t ver) {
	struct capture_context *ctx = data;

	if (!strcmp(interface, wl_output_interface.name)) {
		struct wayland_output *output = av_mallocz(sizeof(*output));

		output->id = id;
		output->output = wl_registry_bind(reg, id, &wl_output_interface, 1);

		wl_output_add_listener(output->output, &output_listener, output);
		wl_list_insert(&ctx->output_list, &output->link);
	}

	if (!strcmp(interface, zwlr_export_dmabuf_manager_v1_interface.name)) {
		ctx->export_manager = wl_registry_bind(reg, id,
				&zwlr_export_dmabuf_manager_v1_interface, 1);
	}
}

static void remove_output(struct wayland_output *out) {
	wl_list_remove(&out->link);
	av_free(out->make);
	av_free(out->model);
	av_free(out);
}

static struct wayland_output *find_output(struct capture_context *ctx,
		struct wl_output *out, int id) {
	struct wayland_output *output, *tmp;
	wl_list_for_each_safe(output, tmp, &ctx->output_list, link) {
		if (output->output == out || (id >= 0 && output->id == (uint32_t)id)
				|| id == -1) {
			return output;
		}
	}
	return NULL;
}

static void registry_handle_remove(void *data, struct wl_registry *reg,
		uint32_t id) {
	remove_output(find_output((struct capture_context *)data, NULL, id));
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_add,
	.global_remove = registry_handle_remove,
};

static void frame_free(void *opaque, uint8_t *data) {
	AVDRMFrameDescriptor *desc = (AVDRMFrameDescriptor *)data;

	if (desc) {
		for (int i = 0; i < desc->nb_objects; ++i) {
			close(desc->objects[i].fd);
		}
		av_free(data);
	}

	zwlr_export_dmabuf_frame_v1_destroy(opaque);
}

static void frame_start(void *data, struct zwlr_export_dmabuf_frame_v1 *frame,
		uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y,
		uint32_t buffer_flags, uint32_t flags, uint32_t format,
		uint32_t mod_high, uint32_t mod_low, uint32_t num_objects) {
	struct capture_context *ctx = data;
	int err = 0;

	/* Allocate DRM specific struct */
	AVDRMFrameDescriptor *desc = av_mallocz(sizeof(*desc));
	if (!desc) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	desc->nb_objects = num_objects;
	desc->objects[0].format_modifier = ((uint64_t)mod_high << 32) | mod_low;

	desc->nb_layers = 1;
	desc->layers[0].format = format;

	/* Allocate a frame */
	AVFrame *f = av_frame_alloc();
	if (!f) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	/* Set base frame properties */
	ctx->current_frame = f;
	f->width = width;
	f->height = height;
	f->format = AV_PIX_FMT_DRM_PRIME;

	/* Set the frame data to the DRM specific struct */
	f->buf[0] = av_buffer_create((uint8_t*)desc, sizeof(*desc),
			&frame_free, frame, 0);
	if (!f->buf[0]) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	f->data[0] = (uint8_t*)desc;

	return;

fail:
	ctx->err = err;
	frame_free(frame, (uint8_t *)desc);
}

static void frame_object(void *data, struct zwlr_export_dmabuf_frame_v1 *frame,
		uint32_t index, int32_t fd, uint32_t size, uint32_t offset,
		uint32_t stride, uint32_t plane_index) {
	struct capture_context *ctx = data;
	AVFrame *f = ctx->current_frame;
	AVDRMFrameDescriptor *desc = (AVDRMFrameDescriptor *)f->data[0];

	desc->objects[index].fd = fd;
	desc->objects[index].size = size;

	desc->layers[0].planes[plane_index].object_index = index;
	desc->layers[0].planes[plane_index].offset = offset;
	desc->layers[0].planes[plane_index].pitch = stride;
}

static enum AVPixelFormat drm_fmt_to_pixfmt(uint32_t fmt) {
	switch (fmt) {
	case DRM_FORMAT_NV12: return AV_PIX_FMT_NV12;
	case DRM_FORMAT_ARGB8888: return AV_PIX_FMT_BGRA;
	case DRM_FORMAT_XRGB8888: return AV_PIX_FMT_BGR0;
	case DRM_FORMAT_ABGR8888: return AV_PIX_FMT_RGBA;
	case DRM_FORMAT_XBGR8888: return AV_PIX_FMT_RGB0;
	case DRM_FORMAT_RGBA8888: return AV_PIX_FMT_ABGR;
	case DRM_FORMAT_RGBX8888: return AV_PIX_FMT_0BGR;
	case DRM_FORMAT_BGRA8888: return AV_PIX_FMT_ARGB;
	case DRM_FORMAT_BGRX8888: return AV_PIX_FMT_0RGB;
	default: return AV_PIX_FMT_NONE;
	};
}

static int attach_drm_frames_ref(struct capture_context *ctx, AVFrame *f,
		enum AVPixelFormat sw_format) {
	int err = 0;
	AVHWFramesContext *hwfc;

	if (ctx->drm_frames_ref) {
		hwfc = (AVHWFramesContext*)ctx->drm_frames_ref->data;
		if (hwfc->width == f->width && hwfc->height == f->height &&
				hwfc->sw_format == sw_format) {
			goto attach;
		}
		av_buffer_unref(&ctx->drm_frames_ref);
	}

	ctx->drm_frames_ref = av_hwframe_ctx_alloc(ctx->drm_device_ref);
	if (!ctx->drm_frames_ref) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	hwfc = (AVHWFramesContext*)ctx->drm_frames_ref->data;

	hwfc->format = f->format;
	hwfc->sw_format = sw_format;
	hwfc->width = f->width;
	hwfc->height = f->height;

	err = av_hwframe_ctx_init(ctx->drm_frames_ref);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "AVHWFramesContext init failed: %s!\n",
				av_err2str(err));
		goto fail;
	}

attach:
	/* Set frame hardware context referencce */
	f->hw_frames_ctx = av_buffer_ref(ctx->drm_frames_ref);
	if (!f->hw_frames_ctx) {
		err = AVERROR(ENOMEM);
		goto fail;
	}

	return 0;

fail:
	av_buffer_unref(&ctx->drm_frames_ref);
	return err;
}

static void register_cb(struct capture_context *ctx);

static void frame_ready(void *data, struct zwlr_export_dmabuf_frame_v1 *frame,
		uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
	struct capture_context *ctx = data;
	AVFrame *f = ctx->current_frame;
	AVDRMFrameDescriptor *desc = (AVDRMFrameDescriptor *)f->data[0];
	enum AVPixelFormat pix_fmt = drm_fmt_to_pixfmt(desc->layers[0].format);
	int err = 0;

	/* Timestamp, nanoseconds timebase */
	f->pts = ((((uint64_t)tv_sec_hi) << 32) | tv_sec_lo) * 1000000000 + tv_nsec;

	if (!ctx->start_pts) {
		ctx->start_pts = f->pts;
	}

	f->pts = av_rescale_q(f->pts - ctx->start_pts, (AVRational){ 1, 1000000000 },
			ctx->avctx->time_base);

	/* Attach the hardware frame context to the frame */
	err = attach_drm_frames_ref(ctx, f, pix_fmt);
	if (err) {
		goto end;
	}

	/* TODO: support multiplane stuff */
	desc->layers[0].nb_planes = av_pix_fmt_count_planes(pix_fmt);

	AVFrame *mapped_frame = av_frame_alloc();
	if (!mapped_frame) {
		err = AVERROR(ENOMEM);
		goto end;
	}

	AVHWFramesContext *mapped_hwfc;
	mapped_hwfc = (AVHWFramesContext *)ctx->mapped_frames_ref->data;
	mapped_frame->format = mapped_hwfc->format;
	mapped_frame->pts = f->pts;

	/* Set frame hardware context referencce */
	mapped_frame->hw_frames_ctx = av_buffer_ref(ctx->mapped_frames_ref);
	if (!mapped_frame->hw_frames_ctx) {
		err = AVERROR(ENOMEM);
		goto end;
	}

	err = av_hwframe_map(mapped_frame, f, 0);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Error mapping: %s!\n", av_err2str(err));
		goto end;
	}

	if (push_to_fifo(&ctx->vid_frames, mapped_frame)) {
		av_log(ctx, AV_LOG_WARNING, "Dropped frame!\n");
	}

	if (!ctx->quit && !ctx->err) {
		register_cb(ctx);
	}

end:
	ctx->err = err;
	av_frame_free(&ctx->current_frame);
}

static void frame_cancel(void *data, struct zwlr_export_dmabuf_frame_v1 *frame,
		uint32_t reason) {
	struct capture_context *ctx = data;
	av_log(ctx, AV_LOG_WARNING, "Frame cancelled!\n");
	av_frame_free(&ctx->current_frame);
	if (reason == ZWLR_EXPORT_DMABUF_FRAME_V1_CANCEL_REASON_PERMANENT) {
		av_log(ctx, AV_LOG_ERROR, "Permanent failure, exiting\n");
		ctx->err = true;
	} else {
		register_cb(ctx);
	}
}

static const struct zwlr_export_dmabuf_frame_v1_listener frame_listener = {
	.frame = frame_start,
	.object = frame_object,
	.ready = frame_ready,
	.cancel = frame_cancel,
};

static void register_cb(struct capture_context *ctx) {
	ctx->frame_callback = zwlr_export_dmabuf_manager_v1_capture_output(
			ctx->export_manager, ctx->with_cursor, ctx->target_output);

	zwlr_export_dmabuf_frame_v1_add_listener(ctx->frame_callback,
			&frame_listener, ctx);
}

static void *vid_encode_thread(void *arg) {
	int err = 0;
	struct capture_context *ctx = arg;

	do {
		AVFrame *f = NULL;
		if (get_fifo_size(&ctx->vid_frames) || !ctx->quit) {
			f = pop_from_fifo(&ctx->vid_frames);
		}

		if (ctx->is_software_encoder && f) {
			AVFrame *soft_frame = av_frame_alloc();
			av_hwframe_transfer_data(soft_frame, f, 0);
			soft_frame->pts = f->pts;
			av_frame_free(&f);
			f = soft_frame;
		}

		err = avcodec_send_frame(ctx->avctx, f);

		av_frame_free(&f);

		if (err) {
			av_log(ctx, AV_LOG_ERROR, "Error encoding: %s!\n", av_err2str(err));
			goto end;
		}

		while (1) {
			AVPacket *pkt = av_packet_alloc();
			int ret = avcodec_receive_packet(ctx->avctx, pkt);
			if (ret == AVERROR(EAGAIN)) {
				av_packet_free(&pkt);
				break;
			} else if (ret == AVERROR_EOF) {
				av_log(ctx, AV_LOG_INFO, "Encoder flushed!\n");
				av_packet_free(&pkt);
				goto end;
			} else if (ret) {
				av_log(ctx, AV_LOG_ERROR, "Error encoding: %s!\n",
						av_err2str(ret));
				av_packet_free(&pkt);
				err = ret;
				goto end;
			}

			pkt->stream_index = 0;
			err = av_interleaved_write_frame(ctx->avf, pkt);

			av_packet_free(&pkt);

			if (err) {
				av_log(ctx, AV_LOG_ERROR, "Writing packet fail: %s!\n",
						av_err2str(err));
				goto end;
			}
		};

		av_log(ctx, AV_LOG_INFO, "Encoded frame %i (%i in queue)\n",
				ctx->avctx->frame_number, get_fifo_size(&ctx->vid_frames));

	} while (!ctx->err);

end:
	if (!ctx->err) {
		ctx->err = err;
	}
	return NULL;
}

static int init_lavu_hwcontext(struct capture_context *ctx) {
	/* DRM hwcontext */
	ctx->drm_device_ref = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_DRM);
	if (!ctx->drm_device_ref)
		return AVERROR(ENOMEM);

	AVHWDeviceContext *ref_data = (AVHWDeviceContext*)ctx->drm_device_ref->data;
	AVDRMDeviceContext *hwctx = ref_data->hwctx;

	/* We don't need a device (we don't even know it and can't open it) */
	hwctx->fd = -1;

	av_hwdevice_ctx_init(ctx->drm_device_ref);

	/* Mapped hwcontext */
	int err = av_hwdevice_ctx_create(&ctx->mapped_device_ref,
			ctx->hw_device_type, ctx->hardware_device, NULL, 0);
	if (err < 0) {
		av_log(ctx, AV_LOG_ERROR, "Failed to create a hardware device: %s\n",
				av_err2str(err));
		return err;
	}

	return 0;
}

static int set_hwframe_ctx(struct capture_context *ctx,
		AVBufferRef *hw_device_ctx) {
	AVHWFramesContext *frames_ctx = NULL;
	int err = 0;

	if (!(ctx->mapped_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
		return AVERROR(ENOMEM);
	}

	AVHWFramesConstraints *cst =
			av_hwdevice_get_hwframe_constraints(ctx->mapped_device_ref, NULL);
	if (!cst) {
		av_log(ctx, AV_LOG_ERROR, "Failed to get hw device constraints!\n");
		av_buffer_unref(&ctx->mapped_frames_ref);
		return AVERROR(ENOMEM);
	}

	frames_ctx = (AVHWFramesContext *)(ctx->mapped_frames_ref->data);
	frames_ctx->format = cst->valid_hw_formats[0];
	frames_ctx->sw_format = ctx->avctx->pix_fmt;
	frames_ctx->width = ctx->avctx->width;
	frames_ctx->height = ctx->avctx->height;

	av_hwframe_constraints_free(&cst);

	if ((err = av_hwframe_ctx_init(ctx->mapped_frames_ref))) {
		av_log(ctx, AV_LOG_ERROR, "Failed to initialize hw frame context: %s!\n",
				av_err2str(err));
		av_buffer_unref(&ctx->mapped_frames_ref);
		return err;
	}

	if (!ctx->is_software_encoder) {
		ctx->avctx->pix_fmt = frames_ctx->format;
		ctx->avctx->hw_frames_ctx = av_buffer_ref(ctx->mapped_frames_ref);
		if (!ctx->avctx->hw_frames_ctx) {
			av_buffer_unref(&ctx->mapped_frames_ref);
			err = AVERROR(ENOMEM);
		}
	}

	return err;
}

static int init_encoding(struct capture_context *ctx) {
	int err;

	/* lavf init */
	err = avformat_alloc_output_context2(&ctx->avf, NULL,
			NULL, ctx->out_filename);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Unable to init lavf context!\n");
		return err;
	}

	AVStream *st = avformat_new_stream(ctx->avf, NULL);
	if (!st) {
		av_log(ctx, AV_LOG_ERROR, "Unable to alloc stream!\n");
		return 1;
	}

	/* Find encoder */
	const AVCodec *out_codec = avcodec_find_encoder_by_name(ctx->encoder_name);
	if (!out_codec) {
		av_log(ctx, AV_LOG_ERROR, "Codec not found (not compiled in lavc?)!\n");
		return AVERROR(EINVAL);
	}
	ctx->avf->oformat = av_guess_format(ctx->encoder_name, NULL, NULL);
	ctx->is_software_encoder = !(out_codec->capabilities & AV_CODEC_CAP_HARDWARE);

	ctx->avctx = avcodec_alloc_context3(out_codec);
	if (!ctx->avctx)
		return 1;

	ctx->avctx->opaque = ctx;
	ctx->avctx->bit_rate = (int)ctx->out_bitrate*1000000.0f;
	ctx->avctx->pix_fmt = ctx->software_format;
	ctx->avctx->time_base = (AVRational){ 1, 1000 };
	ctx->avctx->compression_level = 7;
	ctx->avctx->width = find_output(ctx, ctx->target_output, 0)->width;
	ctx->avctx->height = find_output(ctx, ctx->target_output, 0)->height;

	if (ctx->avf->oformat->flags & AVFMT_GLOBALHEADER) {
		ctx->avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	st->id = 0;
	st->time_base = ctx->avctx->time_base;
	st->avg_frame_rate = find_output(ctx, ctx->target_output, 0)->framerate;

	/* Init hw frames context */
	err = set_hwframe_ctx(ctx, ctx->mapped_device_ref);
	if (err) {
		return err;
	}

	err = avcodec_open2(ctx->avctx, out_codec, &ctx->encoder_opts);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Cannot open encoder: %s!\n",
				av_err2str(err));
		return err;
	}

	if (avcodec_parameters_from_context(st->codecpar, ctx->avctx) < 0) {
		av_log(ctx, AV_LOG_ERROR, "Couldn't copy codec params: %s!\n",
				av_err2str(err));
		return err;
	}

	/* Debug print */
	av_dump_format(ctx->avf, 0, ctx->out_filename, 1);

	/* Open for writing */
	err = avio_open(&ctx->avf->pb, ctx->out_filename, AVIO_FLAG_WRITE);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Couldn't open %s: %s!\n", ctx->out_filename,
				av_err2str(err));
		return err;
	}

	err = avformat_write_header(ctx->avf, NULL);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Couldn't write header: %s!\n", av_err2str(err));
		return err;
	}

	return err;
}

struct capture_context *q_ctx = NULL;

static void on_quit_signal(int signo) {
	printf("\r");
	av_log(q_ctx, AV_LOG_WARNING, "Quitting!\n");
	q_ctx->quit = true;
}

static int main_loop(struct capture_context *ctx) {
	int err;

	q_ctx = ctx;

	if (signal(SIGINT, on_quit_signal) == SIG_ERR) {
		av_log(ctx, AV_LOG_ERROR, "Unable to install signal handler!\n");
		return AVERROR(EINVAL);
	}

	err = init_lavu_hwcontext(ctx);
	if (err) {
		return err;
	}

	err = init_encoding(ctx);
	if (err) {
		return err;
	}

	/* Start video encoding thread */
	err = init_fifo(&ctx->vid_frames, 16);
	if (err) {
		return err;
	}
	pthread_create(&ctx->vid_thread, NULL, vid_encode_thread, ctx);

	/* Start the frame callback */
	register_cb(ctx);

	/* Run capture */
	while (wl_display_dispatch(ctx->display) != -1 && !ctx->err && !ctx->quit);

	/* Join with encoder thread */
	pthread_join(ctx->vid_thread, NULL);

	err = av_write_trailer(ctx->avf);
	if (err) {
		av_log(ctx, AV_LOG_ERROR, "Error writing trailer: %s!\n",
				av_err2str(err));
		return err;
	}

	av_log(ctx, AV_LOG_INFO, "Wrote trailer!\n");

	return ctx->err;
}

static int init(struct capture_context *ctx) {
	ctx->display = wl_display_connect(NULL);
	if (!ctx->display) {
		av_log(ctx, AV_LOG_ERROR, "Failed to connect to display!\n");
		return AVERROR(EINVAL);
	}

	wl_list_init(&ctx->output_list);

	ctx->registry = wl_display_get_registry(ctx->display);
	wl_registry_add_listener(ctx->registry, &registry_listener, ctx);

	// First roundtrip to fetch globals
	wl_display_roundtrip(ctx->display);

	// Second roundtrip to fetch wl_output information
	wl_display_roundtrip(ctx->display);

	if (!ctx->export_manager) {
		av_log(ctx, AV_LOG_ERROR, "Compositor doesn't support %s!\n",
				zwlr_export_dmabuf_manager_v1_interface.name);
		return -1;
	}

	return 0;
}

static void uninit(struct capture_context *ctx);

static const char usage[] = "usage: dmabuf-capture [options...] <destination file path>\n"
	"  -o <output ID>\n"
	"  -t <hardware device type>\n"
	"  -d <device path>\n"
	"  -e <encoder>\n"
	"  -f <pixel format>\n"
	"  -r <bitrate in Mbps>\n"
	"\n"
	"Example:\n"
	"  dmabuf-capture -o 32 -t vaapi -d /dev/dri/renderD129 \\\n"
	"    -e libx264 -f nv12 -r 12 recording.mkv\n";

int main(int argc, char *argv[]) {
	struct capture_context ctx = {
		.hardware_device = "/dev/dri/renderD128",
		.encoder_name = "libx264",
		.out_bitrate = 12,
	};
	int output_id = -1;
	const char *hw_device_type = "vaapi";
	const char *software_format = "nv12";
	int opt;
	while ((opt = getopt(argc, argv, "ho:t:d:e:f:r:")) != -1) {
		char *end;
		switch (opt) {
		case 'o':
			output_id = strtol(optarg, &end, 10);
			if (optarg[0] == '\0' || end[0] != '\0') {
				fprintf(stderr, "Output ID is not an integer\n");
				return 1;
			}
			break;
		case 't':
			hw_device_type = optarg;
			break;
		case 'd':
			ctx.hardware_device = optarg;
			break;
		case 'e':
			ctx.encoder_name = optarg;
			break;
		case 'f':
			software_format = optarg;
			break;
		case 'r':
			ctx.out_bitrate = strtof(optarg, &end);
			if (optarg[0] == '\0' || end[0] != '\0') {
				fprintf(stderr, "Bitrate is not a floating-pointer number\n");
				return 1;
			}
			break;
		default:
			fprintf(stderr, "%s", usage);
			return 1;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "Missing destination file argument\n");
		fprintf(stderr, "%s", usage);
		return 1;
	}
	ctx.out_filename = argv[optind];

	ctx.class = &((AVClass){
		.class_name = "dmabuf-capture",
		.item_name  = av_default_item_name,
		.version    = LIBAVUTIL_VERSION_INT,
	});

	int err = init(&ctx);
	if (err) {
		goto end;
	}

	struct wayland_output *o, *tmp_o;
	wl_list_for_each_reverse_safe(o, tmp_o, &ctx.output_list, link) {
		printf("Capturable output: %s Model: %s: ID: %i\n",
				o->make, o->model, o->id);
	}

	o = find_output(&ctx, NULL, output_id);
	if (!o) {
		printf("Unable to find output with ID %d\n", output_id);
		return 1;
	}

	ctx.target_output = o->output;
	ctx.with_cursor = true;
	ctx.hw_device_type = av_hwdevice_find_type_by_name(hw_device_type);
	ctx.software_format = av_get_pix_fmt(software_format);

	av_dict_set(&ctx.encoder_opts, "preset", "veryfast", 0);

	err = main_loop(&ctx);
	if (err) {
		goto end;
	}

end:
	uninit(&ctx);
	return err;
}

static void uninit(struct capture_context *ctx) {
	struct wayland_output *output, *tmp_o;
	wl_list_for_each_safe(output, tmp_o, &ctx->output_list, link) {
		remove_output(output);
	}

	if (ctx->export_manager) {
		zwlr_export_dmabuf_manager_v1_destroy(ctx->export_manager);
	}

	free_fifo(&ctx->vid_frames);

	av_buffer_unref(&ctx->drm_frames_ref);
	av_buffer_unref(&ctx->drm_device_ref);
	av_buffer_unref(&ctx->mapped_frames_ref);
	av_buffer_unref(&ctx->mapped_device_ref);

	av_dict_free(&ctx->encoder_opts);

	avcodec_close(ctx->avctx);
	if (ctx->avf) {
		avio_closep(&ctx->avf->pb);
	}
	avformat_free_context(ctx->avf);
}
