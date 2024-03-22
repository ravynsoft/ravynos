Single-sampled Color Compression
================================

Starting with Ivy Bridge, Intel graphics hardware provides a form of color
compression for single-sampled surfaces.  In its initial form, this provided an
acceleration of render target clear operations that, in the common case, allows
you to avoid almost all of the bandwidth of a full-surface clear operation.  On
Sky Lake, single-sampled color compression was extended to allow for the
compression color values from actual rendering and not just the initial clear.
From here on, the older Ivy Bridge form of color compression will be called
"fast-clears" and term "color compression" will be reserved for the more
powerful Sky Lake form.

The documentation for Ivy Bridge through Broadwell overloads the term MCS for
referring both to the *multisample control surface* used for multisample
compression and the control surface used for fast-clears. In ISL, the
:c:enumerator:`isl_aux_usage.ISL_AUX_USAGE_MCS` enum always refers to
multisample color compression while the
:c:enumerator:`isl_aux_usage.ISL_AUX_USAGE_CCS_` enums always refer to
single-sampled color compression. Throughout this chapter and the rest of the
ISL documentation, we will use the term "color control surface", abbreviated
CCS, to denote the control surface used for both fast-clears and color
compression.  While this is still an overloaded term, Ivy Bridge fast-clears
are much closer to Sky Lake color compression than they are to multisample
compression.

CCS data
--------

Fast clears and CCS are possibly the single most poorly documented aspect of
surface layout/setup for Intel graphics hardware (with HiZ coming in a neat
second). All the documentation really says is that you can use an MCS buffer on
single-sampled surfaces (we will call it the CCS in this case). It also
provides some documentation on how to program the hardware to perform clear
operations, but that's it.  How big is this buffer?  What does it contain?
Those question are left as exercises to the reader. Almost everything we know
about the contents of the CCS is gleaned from reverse-engineering of the
hardware.  The best bit of documentation we have ever had comes from the
display section of the Sky Lake PRM Vol 12 section on planes (p. 159):

   The Color Control Surface (CCS) contains the compression status of the
   cache-line pairs. The compression state of the cache-line pair is
   specified by 2 bits in the CCS.  Each CCS cache-line represents an area
   on the main surface of 16x16 sets of 128 byte Y-tiled cache-line-pairs.
   CCS is always Y tiled.

While this is technically for color compression and not fast-clears, it
provides a good bit of insight into how color compression and fast-clears
operate.  Each cache-line pair, in the main surface corresponds to 1 or 2 bits
in the CCS.  The primary difference, as far as the current discussion is
concerned, is that fast-clears use only 1 bit per cache-line pair whereas color
compression uses 2 bits.

What is a cache-line pair?  Both the X and Y tiling formats are arranged as an
8x8 grid of cache lines.  (See the :doc:`chapter on tiling <tiling>` for more
details.)  In either case, a cache-line pair is a pair of cache lines whose
starting addresses differ by 512 bytes or 8 cache lines.  This results in the
two cache lines being vertically adjacent when the main surface is X-tiled and
horizontally adjacent when the main surface is Y-tiled.  For an X-tiled surface
this forms an area of 64B x 2rows and for a Y-tiled surface this forms an area
of 32B x 4rows.  In either case, it is guaranteed that, regardless of surface
format, each 2x2 subspan coming out of a shader will land entirely within one
cache-line pair.

What is the correspondence between bits and cache-line pairs?  The best model I
(Faith) know of is to consider the CCS as having a 1-bit color format for
fast-clears and a 2-bit format for color compression and a special tiling
format.  The CCS tiling formats operate on a 1 or 2-bit granularity rather than
the byte granularity of most tiling formats.

The following table represents the bit-layouts that yield the CCS tiling format
on different hardware generations.  Bits 0-11 correspond to the regular swizzle
of bytes within a 4KB page whereas the negative bits represent the address of
the particular 1 or 2-bit portion of a byte. (Note: The Haswell data was
gathered on a dual-channel system so bit-6 swizzling was enabled.  It's unclear
how this affects the CCS layout.)

============ ======== =========== =========== ====================== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== ===========
 Generation   Tiling       11          10               9                 8           7           6           5           4           3           2           1           0          -1          -2          -3
============ ======== =========== =========== ====================== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== ===========
 Ivy Bridge   X or Y  :math:`u_6` :math:`u_5`      :math:`u_4`       :math:`v_7` :math:`v_6` :math:`v_5` :math:`v_4` :math:`v_2` :math:`v_3` :math:`v_1` :math:`v_0` :math:`u_3` :math:`u_2` :math:`u_1` :math:`u_0`
 Haswell        X     :math:`u_6` :math:`u_5` :math:`v_3 \oplus u_1` :math:`v_7` :math:`v_6` :math:`v_5` :math:`v_4` :math:`v_2` :math:`v_3` :math:`v_1` :math:`v_0` :math:`u_4` :math:`u_3` :math:`u_2` :math:`u_0`
 Haswell        Y     :math:`u_6` :math:`u_5` :math:`v_2 \oplus u_1` :math:`v_7` :math:`v_6` :math:`v_5` :math:`v_4` :math:`v_2` :math:`v_3` :math:`v_1` :math:`v_0` :math:`u_4` :math:`u_3` :math:`u_2` :math:`u_0`
 Broadwell      X     :math:`u_6` :math:`u_5`      :math:`u_4`       :math:`v_7` :math:`v_6` :math:`v_5` :math:`v_4` :math:`u_3` :math:`v_3` :math:`u_2` :math:`u_1` :math:`u_0` :math:`v_2` :math:`v_1` :math:`v_0`
 Broadwell      Y     :math:`u_6` :math:`u_5`      :math:`u_4`       :math:`v_7` :math:`v_6` :math:`v_5` :math:`v_4` :math:`v_2` :math:`v_3` :math:`u_3` :math:`u_2` :math:`u_1` :math:`v_1` :math:`v_0` :math:`u_0`
 Sky Lake       Y     :math:`u_6` :math:`u_5`      :math:`u_4`       :math:`v_6` :math:`v_5` :math:`v_4` :math:`v_3` :math:`v_2` :math:`v_1` :math:`u_3` :math:`u_2` :math:`u_1` :math:`v_0` :math:`u_0`
============ ======== =========== =========== ====================== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== =========== ===========

CCS surface layout
------------------

Starting with Broadwell, fast-clears and color compression can be used on
mipmapped and array surfaces.  When considered from a higher level, the CCS is
laid out like any other surface.  The Broadwell and Sky Lake PRMs describe
this as follows:

Broadwell PRM Vol 7, "MCS Buffer for Render Target(s)" (p. 676):

   Mip-mapped and arrayed surfaces are supported with MCS buffer layout with
   these alignments in the RT space: Horizontal Alignment = 256 and Vertical
   Alignment = 128.

Broadwell PRM Vol 2d, "RENDER_SURFACE_STATE" (p. 279):

   For non-multisampled render target's auxiliary surface, MCS, QPitch must be
   computed with Horizontal Alignment = 256 and Surface Vertical Alignment =
   128. These alignments are only for MCS buffer and not for associated render
   target.

Sky Lake PRM Vol 7, "MCS Buffer for Render Target(s)" (p. 632):

   Mip-mapped and arrayed surfaces are supported with MCS buffer layout with
   these alignments in the RT space: Horizontal Alignment = 128 and Vertical
   Alignment = 64.

Sky Lake PRM Vol. 2d, "RENDER_SURFACE_STATE" (p. 435):

   For non-multisampled render target's CCS auxiliary surface, QPitch must be
   computed with Horizontal Alignment = 128 and Surface Vertical Alignment
   = 256. These alignments are only for CCS buffer and not for associated
   render target.

Empirical evidence seems to confirm this.  On Sky Lake, the vertical alignment
is always one cache line.  The horizontal alignment, however, varies by main
surface format: 1 cache line for 32bpp, 2 for 64bpp and 4 cache lines for
128bpp formats.  This nicely corresponds to the alignment of 128x64 pixels in
the primary color surface.  The second PRM citation about Sky Lake CCS above
gives a vertical alignment of 256 rather than 64.  With a little
experimentation, this additional alignment appears to only apply to QPitch and
not to the miplevels within a slice.

On Broadwell, each miplevel in the CCS is aligned to a cache-line pair
boundary: horizontal when the primary surface is X-tiled and vertical when
Y-tiled. For a 32bpp format, this works out to an alignment of 256x128 main
surface pixels regardless of X or Y tiling.  On Sky Lake, the alignment is
a single cache line which works out to an alignment of 128x64 main surface
pixels.

TODO: More than just 32bpp formats on Broadwell!

Once armed with the above alignment information, we can lay out the CCS surface
itself.  The way ISL does CCS layout calculations is by a very careful  and
subtle application of its normal surface layout code.

Above, we described the CCS data layout as mapping of address bits. In
ISL, this is represented by :c:enumerator:`isl_tiling.ISL_TILING_CCS`.  The
logical and physical tile dimensions corresponding to the above mapping.

We also have special :c:enum:`isl_format` enums for CCS.  These formats are 1
bit-per-pixel on Ivy Bridge through Broadwell and 2 bits-per-pixel on Skylake
and above to correspond to the 1 and 2-bit values represented in the CCS data.
They have a block size (similar to a block compressed format such as BC or
ASTC) which says what area (in surface elements) in the main surface is covered
by a single CCS element (1 or 2-bit).  Because this depends on the main surface
tiling and format, we have several different CCS formats.

Once the appropriate :c:enum:`isl_format` has been selected, computing the
size and layout of a CCS surface is as simple as passing the same surface
creation parameters to :c:func:`isl_surf_init_s` as were used to create the
primary surface only with :c:enumerator:`isl_tiling.ISL_TILING_CCS` and the
correct CCS format.  This not only results in a correctly sized surface but
most other ISL helpers for things such as computing offsets into surfaces work
correctly as well.

CCS on Tigerlake and above
--------------------------

Starting with Tigerlake, CCS is no longer done via a surface and, instead, the
term CCS gets overloaded once again (gotta love it!) to now refer to a form of
universal compression which can be applied to almost any surface.  Nothing in
this chapter applies to any hardware with a graphics IP version 12 or above.
