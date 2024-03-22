Name

    MESA_texture_signed_rgba

Name Strings

    GL_MESA_texture_signed_rgba

Contact



Notice



IP Status

    No known IP issues

Status



Version

    0.3, 2009-03-24

Number

    Not assigned ?

Dependencies

    Written based on the wording of the OpenGL 2.0 specification.

    This extension trivially interacts with ARB_texture_float.
    This extension shares some language with ARB_texture_compression_rgtc
    but does not depend on it.

Overview

    OpenGL prior to 3.1 does not support any signed texture formats.
    ARB_texture_compression_rgtc introduces some compressed red and
    red_green signed formats but no uncompressed ones, which might
    still be useful. NV_texture_shader adds signed texture formats,
    but also a lot of functionality which has been superseded by fragment
    shaders.
    It is usually possible to get the same functionality
    using a unsigned format by doing scale and bias in a shader, but this
    is undesirable since modern hardware has direct support for this.
    This extension adds a signed 4-channel texture format by backporting
    the relevant features from OpenGL 3.1, as a means to support this in
    OpenGL implementations only supporting older versions.

Issues

    1) What should this extension be called?

       RESOLVED: MESA_texture_signed_rgba seems reasonable.
       The rgba part is there because only 4 channel format is supported.


    2) Should the full set of signed formats (alpha, luminance, rgb, etc.)
       be supported?

       RESOLVED: NO. To keep this extension simple, only add the most
       universal format, rgba. alpha/luminance can't be trivially supported
       since OpenGL 3.1 does not support them any longer, and there is some
       implied dependency on ARB_texture_rg for red/red_green formats so
       avoid all this. Likewise, only 8 bits per channel is supported.


    3) Should this extension use new enums for the texture formats?

       RESOLVED: NO. Same enums as those used in OpenGL 3.1.


    4) How are signed integer values mapped to floating-point values?

       RESOLVED: Same as described in issue 5) of
       ARB_texture_compression_rgtc (quote):
       A signed 8-bit two's complement value X is computed to
       a floating-point value Xf with the formula:

                { X / 127.0, X > -128
           Xf = {
                { -1.0,      X == -128

       This conversion means -1, 0, and +1 are all exactly representable,
       however -128 and -127 both map to -1.0.  Mapping -128 to -1.0
       avoids the numerical awkwardness of have a representable value
       slightly more negative than -1.0.

       This conversion is intentionally NOT the "byte" conversion listed
       in Table 2.9 for component conversions.  That conversion says:

           Xf = (2*X + 1) / 255.0

       The Table 2.9 conversion is incapable of exactly representing
       zero.

       (Difference to ARB_texture_compression_rgtc):
       This is the same mapping as OpenGL 3.1 uses.
       This is also different to what NV_texture_shader used.
       The above mapping should be considered the reference, but there
       is some leeway so other mappings are allowed for implementations which
       cannot do this. Particularly the mapping given in NV_texture_shader or
       the standard OpenGL byte/float mapping is considered acceptable too, as
       might be a mapping which represents -1.0 by -128, 0.0 by 0 and 1.0 by
       127 (that is, uses different scale factors for negative and positive
       numbers).
       Also, it is ok to store incoming GL_BYTE user data as-is, without
       converting to GL_FLOAT (using the standard OpenGL float/byte mapping)
       and converting back (using the mapping described here).
       Other than those subtle issues there are no other non-standard
       conversions used, so when using for instance CopyTexImage2D with
       a framebuffer clamped to [0,1] all converted numbers will be in the range
       [0, 127] (and not scaled and biased).


    5) How will signed components resulting from RGBA8_SNORM texture
       fetches interact with fragment coloring?

       RESOLVED: Same as described in issue 6) of
       ARB_texture_compression_rgtc (quote):
       The specification language for this extension is silent
       about clamping behavior leaving this to the core specification
       and other extensions.  The clamping or lack of clamping is left
       to the core specification and other extensions.

       For assembly program extensions supporting texture fetches
       (ARB_fragment_program, NV_fragment_program, NV_vertex_program3,
       etc.) or the OpenGL Shading Language, these signed formats will
       appear as expected with unclamped signed components as a result
       of a texture fetch instruction.

       If ARB_color_buffer_float is supported, its clamping controls
       will apply.

       NV_texture_shader extension, if supported, adds support for
       fixed-point textures with signed components and relaxed the
       fixed-function texture environment clamping appropriately.  If the
       NV_texture_shader extension is supported, its specified behavior
       for the texture environment applies where intermediate values
       are clamped to [-1,1] unless stated otherwise as in the case
       of explicitly clamped to [0,1] for GL_COMBINE.  or clamping the
       linear interpolation weight to [0,1] for GL_DECAL and GL_BLEND.

       Otherwise, the conventional core texture environment clamps
       incoming, intermediate, and output color components to [0,1].

       This implies that the conventional texture environment
       functionality of unextended OpenGL 1.5 or OpenGL 2.0 without
       using GLSL (and with none of the extensions referred to above)
       is unable to make proper use of the signed texture formats added
       by this extension because the conventional texture environment
       requires texture source colors to be clamped to [0,1].  Texture
       filtering of these signed formats would be still signed, but
       negative values generated post-filtering would be clamped to
       zero by the core texture environment functionality.  The
       expectation is clearly that this extension would be co-implemented
       with one of the previously referred to extensions or used with
       GLSL for the new signed formats to be useful.


    6) Should the RGBA_SNORM tokens also be accepted by CopyTexImage
       functions?

       RESOLVED: YES.


    7) What to do with GetTexParameter if ARB_texture_float is supported,
       in particular what datatype should this return for TEXTURE_RED_TYPE_ARB,
       TEXTURE_GREEN_TYPE_ARB, TEXTURE_BLUE_TYPE_ARB, TEXTURE_ALPHA_TYPE_ARB?

       RESOLVED: ARB_texture_float states type is either NONE,
       UNSIGNED_NORMALIZED_ARB, or FLOAT. This extension adds a new enum,
       SIGNED_NORMALIZED, which will be returned accordingly. This is the
       same behaviour as in OpenGL 3.1.


New Tokens


    Accepted by the <internalformat> parameter of
    TexImage1D, TexImage2D, TexImage3D, CopyTexImage1D, and CopyTexImage2D:

        RGBA_SNORM                                0x8F93
        RGBA8_SNORM                               0x8F97

    Returned by the <params> parameter of GetTexLevelParameter:

        SIGNED_NORMALIZED                         0x8F9C


Additions to Chapter 3 of the OpenGL 2.0 Specification (Rasterization):

 -- Section 3.8.1, Texture Image Specification

    Add to Table 3.16 (page 154): Sized internal formats

    Sized             Base             R    G    B    A    L    I    D
    Internal Format   Internal Format bits bits bits bits bits bits bits
    ---------------   --------------- ---- ---- ---- ---- ---- ---- ----
    RGBA8_SNORM       RGBA             8    8    8    8    0    0    0


Dependencies on ARB_texture_float extension:

    If ARB_texture_float is supported, GetTexParameter queries with <value>
    of TEXTURE_RED_TYPE_ARB, TEXTURE_GREEN_TYPE_ARB, TEXTURE_BLUE_TYPE_ARB or
    TEXTURE_ALPHA_TYPE_ARB return SIGNED_NORMALIZED if
    the base internal format is RGBA_SNORM.
