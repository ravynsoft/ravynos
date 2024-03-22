Hierarchical Depth (HiZ)
========================

TODO: Add detailed docs like we have for CCS

HiZ/stencil on Sandy Bridge
---------------------------

Properly enabling HiZ on Sandy Bridge requires certain special considerations.
From the Sandy Bridge PRM Vol. 2, Pt. 1, 7.5.3 "Hierarchical Depth Buffer" (p.
312):

   The hierarchical depth buffer does not support the LOD field, it is assumed
   by hardware to be zero. A separate hierarchical depth buffer is required
   for each LOD used, and the corresponding buffer’s state delivered to
   hardware each time a new depth buffer state with modified LOD is delivered.

The ``3DSTATE_STENCIL_BUFFER`` packet for separate stencil (required for HiZ)
on sandy bridge also lacks an LOD field.  Empirically, the hardware doesn't
pull the stencil LOD from ``3DSTATE_DEPTH_BUFFER``, it's just always 0 like
with HiZ.

As stated in the PRM, this means we need a separate HiZ or stencil buffer for
each LOD.  However, it's not quite as simple as that.  If you ignore layered
rendering, things are pretty straightforward: you need one HiZ surface for each
main surface slice. With layered rendering, however, we have to be a bit more
clever because we need a "real" array surface at each LOD.  ISL solves this
with a special miptree layout for layered rendering
:c:enumerator:`isl_dim_layout.ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ` which lays
out the surface as a miptree of layered images instead of an array of miptrees.
See the docs for
:c:enumerator:`isl_dim_layout.ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ` for a nice
description along with an ASCII art diagram of the layout.

Also, neither ``3DSTATE_STENCIL_BUFFER`` nor ``3DSTATE_HIER_DEPTH_BUFFER`` have
their own surface dimensions or layout information on Sandy Bridge.  They're
just an address and a surface pitch.  Instead, all that other information is
pulled from ``3DSTATE_DEPTH_BUFFER``.  When you combine this with the lack of
LOD, this means that, technically, we have a full-sized single-LOD stencil or
HiZ surface at each miplevel of which only the upper left-hand corner of each
array slice ever gets used.  The net effect of this is that, in
:c:enumerator:`isl_dim_layout.ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ`, all LODs
share the same QPitch even though it's horribly wasteful.  This is actually
pretty convenient for ISL because we only have the one
:c:member:`isl_surf.array_pitch_el_rows` field.

Due to difficulties with plumbing relocation deltas through ISL's
depth/stencil/hiz emit interface, we can't handle this all automatically in
ISL.  Instead, it's left up to the driver to do this offsetting.  ISL does
provide helpers for computing the offsets and they work fine with
:c:enumerator:`isl_dim_layout.ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ` so all that's
really required is to call the ISL helper and add the computed offset to the
HiZ or stencil buffer address.  The following is an excerpt from BLORP where we
do this as an example:

.. code-block:: c

   struct blorp_address hiz_address = params->depth.aux_addr;
   #if GFX_VER == 6
   /* Sandy bridge hardware does not technically support mipmapped HiZ.
    * However, we have a special layout that allows us to make it work
    * anyway by manually offsetting to the specified miplevel.
    */
   assert(info.hiz_surf->dim_layout == ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ);
   uint32_t offset_B;
   isl_surf_get_image_offset_B_tile_sa(info.hiz_surf,
                                       info.view->base_level, 0, 0,
                                       &offset_B, NULL, NULL);
   hiz_address.offset += offset_B;
   #endif

   info.hiz_address =
      blorp_emit_reloc(batch, dw + isl_dev->ds.hiz_offset / 4,
                       hiz_address, 0);
