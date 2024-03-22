.. _blend:

Blend
=====

This state controls blending of the final fragments into the target rendering
buffers.

Blend Factors
-------------

The blend factors largely follow the same pattern as their counterparts
in other modern and legacy drawing APIs.

Dual source blend factors are supported for up to 1 MRT, although
you can advertise > 1 MRT, the stack cannot handle them for a few reasons.
There is no definition on how the 1D array of shader outputs should be mapped
to something that would be a 2D array (location, index). No current hardware
exposes > 1 MRT, and we should revisit this issue if anyone ever does.

Logical Operations
------------------

Logical operations, also known as logicops, LOPs, or ROPs, are supported.
Only two-operand logicops are available. When logicops are enabled, all other
blend state is ignored, including per-render-target state, so logicops are
performed on all render targets.

.. warning::
   The blend_enable flag is ignored for all render targets when logical
   operations are enabled.

For a source component ``s`` and destination component ``d``, the logical
operations are defined as taking the bits of each channel of each component,
and performing one of the following operations per-channel:

================== =========================
Operation          Equation
================== =========================
``CLEAR``          :math:`0`
``NOR``            :math:`\lnot(s \lor d)`
``AND_INVERTED``   :math:`\lnot s \land d`
``COPY_INVERTED``  :math:`\lnot s`
``AND_REVERSE``    :math:`s \land \lnot d`
``INVERT``         :math:`\lnot d`
``XOR``            :math:`s \oplus d`
``NAND``           :math:`\lnot(s \land d)`
``AND``            :math:`s \land d`
``EQUIV``          :math:`\lnot(s \oplus d)`
``NOOP``           :math:`d`
``OR_INVERTED``    :math:`\lnot s \lor d`
``COPY``           :math:`s`
``OR_REVERSE``     :math:`s \lor \lnot d`
``OR``             :math:`s \lor d`
``SET``            :math:`1`
================== =========================

.. note::
   The logical operation names and definitions match those of the OpenGL API,
   and are similar to the ROP2 and ROP3 definitions of GDI. This is
   intentional, to ease transitions to Gallium.

Members
-------

These members affect all render targets.

dither
   Whether dithering is enabled.

   .. note::
      Dithering is completely implementation-dependent. It may be ignored by
      drivers for any reason, and some render targets may always or never be
      dithered depending on their format or usage flags.

logicop_enable
   Whether the blender should perform a logicop instead of blending.
logicop_func
   The logicop to use. One of ``PIPE_LOGICOP``.
independent_blend_enable
   If enabled, blend state is different for each render target, and
   for each render target set in the respective member of the rt array.
   If disabled, blend state is the same for all render targets, and only
   the first member of the rt array contains valid data.
rt
   Contains the per-rendertarget blend state.
alpha_to_coverage
   If enabled, the fragment's alpha value is used to override the fragment's
   coverage mask.  The coverage mask will be all zeros if the alpha value is
   zero.  The coverage mask will be all ones if the alpha value is one.
   Otherwise, the number of bits set in the coverage mask will be proportional
   to the alpha value.  Note that this step happens regardless of whether
   multisample is enabled or the destination buffer is multisampled.
alpha_to_one
   If enabled, the fragment's alpha value will be set to one.  As with
   alpha_to_coverage, this step happens regardless of whether multisample
   is enabled or the destination buffer is multisampled.
max_rt
   The index of the max render target (irrespective of whether independent
   blend is enabled), i.e. the number of MRTs minus one.  This is provided
   so that the driver can avoid the overhead of programming unused MRTs.


Per-rendertarget Members
------------------------

blend_enable
   If blending is enabled, perform a blend calculation according to blend
   functions and source/destination factors. Otherwise, the incoming fragment
   color gets passed unmodified (but colormask still applies).
rgb_func
   The blend function to use for RGB channels. One of PIPE_BLEND.
rgb_src_factor
   The blend source factor to use for RGB channels. One of PIPE_BLENDFACTOR.
rgb_dst_factor
   The blend destination factor to use for RGB channels. One of PIPE_BLENDFACTOR.
alpha_func
   The blend function to use for the alpha channel. One of PIPE_BLEND.
alpha_src_factor
   The blend source factor to use for the alpha channel. One of PIPE_BLENDFACTOR.
alpha_dst_factor
   The blend destination factor to use for alpha channel. One of PIPE_BLENDFACTOR.
colormask
   Bitmask of which channels to write. Combination of PIPE_MASK bits.
