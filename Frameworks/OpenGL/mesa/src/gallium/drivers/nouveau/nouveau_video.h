#ifndef __NOUVEAU_VIDEO_H__
#define __NOUVEAU_VIDEO_H__

#include "nv17_mpeg.xml.h"
#include "nv31_mpeg.xml.h"
#include "nv_object.xml.h"

#include "nouveau_winsys.h"

struct nouveau_video_buffer {
   struct pipe_video_buffer base;
   unsigned num_planes;
   struct pipe_resource     *resources[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_planes[VL_NUM_COMPONENTS];
   struct pipe_sampler_view *sampler_view_components[VL_NUM_COMPONENTS];
   struct pipe_surface      *surfaces[VL_NUM_COMPONENTS * 2];
};

struct nouveau_decoder {
   struct pipe_video_codec base;
   struct nouveau_screen *screen;
   struct nouveau_pushbuf *push;
   struct nouveau_object *chan;
   struct nouveau_client *client;
   struct nouveau_bufctx *bufctx;
   struct nouveau_object *mpeg;
   struct nouveau_bo *cmd_bo, *data_bo, *fence_bo;

   unsigned *fence_map;
   unsigned fence_seq;

   unsigned ofs;
   unsigned *cmds;

   unsigned *data;
   unsigned data_pos;
   unsigned picture_structure;

   unsigned past, future, current;
   unsigned num_surfaces;
   struct nouveau_video_buffer *surfaces[8];
};

#define NV31_VIDEO_BIND_IMG(i)  i
#define NV31_VIDEO_BIND_CMD     NV31_MPEG_IMAGE_Y_OFFSET__LEN
#define NV31_VIDEO_BIND_COUNT  (NV31_MPEG_IMAGE_Y_OFFSET__LEN + 1)

static inline void
nouveau_vpe_write(struct nouveau_decoder *dec, unsigned data) {
   dec->cmds[dec->ofs++] = data;
}

static inline void
PUSH_MTHDl(struct nouveau_pushbuf *push, int subc, int mthd,
           struct nouveau_bo *bo, uint32_t offset,
           struct nouveau_bufctx *ctx, int bin, uint32_t rw)
{
   nouveau_bufctx_mthd(ctx, bin, NV04_FIFO_PKHDR(subc, mthd, 1),
                       bo, offset,
                       NOUVEAU_BO_LOW | (bo->flags & NOUVEAU_BO_APER) | rw,
                       0, 0);

   PUSH_DATA(push, bo->offset + offset);
}

#define SUBC_MPEG(mthd) 1, mthd
#define NV31_MPEG(mthd) SUBC_MPEG(NV31_MPEG_##mthd)
#define NV84_MPEG(mthd) SUBC_MPEG(NV84_MPEG_##mthd)

#endif
