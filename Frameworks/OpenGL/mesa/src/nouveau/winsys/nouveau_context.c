#include "nouveau_context.h"

#include "nouveau_device.h"

#include "drm-uapi/nouveau_drm.h"

#include <errno.h>
#include <nouveau/nvif/ioctl.h>
#include <xf86drm.h>

static void
nouveau_ws_subchan_dealloc(int fd, struct nouveau_ws_object *obj)
{
   struct {
      struct nvif_ioctl_v0 ioctl;
      struct nvif_ioctl_del del;
   } args = {
      .ioctl = {
         .object = (uintptr_t)obj,
         .owner = NVIF_IOCTL_V0_OWNER_ANY,
         .route = 0x00,
         .type = NVIF_IOCTL_V0_DEL,
         .version = 0,
      },
   };

   /* TODO returns -ENOENT for unknown reasons */
   drmCommandWrite(fd, DRM_NOUVEAU_NVIF, &args, sizeof(args));
}

#define NOUVEAU_WS_CONTEXT_MAX_CLASSES 16

static int
nouveau_ws_context_query_classes(int fd, int channel, uint32_t classes[NOUVEAU_WS_CONTEXT_MAX_CLASSES])
{
   struct {
      struct nvif_ioctl_v0 ioctl;
      struct nvif_ioctl_sclass_v0 sclass;
      struct nvif_ioctl_sclass_oclass_v0 list[NOUVEAU_WS_CONTEXT_MAX_CLASSES];
   } args = {
      .ioctl = {
         .route = 0xff,
         .token = channel,
         .type = NVIF_IOCTL_V0_SCLASS,
         .version = 0,
      },
      .sclass = {
         .count = NOUVEAU_WS_CONTEXT_MAX_CLASSES,
         .version = 0,
      },
   };

   int ret = drmCommandWriteRead(fd, DRM_NOUVEAU_NVIF, &args, sizeof(args));
   if (ret)
      return ret;

   assert(args.sclass.count <= NOUVEAU_WS_CONTEXT_MAX_CLASSES);
   for (unsigned i = 0; i < NOUVEAU_WS_CONTEXT_MAX_CLASSES; i++)
      classes[i] = args.list[i].oclass;

   return 0;
}

static uint32_t
nouveau_ws_context_find_class(uint32_t classes[NOUVEAU_WS_CONTEXT_MAX_CLASSES], uint8_t type)
{
   uint32_t ret = 0;

   /* find the highest matching one */
   for (unsigned i = 0; i < NOUVEAU_WS_CONTEXT_MAX_CLASSES; i++) {
      uint32_t val = classes[i];
      if ((val & 0xff) == type)
         ret = MAX2(ret, val);
   }

   return ret;
}

static int
nouveau_ws_subchan_alloc(int fd, int channel, uint32_t handle, uint16_t oclass, struct nouveau_ws_object *obj)
{
   struct {
      struct nvif_ioctl_v0 ioctl;
      struct nvif_ioctl_new_v0 new;
   } args = {
      .ioctl = {
         .route = 0xff,
         .token = channel,
         .type = NVIF_IOCTL_V0_NEW,
         .version = 0,
      },
      .new = {
         .handle = handle,
         .object = (uintptr_t)obj,
         .oclass = oclass,
         .route = NVIF_IOCTL_V0_ROUTE_NVIF,
         .token = (uintptr_t)obj,
         .version = 0,
      },
   };

   if (!oclass) {
      assert(!"called with invalid oclass");
      return -EINVAL;
   }

   obj->cls = oclass;

   return drmCommandWrite(fd, DRM_NOUVEAU_NVIF, &args, sizeof(args));
}

static void
nouveau_ws_channel_dealloc(int fd, int channel)
{
   struct drm_nouveau_channel_free req = {
      .channel = channel,
   };

   int ret = drmCommandWrite(fd, DRM_NOUVEAU_CHANNEL_FREE, &req, sizeof(req));
   assert(!ret);
}

int
nouveau_ws_context_create(struct nouveau_ws_device *dev, struct nouveau_ws_context **out)
{
   struct drm_nouveau_channel_alloc req = { };
   uint32_t classes[NOUVEAU_WS_CONTEXT_MAX_CLASSES];
   uint32_t base;

   *out = CALLOC_STRUCT(nouveau_ws_context);
   if (!*out)
      return -ENOMEM;

   int ret = drmCommandWriteRead(dev->fd, DRM_NOUVEAU_CHANNEL_ALLOC, &req, sizeof(req));
   if (ret)
      goto fail_chan;

   ret = nouveau_ws_context_query_classes(dev->fd, req.channel, classes);
   if (ret)
      goto fail_chan;

   base = (0xbeef + req.channel) << 16;
   uint32_t obj_class = nouveau_ws_context_find_class(classes, 0x2d);
   ret = nouveau_ws_subchan_alloc(dev->fd, req.channel, base | 0x902d, obj_class, &(*out)->eng2d);
   if (ret)
      goto fail_2d;

   obj_class = nouveau_ws_context_find_class(classes, 0x40);
   if (!obj_class)
      obj_class = nouveau_ws_context_find_class(classes, 0x39);
   ret = nouveau_ws_subchan_alloc(dev->fd, req.channel, base | 0x323f, obj_class, &(*out)->m2mf);
   if (ret)
      goto fail_subchan;

   obj_class = nouveau_ws_context_find_class(classes, 0xb5);
   ret = nouveau_ws_subchan_alloc(dev->fd, req.channel, 0, obj_class, &(*out)->copy);
   if (ret)
      goto fail_subchan;

   obj_class = nouveau_ws_context_find_class(classes, 0x97);
   ret = nouveau_ws_subchan_alloc(dev->fd, req.channel, base | 0x003d, obj_class, &(*out)->eng3d);
   if (ret)
      goto fail_subchan;

   obj_class = nouveau_ws_context_find_class(classes, 0xc0);
   ret = nouveau_ws_subchan_alloc(dev->fd, req.channel, base | 0x00c0, obj_class, &(*out)->compute);
   if (ret)
      goto fail_subchan;

   (*out)->channel = req.channel;
   (*out)->dev = dev;
   return 0;

fail_subchan:
   nouveau_ws_subchan_dealloc(dev->fd, &(*out)->compute);
   nouveau_ws_subchan_dealloc(dev->fd, &(*out)->eng3d);
   nouveau_ws_subchan_dealloc(dev->fd, &(*out)->copy);
   nouveau_ws_subchan_dealloc(dev->fd, &(*out)->m2mf);
   nouveau_ws_subchan_dealloc(dev->fd, &(*out)->eng2d);
fail_2d:
   nouveau_ws_channel_dealloc(dev->fd, req.channel);
fail_chan:
   FREE(*out);
   return ret;
}

void
nouveau_ws_context_destroy(struct nouveau_ws_context *context)
{
   nouveau_ws_subchan_dealloc(context->dev->fd, &context->compute);
   nouveau_ws_subchan_dealloc(context->dev->fd, &context->eng3d);
   nouveau_ws_subchan_dealloc(context->dev->fd, &context->copy);
   nouveau_ws_subchan_dealloc(context->dev->fd, &context->m2mf);
   nouveau_ws_subchan_dealloc(context->dev->fd, &context->eng2d);
   nouveau_ws_channel_dealloc(context->dev->fd, context->channel);
   FREE(context);
}

bool
nouveau_ws_context_killed(struct nouveau_ws_context *context)
{
   /* we are using the normal pushbuf submission ioctl as this is how nouveau implemented this on
    * the kernel side.
    * And as long as we submit nothing (e.g. nr_push is 0) it's more or less a noop on the kernel
    * side.
    */
   struct drm_nouveau_gem_pushbuf req = {
      .channel = context->channel,
   };
   int ret = drmCommandWriteRead(context->dev->fd, DRM_NOUVEAU_GEM_PUSHBUF, &req, sizeof(req));
   /* nouveau returns ENODEV once the channel was killed */
   return ret == -ENODEV;
}
