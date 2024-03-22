#include "nouveau_device.h"

#include "nouveau_context.h"

#include "drm-uapi/nouveau_drm.h"
#include "util/hash_table.h"
#include "util/u_debug.h"
#include "util/os_file.h"
#include "util/os_misc.h"

#include <fcntl.h>
#include <nouveau/nvif/ioctl.h>
#include <nvif/cl0080.h>
#include <nvif/class.h>
#include <unistd.h>
#include <xf86drm.h>

static uint8_t
sm_for_chipset(uint16_t chipset)
{
   if (chipset >= 0x190)
      return 89;
   // GH100 is older than AD10X, but is SM90
   else if (chipset >= 0x180)
      return 90;
   else if (chipset == 0x17b)
      return 87;
   else if (chipset >= 0x172)
      return 86;
   else if (chipset >= 0x170)
      return 80;
   else if (chipset >= 0x160)
      return 75;
   else if (chipset >= 0x14b)
      return 72;
   else if (chipset >= 0x140)
      return 70;
   else if (chipset >= 0x13b)
      return 62;
   else if (chipset >= 0x132)
      return 61;
   else if (chipset >= 0x130)
      return 60;
   else if (chipset >= 0x12b)
      return 53;
   else if (chipset >= 0x120)
      return 52;
   else if (chipset >= 0x110)
      return 50;
   // TODO: 37
   else if (chipset >= 0x0f0)
      return 35;
   else if (chipset >= 0x0ea)
      return 32;
   else if (chipset >= 0x0e0)
      return 30;
   // GF110 is SM20
   else if (chipset == 0x0c8)
      return 20;
   else if (chipset >= 0x0c1)
      return 21;
   else if (chipset >= 0x0c0)
      return 20;
   else if (chipset >= 0x0a3)
      return 12;
   // GT200 is SM13
   else if (chipset >= 0x0a0)
      return 13;
   else if (chipset >= 0x080)
      return 11;
   // this has to be == because 0x63 is older than 0x50 and has no compute
   else if (chipset == 0x050)
      return 10;
   // no compute
   return 0x00;
}

static uint8_t
max_warps_per_mp_for_sm(uint8_t sm)
{
   switch (sm) {
   case 10:
   case 11:
      return 24;
   case 12:
   case 13:
   case 75:
      return 32;
   case 20:
   case 21:
   case 86:
   case 87:
   case 89:
      return 48;
   case 30:
   case 32:
   case 35:
   case 37:
   case 50:
   case 52:
   case 53:
   case 60:
   case 61:
   case 62:
   case 70:
   case 72:
   case 80:
   case 90:
      return 64;
   default:
      assert(!"unkown SM version");
      // return the biggest known value
      return 64;
   }
}

static uint8_t
mp_per_tpc_for_chipset(uint16_t chipset)
{
   // GP100 is special and has two, otherwise it's a Volta and newer thing to have two
   if (chipset == 0x130 || chipset >= 0x140)
      return 2;
   return 1;
}

static void
nouveau_ws_device_set_dbg_flags(struct nouveau_ws_device *dev)
{
   const struct debug_control flags[] = {
      { "push_dump", NVK_DEBUG_PUSH_DUMP },
      { "push", NVK_DEBUG_PUSH_DUMP },
      { "push_sync", NVK_DEBUG_PUSH_SYNC },
      { "zero_memory", NVK_DEBUG_ZERO_MEMORY },
      { "vm", NVK_DEBUG_VM },
      { "no_cbuf", NVK_DEBUG_NO_CBUF },
      { NULL, 0 },
   };

   dev->debug_flags = parse_debug_string(getenv("NVK_DEBUG"), flags);
}

static int
nouveau_ws_param(int fd, uint64_t param, uint64_t *value)
{
   struct drm_nouveau_getparam data = { .param = param };

   int ret = drmCommandWriteRead(fd, DRM_NOUVEAU_GETPARAM, &data, sizeof(data));
   if (ret)
      return ret;

   *value = data.value;
   return 0;
}

static int
nouveau_ws_device_alloc(int fd, struct nouveau_ws_device *dev)
{
   struct {
      struct nvif_ioctl_v0 ioctl;
      struct nvif_ioctl_new_v0 new;
      struct nv_device_v0 dev;
   } args = {
      .ioctl = {
         .object = 0,
         .owner = NVIF_IOCTL_V0_OWNER_ANY,
         .route = 0x00,
         .type = NVIF_IOCTL_V0_NEW,
         .version = 0,
      },
      .new = {
         .handle = 0,
         .object = (uintptr_t)dev,
         .oclass = NV_DEVICE,
         .route = NVIF_IOCTL_V0_ROUTE_NVIF,
         .token = (uintptr_t)dev,
         .version = 0,
      },
      .dev = {
         .device = ~0ULL,
      },
   };

   return drmCommandWrite(fd, DRM_NOUVEAU_NVIF, &args, sizeof(args));
}

static int
nouveau_ws_device_info(int fd, struct nouveau_ws_device *dev)
{
   struct {
      struct nvif_ioctl_v0 ioctl;
      struct nvif_ioctl_mthd_v0 mthd;
      struct nv_device_info_v0 info;
   } args = {
      .ioctl = {
         .object = (uintptr_t)dev,
         .owner = NVIF_IOCTL_V0_OWNER_ANY,
         .route = 0x00,
         .type = NVIF_IOCTL_V0_MTHD,
         .version = 0,
      },
      .mthd = {
         .method = NV_DEVICE_V0_INFO,
         .version = 0,
      },
      .info = {
         .version = 0,
      },
   };

   int ret = drmCommandWriteRead(fd, DRM_NOUVEAU_NVIF, &args, sizeof(args));
   if (ret)
      return ret;

   dev->info.chipset = args.info.chipset;
   dev->info.vram_size_B = args.info.ram_user;

   switch (args.info.platform) {
   case NV_DEVICE_INFO_V0_IGP:
      dev->info.type = NV_DEVICE_TYPE_IGP;
      break;
   case NV_DEVICE_INFO_V0_SOC:
      dev->info.type = NV_DEVICE_TYPE_SOC;
      break;
   case NV_DEVICE_INFO_V0_PCI:
   case NV_DEVICE_INFO_V0_AGP:
   case NV_DEVICE_INFO_V0_PCIE:
   default:
      dev->info.type = NV_DEVICE_TYPE_DIS;
      break;
   }

   STATIC_ASSERT(sizeof(dev->info.device_name) >= sizeof(args.info.name));
   memcpy(dev->info.device_name, args.info.name, sizeof(args.info.name));

   STATIC_ASSERT(sizeof(dev->info.chipset_name) >= sizeof(args.info.chip));
   memcpy(dev->info.chipset_name, args.info.chip, sizeof(args.info.chip));

   return 0;
}

struct nouveau_ws_device *
nouveau_ws_device_new(drmDevicePtr drm_device)
{
   const char *path = drm_device->nodes[DRM_NODE_RENDER];
   struct nouveau_ws_device *device = CALLOC_STRUCT(nouveau_ws_device);
   uint64_t value = 0;
   drmVersionPtr ver = NULL;

   int fd = open(path, O_RDWR | O_CLOEXEC);
   if (fd < 0)
      goto out_open;

   ver = drmGetVersion(fd);
   if (!ver)
      goto out_err;

   if (strncmp("nouveau", ver->name, ver->name_len) != 0) {
      fprintf(stderr,
              "DRM kernel driver '%.*s' in use. NVK requires nouveau.\n",
              ver->name_len, ver->name);
      goto out_err;
   }

   uint32_t version =
      ver->version_major << 24 |
      ver->version_minor << 8  |
      ver->version_patchlevel;
   drmFreeVersion(ver);
   ver = NULL;

   if (version < 0x01000301)
      goto out_err;

   const uint64_t BDA = 1ull << 38;
   const uint64_t KERN = 1ull << 39;
   const uint64_t TOP = 1ull << 40;
   struct drm_nouveau_vm_init vminit = { KERN, TOP-KERN };
   int ret = drmCommandWrite(fd, DRM_NOUVEAU_VM_INIT, &vminit, sizeof(vminit));
   if (ret == 0) {
      device->has_vm_bind = true;
      util_vma_heap_init(&device->vma_heap, 4096, BDA - 4096);
      util_vma_heap_init(&device->bda_heap, BDA, KERN - BDA);
      simple_mtx_init(&device->vma_mutex, mtx_plain);
   }

   if (nouveau_ws_device_alloc(fd, device))
      goto out_err;

   if (nouveau_ws_param(fd, NOUVEAU_GETPARAM_PCI_DEVICE, &value))
      goto out_err;

   device->info.device_id = value;

   if (nouveau_ws_device_info(fd, device))
      goto out_err;

   if (drm_device->bustype == DRM_BUS_PCI) {
      assert(device->info.type == NV_DEVICE_TYPE_DIS);
      assert(device->info.device_id == drm_device->deviceinfo.pci->device_id);

      device->info.pci.domain       = drm_device->businfo.pci->domain;
      device->info.pci.bus          = drm_device->businfo.pci->bus;
      device->info.pci.dev          = drm_device->businfo.pci->dev;
      device->info.pci.func         = drm_device->businfo.pci->func;
      device->info.pci.revision_id  = drm_device->deviceinfo.pci->revision_id;
   };

   device->fd = fd;

   if (nouveau_ws_param(fd, NOUVEAU_GETPARAM_EXEC_PUSH_MAX, &value))
      device->max_push = NOUVEAU_GEM_MAX_PUSH;
   else
      device->max_push = value;

   if (device->info.vram_size_B == 0)
      device->local_mem_domain = NOUVEAU_GEM_DOMAIN_GART;
   else
      device->local_mem_domain = NOUVEAU_GEM_DOMAIN_VRAM;

   if (nouveau_ws_param(fd, NOUVEAU_GETPARAM_GRAPH_UNITS, &value))
      goto out_err;

   device->info.gpc_count = (value >> 0) & 0x000000ff;
   device->info.tpc_count = (value >> 8) & 0x0000ffff;

   nouveau_ws_device_set_dbg_flags(device);

   struct nouveau_ws_context *tmp_ctx;
   if (nouveau_ws_context_create(device, &tmp_ctx))
      goto out_err;

   device->info.sm = sm_for_chipset(device->info.chipset);
   device->info.cls_copy = tmp_ctx->copy.cls;
   device->info.cls_eng2d = tmp_ctx->eng2d.cls;
   device->info.cls_eng3d = tmp_ctx->eng3d.cls;
   device->info.cls_m2mf = tmp_ctx->m2mf.cls;
   device->info.cls_compute = tmp_ctx->compute.cls;

   // for now we hardcode those values, but in the future Nouveau could provide that information to
   // us instead.
   device->info.max_warps_per_mp = max_warps_per_mp_for_sm(device->info.sm);
   device->info.mp_per_tpc = mp_per_tpc_for_chipset(device->info.chipset);

   nouveau_ws_context_destroy(tmp_ctx);

   simple_mtx_init(&device->bos_lock, mtx_plain);
   device->bos = _mesa_pointer_hash_table_create(NULL);

   return device;

out_err:
   if (device->has_vm_bind) {
      util_vma_heap_finish(&device->vma_heap);
      util_vma_heap_finish(&device->bda_heap);
      simple_mtx_destroy(&device->vma_mutex);
   }
   if (ver)
      drmFreeVersion(ver);
out_open:
   FREE(device);
   close(fd);
   return NULL;
}

void
nouveau_ws_device_destroy(struct nouveau_ws_device *device)
{
   if (!device)
      return;

   _mesa_hash_table_destroy(device->bos, NULL);
   simple_mtx_destroy(&device->bos_lock);

   if (device->has_vm_bind) {
      util_vma_heap_finish(&device->vma_heap);
      util_vma_heap_finish(&device->bda_heap);
      simple_mtx_destroy(&device->vma_mutex);
   }

   close(device->fd);
   FREE(device);
}
