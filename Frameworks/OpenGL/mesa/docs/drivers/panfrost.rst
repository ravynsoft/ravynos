Panfrost
========

The Panfrost driver stack includes an OpenGL ES implementation for Arm Mali
GPUs based on the Midgard and Bifrost microarchitectures. It is **conformant**
on Mali-G52 and Mali-G57 but **non-conformant** on other GPUs. The following
hardware is currently supported:

=========  ============ ============ =======
Product    Architecture OpenGL ES    OpenGL
=========  ============ ============ =======
Mali T620  Midgard (v4) 2.0          2.1
Mali T720  Midgard (v4) 2.0          2.1
Mali T760  Midgard (v5) 3.1          3.1
Mali T820  Midgard (v5) 3.1          3.1
Mali T830  Midgard (v5) 3.1          3.1
Mali T860  Midgard (v5) 3.1          3.1
Mali T880  Midgard (v5) 3.1          3.1
Mali G72   Bifrost (v6) 3.1          3.1
Mali G31   Bifrost (v7) 3.1          3.1
Mali G51   Bifrost (v7) 3.1          3.1
Mali G52   Bifrost (v7) 3.1          3.1
Mali G76   Bifrost (v7) 3.1          3.1
Mali G57   Valhall (v9) 3.1          3.1
=========  ============ ============ =======

Other Midgard and Bifrost chips (T604, G71) are not yet supported.

Older Mali chips based on the Utgard architecture (Mali 400, Mali 450) are
supported in the :doc:`Lima <lima>` driver, not Panfrost. Lima is also
available in Mesa.

Other graphics APIs (Vulkan, OpenCL) are not supported at this time.

Building
--------

Panfrost's OpenGL support is a Gallium driver. Since Mali GPUs are 3D-only and
do not include a display controller, Mesa uses kmsro to support display
controllers paired with Mali GPUs. If your board with a Panfrost supported GPU
has a display controller with mainline Linux support not supported by kmsro,
it's easy to add support, see the commit ``cff7de4bb597e9`` as an example.

LLVM is *not* required by Panfrost's compilers. LLVM support in Mesa can
safely be disabled for most OpenGL ES users with Panfrost.

Build like ``meson . build/ -Dvulkan-drivers=
-Dgallium-drivers=panfrost -Dllvm=disabled`` for a build directory
``build``.

For general information on building Mesa, read :doc:`the install documentation
<../install>`.

Chat
----

Panfrost developers and users hang out on IRC at ``#panfrost`` on OFTC. Note
that registering and authenticating with ``NickServ`` is required to prevent
spam. `Join the chat. <https://webchat.oftc.net/?channels=panfrost>`_

Compressed texture support
--------------------------

In the driver, Panfrost supports ASTC, ETC, and all BCn formats (e.g. RGTC,
S3TC, etc.) However, Panfrost depends on the hardware to support these formats
efficiently.  All supported Mali architectures support these formats, but not
every system-on-chip with a Mali GPU support all these formats. Many lower-end
systems lack support for some BCn formats, which can cause problems when playing
desktop games with Panfrost. To check whether this issue applies to your
system-on-chip, Panfrost includes a ``panfrost_texfeatures`` tool to query
supported formats.

To use this tool, include the option ``-Dtools=panfrost`` when configuring Mesa.
Then inside your Mesa build directory, the tool is located at
``src/panfrost/tools/panfrost_texfeatures``. Copy it to your target device,
set as executable as necessary, and run on the target device. A table of
supported formats will be printed to standard output.

drm-shim
--------

Panfrost implements ``drm-shim``, stubbing out the Panfrost kernel interface.
Use cases for this functionality include:

- Future hardware bring up
- Running shader-db on non-Mali workstations
- Reproducing compiler (and some driver) bugs without Mali hardware

Although Mali hardware is usually paired with an Arm CPU, Panfrost is portable C
code and should work on any Linux machine. In particular, you can test the
compiler on shader-db on an Intel desktop.

To build Mesa with Panfrost drm-shim, configure Meson with
``-Dgallium-drivers=panfrost`` and ``-Dtools=drm-shim``. See the above
building section for a full invocation. The drm-shim binary will be built to
``build/src/panfrost/drm-shim/libpanfrost_noop_drm_shim.so``.

To use, set the ``LD_PRELOAD`` environment variable to the drm-shim binary.  It
may also be necessary to set ``LIBGL_DRIVERS_PATH`` to the location where Mesa
was installed.

By default, drm-shim mocks a Mali-G52 system. To select a specific Mali GPU,
set the ``PAN_GPU_ID`` environment variable to the desired GPU ID:

=========  ============ =======
Product    Architecture GPU ID
=========  ============ =======
Mali-T720  Midgard (v4) 720
Mali-T860  Midgard (v5) 860
Mali-G72   Bifrost (v6) 6221
Mali-G52   Bifrost (v7) 7212
Mali-G57   Valhall (v9) 9093
=========  ============ =======

Additional GPU IDs are enumerated in the ``panfrost_model_list`` list in
``src/panfrost/lib/pan_props.c``.

As an example: assuming Mesa is installed to a local path ``~/lib`` and Mesa's
build directory is ``~/mesa/build``, a shader can be compiled for Mali-G52 as:

.. code-block:: console

   ~/shader-db$ BIFROST_MESA_DEBUG=shaders \
   LIBGL_DRIVERS_PATH=~/lib/dri/ \
   LD_PRELOAD=~/mesa/build/src/panfrost/drm-shim/libpanfrost_noop_drm_shim.so \
   PAN_GPU_ID=7212 \
   ./run shaders/glmark/1-1.shader_test

The same shader can be compiled for Mali-T720 as:

.. code-block:: console

   ~/shader-db$ MIDGARD_MESA_DEBUG=shaders \
   LIBGL_DRIVERS_PATH=~/lib/dri/ \
   LD_PRELOAD=~/mesa/build/src/panfrost/drm-shim/libpanfrost_noop_drm_shim.so \
   PAN_GPU_ID=720 \
   ./run shaders/glmark/1-1.shader_test

These examples set the compilers' ``shaders`` debug flags to dump the optimized
NIR, backend IR after instruction selection, backend IR after register
allocation and scheduling, and a disassembly of the final compiled binary.

As another example, this invocation runs a single dEQP test "on" Mali-G52,
pretty-printing GPU data structures and disassembling all shaders
(``PAN_MESA_DEBUG=trace``) as well as dumping raw GPU memory
(``PAN_MESA_DEBUG=dump``). The ``EGL_PLATFORM=surfaceless`` environment variable
and various flags to dEQP mimic the surfaceless environment that our
continuous integration (CI) uses. This eliminates window system dependencies,
although it requires a specially built CTS:

.. code-block:: console

   ~/VK-GL-CTS/build/external/openglcts/modules$ PAN_MESA_DEBUG=trace,dump \
   LIBGL_DRIVERS_PATH=~/lib/dri/ \
   LD_PRELOAD=~/mesa/build/src/panfrost/drm-shim/libpanfrost_noop_drm_shim.so \
   PAN_GPU_ID=7212 EGL_PLATFORM=surfaceless \
   ./glcts --deqp-surface-type=pbuffer \
   --deqp-gl-config-name=rgba8888d24s8ms0 --deqp-surface-width=256 \
   --deqp-surface-height=256 -n \
   dEQP-GLES31.functional.shaders.builtin_functions.common.abs.float_highp_compute

U-interleaved tiling
---------------------

Panfrost supports u-interleaved tiling. U-interleaved tiling is
indicated by the ``DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED`` modifier.

The tiling reorders whole pixels (blocks). It does not compress or modify the
pixels themselves, so it can be used for any image format. Internally, images
are divided into tiles. Tiles occur in source order, but pixels (blocks) within
each tile are reordered according to a space-filling curve.

For regular formats, 16x16 tiles are used. This harmonizes with the default tile
size for binning and CRCs (transaction elimination). It also means a single line
(16 pixels) at 4 bytes per pixel equals a single 64-byte cache line.

For formats that are already block compressed (S3TC, RGTC, etc), 4x4 tiles are
used, where entire blocks are reorder. Most of these formats compress 4x4
blocks, so this gives an effective 16x16 tiling. This justifies the tile size
intuitively, though it's not a rule: ASTC may uses larger blocks.

Within a tile, the X and Y bits are interleaved (like Morton order), but with a
twist: adjacent bit pairs are XORed. The reason to add XORs is not obvious.
Visually, addresses take the form::

   | y3 | (x3 ^ y3) | y2 | (y2 ^ x2) | y1 | (y1 ^ x1) | y0 | (y0 ^ x0) |

Reference routines to encode/decode u-interleaved images are available in
``src/panfrost/shared/test/test-tiling.cpp``, which documents the space-filling
curve. This reference implementation is used to unit test the optimized
implementation used in production. The optimized implementation is available in
``src/panfrost/shared/pan_tiling.c``.

Although these routines are part of Panfrost, they are also used by Lima, as Arm
introduced the format with Utgard. It is the only tiling supported on Utgard. On
Mali-T760 and newer, Arm Framebuffer Compression (AFBC) is more efficient and
should be used instead where possible. However, not all formats are
compressible, so u-interleaved tiling remains an important fallback on Panfrost.

Instancing
----------

The attribute descriptor lets the attribute unit compute the address of an
attribute given the vertex and instance ID. Unfortunately, the way this works is
rather complicated when instancing is enabled.

To explain this, first we need to explain how compute and vertex threads are
dispatched.  When a quad is dispatched, it receives a single, linear index.
However, we need to translate that index into a (vertex id, instance id) pair.
One option would be to do:

.. math::
   \text{vertex id} = \text{linear id} \% \text{num vertices}

   \text{instance id} = \text{linear id} / \text{num vertices}

but this involves a costly division and modulus by an arbitrary number.
Instead, we could pad num_vertices. We dispatch padded_num_vertices *
num_instances threads instead of num_vertices * num_instances, which results
in some "extra" threads with vertex_id >= num_vertices, which we have to
discard.  The more we pad num_vertices, the more "wasted" threads we
dispatch, but the division is potentially easier.

One straightforward choice is to pad num_vertices to the next power of two,
which means that the division and modulus are just simple bit shifts and
masking. But the actual algorithm is a bit more complicated. The thread
dispatcher has special support for dividing by 3, 5, 7, and 9, in addition
to dividing by a power of two. As a result, padded_num_vertices can be
1, 3, 5, 7, or 9 times a power of two. This results in less wasted threads,
since we need less padding.

padded_num_vertices is picked by the hardware. The driver just specifies the
actual number of vertices. Note that padded_num_vertices is a multiple of four
(presumably because threads are dispatched in groups of 4). Also,
padded_num_vertices is always at least one more than num_vertices, which seems
like a quirk of the hardware. For larger num_vertices, the hardware uses the
following algorithm: using the binary representation of num_vertices, we look at
the most significant set bit as well as the following 3 bits. Let n be the
number of bits after those 4 bits. Then we set padded_num_vertices according to
the following table:

==========  =======================
high bits   padded_num_vertices
==========  =======================
1000		   :math:`9 \cdot 2^n`
1001		   :math:`5 \cdot 2^{n+1}`
101x		   :math:`3 \cdot 2^{n+2}`
110x		   :math:`7 \cdot 2^{n+1}`
111x		   :math:`2^{n+4}`
==========  =======================

For example, if num_vertices = 70 is passed to glDraw(), its binary
representation is 1000110, so n = 3 and the high bits are 1000, and
therefore padded_num_vertices = :math:`9 \cdot 2^3` = 72.

The attribute unit works in terms of the original linear_id. if
num_instances = 1, then they are the same, and everything is simple.
However, with instancing things get more complicated. There are four
possible modes, two of them we can group together:

1. Use the linear_id directly. Only used when there is no instancing.

2. Use the linear_id modulo a constant. This is used for per-vertex
attributes with instancing enabled by making the constant equal
padded_num_vertices. Because the modulus is always padded_num_vertices, this
mode only supports a modulus that is a power of 2 times 1, 3, 5, 7, or 9.
The shift field specifies the power of two, while the extra_flags field
specifies the odd number. If shift = n and extra_flags = m, then the modulus
is :math:`(2m + 1) \cdot 2^n`. As an example, if num_vertices = 70, then as
computed above, padded_num_vertices = :math:`9 \cdot 2^3`, so we should set
extra_flags = 4 and shift = 3. Note that we must exactly follow the hardware
algorithm used to get padded_num_vertices in order to correctly implement
per-vertex attributes.

3. Divide the linear_id by a constant. In order to correctly implement
instance divisors, we have to divide linear_id by padded_num_vertices times
to user-specified divisor. So first we compute padded_num_vertices, again
following the exact same algorithm that the hardware uses, then multiply it
by the GL-level divisor to get the hardware-level divisor. This case is
further divided into two more cases. If the hardware-level divisor is a
power of two, then we just need to shift. The shift amount is specified by
the shift field, so that the hardware-level divisor is just
:math:`2^\text{shift}`.

If it isn't a power of two, then we have to divide by an arbitrary integer.
For that, we use the well-known technique of multiplying by an approximation
of the inverse. The driver must compute the magic multiplier and shift
amount, and then the hardware does the multiplication and shift. The
hardware and driver also use the "round-down" optimization as described in
https://ridiculousfish.com/files/faster_unsigned_division_by_constants.pdf.
The hardware further assumes the multiplier is between :math:`2^{31}` and
:math:`2^{32}`, so the high bit is implicitly set to 1 even though it is set
to 0 by the driver -- presumably this simplifies the hardware multiplier a
little. The hardware first multiplies linear_id by the multiplier and
takes the high 32 bits, then applies the round-down correction if
extra_flags = 1, then finally shifts right by the shift field.

There are some differences between ridiculousfish's algorithm and the Mali
hardware algorithm, which means that the reference code from ridiculousfish
doesn't always produce the right constants. Mali does not use the pre-shift
optimization, since that would make a hardware implementation slower (it
would have to always do the pre-shift, multiply, and post-shift operations).
It also forces the multiplier to be at least :math:`2^{31}`, which means
that the exponent is entirely fixed, so there is no trial-and-error.
Altogether, given the divisor d, the algorithm the driver must follow is:

1. Set shift = :math:`\lfloor \log_2(d) \rfloor`.
2. Compute :math:`m = \lceil 2^{shift + 32} / d \rceil` and :math:`e = 2^{shift + 32} % d`.
3. If :math:`e <= 2^{shift}`, then we need to use the round-down algorithm. Set
   magic_divisor = m - 1 and extra_flags = 1.  4. Otherwise, set magic_divisor =
   m and extra_flags = 0.
