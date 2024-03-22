/*
 * Copyright 2015 Advanced Micro Devices, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors: Marek Olšák <maraeo@gmail.com>
 *
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

#include "r600_pipe_common.h"
#include "r600_query.h"
#include "util/os_time.h"

/* For good accuracy at 1000 fps or lower. This will be inaccurate for higher
 * fps (there are too few samples per frame). */
#define SAMPLES_PER_SEC 10000

#define GRBM_STATUS		0x8010
#define TA_BUSY(x)		(((x) >> 14) & 0x1)
#define GDS_BUSY(x)		(((x) >> 15) & 0x1)
#define VGT_BUSY(x)		(((x) >> 17) & 0x1)
#define IA_BUSY(x)		(((x) >> 19) & 0x1)
#define SX_BUSY(x)		(((x) >> 20) & 0x1)
#define WD_BUSY(x)		(((x) >> 21) & 0x1)
#define SPI_BUSY(x)		(((x) >> 22) & 0x1)
#define BCI_BUSY(x)		(((x) >> 23) & 0x1)
#define SC_BUSY(x)		(((x) >> 24) & 0x1)
#define PA_BUSY(x)		(((x) >> 25) & 0x1)
#define DB_BUSY(x)		(((x) >> 26) & 0x1)
#define CP_BUSY(x)		(((x) >> 29) & 0x1)
#define CB_BUSY(x)		(((x) >> 30) & 0x1)
#define GUI_ACTIVE(x)		(((x) >> 31) & 0x1)

#define SRBM_STATUS2		0x0e4c
#define SDMA_BUSY(x)		(((x) >> 5) & 0x1)

#define CP_STAT                 0x8680
#define PFP_BUSY(x)		(((x) >> 15) & 0x1)
#define MEQ_BUSY(x)		(((x) >> 16) & 0x1)
#define ME_BUSY(x)		(((x) >> 17) & 0x1)
#define SURFACE_SYNC_BUSY(x)	(((x) >> 21) & 0x1)
#define DMA_BUSY(x)		(((x) >> 22) & 0x1)
#define SCRATCH_RAM_BUSY(x)	(((x) >> 24) & 0x1)

#define IDENTITY(x) x

#define UPDATE_COUNTER(field, mask)					\
	do {								\
		if (mask(value))					\
			p_atomic_inc(&counters->named.field.busy);	\
		else							\
			p_atomic_inc(&counters->named.field.idle);	\
	} while (0)

static void r600_update_mmio_counters(struct r600_common_screen *rscreen,
				      union r600_mmio_counters *counters)
{
	uint32_t value = 0;
	bool gui_busy, sdma_busy = false;

	/* GRBM_STATUS */
	rscreen->ws->read_registers(rscreen->ws, GRBM_STATUS, 1, &value);

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

	value = gui_busy || sdma_busy;
	UPDATE_COUNTER(gpu, IDENTITY);
}

#undef UPDATE_COUNTER

static int
r600_gpu_load_thread(void *param)
{
	struct r600_common_screen *rscreen = (struct r600_common_screen*)param;
	const int period_us = 1000000 / SAMPLES_PER_SEC;
	int sleep_us = period_us;
	int64_t cur_time, last_time = os_time_get();

	while (!p_atomic_read(&rscreen->gpu_load_stop_thread)) {
		if (sleep_us)
			os_time_sleep(sleep_us);

		/* Make sure we sleep the ideal amount of time to match
		 * the expected frequency. */
		cur_time = os_time_get();

		if (os_time_timeout(last_time, last_time + period_us,
				    cur_time))
			sleep_us = MAX2(sleep_us - 1, 1);
		else
			sleep_us += 1;

		/*printf("Hz: %.1f\n", 1000000.0 / (cur_time - last_time));*/
		last_time = cur_time;

		/* Update the counters. */
		r600_update_mmio_counters(rscreen, &rscreen->mmio_counters);
	}
	p_atomic_dec(&rscreen->gpu_load_stop_thread);
	return 0;
}

void r600_gpu_load_kill_thread(struct r600_common_screen *rscreen)
{
	if (!rscreen->gpu_load_thread_created)
		return;

	p_atomic_inc(&rscreen->gpu_load_stop_thread);
	thrd_join(rscreen->gpu_load_thread, NULL);
	rscreen->gpu_load_thread_created = false;
}

static uint64_t r600_read_mmio_counter(struct r600_common_screen *rscreen,
				       unsigned busy_index)
{
	/* Start the thread if needed. */
	if (!rscreen->gpu_load_thread_created) {
		mtx_lock(&rscreen->gpu_load_mutex);
		/* Check again inside the mutex. */
		if (!rscreen->gpu_load_thread_created) {
			int ret = u_thread_create(&rscreen->gpu_load_thread, r600_gpu_load_thread, rscreen);
			if (ret == thrd_success) {
				rscreen->gpu_load_thread_created = true;
			}
		}
		mtx_unlock(&rscreen->gpu_load_mutex);
	}

	unsigned busy = p_atomic_read(&rscreen->mmio_counters.array[busy_index]);
	unsigned idle = p_atomic_read(&rscreen->mmio_counters.array[busy_index + 1]);

	return busy | ((uint64_t)idle << 32);
}

static unsigned r600_end_mmio_counter(struct r600_common_screen *rscreen,
				      uint64_t begin, unsigned busy_index)
{
	uint64_t end = r600_read_mmio_counter(rscreen, busy_index);
	unsigned busy = (end & 0xffffffff) - (begin & 0xffffffff);
	unsigned idle = (end >> 32) - (begin >> 32);

	/* Calculate the % of time the busy counter was being incremented.
	 *
	 * If no counters were incremented, return the current counter status.
	 * It's for the case when the load is queried faster than
	 * the counters are updated.
	 */
	if (idle || busy) {
		return busy*100 / (busy + idle);
	} else {
		union r600_mmio_counters counters;

		memset(&counters, 0, sizeof(counters));
		r600_update_mmio_counters(rscreen, &counters);
		return counters.array[busy_index] ? 100 : 0;
	}
}

#define BUSY_INDEX(rscreen, field) (&rscreen->mmio_counters.named.field.busy - \
				    rscreen->mmio_counters.array)

static unsigned busy_index_from_type(struct r600_common_screen *rscreen,
				     unsigned type)
{
	switch (type) {
	case R600_QUERY_GPU_LOAD:
		return BUSY_INDEX(rscreen, gpu);
	case R600_QUERY_GPU_SHADERS_BUSY:
		return BUSY_INDEX(rscreen, spi);
	case R600_QUERY_GPU_TA_BUSY:
		return BUSY_INDEX(rscreen, ta);
	case R600_QUERY_GPU_GDS_BUSY:
		return BUSY_INDEX(rscreen, gds);
	case R600_QUERY_GPU_VGT_BUSY:
		return BUSY_INDEX(rscreen, vgt);
	case R600_QUERY_GPU_IA_BUSY:
		return BUSY_INDEX(rscreen, ia);
	case R600_QUERY_GPU_SX_BUSY:
		return BUSY_INDEX(rscreen, sx);
	case R600_QUERY_GPU_WD_BUSY:
		return BUSY_INDEX(rscreen, wd);
	case R600_QUERY_GPU_BCI_BUSY:
		return BUSY_INDEX(rscreen, bci);
	case R600_QUERY_GPU_SC_BUSY:
		return BUSY_INDEX(rscreen, sc);
	case R600_QUERY_GPU_PA_BUSY:
		return BUSY_INDEX(rscreen, pa);
	case R600_QUERY_GPU_DB_BUSY:
		return BUSY_INDEX(rscreen, db);
	case R600_QUERY_GPU_CP_BUSY:
		return BUSY_INDEX(rscreen, cp);
	case R600_QUERY_GPU_CB_BUSY:
		return BUSY_INDEX(rscreen, cb);
	case R600_QUERY_GPU_SDMA_BUSY:
		return BUSY_INDEX(rscreen, sdma);
	case R600_QUERY_GPU_PFP_BUSY:
		return BUSY_INDEX(rscreen, pfp);
	case R600_QUERY_GPU_MEQ_BUSY:
		return BUSY_INDEX(rscreen, meq);
	case R600_QUERY_GPU_ME_BUSY:
		return BUSY_INDEX(rscreen, me);
	case R600_QUERY_GPU_SURF_SYNC_BUSY:
		return BUSY_INDEX(rscreen, surf_sync);
	case R600_QUERY_GPU_CP_DMA_BUSY:
		return BUSY_INDEX(rscreen, cp_dma);
	case R600_QUERY_GPU_SCRATCH_RAM_BUSY:
		return BUSY_INDEX(rscreen, scratch_ram);
	default:
		unreachable("invalid query type");
	}
}

uint64_t r600_begin_counter(struct r600_common_screen *rscreen, unsigned type)
{
	unsigned busy_index = busy_index_from_type(rscreen, type);
	return r600_read_mmio_counter(rscreen, busy_index);
}

unsigned r600_end_counter(struct r600_common_screen *rscreen, unsigned type,
			  uint64_t begin)
{
	unsigned busy_index = busy_index_from_type(rscreen, type);
	return r600_end_mmio_counter(rscreen, begin, busy_index);
}
