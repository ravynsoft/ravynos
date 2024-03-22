.. _sampler:

Sampler
=======

Texture units have many options for selecting texels from loaded textures;
this state controls an individual texture unit's texel-sampling settings.

Texture coordinates are always treated as four-dimensional, and referred to
with the traditional (S, T, R, Q) notation.

Members
-------

wrap_s
    How to wrap the S coordinate. One of PIPE_TEX_WRAP_*.
wrap_t
    How to wrap the T coordinate. One of PIPE_TEX_WRAP_*.
wrap_r
    How to wrap the R coordinate. One of PIPE_TEX_WRAP_*.

The wrap modes are:

* ``PIPE_TEX_WRAP_REPEAT``: Standard coord repeat/wrap-around mode.
* ``PIPE_TEX_WRAP_CLAMP_TO_EDGE``: Clamp coord to edge of texture, the border
  color is never sampled.
* ``PIPE_TEX_WRAP_CLAMP_TO_BORDER``: Clamp coord to border of texture, the
  border color is sampled when coords go outside the range [0,1].
* ``PIPE_TEX_WRAP_CLAMP``: The coord is clamped to the range [0,1] before
  scaling to the texture size.  This corresponds to the legacy OpenGL GL_CLAMP
  texture wrap mode.  Historically, this mode hasn't acted consistently across
  all graphics hardware.  It sometimes acts like CLAMP_TO_EDGE or
  CLAMP_TO_BORDER.  The behavior may also vary depending on linear vs.
  nearest sampling mode.
* ``PIPE_TEX_WRAP_MIRROR_REPEAT``: If the integer part of the coordinate
  is odd, the coord becomes (1 - coord).  Then, normal texture REPEAT is
  applied to the coord.
* ``PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE``: First, the absolute value of the
  coordinate is computed.  Then, regular CLAMP_TO_EDGE is applied to the coord.
* ``PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER``: First, the absolute value of the
  coordinate is computed.  Then, regular CLAMP_TO_BORDER is applied to the
  coord.
* ``PIPE_TEX_WRAP_MIRROR_CLAMP``: First, the absolute value of the coord is
  computed.  Then, regular CLAMP is applied to the coord.


min_img_filter
    The image filter to use when minifying texels. One of PIPE_TEX_FILTER_*.
mag_img_filter
    The image filter to use when magnifying texels. One of PIPE_TEX_FILTER_*.

The texture image filter modes are:

* ``PIPE_TEX_FILTER_NEAREST``: One texel is fetched from the texture image
  at the texture coordinate.
* ``PIPE_TEX_FILTER_LINEAR``: Two, four or eight texels (depending on the
  texture dimensions; 1D/2D/3D) are fetched from the texture image and
  linearly weighted and blended together.

min_mip_filter
    The filter to use when minifying mipmapped textures. One of
    PIPE_TEX_MIPFILTER_*.

The texture mip filter modes are:

* ``PIPE_TEX_MIPFILTER_NEAREST``: A single mipmap level/image is selected
  according to the texture LOD (lambda) value.
* ``PIPE_TEX_MIPFILTER_LINEAR``: The two mipmap levels/images above/below
  the texture LOD value are sampled from.  The results of sampling from
  those two images are blended together with linear interpolation.
* ``PIPE_TEX_MIPFILTER_NONE``: Mipmap filtering is disabled.  All texels
  are taken from the level 0 image.


compare_mode
    If set to PIPE_TEX_COMPARE_R_TO_TEXTURE, the result of texture sampling
    is not a color but a true/false value which is the result of comparing the
    sampled texture value (typically a Z value from a depth texture) to the
    texture coordinate's R component.
    If set to PIPE_TEX_COMPARE_NONE, no comparison calculation is performed.
compare_func
    The inequality operator used when compare_mode=1.  One of PIPE_FUNC_x.
unnormalized_coords
    If set, incoming texture coordinates are used as-is to compute
    texel addresses. When set, only a subset of the texture wrap
    modes are allowed: PIPE_TEX_WRAP_CLAMP, PIPE_TEX_WRAP_CLAMP_TO_EDGE,
    and PIPE_TEX_WRAP_CLAMP_TO_BORDER. If unset, the incoming texture
    coordinates are assumed to be normalized to the range [0, 1],
    and will be scaled by the texture dimensions to compute texel
    addresses.
lod_bias
    Bias factor which is added to the computed level of detail.
    The normal level of detail is computed from the partial derivatives of
    the texture coordinates and/or the fragment shader TEX/TXB/TXL
    instruction.
min_lod
    Minimum level of detail, used to clamp LOD after bias.  The LOD values
    correspond to mipmap levels where LOD=0 is the level 0 mipmap image.
max_lod
    Maximum level of detail, used to clamp LOD after bias.
border_color
    Color union used for texel coordinates that are outside the [0,width-1],
    [0, height-1] or [0, depth-1] ranges. Interpreted according to sampler
    view format, unless the driver reports
    PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK, in which case special care has to be
    taken (see description of the cap).
max_anisotropy
    Maximum anisotropy ratio to use when sampling from textures.  For example,
    if max_anisotropy=4, a region of up to 1 by 4 texels will be sampled.
    Set to zero to disable anisotropic filtering.  Any other setting enables
    anisotropic filtering, however it's not unexpected some drivers only will
    change their filtering with a setting of 2 and higher.
seamless_cube_map
    If set, the bilinear filter of a cube map may take samples from adjacent
    cube map faces when sampled near a texture border to produce a seamless
    look.
