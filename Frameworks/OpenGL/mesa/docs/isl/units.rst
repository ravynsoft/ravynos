Units
=====

Almost every variable, function parameter, or struct field in ISL that carries
a numeric value has explicit units associated with it.  The units used in ISL
are as follows:

 * Pixels (*px*)
 * Samples (*sa*)
 * Elements (*el*)
 * Tiles (*tl*)
 * Bytes (*B*)
 * Rows of some other unit size (*<unit>_rows*)

These units are fundamental to ISL because they allow us to specify information
about a surface in a canonical way that isn't dependent on hardware generation.
Each field in an ISL data structure that stores any sort of dimension has a
suffix that declares the units for that particular value: "`_el`" for elements,
"`_sa`" for samples, etc.  If the units of the particular field aren't quite
what is wanted by the hardware, we do the conversion when we emit
`RENDER_SURFACE_STATE`.

This is one of the primary differences between ISL and the old miptree code and
one of the core design principles of ISL.  In the old miptree code, we tried to
keep everything in the same units as the hardware expects but this lead to
unnecessary complications as the hardware evolved.  One example of this
difference is QPitch which specifies the distance between array slices.  On
Broadwell and earlier, QPitch field in `RENDER_SURFACE_STATE` was in
rows of samples.  For block-compressed images, this meant it had to be
a multiple of the block height.  On Skylake, it changed to always being in rows
of elements so you have to divide the pitch in samples by the compression
block height.  Since the old surface state code tries to store things in
hardware units, everyone who ever reads :c:expr:`brw_mipmap_tree.qpitch` has
to change their interpretation based on hardware generation and whether or not
the surface was block-compressed.  In ISL, we have
:c:member:`isl_surf.array_pitch_el_rows` which, as the name says, is in rows
of elements.  On Broadwell and earlier, we have to multiply by the block size
of the texture when we finally fill out the hardware packet.  However, the
consistency of always being in rows of elements makes any other users of the
field much simpler because they never have to look at hardware generation or
whether or not the image is block-compressed.

**Pixels** are the most straightforward unit and are where everything starts. A
pixel simply corresponds to a single pixel (or texel if you prefer) in the
surface.  For multisampled surfaces, a pixel may contain one or more samples.
For compressed textures, a compression block may contain one or more pixels.
When initially creating a surface, everything passed to isl_surf_init is
implicitly in terms of pixels because this is what all of the APIs use.

The next unit in ISL's repertoire is **samples**.  In a multisampled surface,
each pixel corresponds to some number of samples given by
:c:member:`isl_surf.samples`.  The exact layout of the samples depends on
the value of :c:member:`isl_surf.msaa_layout`.  If the layout is
:c:enumerator:`ISL_MSAA_LAYOUT_ARRAY` then each logical array in the surface
corresponds to :c:member:`isl_surf.samples` actual slices
in the resulting surface, one per array slice.  If the layout is
:c:enumerator:`ISL_MSAA_LAYOUT_INTERLEAVED` then each pixel corresponds to a
2x1, 2x2, 4x2, or 4x4 grid of samples.  In order to aid in calculations, one of
the first things ISL does is to compute :c:member:`isl_surf.phys_level0_sa`
which gives the dimensions of the base miplevel of the surface in samples.  The
type of :c:member:`isl_surf.phys_level0_sa` is :c:struct:`isl_extent4d`
which allows us to express both the array and interleaved cases. Most of the
calculations of how the different miplevels and array slices are laid out is
done in terms of samples.

Next, we have surface **elements**.  An element is the basic unit of actual
surface memory. For multisampled textures, an element is equal to a single
sample. For block compressed textures, an element corresponds to an entire
compression block. The conversion from samples to elements is given by dividing
by the block width and block height of the surface format. This is true
regardless of whether or not the surface is multisampled; for multisampled
compressed textures (these exist for certain auxiliary formats), the block
width and block height are expressed in samples. Pixels cannot be converted
directly to elements or vice versa; any conversion between pixels and elements
*must* go through samples.

The final surface unit is **tiles**. A tile is a large rectangular block of
surface data that all fits in a single contiguous block of memory (usually a 4K
or 64K page, depending on tile format). Tiles are used to provide an
arrangement of the data in memory that yields better cache performance. The
size of a tile is always specified in surface elements.
