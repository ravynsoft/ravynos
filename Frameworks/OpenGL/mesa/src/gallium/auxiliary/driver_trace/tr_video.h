#ifndef TR_VIDEO_H_
#define TR_VIDEO_H_


#include "pipe/p_video_codec.h"
#include "vl/vl_defines.h"

#include "tr_context.h"


struct trace_video_codec
{
   struct pipe_video_codec base;

   struct pipe_video_codec *video_codec;
};

static inline struct trace_video_codec *
trace_video_codec(struct pipe_video_codec *video_codec)
{
   assert(video_codec);
   return (struct trace_video_codec *)video_codec;
}

struct pipe_video_codec *
trace_video_codec_create(struct trace_context *tr_ctx, struct pipe_video_codec *video_codec);


struct trace_video_buffer
{
   struct pipe_video_buffer base;

   struct pipe_video_buffer *video_buffer;

   struct pipe_sampler_view *sampler_view_planes[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_components[VL_NUM_COMPONENTS];
   struct pipe_surface      *surfaces[VL_MAX_SURFACES];
};

static inline struct trace_video_buffer *
trace_video_buffer(struct pipe_video_buffer *video_buffer)
{
   assert(video_buffer);
   return (struct trace_video_buffer *)video_buffer;
}

struct pipe_video_buffer *
trace_video_buffer_create(struct trace_context *tr_ctx, struct pipe_video_buffer *video_buffer);


#endif /* TR_VIDEO_H_ */
