/*
 * Copyright © 2023 Raspberry Pi Ltd
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

#ifndef V3D_PERFORMANCE_COUNTERS_H
#define V3D_PERFORMANCE_COUNTERS_H

#define V3D_PERFCNT_CATEGORY 0
#define V3D_PERFCNT_NAME 1
#define V3D_PERFCNT_DESCRIPTION 2

#ifndef V3D_VERSION
#  error "The V3D_VERSION macro must be defined"
#endif

#if (V3D_VERSION >= 71)

static const char *v3d_performance_counters[][3] = {
   {"CORE", "cycle-count", "[CORE] Cycle counter"},
   {"CORE", "core-active", "[CORE] Bin/Render/Compute active cycles"},
   {"CLE", "CLE-bin-thread-active-cycles", "[CLE] Bin thread active cycles"},
   {"CLE", "CLE-render-thread-active-cycles", "[CLE] Render thread active cycles"},
   {"CORE", "compute-active-cycles", "[CORE] Compute active cycles"},
   {"FEP", "FEP-valid-primitives-no-rendered-pixels", "[FEP] Valid primitives that result in no rendered pixels, for all rendered tiles"},
   {"FEP", "FEP-valid-primitives-rendered-pixels", "[FEP] Valid primitives for all rendered tiles (primitives may be counted in more than one tile)"},
   {"FEP", "FEP-clipped-quads", "[FEP] Early-Z/Near/Far clipped quads"},
   {"FEP", "FEP-valid-quads", "[FEP] Valid quads"},
   {"TLB", "TLB-quads-not-passing-stencil-test", "[TLB] Quads with no pixels passing the stencil test"},
   {"TLB", "TLB-quads-not-passing-z-and-stencil-test", "[TLB] Quads with no pixels passing the Z and stencil tests"},
   {"TLB", "TLB-quads-passing-z-and-stencil-test", "[TLB] Quads with any pixels passing the Z and stencil tests"},
   {"TLB", "TLB-quads-written-to-color-buffer", "[TLB] Quads with valid pixels written to colour buffer"},
   {"TLB", "TLB-partial-quads-written-to-color-buffer", "[TLB] Partial quads written to the colour buffer"},
   {"PTB", "PTB-primitives-need-clipping", "[PTB] Primitives that need clipping"},
   {"PTB", "PTB-primitives-discarded-outside-viewport", "[PTB] Primitives discarded by being outside the viewport"},
   {"PTB", "PTB-primitives-binned", "[PTB] Total primitives binned"},
   {"PTB", "PTB-primitives-discarded-reversed", "[PTB] Primitives that are discarded because they are reversed"},
   {"QPU", "QPU-total-instr-cache-hit", "[QPU] Total instruction cache hits for all slices"},
   {"QPU", "QPU-total-instr-cache-miss", "[QPU] Total instruction cache misses for all slices"},
   {"QPU", "QPU-total-uniform-cache-hit", "[QPU] Total uniforms cache hits for all slices"},
   {"QPU", "QPU-total-uniform-cache-miss", "[QPU] Total uniforms cache misses for all slices"},
   {"TMU", "TMU-active-cycles", "[TMU] Active cycles"},
   {"TMU", "TMU-stalled-cycles", "[TMU] Stalled cycles"},
   {"TMU", "TMU-total-text-quads-access", "[TMU] Total texture cache accesses"},
   {"TMU", "TMU-cache-x4-active-cycles", "[TMU] Cache active cycles for x4 access"},
   {"TMU", "TMU-cache-x4-stalled-cycles", "[TMU] Cache stalled cycles for x4 access"},
   {"TMU", "TMU-total-text-quads-x4-access", "[TMU] Total texture cache x4 access"},
   {"L2T", "L2T-total-cache-hit", "[L2T] Total Level 2 cache hits"},
   {"L2T", "L2T-total-cache-miss", "[L2T] Total Level 2 cache misses"},
   {"L2T", "L2T-local", "[L2T] Local mode access"},
   {"L2T", "L2T-writeback", "[L2T] Writeback"},
   {"L2T", "L2T-zero", "[L2T] Zero"},
   {"L2T", "L2T-merge", "[L2T] Merge"},
   {"L2T", "L2T-fill", "[L2T] Fill"},
   {"L2T", "L2T-stalls-no-wid", "[L2T] Stalls because no WID available"},
   {"L2T", "L2T-stalls-no-rid", "[L2T] Stalls because no RID available"},
   {"L2T", "L2T-stalls-queue-full", "[L2T] Stalls because internal queue full"},
   {"L2T", "L2T-stalls-wrightback", "[L2T] Stalls because writeback in flight"},
   {"L2T", "L2T-stalls-mem", "[L2T] Stalls because AXI blocks read"},
   {"L2T", "L2T-stalls-fill", "[L2T] Stalls because fill pending for victim cache-line"},
   {"L2T", "L2T-hitq", "[L2T] Sent request via hit queue"},
   {"L2T", "L2T-hitq-full", "[L2T] Sent request via main queue because hit queue is full"},
   {"L2T", "L2T-stalls-read-data", "[L2T] Stalls because waiting for data from SDRAM"},
   {"L2T", "L2T-TMU-read-hits", "[L2T] TMU read hits"},
   {"L2T", "L2T-TMU-read-miss", "[L2T] TMU read misses"},
   {"L2T", "L2T-VCD-read-hits", "[L2T] VCD read hits"},
   {"L2T", "L2T-VCD-read-miss", "[L2T] VCD read misses"},
   {"L2T", "L2T-SLC-read-hits", "[L2T] SLC read hits (all slices)"},
   {"L2T", "L2T-SLC-read-miss", "[L2T] SLC read misses (all slices)"},
   {"AXI", "AXI-writes-seen-watch-0", "[AXI] Writes seen by watch 0"},
   {"AXI", "AXI-reads-seen-watch-0", "[AXI] Reads seen by watch 0"},
   {"AXI", "AXI-writes-stalled-seen-watch-0", "[AXI] Write stalls seen by watch 0"},
   {"AXI", "AXI-reads-stalled-seen-watch-0", "[AXI] Read stalls seen by watch 0"},
   {"AXI", "AXI-write-bytes-seen-watch-0", "[AXI] Total bytes written seen by watch 0"},
   {"AXI", "AXI-read-bytes-seen-watch-0", "[AXI] Total bytes read seen by watch 0"},
   {"AXI", "AXI-writes-seen-watch-1", "[AXI] Writes seen by watch 1"},
   {"AXI", "AXI-reads-seen-watch-1", "[AXI] Reads seen by watch 1"},
   {"AXI", "AXI-writes-stalled-seen-watch-1", "[AXI] Write stalls seen by watch 1"},
   {"AXI", "AXI-reads-stalled-seen-watch-1", "[AXI] Read stalls seen by watch 1"},
   {"AXI", "AXI-write-bytes-seen-watch-1", "[AXI] Total bytes written seen by watch 1"},
   {"AXI", "AXI-read-bytes-seen-watch-1", "[AXI] Total bytes read seen by watch 1"},
   {"CORE", "core-memory-writes", "[CORE] Total memory writes"},
   {"L2T", "L2T-memory-writes", "[L2T] Total memory writes"},
   {"PTB", "PTB-memory-writes", "[PTB] Total memory writes"},
   {"TLB", "TLB-memory-writes", "[TLB] Total memory writes"},
   {"CORE", "core-memory-reads", "[CORE] Total memory reads"},
   {"L2T", "L2T-memory-reads", "[L2T] Total memory reads"},
   {"PTB", "PTB-memory-reads", "[PTB] Total memory reads"},
   {"PSE", "PSE-memory-reads", "[PSE] Total memory reads"},
   {"TLB", "TLB-memory-reads", "[TLB] Total memory reads"},
   {"PTB", "PTB-memory-words-writes", "[PTB] Total memory words written"},
   {"TLB", "TLB-memory-words-writes", "[TLB] Total memory words written"},
   {"PSE", "PSE-memory-words-reads", "[PSE] Total memory words read"},
   {"TLB", "TLB-memory-words-reads", "[TLB] Total memory words read"},
   {"AXI", "AXI-read-trans", "[AXI] Read transaction count"},
   {"AXI", "AXI-write-trans", "[AXI] Write transaction count"},
   {"AXI", "AXI-read-wait-cycles", "[AXI] Read total wait cycles"},
   {"AXI", "AXI-write-wait-cycles", "[AXI] Write total wait cycles"},
   {"AXI", "AXI-max-outstanding-reads", "[AXI] Maximium outstanding read transactions"},
   {"AXI", "AXI-max-outstanding-writes", "[AXI] Maximum outstanding write transactions"},
   {"QPU", "QPU-wait-bubble", "[QPU] Pipeline bubble in qcycles due all threads waiting"},
   {"QPU", "QPU-ic-miss-bubble", "[QPU] Pipeline bubble in qcycles due instruction-cache miss"},
   {"QPU", "QPU-active", "[QPU] Executed shader instruction"},
   {"QPU", "QPU-total-active-clk-cycles-fragment-shading", "[QPU] Total active clock cycles for all QPUs doing fragment shading (counts only when QPU is not stalled)"},
   {"QPU", "QPU-stalls", "[QPU] Stalled qcycles executing shader instruction"},
   {"QPU", "QPU-total-clk-cycles-waiting-fragment-shading", "[QPU] Total stalled clock cycles for all QPUs doing fragment shading"},
   {"QPU", "QPU-stalls-TMU", "[QPU] Stalled qcycles waiting for TMU"},
   {"QPU", "QPU-stalls-TLB", "[QPU] Stalled qcycles waiting for TLB"},
   {"QPU", "QPU-stalls-VPM", "[QPU] Stalled qcycles waiting for VPM"},
   {"QPU", "QPU-stalls-uniforms", "[QPU] Stalled qcycles waiting for uniforms"},
   {"QPU", "QPU-stalls-SFU", "[QPU] Stalled qcycles waiting for SFU"},
   {"QPU", "QPU-stalls-other", "[QPU] Stalled qcycles waiting for any other reason (vary/W/Z)"},
};

#elif (V3D_VERSION >= 42)

static const char *v3d_performance_counters[][3] = {
   {"FEP", "FEP-valid-primitives-no-rendered-pixels", "[FEP] Valid primitives that result in no rendered pixels, for all rendered tiles"},
   {"FEP", "FEP-valid-primitives-rendered-pixels", "[FEP] Valid primitives for all rendered tiles (primitives may be counted in more than one tile)"},
   {"FEP", "FEP-clipped-quads", "[FEP] Early-Z/Near/Far clipped quads"},
   {"FEP", "FEP-valid-quads", "[FEP] Valid quads"},
   {"TLB", "TLB-quads-not-passing-stencil-test", "[TLB] Quads with no pixels passing the stencil test"},
   {"TLB", "TLB-quads-not-passing-z-and-stencil-test", "[TLB] Quads with no pixels passing the Z and stencil tests"},
   {"TLB", "TLB-quads-passing-z-and-stencil-test", "[TLB] Quads with any pixels passing the Z and stencil tests"},
   {"TLB", "TLB-quads-with-zero-coverage", "[TLB] Quads with all pixels having zero coverage"},
   {"TLB", "TLB-quads-with-non-zero-coverage", "[TLB] Quads with any pixels having non-zero coverage"},
   {"TLB", "TLB-quads-written-to-color-buffer", "[TLB] Quads with valid pixels written to colour buffer"},
   {"PTB", "PTB-primitives-discarded-outside-viewport", "[PTB] Primitives discarded by being outside the viewport"},
   {"PTB", "PTB-primitives-need-clipping", "[PTB] Primitives that need clipping"},
   {"PTB", "PTB-primitives-discarded-reversed", "[PTB] Primitives that are discarded because they are reversed"},
   {"QPU", "QPU-total-idle-clk-cycles", "[QPU] Total idle clock cycles for all QPUs"},
   {"QPU", "QPU-total-active-clk-cycles-vertex-coord-shading", "[QPU] Total active clock cycles for all QPUs doing vertex/coordinate/user shading (counts only when QPU is not stalled)"},
   {"QPU", "QPU-total-active-clk-cycles-fragment-shading", "[QPU] Total active clock cycles for all QPUs doing fragment shading (counts only when QPU is not stalled)"},
   {"QPU", "QPU-total-clk-cycles-executing-valid-instr", "[QPU] Total clock cycles for all QPUs executing valid instructions"},
   {"QPU", "QPU-total-clk-cycles-waiting-TMU", "[QPU] Total clock cycles for all QPUs stalled waiting for TMUs only (counter won't increment if QPU also stalling for another reason)"},
   {"QPU", "QPU-total-clk-cycles-waiting-scoreboard", "[QPU] Total clock cycles for all QPUs stalled waiting for Scoreboard only (counter won't increment if QPU also stalling for another reason)"},
   {"QPU", "QPU-total-clk-cycles-waiting-varyings", "[QPU] Total clock cycles for all QPUs stalled waiting for Varyings only (counter won't increment if QPU also stalling for another reason)"},
   {"QPU", "QPU-total-instr-cache-hit", "[QPU] Total instruction cache hits for all slices"},
   {"QPU", "QPU-total-instr-cache-miss", "[QPU] Total instruction cache misses for all slices"},
   {"QPU", "QPU-total-uniform-cache-hit", "[QPU] Total uniforms cache hits for all slices"},
   {"QPU", "QPU-total-uniform-cache-miss", "[QPU] Total uniforms cache misses for all slices"},
   {"TMU", "TMU-total-text-quads-access", "[TMU] Total texture cache accesses"},
   {"TMU", "TMU-total-text-cache-miss", "[TMU] Total texture cache misses (number of fetches from memory/L2cache)"},
   {"VPM", "VPM-total-clk-cycles-VDW-stalled", "[VPM] Total clock cycles VDW is stalled waiting for VPM access"},
   {"VPM", "VPM-total-clk-cycles-VCD-stalled", "[VPM] Total clock cycles VCD is stalled waiting for VPM access"},
   {"CLE", "CLE-bin-thread-active-cycles", "[CLE] Bin thread active cycles"},
   {"CLE", "CLE-render-thread-active-cycles", "[CLE] Render thread active cycles"},
   {"L2T", "L2T-total-cache-hit", "[L2T] Total Level 2 cache hits"},
   {"L2T", "L2T-total-cache-miss", "[L2T] Total Level 2 cache misses"},
   {"CORE", "cycle-count", "[CORE] Cycle counter"},
   {"QPU", "QPU-total-clk-cycles-waiting-vertex-coord-shading", "[QPU] Total stalled clock cycles for all QPUs doing vertex/coordinate/user shading"},
   {"QPU", "QPU-total-clk-cycles-waiting-fragment-shading", "[QPU] Total stalled clock cycles for all QPUs doing fragment shading"},
   {"PTB", "PTB-primitives-binned", "[PTB] Total primitives binned"},
   {"AXI", "AXI-writes-seen-watch-0", "[AXI] Writes seen by watch 0"},
   {"AXI", "AXI-reads-seen-watch-0", "[AXI] Reads seen by watch 0"},
   {"AXI", "AXI-writes-stalled-seen-watch-0", "[AXI] Write stalls seen by watch 0"},
   {"AXI", "AXI-reads-stalled-seen-watch-0", "[AXI] Read stalls seen by watch 0"},
   {"AXI", "AXI-write-bytes-seen-watch-0", "[AXI] Total bytes written seen by watch 0"},
   {"AXI", "AXI-read-bytes-seen-watch-0", "[AXI] Total bytes read seen by watch 0"},
   {"AXI", "AXI-writes-seen-watch-1", "[AXI] Writes seen by watch 1"},
   {"AXI", "AXI-reads-seen-watch-1", "[AXI] Reads seen by watch 1"},
   {"AXI", "AXI-writes-stalled-seen-watch-1", "[AXI] Write stalls seen by watch 1"},
   {"AXI", "AXI-reads-stalled-seen-watch-1", "[AXI] Read stalls seen by watch 1"},
   {"AXI", "AXI-write-bytes-seen-watch-1", "[AXI] Total bytes written seen by watch 1"},
   {"AXI", "AXI-read-bytes-seen-watch-1", "[AXI] Total bytes read seen by watch 1"},
   {"TLB", "TLB-partial-quads-written-to-color-buffer", "[TLB] Partial quads written to the colour buffer"},
   {"TMU", "TMU-total-config-access", "[TMU] Total config accesses"},
   {"L2T", "L2T-no-id-stalled", "[L2T] No ID stall"},
   {"L2T", "L2T-command-queue-stalled", "[L2T] Command queue full stall"},
   {"L2T", "L2T-TMU-writes", "[L2T] TMU write accesses"},
   {"TMU", "TMU-active-cycles", "[TMU] Active cycles"},
   {"TMU", "TMU-stalled-cycles", "[TMU] Stalled cycles"},
   {"CLE", "CLE-thread-active-cycles", "[CLE] Bin or render thread active cycles"},
   {"L2T", "L2T-TMU-reads", "[L2T] TMU read accesses"},
   {"L2T", "L2T-CLE-reads", "[L2T] CLE read accesses"},
   {"L2T", "L2T-VCD-reads", "[L2T] VCD read accesses"},
   {"L2T", "L2T-TMU-config-reads", "[L2T] TMU CFG read accesses"},
   {"L2T", "L2T-SLC0-reads", "[L2T] SLC0 read accesses"},
   {"L2T", "L2T-SLC1-reads", "[L2T] SLC1 read accesses"},
   {"L2T", "L2T-SLC2-reads", "[L2T] SLC2 read accesses"},
   {"L2T", "L2T-TMU-write-miss", "[L2T] TMU write misses"},
   {"L2T", "L2T-TMU-read-miss", "[L2T] TMU read misses"},
   {"L2T", "L2T-CLE-read-miss", "[L2T] CLE read misses"},
   {"L2T", "L2T-VCD-read-miss", "[L2T] VCD read misses"},
   {"L2T", "L2T-TMU-config-read-miss", "[L2T] TMU CFG read misses"},
   {"L2T", "L2T-SLC0-read-miss", "[L2T] SLC0 read misses"},
   {"L2T", "L2T-SLC1-read-miss", "[L2T] SLC1 read misses"},
   {"L2T", "L2T-SLC2-read-miss", "[L2T] SLC2 read misses"},
   {"CORE", "core-memory-writes", "[CORE] Total memory writes"},
   {"L2T", "L2T-memory-writes", "[L2T] Total memory writes"},
   {"PTB", "PTB-memory-writes", "[PTB] Total memory writes"},
   {"TLB", "TLB-memory-writes", "[TLB] Total memory writes"},
   {"CORE", "core-memory-reads", "[CORE] Total memory reads"},
   {"L2T", "L2T-memory-reads", "[L2T] Total memory reads"},
   {"PTB", "PTB-memory-reads", "[PTB] Total memory reads"},
   {"PSE", "PSE-memory-reads", "[PSE] Total memory reads"},
   {"TLB", "TLB-memory-reads", "[TLB] Total memory reads"},
   {"GMP", "GMP-memory-reads", "[GMP] Total memory reads"},
   {"PTB", "PTB-memory-words-writes", "[PTB] Total memory words written"},
   {"TLB", "TLB-memory-words-writes", "[TLB] Total memory words written"},
   {"PSE", "PSE-memory-words-reads", "[PSE] Total memory words read"},
   {"TLB", "TLB-memory-words-reads", "[TLB] Total memory words read"},
   {"TMU", "TMU-MRU-hits", "[TMU] Total MRU hits"},
   {"CORE", "compute-active-cycles", "[CORE] Compute active cycles"},
};

#else
static const char *v3d_performance_counters[][3] = { };
#endif

#endif
