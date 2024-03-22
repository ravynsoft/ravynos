#!/usr/bin/env python3

import re
import sys
import gzip
import io

# Captures per-frame state, including all the renderpasses, and
# time spent in blits and compute jobs:
class Frame:
    def __init__(self):
        self.frame_nr = None
        self.renderpasses = []
        # Times in ns:
        self.times_sysmem = []
        self.times_gmem = []
        self.times_compute = []
        self.times_blit = []

    def print(self):
        print("FRAME[{}]: {} blits ({:,} ns), {} SYSMEM ({:,} ns), {} GMEM ({:,} ns), {} COMPUTE ({:,} ns)".format(
                self.frame_nr,
                len(self.times_blit),    sum(self.times_blit),
                len(self.times_sysmem),  sum(self.times_sysmem),
                len(self.times_gmem),    sum(self.times_gmem),
                len(self.times_compute), sum(self.times_compute)
            ))

        i = 0
        prologue_time = 0
        binning_time = 0
        restore_clear_time = 0
        draw_time = 0
        resolve_time = 0
        elapsed_time = 0
        total_time = sum(self.times_blit) + sum(self.times_sysmem) + sum(self.times_gmem) + sum(self.times_compute)

        for renderpass in self.renderpasses:
            renderpass.print(i)
            prologue_time += renderpass.prologue_time
            binning_time += renderpass.binning_time
            restore_clear_time += renderpass.restore_clear_time
            draw_time += renderpass.draw_time
            resolve_time += renderpass.resolve_time
            elapsed_time += renderpass.elapsed_time
            i += 1

        print("  TOTAL: prologue: {:,} ns ({}%), binning: {:,} ns ({}%), restore/clear: {:,} ns ({}%), draw: {:,} ns ({}%), resolve: {:,} ns ({}%), blit: {:,} ns ({}%), compute: {:,} ns ({}%), GMEM: {:,} ns ({}%), sysmem: {:,} ns ({}%), total: {:,} ns\n".format(
                prologue_time, 100.0 * prologue_time / total_time,
                binning_time, 100.0 * binning_time / total_time,
                restore_clear_time, 100.0 * restore_clear_time / total_time,
                draw_time, 100.0 * draw_time / total_time,
                resolve_time, 100.0 * resolve_time / total_time,
                sum(self.times_blit), 100.0 * sum(self.times_blit) / total_time,
                sum(self.times_compute), 100.0 * sum(self.times_compute) / total_time,
                sum(self.times_gmem), 100.0 * sum(self.times_gmem) / total_time,
                sum(self.times_sysmem), 100.0 * sum(self.times_sysmem) / total_time,
                total_time
            ))

class FramebufferState:
    def __init__(self, width, height, layers, samples, nr_cbufs):
        self.width = width
        self.height = height
        self.layers = layers
        self.samples = samples
        self.nr_cbufs = nr_cbufs
        self.surfaces = []        # per MRT + zsbuf

    def get_formats(self):
        formats = []
        for surface in self.surfaces:
            formats.append(surface.format)
        return formats

class SurfaceState:
    def __init__(self, width, height, samples, format):
        self.width = width
        self.height = height
        self.samples = samples
        self.format = format

class BinningState:
    def __init__(self, nbins_x, nbins_y, bin_w, bin_h):
        self.nbins_x = nbins_x
        self.nbins_y = nbins_y
        self.bin_w = bin_w
        self.bin_h = bin_h

# Captures per-renderpass state, which can be either a binning or
# sysmem pass.  Blits and compute jobs are not tracked separately
# but have their time their times accounted for in the Frame state
class RenderPass:
    def __init__(self, cleared, gmem_reason, num_draws):
        self.cleared = cleared
        self.gmem_reason = gmem_reason
        self.num_draws = num_draws

        # The rest of the parameters aren't known until we see a later trace:
        self.binning_state = None   # None for sysmem passes, else BinningState
        self.fb = None
        self.fast_cleared = None

        self.elapsed_time = 0
        self.state_restore_time = 0
        self.prologue_time = 0
        self.draw_time = 0
        self.restore_clear_time = 0

        # Specific to GMEM passes:
        self.binning_time = 0
        self.vsc_overflow_test_time = 0
        self.resolve_time = 0

    def print_gmem_pass(self, nr):
        print("  GMEM[{}]: {}x{} ({}x{} tiles), {} draws, prologue: {:,} ns, binning: {:,} ns, restore/clear: {:,} ns, draw: {:,} ns, resolve: {:,} ns, total: {:,} ns, rt/zs: {}".format(
                nr, self.fb.width, self.fb.height,
                self.binning_state.nbins_x, self.binning_state.nbins_y,
                self.num_draws, self.prologue_time, self.binning_time,
                self.restore_clear_time, self.draw_time, self.resolve_time,
                self.elapsed_time,
                ", ".join(self.fb.get_formats())
            ))

    def print_sysmem_pass(self, nr):
        print("  SYSMEM[{}]: {}x{}, {} draws, prologue: {:,} ns, clear: {:,} ns, draw: {:,} ns, total: {:,} ns, rt/zs: {}".format(
                nr, self.fb.width, self.fb.height,
                self.num_draws, self.prologue_time,
                self.restore_clear_time, self.draw_time,
                self.elapsed_time,
                ", ".join(self.fb.get_formats())
            ))

    def print(self, nr):
        if self.binning_state:
            self.print_gmem_pass(nr)
        else:
            self.print_sysmem_pass(nr)

def main():
    filename = sys.argv[1]
    if filename.endswith(".gz"):
        file = gzip.open(filename, "r")
        file = io.TextIOWrapper(file)
    else:
        file = open(filename, "r")
    lines = file.read().split('\n')

    flush_batch_match = re.compile(r": flush_batch: (\S+): cleared=(\S+), gmem_reason=(\S+), num_draws=(\S+)")
    framebuffer_match = re.compile(r": framebuffer: (\S+)x(\S+)x(\S+)@(\S+), nr_cbufs: (\S+)")
    surface_match     = re.compile(r": surface: (\S+)x(\S+)@(\S+), fmt=(\S+)")

    # draw/renderpass passes:
    gmem_match          = re.compile(r": render_gmem: (\S+)x(\S+) bins of (\S+)x(\S+)")
    sysmem_match        = re.compile(r": render_sysmem")
    state_restore_match = re.compile(r"\+(\S+): end_state_restore")
    prologue_match      = re.compile(r"\+(\S+): end_prologue")
    binning_ib_match    = re.compile(r"\+(\S+): end_binning_ib")
    vsc_overflow_match  = re.compile(r"\+(\S+): end_vsc_overflow_test")
    draw_ib_match       = re.compile(r"\+(\S+): end_draw_ib")
    resolve_match       = re.compile(r"\+(\S+): end_resolve")
    
    start_clear_restore_match = re.compile(r"start_clear_restore: fast_cleared: (\S+)")
    end_clear_restore_match   = re.compile(r"\+(\S+): end_clear_restore")

    # Non-draw passes:
    compute_match = re.compile(r": start_compute")
    blit_match    = re.compile(r": start_blit")

    # End of pass/frame markers:
    elapsed_match = re.compile(r"ELAPSED: (\S+) ns")
    eof_match     = re.compile(r"END OF FRAME (\S+)")

    frame = Frame()      # current frame state
    renderpass = None    # current renderpass state
    times = None

    # Helper to set the appropriate times table for the current pass,
    # which is expected to only happen once for a given render pass
    def set_times(t):
        nonlocal times
        if times  is not None:
            print("expected times to not be set yet")
        times = t

    for line in lines:
        # Note, we only expect the flush_batch trace for !nondraw:
        match = re.search(flush_batch_match, line)
        if match is not None:
            assert(renderpass is None)
            renderpass = RenderPass(cleared=match.group(2),
                                    gmem_reason=match.group(3),
                                    num_draws=match.group(4))
            frame.renderpasses.append(renderpass)
            continue

        match = re.search(framebuffer_match, line)
        if match is not None:
            assert(renderpass.fb is None)
            renderpass.fb = FramebufferState(width=match.group(1),
                                             height=match.group(2),
                                             layers=match.group(3),
                                             samples=match.group(4),
                                             nr_cbufs=match.group(5))
            continue

        match = re.search(surface_match, line)
        if match is not None:
            surface = SurfaceState(width=match.group(1),
                                   height=match.group(2),
                                   samples=match.group(3),
                                   format=match.group(4))
            renderpass.fb.surfaces.append(surface)
            continue

        match = re.search(gmem_match, line)
        if match is not None:
            assert(renderpass.binning_state is None)
            renderpass.binning_state = BinningState(nbins_x=match.group(1),
                                                    nbins_y=match.group(2),
                                                    bin_w=match.group(3),
                                                    bin_h=match.group(4))
            set_times(frame.times_gmem)
            continue

        match = re.search(sysmem_match, line)
        if match is not None:
            assert(renderpass.binning_state is None)
            set_times(frame.times_sysmem)
            continue

        match = re.search(state_restore_match, line)
        if match is not None:
            renderpass.state_restore_time += int(match.group(1))
            continue

        match = re.search(prologue_match, line)
        if match is not None:
            renderpass.prologue_time += int(match.group(1))
            continue

        match = re.search(binning_ib_match, line)
        if match is not None:
            assert(renderpass.binning_state is not None)
            renderpass.binning_time += int(match.group(1))
            continue

        match = re.search(vsc_overflow_match, line)
        if match is not None:
            assert(renderpass.binning_state is not None)
            renderpass.vsc_overflow_test_time += int(match.group(1))
            continue

        match = re.search(draw_ib_match, line)
        if match is not None:
            renderpass.draw_time += int(match.group(1))
            continue

        match = re.search(resolve_match, line)
        if match is not None:
            assert(renderpass.binning_state is not None)
            renderpass.resolve_time += int(match.group(1))
            continue

        match = re.search(start_clear_restore_match, line)
        if match is not None:
            renderpass.fast_cleared = match.group(1)
            continue

        match = re.search(end_clear_restore_match, line)
        if match is not None:
            renderpass.restore_clear_time += int(match.group(1))
            continue

        match = re.search(compute_match, line)
        if match is not None:
            set_times(frame.times_compute)
            continue

        match = re.search(blit_match, line)
        if match is not None:
            set_times(frame.times_blit)
            continue

        match = re.search(eof_match, line)
        if match is not None:
            frame.frame_nr = int(match.group(1))
            frame.print()
            frame = Frame()
            times = None
            renderpass = None
            continue

        match = re.search(elapsed_match, line)
        if match is not None:
            time = int(match.group(1))
            #print("ELAPSED: " + str(time) + " ns")
            if renderpass is not None:
                renderpass.elapsed_time = time
            times.append(time)
            times = None
            renderpass = None
            continue


if __name__ == "__main__":
    main()

