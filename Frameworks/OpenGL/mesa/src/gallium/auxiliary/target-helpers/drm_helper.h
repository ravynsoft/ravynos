#ifndef DRM_HELPER_H
#define DRM_HELPER_H

#include <stdio.h>
#include "target-helpers/inline_debug_helper.h"
#include "target-helpers/drm_helper_public.h"
#include "frontend/drm_driver.h"
#include "util/driconf.h"

/**
 * Instantiate a drm_driver_descriptor struct.
 */
#define DEFINE_DRM_DRIVER_DESCRIPTOR(descriptor_name, driver, _driconf, _driconf_count, func) \
const struct drm_driver_descriptor descriptor_name = {         \
   .driver_name = #driver,                                     \
   .driconf = _driconf,                                        \
   .driconf_count = _driconf_count,                            \
   .create_screen = func,                                      \
};

/* The static pipe loader refers to the *_driver_descriptor structs for all
 * drivers, regardless of whether they are configured in this Mesa build, or
 * whether they're included in the specific gallium target.  The target (dri,
 * vdpau, etc.) will include this header with the #defines for the specific
 * drivers it's including, and the disabled drivers will have a descriptor
 * with a stub create function logging the failure.
 *
 * The dynamic pipe loader instead has target/pipeloader/pipe_*.c including
 * this header in a pipe_*.so for each driver which will have one driver's
 * GALLIUM_* defined.  We make a single driver_descriptor entrypoint that is
 * dlsym()ed by the dynamic pipe loader.
 */

#ifdef PIPE_LOADER_DYNAMIC

#define DRM_DRIVER_DESCRIPTOR(driver, driconf, driconf_count)           \
   PUBLIC DEFINE_DRM_DRIVER_DESCRIPTOR(driver_descriptor, driver, driconf, driconf_count, pipe_##driver##_create_screen)

#define DRM_DRIVER_DESCRIPTOR_STUB(driver)

#define DRM_DRIVER_DESCRIPTOR_ALIAS(driver, alias, driconf, driconf_count)

#else

#define DRM_DRIVER_DESCRIPTOR(driver, driconf, driconf_count)                          \
   DEFINE_DRM_DRIVER_DESCRIPTOR(driver##_driver_descriptor, driver, driconf, driconf_count, pipe_##driver##_create_screen)

#define DRM_DRIVER_DESCRIPTOR_STUB(driver)                              \
   static struct pipe_screen *                                          \
   pipe_##driver##_create_screen(int fd, const struct pipe_screen_config *config) \
   {                                                                    \
      fprintf(stderr, #driver ": driver missing\n");                    \
      return NULL;                                                      \
   }                                                                    \
   DRM_DRIVER_DESCRIPTOR(driver, NULL, 0)

#define DRM_DRIVER_DESCRIPTOR_ALIAS(driver, alias, driconf, driconf_count) \
   DEFINE_DRM_DRIVER_DESCRIPTOR(alias##_driver_descriptor, alias, driconf, \
                                driconf_count, pipe_##driver##_create_screen)

#endif

#ifdef GALLIUM_KMSRO_ONLY
#undef GALLIUM_V3D
#undef GALLIUM_VC4
#undef GALLIUM_FREEDRENO
#undef GALLIUM_ETNAVIV
#undef GALLIUM_PANFROST
#undef GALLIUM_LIMA
#undef GALLIUM_ASAHI
#endif

#ifdef GALLIUM_I915
#include "i915/drm/i915_drm_public.h"
#include "i915/i915_public.h"

static struct pipe_screen *
pipe_i915_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct i915_winsys *iws;
   struct pipe_screen *screen;

   iws = i915_drm_winsys_create(fd);
   if (!iws)
      return NULL;

   screen = i915_screen_create(iws);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(i915, NULL, 0)
#else
DRM_DRIVER_DESCRIPTOR_STUB(i915)
#endif

#ifdef GALLIUM_IRIS
#include "iris/drm/iris_drm_public.h"

static struct pipe_screen *
pipe_iris_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = iris_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription iris_driconf[] = {
      #include "iris/driinfo_iris.h"
};
DRM_DRIVER_DESCRIPTOR(iris, iris_driconf, ARRAY_SIZE(iris_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(iris)
#endif

#ifdef GALLIUM_CROCUS
#include "crocus/drm/crocus_drm_public.h"

static struct pipe_screen *
pipe_crocus_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = crocus_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription crocus_driconf[] = {
      #include "crocus/driinfo_crocus.h"
};
DRM_DRIVER_DESCRIPTOR(crocus, crocus_driconf, ARRAY_SIZE(crocus_driconf))
#else
DRM_DRIVER_DESCRIPTOR_STUB(crocus)
#endif

#ifdef GALLIUM_NOUVEAU
#include "nouveau/drm/nouveau_drm_public.h"

static struct pipe_screen *
pipe_nouveau_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = nouveau_drm_screen_create(fd);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(nouveau, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(nouveau)
#endif

#if defined(GALLIUM_VC4) || defined(GALLIUM_V3D)
const driOptionDescription v3d_driconf[] = {
      #include "v3d/driinfo_v3d.h"
};
#endif

#ifdef GALLIUM_R300
#include "winsys/radeon_winsys.h"
#include "r300/r300_public.h"

static struct pipe_screen *
pipe_r300_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct radeon_winsys *rw;

   rw = radeon_drm_winsys_create(fd, config, r300_screen_create);
   return rw ? debug_screen_wrap(rw->screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(r300, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(r300)
#endif

#ifdef GALLIUM_R600
#include "winsys/radeon_winsys.h"
#include "r600/r600_public.h"

static struct pipe_screen *
pipe_r600_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct radeon_winsys *rw;

   rw = radeon_drm_winsys_create(fd, config, r600_screen_create);
   return rw ? debug_screen_wrap(rw->screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(r600, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(r600)
#endif

#ifdef GALLIUM_RADEONSI
#include "radeonsi/si_public.h"

static struct pipe_screen *
pipe_radeonsi_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen = radeonsi_screen_create(fd, config);

   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription radeonsi_driconf[] = {
      #include "radeonsi/driinfo_radeonsi.h"
};
DRM_DRIVER_DESCRIPTOR(radeonsi, radeonsi_driconf, ARRAY_SIZE(radeonsi_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(radeonsi)
#endif

#ifdef GALLIUM_VMWGFX
#include "svga/drm/svga_drm_public.h"
#include "svga/svga_public.h"

static struct pipe_screen *
pipe_vmwgfx_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct svga_winsys_screen *sws;
   struct pipe_screen *screen;

   sws = svga_drm_winsys_screen_create(fd);
   if (!sws)
      return NULL;

   screen = svga_screen_create(sws);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(vmwgfx, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(vmwgfx)
#endif

#ifdef GALLIUM_FREEDRENO
#include "freedreno/drm/freedreno_drm_public.h"

static struct pipe_screen *
pipe_msm_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = fd_drm_screen_create_renderonly(fd, NULL, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription msm_driconf[] = {
#ifdef GALLIUM_FREEDRENO
      #include "freedreno/driinfo_freedreno.h"
#endif
};
DRM_DRIVER_DESCRIPTOR(msm, msm_driconf, ARRAY_SIZE(msm_driconf))
DRM_DRIVER_DESCRIPTOR_ALIAS(msm, kgsl, msm_driconf, ARRAY_SIZE(msm_driconf))
#else
DRM_DRIVER_DESCRIPTOR_STUB(msm)
DRM_DRIVER_DESCRIPTOR_STUB(kgsl)
#endif

#if defined(GALLIUM_VIRGL) || (defined(GALLIUM_FREEDRENO) && !defined(PIPE_LOADER_DYNAMIC))
#include "virgl/drm/virgl_drm_public.h"
#include "virgl/virgl_public.h"

static struct pipe_screen *
pipe_virtio_gpu_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen = NULL;

   /* Try native guest driver(s) first, and then fallback to virgl: */
#ifdef GALLIUM_FREEDRENO
   if (!screen)
      screen = fd_drm_screen_create_renderonly(fd, NULL, config);
#endif
#ifdef GALLIUM_VIRGL
   if (!screen)
      screen = virgl_drm_screen_create(fd, config);
#endif
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription virgl_driconf[] = {
      #include "virgl/virgl_driinfo.h.in"
};
DRM_DRIVER_DESCRIPTOR(virtio_gpu, virgl_driconf, ARRAY_SIZE(virgl_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(virtio_gpu)
#endif

#ifdef GALLIUM_VC4
#include "vc4/drm/vc4_drm_public.h"

static struct pipe_screen *
pipe_vc4_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = vc4_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(vc4, v3d_driconf, ARRAY_SIZE(v3d_driconf))
#else
DRM_DRIVER_DESCRIPTOR_STUB(vc4)
#endif

#ifdef GALLIUM_V3D
#include "v3d/drm/v3d_drm_public.h"

static struct pipe_screen *
pipe_v3d_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = v3d_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

DRM_DRIVER_DESCRIPTOR(v3d, v3d_driconf, ARRAY_SIZE(v3d_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(v3d)
#endif

#ifdef GALLIUM_PANFROST
#include "panfrost/drm/panfrost_drm_public.h"

static struct pipe_screen *
pipe_panfrost_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = panfrost_drm_screen_create(fd);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(panfrost, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(panfrost)
#endif

#ifdef GALLIUM_ASAHI
#include "asahi/drm/asahi_drm_public.h"

static struct pipe_screen *
pipe_asahi_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = asahi_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription asahi_driconf[] = {
      #include "asahi/driinfo_asahi.h"
};
DRM_DRIVER_DESCRIPTOR(asahi, asahi_driconf, ARRAY_SIZE(asahi_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(asahi)
#endif

#ifdef GALLIUM_ETNAVIV
#include "etnaviv/drm/etnaviv_drm_public.h"

static struct pipe_screen *
pipe_etnaviv_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = etna_drm_screen_create(fd);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(etnaviv, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(etnaviv)
#endif

#ifdef GALLIUM_TEGRA
#include "tegra/drm/tegra_drm_public.h"

static struct pipe_screen *
pipe_tegra_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = tegra_drm_screen_create(fd);

   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(tegra, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(tegra)
#endif

#ifdef GALLIUM_LIMA
#include "lima/drm/lima_drm_public.h"

static struct pipe_screen *
pipe_lima_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = lima_drm_screen_create(fd);
   return screen ? debug_screen_wrap(screen) : NULL;
}
DRM_DRIVER_DESCRIPTOR(lima, NULL, 0)

#else
DRM_DRIVER_DESCRIPTOR_STUB(lima)
#endif

#ifdef GALLIUM_ZINK
#include "zink/zink_public.h"

static struct pipe_screen *
pipe_zink_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;
   screen = zink_drm_create_screen(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}

const driOptionDescription zink_driconf[] = {
      #include "zink/driinfo_zink.h"
};
DRM_DRIVER_DESCRIPTOR(zink, zink_driconf, ARRAY_SIZE(zink_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(zink)
#endif

#ifdef GALLIUM_KMSRO
#include "kmsro/drm/kmsro_drm_public.h"

static struct pipe_screen *
pipe_kmsro_create_screen(int fd, const struct pipe_screen_config *config)
{
   struct pipe_screen *screen;

   screen = kmsro_drm_screen_create(fd, config);
   return screen ? debug_screen_wrap(screen) : NULL;
}
const driOptionDescription kmsro_driconf[] = {
#if defined(GALLIUM_VC4) || defined(GALLIUM_V3D)
      #include "v3d/driinfo_v3d.h"
#endif
#ifdef GALLIUM_ASAHI
      #include "asahi/driinfo_asahi.h"
#endif
#ifdef GALLIUM_FREEDRENO
      #include "freedreno/driinfo_freedreno.h"
#endif
};
DRM_DRIVER_DESCRIPTOR(kmsro, kmsro_driconf, ARRAY_SIZE(kmsro_driconf))

#else
DRM_DRIVER_DESCRIPTOR_STUB(kmsro)
#endif

/* kmsro should be the last entry in the file. */

#endif /* DRM_HELPER_H */
