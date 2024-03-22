#include "tr_public.h"
#include "tr_dump.h"
#include "tr_dump_state.h"
#include "tr_texture.h"
#include "u_inlines.h"
#include "u_video.h"
#include "tr_video.h"
#include "pipe/p_video_codec.h"
#include "pipe/p_video_enums.h"
#include "util/macros.h"
#include "util/u_memory.h"
#include "vl/vl_defines.h"

static void
trace_video_codec_destroy(struct pipe_video_codec *_codec)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *video_codec = tr_vcodec->video_codec;

    trace_dump_call_begin("pipe_video_codec", "destroy");
    trace_dump_arg(ptr, video_codec);
    trace_dump_call_end();

    video_codec->destroy(video_codec);

    ralloc_free(tr_vcodec);
}

static void
unwrap_refrence_frames_in_place(struct pipe_video_buffer **refrence_frames, unsigned max_num_refrence_frame)
{
    for (unsigned i=0; i < max_num_refrence_frame; i++) {
        if (refrence_frames[i]) {
            struct trace_video_buffer *tr_buffer = trace_video_buffer(refrence_frames[i]);
            refrence_frames[i] = tr_buffer->video_buffer;
        }
    }
}

static bool
unwrap_refrence_frames(struct pipe_picture_desc **picture)
{
    // only decode pictures use video buffers for refrences
    if ((*picture)->entry_point != PIPE_VIDEO_ENTRYPOINT_BITSTREAM)
        return false;
    switch (u_reduce_video_profile((*picture)->profile)) {
    case PIPE_VIDEO_FORMAT_MPEG12: {
        struct pipe_mpeg12_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_mpeg12_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_MPEG4: {
        struct pipe_mpeg4_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_mpeg4_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_VC1:{
        struct pipe_vc1_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_vc1_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_MPEG4_AVC: {
        struct pipe_h264_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_h264_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_HEVC:{
        struct pipe_h265_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_h265_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_JPEG:
        return false;
    case PIPE_VIDEO_FORMAT_VP9:{
        struct pipe_vp9_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_vp9_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_AV1:{
        struct pipe_av1_picture_desc *copied = mem_dup(*picture, sizeof(struct pipe_av1_picture_desc));
        assert(copied);
        unwrap_refrence_frames_in_place(copied->ref, ARRAY_SIZE(copied->ref));
        unwrap_refrence_frames_in_place(&copied->film_grain_target, 1);
        *picture = (struct pipe_picture_desc*)copied;
        return true;
    }
    case PIPE_VIDEO_FORMAT_UNKNOWN:
    default:
        unreachable("unknown video format");
    }
}

static void
trace_video_codec_begin_frame(struct pipe_video_codec *_codec,
                    struct pipe_video_buffer *_target,
                    struct pipe_picture_desc *picture)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_target = trace_video_buffer(_target);
    struct pipe_video_buffer *target = tr_target->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "begin_frame");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, target);
    trace_dump_arg(pipe_picture_desc, picture);
    trace_dump_call_end();

    bool copied = unwrap_refrence_frames(&picture);
    codec->begin_frame(codec, target, picture);
    if (copied)
        FREE(picture);
}

static void
trace_video_codec_decode_macroblock(struct pipe_video_codec *_codec,
                            struct pipe_video_buffer *_target,
                            struct pipe_picture_desc *picture,
                            const struct pipe_macroblock *macroblocks,
                            unsigned num_macroblocks)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_target = trace_video_buffer(_target);
    struct pipe_video_buffer *target = tr_target->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "decode_macroblock");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, target);
    trace_dump_arg(pipe_picture_desc, picture);
    // TODO: how to dump pipe_macroblocks? It's only a single pointer,
    //  but each struct has codec dependent size, so can't use generic trace_dump_arg_array
    trace_dump_arg(ptr, macroblocks);
    trace_dump_arg(uint, num_macroblocks);
    trace_dump_call_end();

    bool copied = unwrap_refrence_frames(&picture);
    codec->decode_macroblock(codec, target, picture, macroblocks, num_macroblocks);
    if (copied)
        FREE(picture);
}

static void 
trace_video_codec_decode_bitstream(struct pipe_video_codec *_codec,
                        struct pipe_video_buffer *_target,
                        struct pipe_picture_desc *picture,
                        unsigned num_buffers,
                        const void * const *buffers,
                        const unsigned *sizes)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_target = trace_video_buffer(_target);
    struct pipe_video_buffer *target = tr_target->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "decode_bitstream");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, target);
    trace_dump_arg(pipe_picture_desc, picture);

    trace_dump_arg(uint, num_buffers);
    trace_dump_arg_array(ptr, buffers, num_buffers);
    trace_dump_arg_array(uint, sizes, num_buffers);
    trace_dump_call_end();

    bool copied = unwrap_refrence_frames(&picture);
    codec->decode_bitstream(codec, target, picture, num_buffers, buffers, sizes);
    if (copied)
        FREE(picture);
}

static void
trace_video_codec_encode_bitstream(struct pipe_video_codec *_codec,
                        struct pipe_video_buffer *_source,
                        struct pipe_resource *destination,
                        void **feedback)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_source = trace_video_buffer(_source);
    struct pipe_video_buffer *source = tr_source->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "encode_bitstream");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, source);
    trace_dump_arg(ptr, destination);
    trace_dump_arg(ptr, feedback);
    trace_dump_call_end();

    codec->encode_bitstream(codec, source, destination, feedback);
}

static void
trace_video_codec_process_frame(struct pipe_video_codec *_codec,
                        struct pipe_video_buffer *_source,
                        const struct pipe_vpp_desc *process_properties)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_source = trace_video_buffer(_source);
    struct pipe_video_buffer *source = tr_source->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "process_frame");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, source);
    trace_dump_arg(pipe_vpp_desc, process_properties);
    trace_dump_call_end();

    codec->process_frame(codec, source, process_properties);
}

static void
trace_video_codec_end_frame(struct pipe_video_codec *_codec,
                    struct pipe_video_buffer *_target,
                    struct pipe_picture_desc *picture)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_target = trace_video_buffer(_target);
    struct pipe_video_buffer *target = tr_target->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "end_frame");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, target);
    trace_dump_arg(pipe_picture_desc, picture);
    trace_dump_call_end();

    bool copied = unwrap_refrence_frames(&picture);
    codec->end_frame(codec, target, picture);
    if (copied)
        FREE(picture);
}

static void
trace_video_codec_flush(struct pipe_video_codec *_codec)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;

    trace_dump_call_begin("pipe_video_codec", "flush");
    trace_dump_arg(ptr, codec);
    trace_dump_call_end();

    codec->flush(codec);
}

static void
trace_video_codec_get_feedback(struct pipe_video_codec *_codec,
                               void *feedback,
                               unsigned *size,
                               struct pipe_enc_feedback_metadata* metadata)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;

    trace_dump_call_begin("pipe_video_codec", "get_feedback");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, feedback);
    trace_dump_arg(ptr, size);
    trace_dump_call_end();

    codec->get_feedback(codec, feedback, size, metadata);
}

static int
trace_video_codec_get_decoder_fence(struct pipe_video_codec *_codec,
                        struct pipe_fence_handle *fence,
                        uint64_t timeout)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;

    trace_dump_call_begin("pipe_video_codec", "get_decoder_fence");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, fence);
    trace_dump_arg(uint, timeout);

    int ret = codec->get_decoder_fence(codec, fence, timeout);

    trace_dump_ret(int, ret);
    trace_dump_call_end();

    return ret;
}

static int
trace_video_codec_get_processor_fence(struct pipe_video_codec *_codec,
                            struct pipe_fence_handle *fence,
                            uint64_t timeout)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;

    trace_dump_call_begin("pipe_video_codec", "get_processor_fence");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, fence);
    trace_dump_arg(uint, timeout);

    int ret = codec->get_processor_fence(codec, fence, timeout);

    trace_dump_ret(int, ret);
    trace_dump_call_end();

    return ret;
}

static void
trace_video_codec_update_decoder_target(struct pipe_video_codec *_codec,
                                struct pipe_video_buffer *_old,
                                struct pipe_video_buffer *_updated)
{
    struct trace_video_codec *tr_vcodec = trace_video_codec(_codec);
    struct pipe_video_codec *codec = tr_vcodec->video_codec;
    struct trace_video_buffer *tr_old = trace_video_buffer(_old);
    struct pipe_video_buffer *old = tr_old->video_buffer;
    struct trace_video_buffer *tr_updated = trace_video_buffer(_updated);
    struct pipe_video_buffer *updated = tr_updated->video_buffer;

    trace_dump_call_begin("pipe_video_codec", "update_decoder_target");
    trace_dump_arg(ptr, codec);
    trace_dump_arg(ptr, old);
    trace_dump_arg(ptr, updated);
    trace_dump_call_end();

    codec->update_decoder_target(codec, old, updated);
}

struct pipe_video_codec *
trace_video_codec_create(struct trace_context *tr_ctx,
                         struct pipe_video_codec *video_codec)
{
   struct trace_video_codec *tr_vcodec;

   if (!video_codec)
      goto error1;

   if (!trace_enabled())
      goto error1;

   tr_vcodec = rzalloc(NULL, struct trace_video_codec);
   if (!tr_vcodec)
      goto error1;

    memcpy(&tr_vcodec->base, video_codec, sizeof(struct pipe_video_codec));
    tr_vcodec->base.context = &tr_ctx->base;

#define TR_VC_INIT(_member) \
   tr_vcodec->base . _member = video_codec -> _member ? trace_video_codec_ ## _member : NULL

    TR_VC_INIT(destroy);
    TR_VC_INIT(begin_frame);
    TR_VC_INIT(decode_macroblock);
    TR_VC_INIT(decode_bitstream);
    TR_VC_INIT(encode_bitstream);
    TR_VC_INIT(process_frame);
    TR_VC_INIT(end_frame);
    TR_VC_INIT(flush);
    TR_VC_INIT(get_feedback);
    TR_VC_INIT(get_decoder_fence);
    TR_VC_INIT(get_processor_fence);
    TR_VC_INIT(update_decoder_target);

#undef TR_VC_INIT

   tr_vcodec->video_codec = video_codec;

   return &tr_vcodec->base;

error1:
   return video_codec;
}


static void
trace_video_buffer_destroy(struct pipe_video_buffer *_buffer)
{
    struct trace_video_buffer *tr_vbuffer = trace_video_buffer(_buffer);
    struct pipe_video_buffer *video_buffer = tr_vbuffer->video_buffer;

    trace_dump_call_begin("pipe_video_buffer", "destroy");
    trace_dump_arg(ptr, video_buffer);
    trace_dump_call_end();

    for (int i=0; i < VL_NUM_COMPONENTS; i++) {
        pipe_sampler_view_reference(&tr_vbuffer->sampler_view_planes[i], NULL);
        pipe_sampler_view_reference(&tr_vbuffer->sampler_view_components[i], NULL);
    }
    for (int i=0; i < VL_MAX_SURFACES; i++) {
        pipe_surface_reference(&tr_vbuffer->surfaces[i], NULL);
    }
    video_buffer->destroy(video_buffer);

    ralloc_free(tr_vbuffer);
}

static void
trace_video_buffer_get_resources(struct pipe_video_buffer *_buffer, struct pipe_resource **resources)
{
    struct trace_video_buffer *tr_vbuffer = trace_video_buffer(_buffer);
    struct pipe_video_buffer *buffer = tr_vbuffer->video_buffer;

    trace_dump_call_begin("pipe_video_buffer", "get_resources");
    trace_dump_arg(ptr, buffer);

    buffer->get_resources(buffer, resources);

    // TODO: A `trace_dump_ret_arg` style of function would be more appropriate
    trace_dump_arg_array(ptr, resources, VL_NUM_COMPONENTS);
    trace_dump_call_end();
}

static struct pipe_sampler_view **
trace_video_buffer_get_sampler_view_planes(struct pipe_video_buffer *_buffer)
{
    struct trace_context *tr_ctx = trace_context(_buffer->context);
    struct trace_video_buffer *tr_vbuffer = trace_video_buffer(_buffer);
    struct pipe_video_buffer *buffer = tr_vbuffer->video_buffer;

    trace_dump_call_begin("pipe_video_buffer", "get_sampler_view_planes");
    trace_dump_arg(ptr, buffer);

    struct pipe_sampler_view **view_planes = buffer->get_sampler_view_planes(buffer);

    trace_dump_ret_array(ptr, view_planes, VL_NUM_COMPONENTS);
    trace_dump_call_end();

    for (int i=0; i < VL_NUM_COMPONENTS; i++) {
        if (!view_planes || !view_planes[i]) {
            pipe_sampler_view_reference(&tr_vbuffer->sampler_view_planes[i], NULL);
        } else if (tr_vbuffer->sampler_view_planes[i] == NULL || (trace_sampler_view(tr_vbuffer->sampler_view_planes[i])->sampler_view != view_planes[i])) {
            pipe_sampler_view_reference(&tr_vbuffer->sampler_view_planes[i], trace_sampler_view_create(tr_ctx, view_planes[i]->texture, view_planes[i]));
        }
    }

    return view_planes ? tr_vbuffer->sampler_view_planes : NULL;
}

static struct pipe_sampler_view **
trace_video_buffer_get_sampler_view_components(struct pipe_video_buffer *_buffer)
{
    struct trace_context *tr_ctx = trace_context(_buffer->context);
    struct trace_video_buffer *tr_vbuffer = trace_video_buffer(_buffer);
    struct pipe_video_buffer *buffer = tr_vbuffer->video_buffer;

    trace_dump_call_begin("pipe_video_buffer", "get_sampler_view_components");
    trace_dump_arg(ptr, buffer);

    struct pipe_sampler_view **view_components = buffer->get_sampler_view_components(buffer);

    trace_dump_ret_array(ptr, view_components, VL_NUM_COMPONENTS);
    trace_dump_call_end();

    for (int i=0; i < VL_NUM_COMPONENTS; i++) {
        if (!view_components || !view_components[i]) {
            pipe_sampler_view_reference(&tr_vbuffer->sampler_view_components[i], NULL);
        } else if (tr_vbuffer->sampler_view_components[i] == NULL || (trace_sampler_view(tr_vbuffer->sampler_view_components[i])->sampler_view != view_components[i])) {
            pipe_sampler_view_reference(&tr_vbuffer->sampler_view_components[i], trace_sampler_view_create(tr_ctx, view_components[i]->texture, view_components[i]));
        }
    }

    return view_components ? tr_vbuffer->sampler_view_components : NULL;
}

static struct pipe_surface **
trace_video_buffer_get_surfaces(struct pipe_video_buffer *_buffer)
{
    struct trace_context *tr_ctx = trace_context(_buffer->context);
    struct trace_video_buffer *tr_vbuffer = trace_video_buffer(_buffer);
    struct pipe_video_buffer *buffer = tr_vbuffer->video_buffer;

    trace_dump_call_begin("pipe_video_buffer", "get_surfaces");
    trace_dump_arg(ptr, buffer);

    struct pipe_surface **surfaces = buffer->get_surfaces(buffer);

    trace_dump_ret_array(ptr, surfaces, VL_MAX_SURFACES);
    trace_dump_call_end();

    for (int i=0; i < VL_MAX_SURFACES; i++) {
        if (!surfaces || !surfaces[i]) {
            pipe_surface_reference(&tr_vbuffer->surfaces[i], NULL);
        } else if (tr_vbuffer->surfaces[i] == NULL || (trace_surface(tr_vbuffer->surfaces[i])->surface != surfaces[i])){
            pipe_surface_reference(&tr_vbuffer->surfaces[i], trace_surf_create(tr_ctx, surfaces[i]->texture, surfaces[i]));
        }
    }

    return surfaces ? tr_vbuffer->surfaces : NULL;
}


struct pipe_video_buffer *
trace_video_buffer_create(struct trace_context *tr_ctx,
                          struct pipe_video_buffer *video_buffer)
{
   struct trace_video_buffer *tr_vbuffer;

   if (!video_buffer)
      goto error1;

   if (!trace_enabled())
      goto error1;

   tr_vbuffer = rzalloc(NULL, struct trace_video_buffer);
   if (!tr_vbuffer)
      goto error1;

    memcpy(&tr_vbuffer->base, video_buffer, sizeof(struct pipe_video_buffer));
    tr_vbuffer->base.context = &tr_ctx->base;

#define TR_VB_INIT(_member) \
   tr_vbuffer->base . _member = video_buffer -> _member ? trace_video_buffer_ ## _member : NULL

    TR_VB_INIT(destroy);
    TR_VB_INIT(get_resources);
    TR_VB_INIT(get_sampler_view_planes);
    TR_VB_INIT(get_sampler_view_components);
    TR_VB_INIT(get_surfaces);

#undef TR_VB_INIT

   tr_vbuffer->video_buffer = video_buffer;

   return &tr_vbuffer->base;

error1:
   return video_buffer;
}
