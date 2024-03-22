#include <assert.h>

#include "vl_decoder.h"
#include "vl_mpeg12_bitstream.h"
#include "vl_mpeg12_decoder.h"
#include "vl_video_buffer.h"
#include "vl_zscan.h"


/*
 * vl_decoder stubs
 */
bool
vl_profile_supported(struct pipe_screen *screen,
                     enum pipe_video_profile profile,
                     enum pipe_video_entrypoint entrypoint)
{
   assert(0);
   return false;
}

int
vl_level_supported(struct pipe_screen *screen,
                   enum pipe_video_profile profile)
{
   assert(0);
   return 0;
}

struct pipe_video_codec *
vl_create_decoder(struct pipe_context *pipe,
                  const struct pipe_video_codec *templat)
{
   assert(0);
   return NULL;
}


/*
 * vl_video_buffer stubs
 */
void
vl_get_video_buffer_formats(struct pipe_screen *screen, enum pipe_format format,
                            enum pipe_format out_format[VL_NUM_COMPONENTS])
{
   assert(0);
}

bool
vl_video_buffer_is_format_supported(struct pipe_screen *screen,
                                    enum pipe_format format,
                                    enum pipe_video_profile profile,
                                    enum pipe_video_entrypoint entrypoint)
{
   assert(0);
   return false;
}

unsigned
vl_video_buffer_max_size(struct pipe_screen *screen)
{
   assert(0);
   return 0;
}

void
vl_video_buffer_set_associated_data(struct pipe_video_buffer *vbuf,
                                    struct pipe_video_codec *vcodec,
                                    void *associated_data,
                                    void (*destroy_associated_data)(void *))
{
   assert(0);
}

void *
vl_video_buffer_get_associated_data(struct pipe_video_buffer *vbuf,
                                    struct pipe_video_codec *vcodec)
{
   assert(0);
   return NULL;
}

void
vl_video_buffer_template(struct pipe_resource *templ,
                         const struct pipe_video_buffer *tmpl,
                         enum pipe_format resource_format,
                         unsigned depth, unsigned array_size,
                         unsigned usage, unsigned plane,
                         enum pipe_video_chroma_format chroma_format)
{
   assert(0);
}

struct pipe_video_buffer *
vl_video_buffer_create(struct pipe_context *pipe,
                       const struct pipe_video_buffer *tmpl)
{
   assert(0);
   return NULL;
}

struct pipe_video_buffer *
vl_video_buffer_create_ex2(struct pipe_context *pipe,
                           const struct pipe_video_buffer *tmpl,
                           struct pipe_resource *resources[VL_NUM_COMPONENTS])
{
   assert(0);
   return NULL;
}

void
vl_video_buffer_destroy(struct pipe_video_buffer *buffer)
{
   assert(0);
}

/*
 * vl_mpeg12_bitstream stubs
 */
void
vl_mpg12_bs_init(struct vl_mpg12_bs *bs, struct pipe_video_codec *decoder)
{
   assert(0);
}

void
vl_mpg12_bs_decode(struct vl_mpg12_bs *bs,
                   struct pipe_video_buffer *target,
                   struct pipe_mpeg12_picture_desc *picture,
                   unsigned num_buffers,
                   const void * const *buffers,
                   const unsigned *sizes)
{
   assert(0);
}


/*
 * vl_mpeg12_decoder stubs
 */
struct pipe_video_codec *
vl_create_mpeg12_decoder(struct pipe_context *pipe,
                         const struct pipe_video_codec *templat)
{
   assert(0);
   return NULL;
}

/*
 * vl_zscan
 */
const int vl_zscan_normal[] = {0};
const int vl_zscan_alternate[] = {0};
