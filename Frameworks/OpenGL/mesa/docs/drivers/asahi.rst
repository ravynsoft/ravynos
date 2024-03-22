Asahi
=====

The Asahi driver aims to provide an OpenGL implementation for the Apple M1.

Wrap (macOS only)
-----------------

Mesa includes a library that wraps the key IOKit entrypoints used in the macOS
UABI for AGX. The wrapped routines print information about the kernel calls made
and dump work submitted to the GPU using agxdecode. This facilitates
reverse-engineering the hardware, as glue to get at the "interesting" GPU
memory.

The library is only built if ``-Dtools=asahi`` is passed. It builds a single
``wrap.dylib`` file, which should be inserted into a process with the
``DYLD_INSERT_LIBRARIES`` environment variable.

For example, to trace an app ``./app``, run:

   DYLD_INSERT_LIBRARIES=~/mesa/build/src/asahi/lib/libwrap.dylib ./app

Hardware varyings
-----------------

At an API level, vertex shader outputs need to be interpolated to become
fragment shader inputs. This process is logically pipelined in AGX, with a value
traveling from a vertex shader to remapping hardware to coefficient register
setup to the fragment shader to the iterator hardware. Each stage is described
below.

Vertex shader
`````````````

A vertex shader (running on the :term:`Unified Shader Cores`) outputs varyings with the
``st_var`` instruction. ``st_var`` takes a *vertex output index* and a 32-bit
value. The maximum number of *vertex outputs* is specified as the "output count"
of the shader in the "Bind Vertex Pipeline" packet. The value may be interpreted
consist of a single 32-bit value or an aligned 16-bit register pair, depending
on whether interpolation should happen at 32-bit or 16-bit. Vertex outputs are
indexed starting from 0, with the *vertex position* always coming first, the
32-bit user varyings coming next with perspective, flat, and linear interpolated
varyings grouped in that order, then 16-bit user varyings with the same groupings,
and finally *point size* and *clip distances* at the end if present. Note that
*clip distances* are not accessible from the fragment shader; if the fragment
shader needs to read the interpolated clip distance, the vertex shader must
*also* write the clip distance values to a user varying for the fragment shader
to interpolate. Also note there is no clip plane enable mask anywhere; that must
lowered for APIs that require this (OpenGL but not Vulkan).

.. list-table:: Ordering of vertex outputs with all outputs used
   :widths: 25 75
   :header-rows: 1

   * - Size (words)
     - Value
   * - 4
     - Vertex position
   * - 1
     - 32-bit smooth varying 0
   * -
     - ...
   * - 1
     - 32-bit smooth varying m
   * - 1
     - 32-bit flat varying 0
   * -
     - ...
   * - 1
     - 32-bit flat varying n
   * - 1
     - 32-bit linear varying 0
   * -
     - ...
   * - 1
     - 32-bit linear varying o
   * - 1
     - Packed pair of 16-bit smooth varyings 0
   * -
     - ...
   * - 1
     - Packed pair of 16-bit smooth varyings p
   * - 1
     - Packed pair of 16-bit flat varyings 0
   * -
     - ...
   * - 1
     - Packed pair of 16-bit flat varyings q
   * - 1
     - Packed pair of 16-bit linear varyings 0
   * -
     - ...
   * - 1
     - Packed pair of 16-bit linear varyings r
   * - 1
     - Point size
   * - 1
     - Clip distance for plane 0
   * -
     - ...
   * - 1
     - Clip distance for plane 15

Remapping
`````````

Vertex outputs are remapped to varying slots to be interpolated.
The output of remapping consists of the following items: the *W* fragment
coordinate, the *Z* fragment coordinate, user varyings in the vertex
output order. *Z* may be omitted, but *W* may not be. This remapping is
configured by the "Output select" word.

.. list-table:: Ordering of remapped slots
   :widths: 25 75
   :header-rows: 1

   * - Index
     - Value
   * - 0
     - Fragment coord W
   * - 1
     - Fragment coord Z
   * - 2
     - 32-bit varying 0
   * -
     - ...
   * - 2 + m
     - 32-bit varying m
   * - 2 + m + 1
     - Packed pair of 16-bit varyings 0
   * -
     - ...
   * - 2 + m + n + 1
     - Packed pair of 16-bit varyings n

Coefficient registers
`````````````````````

The fragment shader does not see the physical slots.
Instead, it references varyings through *coefficient registers*. A coefficient
register is a register allocated constant for all fragment shader invocations in
a given polygon. Physically, it contains the values output by the vertex shader
for each vertex of the polygon. Coefficient registers are preloaded with values
from varying slots. This preloading appears to occur in fixed function hardware,
a simplification from PowerVR which requires a specialized program for the
programmable data sequencer to do the preload.

The "Bind fragment pipeline" packet points to coefficient register bindings,
preceded by a header. The header contains the number of 32-bit varying slots. As
the *W* slot is always present, this field is always nonzero. Slots whose index
is below this count are treated as 32-bit. The remaining slots are treated as
16-bits.

The header also contains the total number of coefficient registers bound.

Each binding that follows maps a (vector of) varying slots to a (consecutive)
coefficient registers. Some details about the varying (perspective
interpolation, flat shading, point sprites) are configured here.

Coefficient registers may be ordered the same as the internal varying slots.
However, this may be inconvenient for some APIs that require a separable shader
model. For these APIs, the flexibility to mix-and-match slots and coefficient
registers allows mixing shaders without shader variants. In that case, the
bindings should be generated outside of the compiler. For simple APIs where the
bindings are fixed and known at compile-time, the bindings could be generated
within the compiler.

Fragment shader
```````````````

In the fragment shader, coefficient registers, identified by the prefix ``cf``
followed by a decimal index, act as opaque handles to varyings. For flat
shading, coefficient registers may be loaded into general registers with the
``ldcf`` instruction. For smooth shading, the coefficient register corresponding
to the desired varying is passed as an argument to the "iterate" instruction
``iter`` in order to "iterate" (interpolate) a varying. As perspective correct
interpolation also requires the W component of the fragment coordinate, the
coefficient register for W is passed as a second argument. As an example, if
there's a single varying to interpolate, an instruction like ``iter r0, cf1, cf0``
is used.

Iterator
````````

To actually interpolate varyings, AGX provides fixed-function iteration hardware
to multiply the specified coefficient registers with the required barycentrics,
producing an interpolated value, hence the name "coefficient register". This
operation is purely mathematical and does not require any memory access, as
the required coefficients are preloaded before the shader begins execution.
That means the iterate instruction executes in constant time, does not signal
a data fence, and does not require the shader to wait on a data fence before
using the value.

Image layouts
-------------

AGX supports several image layouts, described here. To work with image layouts
in the drivers, use the ail library, located in ``src/asahi/layout``.

The simplest layout is **strided linear**. Pixels are stored in raster-order in
memory with a software-controlled stride. Strided linear images are useful for
working with modifier-unaware window systems, however performance will suffer.
Strided linear images have numerous limitations:

- Strides must be a multiple of 16 bytes.
- Strides must be nonzero. For 1D images where the stride is logically
  irrelevant, ail will internally select the minimal stride.
- Only 1D and 2D images may be linear. In particular, no 3D or cubemaps.
- Array texture may not be linear. No 2D arrays or cubemap arrays.
- 2D images must not be mipmapped.
- Block-compressed formats and multisampled images are unsupported. Elements of
  a strided linear image are simply pixels.

With these limitations, addressing into a strided linear image is as simple as

.. math::

   \text{address} = (y \cdot \text{stride}) + (x \cdot \text{bytes per pixel})

In practice, this suffices for window system integration and little else.

The most common uncompressed layout is **twiddled**. The image is divided into
power-of-two sized tiles. The tiles themselves are stored in raster-order.
Within each tile, elements (pixels/blocks) are stored in Morton (Z) order.

The tile size used depends on both the image size and the block size of the
image format. For large images, :math:`n \times n` or :math:`2n \times n` tiles
are used (:math:`n` power-of-two). :math:`n` is such that each page contains
exactly one tile. Only power-of-two block sizes are supported in hardware,
ensuring such a tile size always exists. The hardware uses 16 KiB pages, so tile
sizes are as follows:

.. list-table:: Tile sizes for large images
   :widths: 50 50
   :header-rows: 1

   * - Bytes per block
     - Tile size
   * - 1
     - 128 x 128
   * - 2
     - 128 x 64
   * - 4
     - 64 x 64
   * - 8
     - 64 x 32
   * - 16
     - 32 x 32

The dimensions of large images are rounded up to be multiples of the tile size.
In addition, non-power-of-two large images have extra padding tiles when
mipmapping is used, see below.

That rounding would waste a great deal of memory for small images. If
an image is smaller than this tile size, a smaller tile size is used to reduce
the memory footprint. For small images, the tile size is :math:`m \times m`
where

.. math::

   m = 2^{\lceil \log_2( \min \{ \text{width}, \text{ height} \}) \rceil}

In other words, small images use the smallest square power-of-two tile such that
the image's minor axis fits in one tile.

For mipmapped images, tile sizes are determined independently for each level.
Typically, the first levels of an image are "large" and the remaining levels are
"small". This scheme reduces the memory footprint of mipmapping, compared to a
fixed tile size for the whole image. Each mip level are padded to fill at least
one cache line (128 bytes), ensure no cache line contains multiple mip levels.

There is a wrinkle: the dimensions of large mip levels in tiles are determined
by the dimensions of level 0. For power-of-two images, the two calculations are
equivalent. However, they differ subtly for non-power-of-two images. To
determine the number of tiles to allocate for level :math:`l`, the number of
tiles for level 0 should be right-shifted by :math:`2l`. That appears to divide
by :math:`2^l` in both width and height, matching the definition of mipmapping,
however it rounds down incorrectly. To compensate, the level contains one extra
row, column, or both (with the corner) as required if any of the first :math:`l`
levels were rounded down. This hurt the memory footprint. However, it means
non-power-of-two integer multiplication is only required for level 0.
Calculating the sizes for subsequent levels requires only addition and bitwise
math. That simplifies the hardware (but complicates software).

A 2D image consists of a full miptree (constructed as above) rounded up to the
page size (16 KiB).

3D images consist simply of an array of 2D layers (constructed as above). That
means cube maps, 2D arrays, cube map arrays, and 3D images all use the same
layout. The only difference is the number of layers. Notably, 3D images (like
``GL_TEXTURE_3D``) reserve space even for mip levels that do not exist
logically. These extra levels pad out layers of 3D images to the size of the
first layer, simplifying layout calculations for both software and hardware.
Although the padding is logically unnecessary, it wastes little space compared
to the sizes of large mipmapped 3D textures.

drm-shim (Linux only)
---------------------

Mesa includes a library that mocks out the DRM UABI used by the Asahi driver
stack, allowing the Mesa driver to run on non-M1 Linux hardware. This can be
useful for exercising the compiler. To build, use options:

::

   -Dgallium-drivers=asahi -Dtools=drm-shim

Then run an OpenGL workload with environment variable:

.. code-block:: console

   LD_PRELOAD=~/mesa/build/src/asahi/drm-shim/libasahi_noop_drm_shim.so

For example to compile a shader with shaderdb and print some statistics along
with the IR:

.. code-block:: console

   ~/shader-db$ AGX_MESA_DEBUG=shaders,shaderdb ASAHI_MESA_DEBUG=precompile LIBGL_DRIVERS_PATH=~/lib/dri/ LD_PRELOAD=~/mesa/build/src/asahi/drm-shim/libasahi_noop_drm_shim.so ./run shaders/glmark/1-12.shader_test

The drm-shim implementation for Asahi is located in ``src/asahi/drm-shim``. The
drm-shim implementation there should be updated as new UABI is added.

Hardware glossary
-----------------

AGX is a tiled renderer descended from the PowerVR architecture. Some hardware
concepts used in PowerVR GPUs appear in AGX.

.. glossary:: :sorted:

   VDM
   Vertex Data Master
      Dispatches vertex shaders.

   PDM
   Pixel Data Master
      Dispatches pixel shaders.

   CDM
   Compute Data Master
      Dispatches compute kernels.

   USC
   Unified Shader Cores
      A unified shader core is a small cpu that runs shader code. The core is
      unified because a single ISA is used for vertex, pixel and compute
      shaders. This differs from older GPUs where the vertex, fragment and
      compute have separate ISAs for shader stages.

   PPP
   Primitive Processing Pipeline
      The Primitive Processing Pipeline is a hardware unit that does primitive
      assembly. The PPP is between the :term:`VDM` and :term:`ISP`.

   ISP
   Image Synthesis Processor
      The Image Synthesis Processor is responsible for the rasterization stage
      of the rendering pipeline.

   PBE
   Pixel BackEnd
      Hardware unit which writes to color attachements and images. Also the
      name for a descriptor passed to :term:`PBE` instructions.
