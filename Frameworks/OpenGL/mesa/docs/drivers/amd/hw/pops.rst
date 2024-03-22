Primitive Ordered Pixel Shading
===============================

Primitive Ordered Pixel Shading (POPS) is the feature available starting from
GFX9 that provides the Fragment Shader Interlock or Fragment Shader Ordering
functionality.

It allows a part of a fragment shader — an ordered section (or a critical
section) — to be executed sequentially in rasterization order for different
invocations covering the same pixel position.

This article describes how POPS is set up in shader code and the registers. The
information here is currently provided for architecture generations up to GFX11.

Note that the information in this article is **not official** and may contain
inaccuracies, as well as incomplete or incorrect assumptions. It is based on the
shader code output of the Radeon GPU Analyzer for Rasterizer Ordered View usage
in Direct3D shaders, AMD's Platform Abstraction Library (PAL), ISA references,
and experimentation with the hardware.

Shader code
-----------

With POPS, a wave can dynamically execute up to one ordered section. It is fine
for a wave not to enter an ordered section at all if it doesn't need ordering on
its execution path, however.

The setup of the ordered section consists of three parts:

1. Entering the ordered section in the current wave — awaiting the completion of
   ordered sections in overlapped waves.
2. Resolving overlap within the current wave — intrawave collisions (optional
   and GFX9–10.3 only).
3. Exiting the ordered section — resuming overlapping waves trying to enter
   their ordered sections.

GFX9–10.3: Entering the ordered section in the wave
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Awaiting the completion of ordered sections in overlapped waves is performed by
setting the POPS packer hardware register, and then polling the volatile
``pops_exiting_wave_id`` ALU operand source until its value exceeds the newest
overlapped wave ID for the current wave.

The information needed for the wave to perform the waiting is provided to it via
the SGPR argument ``COLLISION_WAVEID``. Its loading needs to be enabled in the
``SPI_SHADER_PGM_RSRC2_PS`` and ``PA_SC_SHADER_CONTROL`` registers (note that
the POPS arguments specifically need to be enabled not only in ``RSRC`` unlike
various other arguments, but in ``PA_SC_SHADER_CONTROL`` as well).

The collision wave ID argument contains the following unsigned values:

* [31]: Whether overlap has occurred.
* [29:28] (GFX10+) / [28] (GFX9): ID of the packer the wave should be associated
  with.
* [25:16]: Newest overlapped wave ID.
* [9:0]: Current wave ID.

The 2020 RDNA and RDNA 2 ISA references contain incorrect offsets and widths of
the fields, possibly from an early development iteration, but the meanings of
them are accurate there.

The wait must not be performed if the "did overlap" bit 31 is set to 0,
otherwise it will result in a hang. Also, the bit being set to 0 indicates that
there are *both* no wave overlap *and no intrawave collisions* for the current
wave — so if the bit is 0, it's safe for the wave to skip all of the POPS logic
completely and execute the contents of the ordered section simply as usual with
unordered access as a potential additional optimization. The packer hardware
register, however, may be set even without overlap safely — it's the wait loop
itself that must not be executed if it was reported that there was no overlap.

The packer ID needs to be passed to the packer hardware register using
``s_setreg_b32`` so the wave can poll ``pops_exiting_wave_id`` on that packer.

On GFX9, the ``MODE`` (1) hardware register has two bits specifying which packer
the wave is associated with:

* [25]: The wave is associated with packer 1.
* [24]: The wave is associated with packer 0.

Initially, both of these bits are set 0, meaning that POPS is disabled for the
wave. If the wave needs to enter the ordered section, it must set bit 24 to 1 if
the packer ID in ``COLLISION_WAVEID`` is 0, or set bit 25 to 1 if the packer ID
is 1.

Starting from GFX10, the ``POPS_PACKER`` (25) hardware register is used instead,
containing the following fields:

* [2:1]: Packer ID.
* [0]: POPS enabled for the wave.

Initially, POPS is disabled for a wave. To start entering the ordered section,
bits 2:1 must be set to the packer ID from ``COLLISION_WAVEID``, and bit 0 needs
to be set to 1.

The wave IDs, both in ``COLLISION_WAVEID`` and ``pops_exiting_wave_id``, are
10-bit values wrapping around on overflow — consecutive waves are numbered 1022,
1023, 0, 1… This wraparound needs to be taken into account when comparing the
exiting wave ID and the newest overlapped wave ID.

Specifically, until the current wave exits the ordered section, its ID can't be
smaller than the newest overlapped wave ID or the exiting wave ID. So
``current_wave_id + 1`` can be subtracted from 10-bit wave IDs to remap them to
monotonically increasing unsigned values. In this case, the largest value,
0xFFFFFFFF, will correspond to the current wave, 10-bit values up to the current
wave ID will be in a range near 0xFFFFFFFF growing towards it, and wave IDs from
before the last wraparound will be near 0 increasing away from it. Subtracting
``current_wave_id + 1`` is equivalent to adding ``~current_wave_id``.

GFX9 has an off-by-one error in the newest overlapped wave ID: if the 10-bit
newest overlapped wave ID is greater than the 10-bit current wave ID (meaning
that it's behind the last wraparound point), 1 needs to be added to the newest
overlapped wave ID before using it in the comparison. This was corrected in
GFX10.

The exiting wave ID (not to be confused with "exited" — the exiting wave ID is
the wave that will exit the ordered section next) is queried via the
``pops_exiting_wave_id`` ALU operand source, numbered 239. Normally, it will be
one of the arguments of ``s_add_i32`` that remaps it from a wrapping 10-bit wave
ID to monotonically increasing one.

It's a volatile operand, and it needs to be read in a loop until its value
becomes greater than the newest overlapped wave ID (after remapping both to
monotonic). However, if it's too early for the current wave to enter the ordered
section, it needs to yield execution to other waves that may potentially be
overlapped — via ``s_sleep``. GFX9 requires a finite amount of delay to be
specified, AMD uses 3. Starting from GFX10, exiting the ordered section wakes up
the waiting waves, so the maximum delay of 0xFFFF can be used.

In pseudocode, the entering logic would look like this::

   bool did_overlap = collision_wave_id[31];
   if (did_overlap) {
      if (gfx_level >= GFX10) {
         uint packer_id = collision_wave_id[29:28];
         s_setreg_b32(HW_REG_POPS_PACKER[2:0], 1 | (packer_id << 1));
      } else {
         uint packer_id = collision_wave_id[28];
         s_setreg_b32(HW_REG_MODE[25:24], packer_id ? 0b10 : 0b01);
      }

      uint current_10bit_wave_id = collision_wave_id[9:0];
      // Or -(current_10bit_wave_id + 1).
      uint wave_id_remap_offset = ~current_10bit_wave_id;

      uint newest_overlapped_10bit_wave_id = collision_wave_id[25:16];
      if (gfx_level < GFX10 &&
          newest_overlapped_10bit_wave_id > current_10bit_wave_id) {
         ++newest_overlapped_10bit_wave_id;
      }
      uint newest_overlapped_wave_id =
         newest_overlapped_10bit_wave_id + wave_id_remap_offset;

      while (!(src_pops_exiting_wave_id + wave_id_remap_offset >
               newest_overlapped_wave_id)) {
         s_sleep(gfx_level >= GFX10 ? 0xFFFF : 3);
      }
   }

The SPIR-V fragment shader interlock specification requires an invocation — an
individual invocation, not the whole subgroup — to execute
``OpBeginInvocationInterlockEXT`` exactly once. However, if there are multiple
begin instructions, or even multiple begin/end pairs, under divergent
conditions, a wave may end up waiting for the overlapped waves multiple times.
Thankfully, it's safe to set the POPS packer hardware register to the same
value, or to run the wait loop, multiple times during the wave's execution, as
long as the ordered section isn't exited in between by the wave.

GFX11: Entering the ordered section in the wave
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Instead of exposing wave IDs to shaders, GFX11 uses the "export ready" wave
status flag to report that the wave may enter the ordered section. It's awaited
by the ``s_wait_event`` instruction, with the bit 0 ("don't wait for
``export_ready``") of the immediate operand set to 0. On GFX11 specifically, AMD
passes 0 as the whole immediate operand.

The "export ready" wait can be done multiple times safely.

GFX9–10.3: Resolving intrawave collisions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On GFX9–10.3, it's possible for overlapping fragment shader invocations to be
placed not only in different waves, but also in the same wave, with the shader
code making sure that the ordered section is executed for overlapping
invocations in order.

This functionality is optional — it can be activated by enabling loading of the
``INTRAWAVE_COLLISION`` SGPR argument in ``SPI_SHADER_PGM_RSRC2_PS`` and
``PA_SC_SHADER_CONTROL``.

The lower 8 or 16 (depending on the wave size) bits of ``INTRAWAVE_COLLISION``
contain the mask of whether each quad in the wave starts a new layer of
overlapping invocations, and thus the ordered section code for them needs to be
executed after running it for all lanes with indices preceding that quad index
multiplied by 4. The rest of the bits in the argument need to be ignored — AMD
explicitly masks them out in shader code (although this is not necessary if the
shader uses "find first 1" to obtain the start of the next set of overlapping
quads or expands this quad mask into a lane mask).

For example, if the intrawave collision mask is 0b0000001110000100, or
``(1 << 2) | (1 << 7) | (1 << 8) | (1 << 9)``, the code of the ordered section
needs to be executed first only for quads 1:0 (lanes 7:0), then only for quads
6:2 (lanes 27:8), then for quad 7 (lanes 31:28), then for quad 8 (lanes 35:32),
and then for the remaining quads 15:9 (lanes 63:36).

This effectively causes the ordered section to be executed as smaller
"sub-subgroups" within the original subgroup.

However, this is not always compatible with the execution model of SPIR-V or
GLSL fragment shaders, so enabling intrawave collisions and wrapping a part of
the shader in a loop may be unsafe in some cases. One particular example is when
the shader uses subgroup operations influenced by lanes outside the current
quad. In this case, the code outside and inside the ordered section may be
executed with different sets of active invocations, affecting the results of
subgroup operations. But in SPIR-V and GLSL, fragment shader interlock is not
supposed to modify the set of active invocations in any way. So the intrawave
collision loop may break the results of subgroup operations in unpredictable
ways, even outside the driver's compiler infrastructure. Even if the driver
splits the subgroup exactly at ``OpBeginInvocationInterlockEXT`` and makes the
lane subsets rejoin exactly at ``OpEndInvocationInterlockEXT``, the application
and the compilers that created the source shader are still not aware of that
happening — the input SPIR-V or GLSL shader might have already gone through
various optimizations, such as common subexpression elimination which might
have considered a subgroup operation before ``OpBeginInvocationInterlockEXT``
and one after it equivalent.

The idea behind reporting intrawave collisions to shaders is to reduce the
impact on the parallelism of the part of the shader that doesn't depend on the
ordering, to avoid wasting lanes in the wave and to allow the code outside the
ordered section in different invocations to run in parallel lanes as usual. This
may be especially helpful if the ordered section is small compared to the rest
of the shader — for instance, a custom blending equation in the end of the usual
fragment shader for a surface in the world.

However, whether handling intrawave collisions is preferred is not a question
with one universal answer. Intrawave collisions are pretty uncommon without
multisampling, or when using sample interlock with multisampling, although
they're highly frequent with pixel interlock with multisampling, when adjacent
primitives cover the same pixels along the shared edge (though that's an
extremely expensive situation in general). But resolving intrawave collisions
adds some overhead costs to the shader. If intrawave overlap is unlikely to
happen often, or even more importantly, if the majority of the shader is inside
the ordered section, handling it in the shader may cause more harm than good.

GFX11 removes this concept entirely, instead overlapping invocations are always
placed in different waves.

GFX9–10.3: Exiting the ordered section in the wave
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To exit the ordered section and let overlapping waves resume execution and enter
their ordered sections, the wave needs to send the ``ORDERED_PS_DONE`` message
(7) using ``s_sendmsg``.

If the wave has enabled POPS by setting the packer hardware register, it *must
not* execute ``s_endpgm`` without having sent ``ORDERED_PS_DONE`` once, so the
message must be sent on all execution paths after the packer register setup.
However, if the wave exits before having configured the packer register, sending
the message is not required, though it's still fine to send it regardless of
that.

Note that if the shader has multiple ``OpEndInvocationInterlockEXT``
instructions executed in the same wave (depending on a divergent condition, for
example), it must still be ensured that ``ORDERED_PS_DONE`` is sent by the wave
only once, and especially not before any awaiting of overlapped waves.

Before the message is sent, all counters for memory accesses that need to be
primitive-ordered, both writes and (in case something after the ordered section
depends on the per-pixel data, for instance, the tail blending fallback in
order-independent transparency) reads, must be awaited. Those may include
``vm``, ``vs``, and in some cases ``lgkm`` (though normally primitive-ordered
memory accesses will be done through VMEM with divergent addresses, not SMEM, as
there's no synchronization between fragments at different pixel coordinates, but
it's still technically possible for a shader, even though pointless and
nonoptimal, to explicitly perform them in a waterfall loop, for instance, and
that must work correctly too). Without that, a race condition will occur when
the newly resumed waves start accessing the memory locations to which there
still are outstanding accesses in the current wave.

Another option for exiting is the ``s_endpgm_ordered_ps_done`` instruction,
which combines waiting for all the counters, sending the ``ORDERED_PS_DONE``
message, and ending the program. Generally, however, it's desirable to resume
overlapping waves as early as possible, including before the export, as it may
stall the wave for some time too.

GFX11: Exiting the ordered section in the wave
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The overlapping waves are resumed when the wave performs the last export (with
the ``done`` flag).

The same requirements for awaiting the memory access counters as on GFX9–10.3
still apply.

Memory access requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^

The compiler needs to ensure that entering the ordered section implements
acquire semantics, and exiting it implements release semantics, in the fragment
interlock memory scope for ``UniformMemory`` and ``ImageMemory`` SPIR-V storage
classes.

A fragment interlock memory scope instance includes overlapping fragment shader
invocations executed by commands inside a single subpass. It may be considered a
subset of a queue family memory scope instance from the perspective of memory
barriers.

Fragment shader interlock doesn't perform implicit memory availability or
visibility operations. Shaders must do them by themselves for accesses requiring
primitive ordering, such as via ``coherent`` (``queuefamilycoherent``) in GLSL
or ``MakeAvailable`` and ``MakeVisible`` in at least the ``QueueFamily`` scope
in SPIR-V.

On AMD hardware, this means that the accessed memory locations must be made
available or visible between waves that may be executed on any compute unit — so
accesses must go directly to the global L2 cache, bypassing L0$ via the GLC flag
and L1$ via DLC.

However, it should be noted that memory accesses in the ordered section may be
expected by the application to be done in primitive order even if they don't
have the GLC and DLC flags. Coherent access not only bypasses, but also
invalidates the lower-level caches for the accessed memory locations. Thus,
considering that normally per-pixel data is accessed exclusively by the
invocation executing the ordered section, it's not necessary to make all reads
or writes in the ordered section for one memory location to be GLC/DLC — just
the first read and the last write: it doesn't matter if per-pixel data is cached
in L0/L1 in the middle of a dependency chain in the ordered section, as long as
it's invalidated in them in the beginning and flushed to L2 in the end.
Therefore, optimizations in the compiler must not simply assume that only
coherent accesses need primitive ordering — and moreover, the compiler must also
take into account that the same data may be accessed through different bindings.

Export requirements
^^^^^^^^^^^^^^^^^^^

With POPS, on all hardware generations, the shader must have at least one
export, though it can be a null or an ``off, off, off, off`` one.

Also, even if the shader doesn't need to export any real data, the export
skipping that was added in GFX10 must not be used, and some space must be
allocated in the export buffer, such as by setting ``SPI_SHADER_COL_FORMAT`` for
some color output to ``SPI_SHADER_32_R``.

Without this, the shader will be executed without the needed synchronization on
GFX10, and will hang on GFX11.

Drawing context setup
---------------------

Configuring POPS
^^^^^^^^^^^^^^^^

Most of the configuration is performed via the ``DB_SHADER_CONTROL`` register.

To enable POPS for the draw,
``DB_SHADER_CONTROL.PRIMITIVE_ORDERED_PIXEL_SHADER`` should be set to 1.

On GFX9–10.3, ``DB_SHADER_CONTROL.POPS_OVERLAP_NUM_SAMPLES`` controls which
fragment shader invocations are considered overlapping:

* For pixel interlock, it must be set to 0 (1 sample).
* If sample interlock is sufficient (only synchronizing between invocations that
  have any common sample mask bits), it may be set to
  ``PA_SC_AA_CONFIG.MSAA_EXPOSED_SAMPLES`` — the number of sample coverage mask
  bits passed to the shader which is expected to use the sample mask to
  determine whether it's allowed to access the data for each of the samples. As
  of April 2023, PAL for some reason doesn't use non-1x
  ``POPS_OVERLAP_NUM_SAMPLES`` at all, even when using Direct3D Rasterizer
  Ordered Views or ``GL_INTEL_fragment_shader_ordering`` with sample shading
  (those APIs tie the interlock granularity to the shading frequency — Vulkan
  and OpenGL fragment shader interlock, however, allows specifying the interlock
  granularity independently of it, making it possible both to ask for finer
  synchronization guarantees and to require stronger ones than Direct3D ROVs can
  provide). However, with MSAA, on AMD hardware, pixel interlock generally
  performs *massively*, sometimes prohibitively, slower than sample interlock,
  because it causes fragment shader invocations along the common edge of
  adjacent primitives to be ordered as they cover the same pixels (even though
  they don't cover any common samples). So it's highly desirable for the driver
  to provide sample interlock, and to set ``POPS_OVERLAP_NUM_SAMPLES``
  accordingly, if the shader declares that it's enough for it via the execution
  mode.

On GFX11, when POPS is enabled, ``DB_SHADER_CONTROL.OVERRIDE_INTRINSIC_RATE`` is
used in place of ``DB_SHADER_CONTROL.POPS_OVERLAP_NUM_SAMPLES`` from the earlier
architecture generations (and has a different bit offset in the register), and
``DB_SHADER_CONTROL.OVERRIDE_INTRINSIC_RATE_ENABLE`` must be set to 1. The GFX11
blending performance workaround overriding the intrinsic rate must not be
applied if POPS is used in the draw — the intrinsic rate override must be used
solely to control the interlock granularity in this case.

No explicit flushes/synchronization are needed when changing the pipeline state
variables that may be involved in POPS, such as the rasterization sample count.
POPS automatically keeps synchronizing invocations even between draws with
different sample counts (invocations with common coverage mask bits are
considered overlapping by the hardware, regardless of what those samples
actually are — only the indices are important).

Also, on GFX11, POPS uses ``DB_Z_INFO.NUM_SAMPLES`` to determine the coverage
sample count, and it must be equal to ``PA_SC_AA_CONFIG.MSAA_EXPOSED_SAMPLES``
even if there's no depth/stencil target.

Hardware bug workarounds
^^^^^^^^^^^^^^^^^^^^^^^^

Early revisions of GFX9 — ``CHIP_VEGA10`` and ``CHIP_RAVEN`` — contain a
hardware bug that may result in a hang, and need a workaround to be enabled.
Specifically, if POPS is used with 8 or more rasterization samples, or with 8 or
more depth/stencil target samples, ``DB_DFSM_CONTROL.POPS_DRAIN_PS_ON_OVERLAP``
must be set to 1 for draws that satisfy this condition. In PAL, this is the
``waMiscPopsMissedOverlap`` workaround. It results in slightly lower performance
in those cases, increasing the frame time by around 1.5 to 2 times in
`nvpro-samples/vk_order_independent_transparency <https://github.com/nvpro-samples/vk_order_independent_transparency>`_
on the RX Vega 10, but it's required in a pretty rare case (8x+ MSAA) and is
mandatory to ensure stability.

Also, even though ``DB_DFSM_CONTROL.POPS_DRAIN_PS_ON_OVERLAP`` is not required
on chips other than the ``CHIP_VEGA10`` and ``CHIP_RAVEN`` GFX9 revisions, if
it's enabled for some reason on GFX10.1 (``CHIP_NAVI10``, ``CHIP_NAVI12``,
``CHIP_NAVI14``), and the draw uses POPS,
``DB_RENDER_OVERRIDE2.PARTIAL_SQUAD_LAUNCH_CONTROL`` must be set to
``PSLC_ON_HANG_ONLY`` to avoid a hang (see ``waStalledPopsMode`` in PAL).

Out-of-order rasterization interaction
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is a largely unresearched topic currently. However, considering that POPS
is primarily the functionality of the Depth Block, similarity to the behavior of
out-of-order rasterization in depth/stencil testing may possibly be expected.

If the shader specifies an ordered interlock execution mode, out-of-order
rasterization likely must not be enabled implicitly.

As of April 2023, PAL doesn't have any rules specifically for POPS in the logic
determining whether out-of-order rasterization can be enabled automatically.
Some of the POPS usage cases may possibly be covered by the rule that always
disables out-of-order rasterization if the shader writes to Unordered Access
Views (storage resources), though fragment shader interlock can be used for
read-only purposes too (for ordering between draws that only read per-pixel data
and draws that may write it), so that may be an oversight.

Explicitly enabled relaxed rasterization order modifies the concept of
rasterization order itself in Vulkan, so from the point of view of the
specification of fragment shader interlock, relaxed rasterization order should
still be applicable regardless of whether the shader requests ordered interlock.
PAL also doesn't make any POPS-specific exceptions here as of April 2023.

Variable-rate shading interaction
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On GFX10.3, enabling ``DB_SHADER_CONTROL.PRIMITIVE_ORDERED_PIXEL_SHADER`` forces
the shading rate to be 1x1, thus the
``fragmentShadingRateWithFragmentShaderInterlock`` Vulkan device property must
be false.

On GFX11, by default, POPS itself can work with non-1x1 shading rates, and the
``fragmentShadingRateWithFragmentShaderInterlock`` property must be true.
However, if ``PA_SC_VRS_SURFACE_CNTL_1.FORCE_SC_VRS_RATE_FINE_POPS`` is set,
enabling POPS will force 1x1 shading rate.

The widest interlock granularity available on GFX11 — with the lowest possible
Depth Block intrinsic rate, 1x — is per-fine-pixel, however. There's no
synchronization between coarse fragment shader invocations if they don't cover
common fine pixels, so the ``fragmentShaderShadingRateInterlock`` Vulkan device
feature is not available.

Additional configuration
^^^^^^^^^^^^^^^^^^^^^^^^

These are some largely unresearched options found in the register declarations.
PAL doesn't use them, so it's unknown if they make any significant difference.
No effect was found in `nvpro-samples/vk_order_independent_transparency <https://github.com/nvpro-samples/vk_order_independent_transparency>`_
during testing on GFX9 ``CHIP_RAVEN`` and GFX11 ``CHIP_NAVI31``.

* ``DB_SHADER_CONTROL.EXEC_IF_OVERLAPPED`` on GFX9–10.3.
* ``PA_SC_BINNER_CNTL_0.BIN_MAPPING_MODE = BIN_MAP_MODE_POPS`` on GFX10+.
