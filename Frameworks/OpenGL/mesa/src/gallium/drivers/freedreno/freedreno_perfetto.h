/*
 * Copyright © 2021 Google, Inc.
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
 */

#ifndef FREEDRENO_PERFETTO_H_
#define FREEDRENO_PERFETTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_PERFETTO

/**
 * Render-stage id's
 */
enum fd_stage_id {
   SURFACE_STAGE_ID, /* Surface is a sort of meta-stage for render-target info */
   BINNING_STAGE_ID,
   GMEM_STAGE_ID,
   BYPASS_STAGE_ID,
   BLIT_STAGE_ID,
   COMPUTE_STAGE_ID,
   CLEAR_STAGE_ID,
   TILE_LOAD_STAGE_ID,
   TILE_STORE_STAGE_ID,
   STATE_RESTORE_STAGE_ID,
   VSC_OVERFLOW_STAGE_ID,
   PROLOGUE_STAGE_ID,

   NUM_STAGES
};

static const struct {
   const char *name;
   const char *desc;
} stages[] = {
   [SURFACE_STAGE_ID] = {"Surface"},
   [BINNING_STAGE_ID] = {"Binning", "Perform Visibility pass and determine target bins"},
   [GMEM_STAGE_ID]    = {"Render", "Rendering to GMEM"},
   [BYPASS_STAGE_ID]  = {"Render", "Rendering to system memory"},
   [BLIT_STAGE_ID]    = {"Blit", "Performing a Blit operation"},
   [COMPUTE_STAGE_ID] = {"Compute", "Compute job"},
   [CLEAR_STAGE_ID]   = {"Clear", "Clear (sysmem) or per-tile clear (GMEM)"},
   [TILE_LOAD_STAGE_ID]  = {"Tile Load", "Per tile load (system memory to GMEM)"},
   [TILE_STORE_STAGE_ID] = {"Tile Store", "Per tile store (GMEM to system memory)"},
   [STATE_RESTORE_STAGE_ID] = {"State Restore", "Setup at the beginning of new cmdstream buffer"},
   [VSC_OVERFLOW_STAGE_ID] = {"VSC Overflow Test", ""},
   [PROLOGUE_STAGE_ID] = {"Prologue", "Preemble cmdstream (executed once before first tile)"},
};

/**
 * Queue-id's
 */
enum {
   DEFAULT_HW_QUEUE_ID,
};

static const struct {
   const char *name;
   const char *desc;
} queues[] = {
   [DEFAULT_HW_QUEUE_ID] = {"GPU Queue 0", "Default Adreno Hardware Queue"},
};

/**
 * The u_trace tracepoints which are used to capture GPU timestamps and
 * trigger perfetto events tend to come in begin/end pairs (ie. start
 * and end of binning pass, etc), but perfetto wants one event for the
 * whole pass.  So we need to buffer up some state at the "begin" trae
 * callback, and then emit the perfetto event at the "end" event based
 * on previously recorded timestamp/data.  This struct is where we can
 * accumulate that state.
 */
struct fd_perfetto_state {
   uint64_t start_ts[NUM_STAGES];

   /*
    * Surface state for the renderpass:
    */
   uint32_t submit_id;
   enum pipe_format cbuf0_format : 16;
   enum pipe_format zs_format : 16;
   uint16_t width;
   uint16_t height;
   uint8_t mrts;
   uint8_t samples;
   uint16_t nbins;
   uint16_t binw;
   uint16_t binh;
   // TODO # of draws and possibly estimated cost might be useful addition..

   /*
    * Compute state for grids:
    */
   uint8_t indirect;
   uint8_t work_dim;
   uint16_t local_size_x;
   uint16_t local_size_y;
   uint16_t local_size_z;
   uint32_t num_groups_x;
   uint32_t num_groups_y;
   uint32_t num_groups_z;
   uint32_t shader_id;
};

void fd_perfetto_init(void);

struct fd_context;
void fd_perfetto_submit(struct fd_context *ctx);

#endif

#ifdef __cplusplus
}
#endif

#endif /* FREEDRENO_PERFETTO_H_ */
