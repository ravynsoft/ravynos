/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <math.h>

#include "util/macros.h"

#include "intel_l3_config.h"

struct intel_l3_list {
   const struct intel_l3_config *configs;
   int length;
};

#define DECLARE_L3_LIST(hw) \
   struct intel_l3_list hw##_l3_list = \
   { .configs = hw##_l3_configs, .length = ARRAY_SIZE(hw##_l3_configs) }

/**
 * IVB/HSW validated L3 configurations.  The first entry will be used as
 * default by gfx7_restore_default_l3_config(), otherwise the ordering is
 * unimportant.
 */
static const struct intel_l3_config ivb_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 32,  0,  0, 32,  0,  0,  0 }},
   {{  0, 32,  0, 16, 16,  0,  0,  0 }},
   {{  0, 32,  0,  4,  0,  8,  4, 16 }},
   {{  0, 28,  0,  8,  0,  8,  4, 16 }},
   {{  0, 28,  0, 16,  0,  8,  4,  8 }},
   {{  0, 28,  0,  8,  0, 16,  4,  8 }},
   {{  0, 28,  0,  0,  0, 16,  4, 16 }},
   {{  0, 32,  0,  0,  0, 16,  0, 16 }},
   {{  0, 28,  0,  4, 32,  0,  0,  0 }},
   {{ 16, 16,  0, 16, 16,  0,  0,  0 }},
   {{ 16, 16,  0,  8,  0,  8,  8,  8 }},
   {{ 16, 16,  0,  4,  0,  8,  4, 16 }},
   {{ 16, 16,  0,  4,  0, 16,  4,  8 }},
   {{ 16, 16,  0,  0, 32,  0,  0,  0 }},
};
DECLARE_L3_LIST(ivb);

/**
 * VLV validated L3 configurations.  \sa ivb_l3_configs.
 */
static const struct intel_l3_config vlv_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 64,  0,  0, 32,  0,  0,  0 }},
   {{  0, 80,  0,  0, 16,  0,  0,  0 }},
   {{  0, 80,  0,  8,  8,  0,  0,  0 }},
   {{  0, 64,  0, 16, 16,  0,  0,  0 }},
   {{  0, 60,  0,  4, 32,  0,  0,  0 }},
   {{ 32, 32,  0, 16, 16,  0,  0,  0 }},
   {{ 32, 40,  0,  8, 16,  0,  0,  0 }},
   {{ 32, 40,  0, 16,  8,  0,  0,  0 }},
};
DECLARE_L3_LIST(vlv);

/**
 * BDW validated L3 configurations.  \sa ivb_l3_configs.
 */
static const struct intel_l3_config bdw_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 48, 48,  0,  0,  0,  0,  0 }},
   {{  0, 48,  0, 16, 32,  0,  0,  0 }},
   {{  0, 32,  0, 16, 48,  0,  0,  0 }},
   {{  0, 32,  0,  0, 64,  0,  0,  0 }},
   {{  0, 32, 64,  0,  0,  0,  0,  0 }},
   {{ 24, 16, 48,  0,  0,  0,  0,  0 }},
   {{ 24, 16,  0, 16, 32,  0,  0,  0 }},
   {{ 24, 16,  0, 32, 16,  0,  0,  0 }},
};
DECLARE_L3_LIST(bdw);

/**
 * CHV/SKL validated L3 configurations.  \sa ivb_l3_configs.
 */
static const struct intel_l3_config chv_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 48, 48,  0,  0,  0,  0,  0 }},
   {{  0, 48,  0, 16, 32,  0,  0,  0 }},
   {{  0, 32,  0, 16, 48,  0,  0,  0 }},
   {{  0, 32,  0,  0, 64,  0,  0,  0 }},
   {{  0, 32, 64,  0,  0,  0,  0,  0 }},
   {{ 32, 16, 48,  0,  0,  0,  0,  0 }},
   {{ 32, 16,  0, 16, 32,  0,  0,  0 }},
   {{ 32, 16,  0, 32, 16,  0,  0,  0 }},
};
DECLARE_L3_LIST(chv);

/**
 * BXT 2x6 validated L3 configurations.  \sa ivb_l3_configs.
 */
static const struct intel_l3_config bxt_2x6_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 32, 48,  0,  0,  0,  0,  0 }},
   {{  0, 32,  0,  8, 40,  0,  0,  0 }},
   {{  0, 32,  0, 32, 16,  0,  0,  0 }},
   {{ 16, 16, 48,  0,  0,  0,  0,  0 }},
   {{ 16, 16,  0, 40,  8,  0,  0,  0 }},
   {{ 16, 16,  0, 16, 32,  0,  0,  0 }},
};
DECLARE_L3_LIST(bxt_2x6);

/**
 * ICL validated L3 configurations.  \sa icl_l3_configs.
 * Zeroth entry in below table has been commented out intentionally
 * due to known issues with this configuration. Many other entries
 * suggested by h/w specification aren't added here because they
 * do under allocation of L3 cache with below partitioning.
 */
static const struct intel_l3_config icl_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   /*{{  0, 16, 80,  0,  0,  0,  0,  0 }},*/
   {{  0, 32, 64,  0,  0,  0,  0,  0 }},
};
DECLARE_L3_LIST(icl);

/**
 * TGL validated L3 configurations.  \sa tgl_l3_configs.
 */
static const struct intel_l3_config tgl_l3_configs[] = {
   /* SLM URB ALL DC  RO  IS   C   T */
   {{  0, 32,  88,  0,  0,  0,  0,  0 }},
   {{  0, 16, 104,  0,  0,  0,  0,  0 }},
};
DECLARE_L3_LIST(tgl);

/**
 * Empty L3 configurations.  \sa empty_l3_configs.
 */
static const struct intel_l3_config empty_l3_configs[] = {
   /* No configurations. L3FullWayAllocationEnable is always set. */
};
DECLARE_L3_LIST(empty);

/**
 * DG2 validated L3 configurations. \sa dg2_l3_configs.
 */
static const struct intel_l3_config dg2_l3_configs[] = {
   /* SLM URB  ALL   DC   RO  IS   C   T  TC */
   {{  0,  0,  128,   0,   0,  0,  0,  0,  0 }},
   {{  0,  0,   96,   0,   0,  0,  0,  0, 32 }},
   {{  0,  0,   64,   0,   0,  0,  0,  0, 64 }},
};
DECLARE_L3_LIST(dg2);

/**
 * Return a zero-terminated array of validated L3 configurations for the
 * specified device.
 */
static const struct intel_l3_list *
get_l3_list(const struct intel_device_info *devinfo)
{
   switch (devinfo->ver) {
   case 7:
      return (devinfo->platform == INTEL_PLATFORM_BYT ? &vlv_l3_list : &ivb_l3_list);

   case 8:
      return (devinfo->platform == INTEL_PLATFORM_CHV ? &chv_l3_list : &bdw_l3_list);

   case 9:
      if (devinfo->l3_banks == 1)
         return &bxt_2x6_l3_list;
      return &chv_l3_list;

   case 11:
      return &icl_l3_list;

   case 12:
      if (intel_device_info_is_dg2(devinfo) ||
          intel_device_info_is_mtl(devinfo)) {
         /* XXX - Some MTL configs may need special-casing here, but
          *       we have no way to identify them right now.
          */
         return &dg2_l3_list;
      } else if (devinfo->platform == INTEL_PLATFORM_DG1 || devinfo->verx10 == 125)
         return &empty_l3_list;
      else
         return &tgl_l3_list;

   default:
      unreachable("Not implemented");
   }
}

/**
 * L1-normalize a vector of L3 partition weights.
 */
static struct intel_l3_weights
norm_l3_weights(struct intel_l3_weights w)
{
   float sz = 0;

   for (unsigned i = 0; i < INTEL_NUM_L3P; i++)
      sz += w.w[i];

   for (unsigned i = 0; i < INTEL_NUM_L3P; i++)
      w.w[i] /= sz;

   return w;
}

/**
 * Get the relative partition weights of the specified L3 configuration.
 */
struct intel_l3_weights
intel_get_l3_config_weights(const struct intel_l3_config *cfg)
{
   if (cfg) {
      struct intel_l3_weights w;

      for (unsigned i = 0; i < INTEL_NUM_L3P; i++)
         w.w[i] = cfg->n[i];

      return norm_l3_weights(w);
   } else {
      const struct intel_l3_weights w = { { 0 } };
      return w;
   }
}

/**
 * Distance between two L3 configurations represented as vectors of weights.
 * Usually just the L1 metric except when the two configurations are
 * considered incompatible in which case the distance will be infinite.  Note
 * that the compatibility condition is asymmetric -- They will be considered
 * incompatible whenever the reference configuration \p w0 requires SLM, DC,
 * or URB but \p w1 doesn't provide it.
 */
float
intel_diff_l3_weights(struct intel_l3_weights w0, struct intel_l3_weights w1)
{
   if ((w0.w[INTEL_L3P_SLM] && !w1.w[INTEL_L3P_SLM]) ||
       (w0.w[INTEL_L3P_DC] && !w1.w[INTEL_L3P_DC] && !w1.w[INTEL_L3P_ALL]) ||
       (w0.w[INTEL_L3P_URB] && !w1.w[INTEL_L3P_URB])) {
      return HUGE_VALF;

   } else {
      float dw = 0;

      for (unsigned i = 0; i < INTEL_NUM_L3P; i++)
         dw += fabsf(w0.w[i] - w1.w[i]);

      return dw;
   }
}

/**
 * Return a reasonable default L3 configuration for the specified device based
 * on whether SLM and DC are required.  In the non-SLM non-DC case the result
 * is intended to approximately resemble the hardware defaults.
 */
struct intel_l3_weights
intel_get_default_l3_weights(const struct intel_device_info *devinfo,
                             bool needs_dc, bool needs_slm)
{
   struct intel_l3_weights w = {{ 0 }};

   w.w[INTEL_L3P_SLM] = devinfo->ver < 11 && needs_slm;
   w.w[INTEL_L3P_URB] = devinfo->verx10 < 125 ? 1.0 : 0.0;

   if (devinfo->ver >= 8) {
      w.w[INTEL_L3P_ALL] = 1.0;
   } else {
      w.w[INTEL_L3P_DC] = needs_dc ? 0.1 : 0;
      w.w[INTEL_L3P_RO] = devinfo->platform == INTEL_PLATFORM_BYT ? 0.5 : 1.0;
   }

   return norm_l3_weights(w);
}

/**
 * Get the default L3 configuration
 */
const struct intel_l3_config *
intel_get_default_l3_config(const struct intel_device_info *devinfo)
{
   /* For efficiency assume that the first entry of the array matches the
    * default configuration.
    */
   const struct intel_l3_list *const list = get_l3_list(devinfo);
   assert(list->length > 0 || devinfo->ver >= 12);
   if (list->length > 0) {
      const struct intel_l3_config *const cfg = &list->configs[0];
      assert(cfg == intel_get_l3_config(devinfo,
                       intel_get_default_l3_weights(devinfo, false, false)));
      return cfg;
   } else {
      return NULL;
   }
}

/**
 * Return the closest validated L3 configuration for the specified device and
 * weight vector.
 */
const struct intel_l3_config *
intel_get_l3_config(const struct intel_device_info *devinfo,
                    struct intel_l3_weights w0)
{
   const struct intel_l3_list *const list = get_l3_list(devinfo);
   const struct intel_l3_config *const cfgs = list->configs;
   const struct intel_l3_config *cfg_best = NULL;
   float dw_best = HUGE_VALF;

   for (int i = 0; i < list->length; i++) {
      const struct intel_l3_config *cfg = &cfgs[i];
      const float dw = intel_diff_l3_weights(w0, intel_get_l3_config_weights(cfg));

      if (dw < dw_best) {
         cfg_best = cfg;
         dw_best = dw;
      }
   }

   assert(cfg_best || devinfo->ver >= 12);
   return cfg_best;
}

/**
 * Return the size of an L3 way in KB.
 */
static unsigned
get_l3_way_size(const struct intel_device_info *devinfo)
{
   /*  Only MTL N/S/M have an 8KB way size, other MTL configs have 4KB
    *  ways.  See BSpec 45319.
    */
   const unsigned way_size_per_bank =
      devinfo->platform == INTEL_PLATFORM_MTL_U ? 8 :
      (devinfo->ver >= 9 && devinfo->l3_banks == 1) || devinfo->ver >= 11 ? 4 :
      2;

   assert(devinfo->l3_banks);
   return way_size_per_bank * devinfo->l3_banks;
}

/**
 * Return the unit brw_context::urb::size is expressed in, in KB.  \sa
 * intel_device_info::urb::size.
 */
static unsigned
get_urb_size_scale(const struct intel_device_info *devinfo)
{
   return (devinfo->ver >= 8 ? devinfo->num_slices : 1);
}

unsigned
intel_get_l3_config_urb_size(const struct intel_device_info *devinfo,
                             const struct intel_l3_config *cfg)
{
   unsigned urb_size;

   /* We don't have to program the URB size for some platforms. It's a fixed
    * value.
    */
   if (cfg == NULL) {
      ASSERTED const struct intel_l3_list *const list = get_l3_list(devinfo);
      assert(list->length == 0);
      urb_size = 0;
   } else {
      urb_size = intel_get_l3_partition_size(devinfo, cfg, INTEL_L3P_URB);
   }

   if (urb_size == 0)
      return devinfo->urb.size;

   /* From the SKL "L3 Allocation and Programming" documentation:
    *
    * "URB is limited to 1008KB due to programming restrictions.  This is not
    * a restriction of the L3 implementation, but of the FF and other clients.
    * Therefore, in a GT4 implementation it is possible for the programmed
    * allocation of the L3 data array to provide 3*384KB=1152KB for URB, but
    * only 1008KB of this will be used."
    */
   const unsigned max = (devinfo->ver == 9 ? 1008 : ~0);
   return MIN2(max, urb_size) / get_urb_size_scale(devinfo);
}

/**
 * Return the size of the specified L3 partition in KB.
 */
unsigned
intel_get_l3_partition_size(const struct intel_device_info *devinfo,
                            const struct intel_l3_config *cfg,
                            enum intel_l3_partition i)
{
   return cfg->n[i] * get_l3_way_size(devinfo);
}

/**
 * Print out the specified L3 configuration.
 */
void
intel_dump_l3_config(const struct intel_l3_config *cfg, FILE *fp)
{
   fprintf(stderr, "SLM=%d URB=%d ALL=%d DC=%d RO=%d IS=%d C=%d T=%d\n",
           cfg->n[INTEL_L3P_SLM], cfg->n[INTEL_L3P_URB], cfg->n[INTEL_L3P_ALL],
           cfg->n[INTEL_L3P_DC], cfg->n[INTEL_L3P_RO],
           cfg->n[INTEL_L3P_IS], cfg->n[INTEL_L3P_C], cfg->n[INTEL_L3P_T]);
}
