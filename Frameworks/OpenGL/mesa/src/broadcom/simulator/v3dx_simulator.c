/*
 * Copyright © 2014-2017 Broadcom
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

/**
 * @file v3dx_simulator.c
 *
 * Implements the actual HW interaction betweeh the GL driver's V3D simulator and the simulator.
 *
 * The register headers between V3D versions will have conflicting defines, so
 * all register interactions appear in this file and are compiled per V3D version
 * we support.
 */

#ifdef USE_V3D_SIMULATOR

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "v3d_simulator.h"
#include "v3d_simulator_wrapper.h"

#include "common/v3d_performance_counters.h"

#include "util/macros.h"
#include "util/bitscan.h"
#include "drm-uapi/v3d_drm.h"

#define HW_REGISTER_RO(x) (x)
#define HW_REGISTER_RW(x) (x)
#if V3D_VERSION == 71
#include "libs/core/v3d/registers/7.1.6.0/v3d.h"
#else
#if V3D_VERSION == 42
#include "libs/core/v3d/registers/4.2.14.0/v3d.h"
#endif
#endif

#define V3D_WRITE(reg, val) v3d_hw_write_reg(v3d, reg, val)
#define V3D_READ(reg) v3d_hw_read_reg(v3d, reg)

/* Invalidates the L2C cache.  This is a read-only cache for uniforms and instructions. */
static void
v3d_invalidate_l2c(struct v3d_hw *v3d)
{
        if (V3D_VERSION >= 33)
                return;

        V3D_WRITE(V3D_CTL_0_L2CACTL,
                  V3D_CTL_0_L2CACTL_L2CCLR_SET |
                  V3D_CTL_0_L2CACTL_L2CENA_SET);
}

enum v3d_l2t_cache_flush_mode {
        V3D_CACHE_FLUSH_MODE_FLUSH,
        V3D_CACHE_FLUSH_MODE_CLEAR,
        V3D_CACHE_FLUSH_MODE_CLEAN,
};

/* Invalidates texture L2 cachelines */
static void
v3d_invalidate_l2t(struct v3d_hw *v3d)
{
        V3D_WRITE(V3D_CTL_0_L2TFLSTA, 0);
        V3D_WRITE(V3D_CTL_0_L2TFLEND, ~0);
        V3D_WRITE(V3D_CTL_0_L2TCACTL,
                  V3D_CTL_0_L2TCACTL_L2TFLS_SET |
                  (V3D_CACHE_FLUSH_MODE_FLUSH << V3D_CTL_0_L2TCACTL_L2TFLM_LSB));
}

/*
 * Wait for l2tcactl, used for flushes.
 *
 * FIXME: for a multicore scenario we should pass here the core. All wrapper
 * assumes just one core, so would be better to handle that on that case.
 */
static UNUSED void v3d_core_wait_l2tcactl(struct v3d_hw *v3d,
                                          uint32_t ctrl)
{
   assert(!(ctrl & ~(V3D_CTL_0_L2TCACTL_TMUWCF_SET | V3D_CTL_0_L2TCACTL_L2TFLS_SET)));

   while (V3D_READ(V3D_CTL_0_L2TCACTL) & ctrl) {
           v3d_hw_tick(v3d);
   }
}

/* Flushes dirty texture cachelines from the L1 write combiner */
static void
v3d_flush_l1td(struct v3d_hw *v3d)
{
        V3D_WRITE(V3D_CTL_0_L2TCACTL,
                  V3D_CTL_0_L2TCACTL_TMUWCF_SET);

        /* Note: here the kernel (and previous versions of the simulator
         * wrapper) is using V3D_CTL_0_L2TCACTL_L2TFLS_SET, as with l2t. We
         * understand that it makes more sense to do like this. We need to
         * confirm which one is doing it correctly. So far things work fine on
         * the simulator this way.
         */
        v3d_core_wait_l2tcactl(v3d, V3D_CTL_0_L2TCACTL_TMUWCF_SET);
}

/* Flushes dirty texture L2 cachelines */
static void
v3d_flush_l2t(struct v3d_hw *v3d)
{
        V3D_WRITE(V3D_CTL_0_L2TFLSTA, 0);
        V3D_WRITE(V3D_CTL_0_L2TFLEND, ~0);
        V3D_WRITE(V3D_CTL_0_L2TCACTL,
                  V3D_CTL_0_L2TCACTL_L2TFLS_SET |
                  (V3D_CACHE_FLUSH_MODE_CLEAN << V3D_CTL_0_L2TCACTL_L2TFLM_LSB));

        v3d_core_wait_l2tcactl(v3d, V3D_CTL_0_L2TCACTL_L2TFLS_SET);
}

/* Invalidates the slice caches.  These are read-only caches. */
static void
v3d_invalidate_slices(struct v3d_hw *v3d)
{
        V3D_WRITE(V3D_CTL_0_SLCACTL, ~0);
}

static void
v3d_invalidate_caches(struct v3d_hw *v3d)
{
        v3d_invalidate_l2c(v3d);
        v3d_invalidate_l2t(v3d);
        v3d_invalidate_slices(v3d);
}

static uint32_t g_gmp_ofs;
static void
v3d_reload_gmp(struct v3d_hw *v3d)
{
        /* Completely reset the GMP. */
        V3D_WRITE(V3D_GMP_CFG,
                  V3D_GMP_CFG_PROTENABLE_SET);
        V3D_WRITE(V3D_GMP_TABLE_ADDR, g_gmp_ofs);
        V3D_WRITE(V3D_GMP_CLEAR_LOAD, ~0);
        while (V3D_READ(V3D_GMP_STATUS) &
               V3D_GMP_STATUS_CFG_BUSY_SET) {
                ;
        }
}

static UNUSED void
v3d_flush_caches(struct v3d_hw *v3d)
{
        v3d_flush_l1td(v3d);
        v3d_flush_l2t(v3d);
}

#if V3D_VERSION < 71
#define TFU_REG(NAME) V3D_TFU_ ## NAME
#else
#define TFU_REG(NAME) V3D_IFC_ ## NAME
#endif


int
v3dX(simulator_submit_tfu_ioctl)(struct v3d_hw *v3d,
                                 struct drm_v3d_submit_tfu *args)
{
        int last_vtct = V3D_READ(TFU_REG(CS)) & V3D_TFU_CS_CVTCT_SET;

        V3D_WRITE(TFU_REG(IIA), args->iia);
        V3D_WRITE(TFU_REG(IIS), args->iis);
        V3D_WRITE(TFU_REG(ICA), args->ica);
        V3D_WRITE(TFU_REG(IUA), args->iua);
        V3D_WRITE(TFU_REG(IOA), args->ioa);
#if V3D_VERSION >= 71
        V3D_WRITE(TFU_REG(IOC), args->v71.ioc);
#endif
        V3D_WRITE(TFU_REG(IOS), args->ios);
        V3D_WRITE(TFU_REG(COEF0), args->coef[0]);
        V3D_WRITE(TFU_REG(COEF1), args->coef[1]);
        V3D_WRITE(TFU_REG(COEF2), args->coef[2]);
        V3D_WRITE(TFU_REG(COEF3), args->coef[3]);

        V3D_WRITE(TFU_REG(ICFG), args->icfg);

        while ((V3D_READ(TFU_REG(CS)) & V3D_TFU_CS_CVTCT_SET) == last_vtct) {
                v3d_hw_tick(v3d);
        }

        return 0;
}

int
v3dX(simulator_submit_csd_ioctl)(struct v3d_hw *v3d,
                                 struct drm_v3d_submit_csd *args,
                                 uint32_t gmp_ofs)
{
#if V3D_VERSION >= 42
        int last_completed_jobs = (V3D_READ(V3D_CSD_0_STATUS) &
                                   V3D_CSD_0_STATUS_NUM_COMPLETED_JOBS_SET);
        g_gmp_ofs = gmp_ofs;
        v3d_reload_gmp(v3d);

        v3d_invalidate_caches(v3d);

        V3D_WRITE(V3D_CSD_0_QUEUED_CFG1, args->cfg[1]);
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG2, args->cfg[2]);
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG3, args->cfg[3]);
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG4, args->cfg[4]);
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG5, args->cfg[5]);
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG6, args->cfg[6]);
#if V3D_VERSION >= 71
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG7, 0);
#endif
        /* CFG0 kicks off the job */
        V3D_WRITE(V3D_CSD_0_QUEUED_CFG0, args->cfg[0]);

        /* Now we wait for the dispatch to finish. The safest way is to check
         * if NUM_COMPLETED_JOBS has increased. Note that in spite of that
         * name that register field is about the number of completed
         * dispatches.
         */
        while ((V3D_READ(V3D_CSD_0_STATUS) &
                V3D_CSD_0_STATUS_NUM_COMPLETED_JOBS_SET) == last_completed_jobs) {
                v3d_hw_tick(v3d);
        }

        v3d_flush_caches(v3d);

        return 0;
#else
        return -1;
#endif
}

int
v3dX(simulator_get_param_ioctl)(struct v3d_hw *v3d,
                                struct drm_v3d_get_param *args)
{
        static const uint32_t reg_map[] = {
                [DRM_V3D_PARAM_V3D_UIFCFG] = V3D_HUB_CTL_UIFCFG,
                [DRM_V3D_PARAM_V3D_HUB_IDENT1] = V3D_HUB_CTL_IDENT1,
                [DRM_V3D_PARAM_V3D_HUB_IDENT2] = V3D_HUB_CTL_IDENT2,
                [DRM_V3D_PARAM_V3D_HUB_IDENT3] = V3D_HUB_CTL_IDENT3,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT0] = V3D_CTL_0_IDENT0,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT1] = V3D_CTL_0_IDENT1,
                [DRM_V3D_PARAM_V3D_CORE0_IDENT2] = V3D_CTL_0_IDENT2,
        };

        switch (args->param) {
        case DRM_V3D_PARAM_SUPPORTS_TFU:
                args->value = 1;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_CSD:
                args->value = V3D_VERSION >= 42;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_CACHE_FLUSH:
                args->value = 1;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_PERFMON:
                args->value = V3D_VERSION >= 42;
                return 0;
        case DRM_V3D_PARAM_SUPPORTS_MULTISYNC_EXT:
                args->value = 1;
                return 0;
	case DRM_V3D_PARAM_SUPPORTS_CPU_QUEUE:
		args->value = 1;
		return 0;
        }

        if (args->param < ARRAY_SIZE(reg_map) && reg_map[args->param]) {
                args->value = V3D_READ(reg_map[args->param]);
                return 0;
        }

        fprintf(stderr, "Unknown DRM_IOCTL_V3D_GET_PARAM(%lld)\n",
                (long long)args->value);
        abort();
}

static struct v3d_hw *v3d_isr_hw;


static void
v3d_isr_core(struct v3d_hw *v3d,
             unsigned core)
{
        /* FIXME: so far we are assuming just one core, and using only the _0_
         * registers. If we add multiple-core on the simulator, we would need
         * to pass core as a parameter, and chose the proper registers.
         */
        assert(core == 0);
        uint32_t core_status = V3D_READ(V3D_CTL_0_INT_STS);
        V3D_WRITE(V3D_CTL_0_INT_CLR, core_status);

        if (core_status & V3D_CTL_0_INT_STS_INT_OUTOMEM_SET) {
                uint32_t size = 256 * 1024;
                uint32_t offset = v3d_simulator_get_spill(size);

                v3d_reload_gmp(v3d);

                V3D_WRITE(V3D_PTB_0_BPOA, offset);
                V3D_WRITE(V3D_PTB_0_BPOS, size);
                return;
        }

#if V3D_VERSION <= 42
        if (core_status & V3D_CTL_0_INT_STS_INT_GMPV_SET) {
                fprintf(stderr, "GMP violation at 0x%08x\n",
                        V3D_READ(V3D_GMP_VIO_ADDR));
        } else {
                fprintf(stderr,
                        "Unexpected ISR with core status 0x%08x\n",
                        core_status);
        }
        abort();
#endif
}

static void
handle_mmu_interruptions(struct v3d_hw *v3d,
                         uint32_t hub_status)
{
        bool wrv = hub_status & V3D_HUB_CTL_INT_STS_INT_MMU_WRV_SET;
        bool pti = hub_status & V3D_HUB_CTL_INT_STS_INT_MMU_PTI_SET;
        bool cap = hub_status & V3D_HUB_CTL_INT_STS_INT_MMU_CAP_SET;

        if (!(pti || cap || wrv))
                return;

        const char *client = "?";
        uint32_t axi_id = V3D_READ(V3D_MMU_VIO_ID);
        uint32_t va_width = 30;

        static const char *const v3d42_axi_ids[] = {
                "L2T",
                "PTB",
                "PSE",
                "TLB",
                "CLE",
                "TFU",
                "MMU",
                "GMP",
        };

        axi_id = axi_id >> 5;
        if (axi_id < ARRAY_SIZE(v3d42_axi_ids))
                client = v3d42_axi_ids[axi_id];

        uint32_t mmu_debug = V3D_READ(V3D_MMU_DEBUG_INFO);

        va_width += ((mmu_debug & V3D_MMU_DEBUG_INFO_VA_WIDTH_SET)
                     >> V3D_MMU_DEBUG_INFO_VA_WIDTH_LSB);

        /* Only the top bits (final number depends on the gen) of the virtual
         * address are reported in the MMU VIO_ADDR register.
         */
        uint64_t vio_addr = ((uint64_t)V3D_READ(V3D_MMU_VIO_ADDR) <<
                             (va_width - 32));

        /* Difference with the kernel: here were are going to abort after
         * logging, so we don't bother with some stuff that the kernel does,
         * like restoring the MMU ctrl bits
         */

        fprintf(stderr, "MMU error from client %s (%d) at 0x%llx%s%s%s\n",
                client, axi_id, (long long) vio_addr,
                wrv ? ", write violation" : "",
                pti ? ", pte invalid" : "",
                cap ? ", cap exceeded" : "");

        abort();
}

static void
v3d_isr_hub(struct v3d_hw *v3d)
{
        uint32_t hub_status = V3D_READ(V3D_HUB_CTL_INT_STS);

        /* Acknowledge the interrupts we're handling here */
        V3D_WRITE(V3D_HUB_CTL_INT_CLR, hub_status);

        if (hub_status & V3D_HUB_CTL_INT_STS_INT_TFUC_SET) {
                /* FIXME: we were not able to raise this exception. We let the
                 * unreachable here, so we could get one if it is raised on
                 * the future. In any case, note that for this case we would
                 * only be doing debugging log.
                 */
                unreachable("TFU Conversion Complete interrupt not handled");
        }

        handle_mmu_interruptions(v3d, hub_status);

#if V3D_VERSION == 71
        if (hub_status & V3D_HUB_CTL_INT_STS_INT_GMPV_SET) {
                fprintf(stderr, "GMP violation at 0x%08x\n",
                        V3D_READ(V3D_GMP_VIO_ADDR));
        } else {
                fprintf(stderr,
                        "Unexpected ISR with status 0x%08x\n",
                        hub_status);
        }
        abort();
#endif
}

static void
v3d_isr(uint32_t hub_status)
{
        struct v3d_hw *v3d = v3d_isr_hw;
        uint32_t mask = hub_status;

        /* Check the hub_status bits */
        while (mask) {
                unsigned core = u_bit_scan(&mask);

                if (core == v3d_hw_get_hub_core())
                        v3d_isr_hub(v3d);
                else
                        v3d_isr_core(v3d, core);
        }

        return;
}

void
v3dX(simulator_init_regs)(struct v3d_hw *v3d)
{
        /* FIXME: the kernel captures some additional core interrupts here,
         * for tracing. Perhaps we should evaluate to do the same here and add
         * some debug options.
         */
        uint32_t core_interrupts = V3D_CTL_0_INT_STS_INT_OUTOMEM_SET;
#if V3D_VERSION <= 42
        core_interrupts |= V3D_CTL_0_INT_STS_INT_GMPV_SET;
#endif

        V3D_WRITE(V3D_CTL_0_INT_MSK_SET, ~core_interrupts);
        V3D_WRITE(V3D_CTL_0_INT_MSK_CLR, core_interrupts);

        uint32_t hub_interrupts =
           (V3D_HUB_CTL_INT_STS_INT_MMU_WRV_SET |  /* write violation */
            V3D_HUB_CTL_INT_STS_INT_MMU_PTI_SET |  /* page table invalid */
            V3D_HUB_CTL_INT_STS_INT_MMU_CAP_SET |  /* CAP exceeded */
            V3D_HUB_CTL_INT_STS_INT_TFUC_SET); /* TFU conversion */

#if V3D_VERSION == 71
        hub_interrupts |= V3D_HUB_CTL_INT_STS_INT_GMPV_SET;
#endif
        V3D_WRITE(V3D_HUB_CTL_INT_MSK_SET, ~hub_interrupts);
        V3D_WRITE(V3D_HUB_CTL_INT_MSK_CLR, hub_interrupts);

        v3d_isr_hw = v3d;
        v3d_hw_set_isr(v3d, v3d_isr);
}

void
v3dX(simulator_submit_cl_ioctl)(struct v3d_hw *v3d,
                                struct drm_v3d_submit_cl *submit,
                                uint32_t gmp_ofs)
{
        int last_bfc = (V3D_READ(V3D_CLE_0_BFC) &
                        V3D_CLE_0_BFC_BMFCT_SET);

        int last_rfc = (V3D_READ(V3D_CLE_0_RFC) &
                        V3D_CLE_0_RFC_RMFCT_SET);

        g_gmp_ofs = gmp_ofs;
        v3d_reload_gmp(v3d);

        v3d_invalidate_caches(v3d);

        if (submit->qma) {
                V3D_WRITE(V3D_CLE_0_CT0QMA, submit->qma);
                V3D_WRITE(V3D_CLE_0_CT0QMS, submit->qms);
        }
        if (submit->qts) {
                V3D_WRITE(V3D_CLE_0_CT0QTS,
                          V3D_CLE_0_CT0QTS_CTQTSEN_SET |
                          submit->qts);
        }
        V3D_WRITE(V3D_CLE_0_CT0QBA, submit->bcl_start);
        V3D_WRITE(V3D_CLE_0_CT0QEA, submit->bcl_end);

        /* Wait for bin to complete before firing render.  The kernel's
         * scheduler implements this using the GPU scheduler blocking on the
         * bin fence completing.  (We don't use HW semaphores).
         */
        while ((V3D_READ(V3D_CLE_0_BFC) &
                V3D_CLE_0_BFC_BMFCT_SET) == last_bfc) {
                v3d_hw_tick(v3d);
        }

        v3d_invalidate_caches(v3d);

        V3D_WRITE(V3D_CLE_0_CT1QBA, submit->rcl_start);
        V3D_WRITE(V3D_CLE_0_CT1QEA, submit->rcl_end);

        while ((V3D_READ(V3D_CLE_0_RFC) &
                V3D_CLE_0_RFC_RMFCT_SET) == last_rfc) {
                v3d_hw_tick(v3d);
        }
}

#define V3D_PCTR_0_PCTR_N(x) (V3D_PCTR_0_PCTR0 + 4 * (x))
#define V3D_PCTR_0_SRC_N(x) (V3D_PCTR_0_SRC_0_3 + 4 * (x))
#define V3D_PCTR_0_SRC_N_SHIFT(x) ((x) * 8)
#define V3D_PCTR_0_SRC_N_MASK(x) (BITFIELD_RANGE(V3D_PCTR_0_SRC_N_SHIFT(x), \
                                                 V3D_PCTR_0_SRC_N_SHIFT(x) + \
                                                 V3D_PCTR_0_SRC_0_3_PCTRS0_MSB))

void
v3dX(simulator_perfmon_start)(struct v3d_hw *v3d,
                              uint32_t ncounters,
                              uint8_t *events)
{
        int i, j;
        uint32_t source;
        uint32_t mask = BITFIELD_RANGE(0, ncounters);

        for (i = 0; i < ncounters; i+=4) {
                source = i / 4;
                uint32_t channels = 0;
                for (j = 0; j < 4 && (i + j) < ncounters; j++)
                        channels |= events[i + j] << V3D_PCTR_0_SRC_N_SHIFT(j);
                V3D_WRITE(V3D_PCTR_0_SRC_N(source), channels);
        }
        V3D_WRITE(V3D_PCTR_0_CLR, mask);
        V3D_WRITE(V3D_PCTR_0_OVERFLOW, mask);
        V3D_WRITE(V3D_PCTR_0_EN, mask);
}

void v3dX(simulator_perfmon_stop)(struct v3d_hw *v3d,
                                  uint32_t ncounters,
                                  uint64_t *values)
{
        int i;

        for (i = 0; i < ncounters; i++)
                values[i] += V3D_READ(V3D_PCTR_0_PCTR_N(i));

        V3D_WRITE(V3D_PCTR_0_EN, 0);
}

void v3dX(simulator_get_perfcnt_total)(uint32_t *count)
{
        *count = ARRAY_SIZE(v3d_performance_counters);
}

#endif /* USE_V3D_SIMULATOR */
