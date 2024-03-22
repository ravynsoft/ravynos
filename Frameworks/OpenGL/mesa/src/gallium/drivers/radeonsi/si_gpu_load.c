/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* The GPU load is measured as follows.
 *
 * There is a thread which samples the GRBM_STATUS register at a certain
 * frequency and the "busy" or "idle" counter is incremented based on
 * whether the GUI_ACTIVE bit is set or not.
 *
 * Then, the user can sample the counters twice and calculate the average
 * GPU load between the two samples.
 */

#include "si_pipe.h"
#include "si_query.h"
#include "util/os_time.h"

/* For good accuracy at 1000 fps or lower. This will be inaccurate for higher
 * fps (there are too few samples per frame). */
#define SAMPLES_PER_SEC 10000

#define GRBM_STATUS   0x8010
#define TA_BUSY(x)    (((x) >> 14) & 0x1)
#define GDS_BUSY(x)   (((x) >> 15) & 0x1)
#define VGT_BUSY(x)   (((x) >> 17) & 0x1)
#define IA_BUSY(x)    (((x) >> 19) & 0x1)
#define SX_BUSY(x)    (((x) >> 20) & 0x1)
#define WD_BUSY(x)    (((x) >> 21) & 0x1)
#define SPI_BUSY(x)   (((x) >> 22) & 0x1)
#define BCI_BUSY(x)   (((x) >> 23) & 0x1)
#define SC_BUSY(x)    (((x) >> 24) & 0x1)
#define PA_BUSY(x)    (((x) >> 25) & 0x1)
#define DB_BUSY(x)    (((x) >> 26) & 0x1)
#define CP_BUSY(x)    (((x) >> 29) & 0x1)
#define CB_BUSY(x)    (((x) >> 30) & 0x1)
#define GUI_ACTIVE(x) (((x) >> 31) & 0x1)

#define SRBM_STATUS2 0x0e4c
#define SDMA_BUSY(x) (((x) >> 5) & 0x1)

#define CP_STAT              0x8680
#define PFP_BUSY(x)          (((x) >> 15) & 0x1)
#define MEQ_BUSY(x)          (((x) >> 16) & 0x1)
#define ME_BUSY(x)           (((x) >> 17) & 0x1)
#define SURFACE_SYNC_BUSY(x) (((x) >> 21) & 0x1)
#define DMA_BUSY(x)          (((x) >> 22) & 0x1)
#define SCRATCH_RAM_BUSY(x)  (((x) >> 24) & 0x1)

#define IDENTITY(x) x

#define UPDATE_COUNTER(field, mask)                                                                \
   do {                                                                                            \
      if (mask(value))                                                                             \
         p_atomic_inc(&counters->named.field.busy);                                                \
      else                                                                                         \
         p_atomic_inc(&counters->named.field.idle);                                                \
   } while (0)

static void si_update_mmio_counters(struct si_screen *sscreen, union si_mmio_counters *counters)
{
   uint32_t value = 0;
   bool gui_busy, sdma_busy = false;

   /* GRBM_STATUS */
   sscreen->ws->read_registers(sscreen->ws, GRBM_STATUS, 1, &value);

   UPDATE_COUNTER(ta, TA_BUSY);
   UPDATE_COUNTER(gds, GDS_BUSY);
   UPDATE_COUNTER(vgt, VGT_BUSY);
   UPDATE_COUNTER(ia, IA_BUSY);
   UPDATE_COUNTER(sx, SX_BUSY);
   UPDATE_COUNTER(wd, WD_BUSY);
   UPDATE_COUNTER(spi, SPI_BUSY);
   UPDATE_COUNTER(bci, BCI_BUSY);
   UPDATE_COUNTER(sc, SC_BUSY);
   UPDATE_COUNTER(pa, PA_BUSY);
   UPDATE_COUNTER(db, DB_BUSY);
   UPDATE_COUNTER(cp, CP_BUSY);
   UPDATE_COUNTER(cb, CB_BUSY);
   UPDATE_COUNTER(gui, GUI_ACTIVE);
   gui_busy = GUI_ACTIVE(value);

   if (sscreen->info.gfx_level == GFX7 || sscreen->info.gfx_level == GFX8) {
      /* SRBM_STATUS2 */
      sscreen->ws->read_registers(sscreen->ws, SRBM_STATUS2, 1, &value);

      UPDATE_COUNTER(sdma, SDMA_BUSY);
      sdma_busy = SDMA_BUSY(value);
   }

   if (sscreen->info.gfx_level >= GFX8) {
      /* CP_STAT */
      sscreen->ws->read_registers(sscreen->ws, CP_STAT, 1, &value);

      UPDATE_COUNTER(pfp, PFP_BUSY);
      UPDATE_COUNTER(meq, MEQ_BUSY);
      UPDATE_COUNTER(me, ME_BUSY);
      UPDATE_COUNTER(surf_sync, SURFACE_SYNC_BUSY);
      UPDATE_COUNTER(cp_dma, DMA_BUSY);
      UPDATE_COUNTER(scratch_ram, SCRATCH_RAM_BUSY);
   }

   value = gui_busy || sdma_busy;
   UPDATE_COUNTER(gpu, IDENTITY);
}

#undef UPDATE_COUNTER

static int si_gpu_load_thread(void *param)
{
   struct si_screen *sscreen = (struct si_screen *)param;
   const int period_us = 1000000 / SAMPLES_PER_SEC;
   int sleep_us = period_us;
   int64_t cur_time, last_time = os_time_get();

   while (!p_atomic_read(&sscreen->gpu_load_stop_thread)) {
      if (sleep_us)
         os_time_sleep(sleep_us);

      /* Make sure we sleep the ideal amount of time to match
       * the expected frequency. */
      cur_time = os_time_get();

      if (os_time_timeout(last_time, last_time + period_us, cur_time))
         sleep_us = MAX2(sleep_us - 1, 1);
      else
         sleep_us += 1;

      /*printf("Hz: %.1f\n", 1000000.0 / (cur_time - last_time));*/
      last_time = cur_time;

      /* Update the counters. */
      si_update_mmio_counters(sscreen, &sscreen->mmio_counters);
   }
   p_atomic_dec(&sscreen->gpu_load_stop_thread);
   return 0;
}

void si_gpu_load_kill_thread(struct si_screen *sscreen)
{
   if (!sscreen->gpu_load_thread_created)
      return;

   p_atomic_inc(&sscreen->gpu_load_stop_thread);
   thrd_join(sscreen->gpu_load_thread, NULL);
   sscreen->gpu_load_thread_created = false;
}

static uint64_t si_read_mmio_counter(struct si_screen *sscreen, unsigned busy_index)
{
   /* Start the thread if needed. */
   if (!sscreen->gpu_load_thread_created) {
      simple_mtx_lock(&sscreen->gpu_load_mutex);
      /* Check again inside the mutex. */
      if (!sscreen->gpu_load_thread_created) {
         if (thrd_success == u_thread_create(&sscreen->gpu_load_thread, si_gpu_load_thread, sscreen)) {
            sscreen->gpu_load_thread_created = true;
         }
      }
      simple_mtx_unlock(&sscreen->gpu_load_mutex);
   }

   unsigned busy = p_atomic_read(&sscreen->mmio_counters.array[busy_index]);
   unsigned idle = p_atomic_read(&sscreen->mmio_counters.array[busy_index + 1]);

   return busy | ((uint64_t)idle << 32);
}

static unsigned si_end_mmio_counter(struct si_screen *sscreen, uint64_t begin, unsigned busy_index)
{
   uint64_t end = si_read_mmio_counter(sscreen, busy_index);
   unsigned busy = (end & 0xffffffff) - (begin & 0xffffffff);
   unsigned idle = (end >> 32) - (begin >> 32);

   /* Calculate the % of time the busy counter was being incremented.
    *
    * If no counters were incremented, return the current counter status.
    * It's for the case when the load is queried faster than
    * the counters are updated.
    */
   if (idle || busy) {
      return busy * 100 / (busy + idle);
   } else {
      union si_mmio_counters counters;

      memset(&counters, 0, sizeof(counters));
      si_update_mmio_counters(sscreen, &counters);
      return counters.array[busy_index] ? 100 : 0;
   }
}

#define BUSY_INDEX(sscreen, field)                                                                 \
   (&sscreen->mmio_counters.named.field.busy - sscreen->mmio_counters.array)

static unsigned busy_index_from_type(struct si_screen *sscreen, unsigned type)
{
   switch (type) {
   case SI_QUERY_GPU_LOAD:
      return BUSY_INDEX(sscreen, gpu);
   case SI_QUERY_GPU_SHADERS_BUSY:
      return BUSY_INDEX(sscreen, spi);
   case SI_QUERY_GPU_TA_BUSY:
      return BUSY_INDEX(sscreen, ta);
   case SI_QUERY_GPU_GDS_BUSY:
      return BUSY_INDEX(sscreen, gds);
   case SI_QUERY_GPU_VGT_BUSY:
      return BUSY_INDEX(sscreen, vgt);
   case SI_QUERY_GPU_IA_BUSY:
      return BUSY_INDEX(sscreen, ia);
   case SI_QUERY_GPU_SX_BUSY:
      return BUSY_INDEX(sscreen, sx);
   case SI_QUERY_GPU_WD_BUSY:
      return BUSY_INDEX(sscreen, wd);
   case SI_QUERY_GPU_BCI_BUSY:
      return BUSY_INDEX(sscreen, bci);
   case SI_QUERY_GPU_SC_BUSY:
      return BUSY_INDEX(sscreen, sc);
   case SI_QUERY_GPU_PA_BUSY:
      return BUSY_INDEX(sscreen, pa);
   case SI_QUERY_GPU_DB_BUSY:
      return BUSY_INDEX(sscreen, db);
   case SI_QUERY_GPU_CP_BUSY:
      return BUSY_INDEX(sscreen, cp);
   case SI_QUERY_GPU_CB_BUSY:
      return BUSY_INDEX(sscreen, cb);
   case SI_QUERY_GPU_SDMA_BUSY:
      return BUSY_INDEX(sscreen, sdma);
   case SI_QUERY_GPU_PFP_BUSY:
      return BUSY_INDEX(sscreen, pfp);
   case SI_QUERY_GPU_MEQ_BUSY:
      return BUSY_INDEX(sscreen, meq);
   case SI_QUERY_GPU_ME_BUSY:
      return BUSY_INDEX(sscreen, me);
   case SI_QUERY_GPU_SURF_SYNC_BUSY:
      return BUSY_INDEX(sscreen, surf_sync);
   case SI_QUERY_GPU_CP_DMA_BUSY:
      return BUSY_INDEX(sscreen, cp_dma);
   case SI_QUERY_GPU_SCRATCH_RAM_BUSY:
      return BUSY_INDEX(sscreen, scratch_ram);
   default:
      unreachable("invalid query type");
   }
}

uint64_t si_begin_counter(struct si_screen *sscreen, unsigned type)
{
   unsigned busy_index = busy_index_from_type(sscreen, type);
   return si_read_mmio_counter(sscreen, busy_index);
}

unsigned si_end_counter(struct si_screen *sscreen, unsigned type, uint64_t begin)
{
   unsigned busy_index = busy_index_from_type(sscreen, type);
   return si_end_mmio_counter(sscreen, begin, busy_index);
}
