Name

    MESA_texture_array

Name Strings

    GL_MESA_texture_array

Contact

    Ian Romanick, IBM (idr 'at' us.ibm.com)

IP Status

    No known IP issues.

Status

    DEPRECATED - Support removed in Mesa 10.1.

Version


Number

    TBD

Dependencies

    OpenGL 1.2 or GL_EXT_texture3D is required.

    Support for ARB_fragment_program is assumed, but not required.

    Support for ARB_fragment_program_shadow is assumed, but not required.

    Support for EXT_framebuffer_object is assumed, but not required.

    Written based on the wording of the OpenGL 2.0 specification and
    ARB_fragment_program_shadow but not dependent on them.

Overview

    There are a number of circumstances where an application may wish to
    blend two textures out of a larger set of textures.  Moreover, in some
    cases the selected textures may vary on a per-fragment basis within
    a polygon.  Several examples include:

       1. High dynamic range textures.  The application stores several
       different "exposures" of an image as different textures.  On a
       per-fragment basis, the application selects which exposures are
       used.

       2. A terrain engine where the altitude of a point determines the
       texture applied to it.  If the transition is from beach sand to
       grass to rocks to snow, the application will store each texture
       in a different texture map, and dynamically select which two
       textures to blend at run-time.

       3. Storing short video clips in textures.  Each depth slice is a
       single frame of video.

    Several solutions to this problem have been proposed, but they either
    involve using a separate texture unit for each texture map or using 3D
    textures without mipmaps.  Both of these options have major drawbacks.

    This extension provides a third alternative that eliminates the major
    drawbacks of both previous methods.  A new texture target,
    TEXTURE_2D_ARRAY, is added that functions identically to TEXTURE_3D in
    all aspects except the sizes of the non-base level images.  In
    traditional 3D texturing, the size of the N+1 LOD is half the size
    of the N LOD in all three dimensions.  For the TEXTURE_2D_ARRAY target,
    the height and width of the N+1 LOD is halved, but the depth is the
    same for all levels of detail. The texture then becomes an array of
    2D textures.  The per-fragment texel is selected by the R texture
    coordinate.

    References:

        https://www.opengl.org/discussion_boards/cgi_directory/ultimatebb.cgi?ubb=get_topic;f=3;t=011557
        https://www.opengl.org/discussion_boards/cgi_directory/ultimatebb.cgi?ubb=get_topic;f=3;t=000516
        https://www.opengl.org/discussion_boards/cgi_directory/ultimatebb.cgi?ubb=get_topic;f=3;t=011903
        http://www.delphi3d.net/articles/viewarticle.php?article=terraintex.htm

New Procedures and Functions

    All functions come directly from EXT_texture_array.

    void FramebufferTextureLayerEXT(enum target, enum attachment,
                                    uint texture, int level, int layer);

New Tokens

    All token names and values come directly from EXT_texture_array.

    Accepted by the <cap> parameter of Enable, Disable, and IsEnabled, by
    the <pname> parameter of GetBooleanv, GetIntegerv, GetFloatv, and
    GetDoublev, and by the <target> parameter of TexImage3D, GetTexImage,
    GetTexLevelParameteriv, GetTexLevelParameterfv, GetTexParameteriv, and
    GetTexParameterfv:

        TEXTURE_1D_ARRAY_EXT                            0x8C18
        TEXTURE_2D_ARRAY_EXT                            0x8C1A

    Accepted by the <target> parameter of TexImage2D, TexSubImage2D,
    CopyTexImage2D, CopyTexSubImage2D, CompressedTexImage2D,
    CompressedTexSubImage2D, GetTexLevelParameteriv, and 
    GetTexLevelParameterfv:

        TEXTURE_1D_ARRAY_EXT
        PROXY_TEXTURE_1D_ARRAY_EXT                      0x8C19

    Accepted by the <target> parameter of TexImage3D, TexSubImage3D,
    CopyTexSubImage3D, CompressedTexImage3D, CompressedTexSubImage3D,
    GetTexLevelParameteriv, and GetTexLevelParameterfv:

        TEXTURE_2D_ARRAY_EXT
        PROXY_TEXTURE_2D_ARRAY_EXT                      0x8C1B

    Accepted by the <pname> parameter of GetBooleanv, GetIntegerv,
    GetFloatv, and GetDoublev

        TEXTURE_BINDING_1D_ARRAY_EXT                    0x8C1C
        TEXTURE_BINDING_2D_ARRAY_EXT                    0x8C1D
        MAX_ARRAY_TEXTURE_LAYERS_EXT                    0x88FF

    Accepted by the <param> parameter of TexParameterf, TexParameteri,
    TexParameterfv, and TexParameteriv when the <pname> parameter is
    TEXTURE_COMPARE_MODE_ARB:

        COMPARE_REF_DEPTH_TO_TEXTURE_EXT                0x884E

    (Note:  COMPARE_REF_DEPTH_TO_TEXTURE_EXT is simply an alias for the
    existing COMPARE_R_TO_TEXTURE token in OpenGL 2.0; the alternate name
    reflects the fact that the R coordinate is not always used.)

    Accepted by the <internalformat> parameter of TexImage3D and
    CompressedTexImage3D, and by the <format> parameter of
    CompressedTexSubImage3D:

        COMPRESSED_RGB_S3TC_DXT1_EXT
        COMPRESSED_RGBA_S3TC_DXT1_EXT
        COMPRESSED_RGBA_S3TC_DXT3_EXT
        COMPRESSED_RGBA_S3TC_DXT5_EXT

    Accepted by the <pname> parameter of
    GetFramebufferAttachmentParameterivEXT:

        FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT          0x8CD4

    (Note:  FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER is simply an alias for the
    FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT token provided in
    EXT_framebuffer_object.  This extension generalizes the notion of
    "<zoffset>" to include layers of an array texture.)

Additions to Chapter 2 of the OpenGL 2.0 Specification (OpenGL Operation)

    None

Additions to Chapter 3 of the OpenGL 2.0 Specification (Rasterization)

    -- Section 3.8.1 "Texture Image Specification"

       Change the first paragraph (page 150) to say (spec changes identical to
       EXT_texture_array):

       "The command

         void TexImage3D(enum target, int level, int internalformat,
                         sizei width, sizei height, sizei depth, int border,
                         enum format, enum type, void *data);

       is used to specify a three-dimensional texture image. target must be one
       one of TEXTURE_3D for a three-dimensional texture or
       TEXTURE_2D_ARRAY_EXT for an two-dimensional array texture.
       Additionally, target may be either PROXY_TEXTURE_3D for a
       three-dimensional proxy texture, or PROXY_TEXTURE_2D_ARRAY_EXT for a
       two-dimensional proxy array texture."

       Change the fourth paragraph on page 151 to say (spec changes identical
       to EXT_texture_array):

       "Textures with a base internal format of DEPTH_COMPONENT are supported
       by texture image specification commands only if target is TEXTURE_1D,
       TEXTURE_2D, TEXTURE_1D_ARRAY_EXT, TEXTURE_2D_ARRAY_EXT,
       PROXY_TEXTURE_1D, PROXY_TEXTURE_2D, PROXY_TEXTURE_1D_ARRAY_EXT, or
       PROXY_TEXTURE_2D_ARRAY_EXT. Using this format in conjunction with any
       other target will result in an INVALID_OPERATION error."


       Change the fourth paragraph on page 156 to say (spec changes identical
       to EXT_texture_array):
       
       "The command

         void TexImage2D(enum target, int level,
                         int internalformat, sizei width, sizei height,
                         int border, enum format, enum type, void *data);

       is used to specify a two-dimensional texture image. target must be one
       of TEXTURE_2D for a two-dimensional texture, TEXTURE_1D_ARRAY_EXT for a
       one-dimensional array texture, or one of TEXTURE_CUBE_MAP_POSITIVE_X,
       TEXTURE_CUBE_MAP_NEGATIVE_X, TEXTURE_CUBE_MAP_POSITIVE_Y,
       TEXTURE_CUBE_MAP_NEGATIVE_Y, TEXTURE_CUBE_MAP_POSITIVE_Z, or
       TEXTURE_CUBE_MAP_NEGATIVE_Z for a cube map texture. Additionally,
       target may be either PROXY_TEXTURE_2D for a two-dimensional proxy
       texture, PROXY_TEXTURE_1D_ARRAY_EXT for a one-dimensional proxy array
       texture, or PROXY TEXTURE_CUBE_MAP for a cube map proxy texture in the
       special case discussed in section 3.8.11.  The other parameters match
       the corresponding parameters of TexImage3D.

       For the purposes of decoding the texture image, TexImage2D is
       equivalent to calling TexImage3D with corresponding arguments and depth
       of 1, except that

         * The border depth, d_b, is zero, and the depth of the image is
           always 1 regardless of the value of border. 

         * The border height, h_b, is zero if <target> is
           TEXTURE_1D_ARRAY_EXT, and <border> otherwise.

         * Convolution will be performed on the image (possibly changing its
           width and height) if SEPARABLE 2D or CONVOLUTION 2D is enabled.

         * UNPACK SKIP IMAGES is ignored."

    -- Section 3.8.2 "Alternate Texture Image Specification Commands"

       Change the second paragraph (page 159) (spec changes identical
       to EXT_texture_array):

       "The command

         void CopyTexImage2D(enum target, int level,
                             enum internalformat, int x, int y, sizei width,
                             sizei height, int border);

       defines a two-dimensional texture image in exactly the manner of
       TexImage2D, except that the image data are taken from the framebuffer
       rather than from client memory. Currently, target must be one of
       TEXTURE_2D, TEXTURE_1D_ARRAY_EXT, TEXTURE_CUBE_MAP_POSITIVE_X,
       TEXTURE_CUBE_MAP_NEGATIVE_X, TEXTURE_CUBE MAP_POSITIVE_Y,
       TEXTURE_CUBE_MAP_NEGATIVE_Y, TEXTURE_CUBE_MAP_POSITIVE_Z, or
       TEXTURE_CUBE_MAP_NEGATIVE_Z.


       Change the last paragraph on page 160 to say (spec changes identical
       to EXT_texture_array):

       "Currently the target arguments of TexSubImage1D and CopyTexSubImage1D
       must be TEXTURE_1D, the target arguments of TexSubImage2D and
       CopyTexSubImage2D must be one of TEXTURE_2D, TEXTURE_1D_ARRAY_EXT,
       TEXTURE_CUBE_MAP_POSITIVE_X, TEXTURE_CUBE_MAP_NEGATIVE_X,
       TEXTURE_CUBE_MAP_POSITIVE_Y, TEXTURE_CUBE_MAP_NEGATIVE_Y,
       TEXTURE_CUBE_MAP_POSITIVE_Z, or TEXTURE_CUBE_MAP_NEGATIVE_Z, and the
       target arguments of TexSubImage3D and CopyTexSubImage3D must be
       TEXTURE_3D or TEXTURE_2D_ARRAY_EXT. ..."


    -- Section 3.8.4 "Texture Parameters"

       Change the first paragraph (page 166) to say:

       "Various parameters control how the texel array is treated when
       specified or changed, and when applied to a fragment. Each parameter is
       set by calling

         void TexParameter{if}(enum target, enum pname, T param); 
         void TexParameter{if}v(enum target, enum pname, T params);

       target is the target, either TEXTURE_1D, TEXTURE_2D, TEXTURE_3D,
       TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, or TEXTURE_2D_ARRAY_EXT."


    -- Section 3.8.8 "Texture Minification" in the section "Scale Factor and Level of Detail"

       Change the first paragraph (page 172) to say:

       "Let s(x,y) be the function that associates an s texture coordinate
       with each set of window coordinates (x,y) that lie within a primitive;
       define t(x,y) and r(x,y) analogously.  Let u(x,y) = w_t * s(x,y),
       v(x,y) = h_t * t(x,y), and w(x,y) = d_t * r(x,y), where w_t, h_t,
       and d_t are as defined by equations 3.15, 3.16, and 3.17 with
       w_s, h_s, and d_s equal to the width, height, and depth of the
       image array whose level is level_base.  For a one-dimensional
       texture or a one-dimensional array texture, define v(x,y) = 0 and
       w(x,y) = 0; for a two-dimensional texture or a two-dimensional array
       texture, define w(x,y) = 0..."

    -- Section 3.8.8 "Texture Minification" in the section "Mipmapping"

       Change the third paragraph (page 174) to say:
       
       "For a two-dimensional texture, two-dimensional array texture, or
       cube map texture,"

       Change the fourth paragraph (page 174) to say:

       "And for a one-dimensional texture or a one-dimensional array texture,"

       After the first paragraph (page 175) add:

       "For one-dimensional array textures, h_b and d_b are treated as 1,
       regardless of the actual values, when performing mipmap calculations.
       For two-dimensional array textures, d_b is always treated as one,
       regardless of the actual value, when performing mipmap calculations."

    -- Section 3.8.8 "Automatic Mipmap Generation" in the section "Mipmapping"

       Change the third paragraph (page 176) to say (spec changes identical
       to EXT_texture_array):

       "The contents of the derived arrays are computed by repeated, filtered
       reduction of the level_base array.  For one- and two-dimensional array
       textures, each layer is filtered independently.  ..."

    -- Section 3.8.8 "Manual Mipmap Generation" in the section "Mipmapping"

       Change first paragraph to say (spec changes identical to
       EXT_texture_array):

       "Mipmaps can be generated manually with the command

         void GenerateMipmapEXT(enum target);

       where <target> is one of TEXTURE_1D, TEXTURE_2D, TEXTURE_CUBE_MAP,
       TEXTURE_3D, TEXTURE_1D_ARRAY, or TEXTURE_2D_ARRAY.  Mipmap generation
       affects the texture image attached to <target>.  ..."

    -- Section 3.8.10 "Texture Completeness"

       Change the second paragraph (page 177) to say (spec changes identical
       to EXT_texture_array):

       "For one-, two-, or three-dimensional textures and one- or
       two-dimensional array textures, a texture is complete if the following
       conditions all hold true:"

    -- Section 3.8.11 "Texture State and Proxy State"

       Change the second and third paragraphs (page 179) to say (spec changes
       identical to EXT_texture_array):

       "In addition to image arrays for one-, two-, and three-dimensional
       textures, one- and two-dimensional array textures, and the six image
       arrays for the cube map texture, partially instantiated image arrays
       are maintained for one-, two-, and three-dimensional textures and one-
       and two-dimensional array textures.  Additionally, a single proxy image
       array is maintained for the cube map texture.  Each proxy image array
       includes width, height, depth, border width, and internal format state
       values, as well as state for the red, green, blue, alpha, luminance,
       and intensity component resolutions. Proxy image arrays do not include
       image data, nor do they include texture properties. When TexImage3D is
       executed with target specified as PROXY_TEXTURE_3D, the
       three-dimensional proxy state values of the specified level-of-detail
       are recomputed and updated. If the image array would not be supported
       by TexImage3D called with target set to TEXTURE 3D, no error is
       generated, but the proxy width, height, depth, border width, and
       component resolutions are set to zero. If the image array would be
       supported by such a call to TexImage3D, the proxy state values are set
       exactly as though the actual image array were being specified. No pixel
       data are transferred or processed in either case.

       Proxy arrays for one- and two-dimensional textures and one- and
       two-dimensional array textures are operated on in the same way when
       TexImage1D is executed with target specified as PROXY_TEXTURE_1D,
       TexImage2D is executed with target specified as PROXY_TEXTURE_2D or
       PROXY_TEXTURE_1D_ARRAY_EXT, or TexImage3D is executed with target
       specified as PROXY_TETXURE_2D_ARRAY_EXT."

    -- Section 3.8.12 "Texture Objects"

       Change section (page 180) to say (spec changes identical to 
       EXT_texture_array):

       "In addition to the default textures TEXTURE_1D, TEXTURE_2D,
       TEXTURE_3D, TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, and TEXTURE_2D_EXT,
       named one-, two-, and three-dimensional, cube map, and one- and
       two-dimensional array texture objects can be created and operated upon.
       The name space for texture objects is the unsigned integers, with zero
       reserved by the GL.

       A texture object is created by binding an unused name to TEXTURE_1D,
       TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, or
       TEXTURE_2D_ARRAY_EXT. The binding is effected by calling

         void BindTexture(enum target, uint texture);

       with <target> set to the desired texture target and <texture> set to
       the unused name.  The resulting texture object is a new state vector,
       comprising all the state values listed in section 3.8.11, set to the
       same initial values. If the new texture object is bound to TEXTURE_1D,
       TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, or
       TEXTURE_2D_ARRAY_EXT, it is and remains a one-, two-,
       three-dimensional, cube map, one- or two-dimensional array texture
       respectively until it is deleted.

       BindTexture may also be used to bind an existing texture object to
       either TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP,
       TEXTURE_1D_ARRAY_EXT, or TEXTURE_2D_ARRAY_EXT. The error
       INVALID_OPERATION is generated if an attempt is made to bind a texture
       object of different dimensionality than the specified target. If the
       bind is successful no change is made to the state of the bound texture
       object, and any previous binding to target is broken.

       While a texture object is bound, GL operations on the target to which
       it is bound affect the bound object, and queries of the target to which
       it is bound return state from the bound object. If texture mapping of
       the dimensionality of the target to which a texture object is bound is
       enabled, the state of the bound texture object directs the texturing
       operation.

       In the initial state, TEXTURE_1D, TEXTURE_2D, TEXTURE_3D,
       TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, and TEXTURE_2D_ARRAY_EXT have
       one-, two-, three-dimensional, cube map, and one- and two-dimensional
       array texture state vectors respectively associated with them. In order
       that access to these initial textures not be lost, they are treated as
       texture objects all of whose names are 0. The initial one-, two-,
       three-dimensional, cube map, one- and two-dimensional array textures
       are therefore operated upon, queried, and applied as TEXTURE_1D,
       TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, and
       TEXTURE_2D_ARRAY_EXT respectively while 0 is bound to the corresponding
       targets.

       Change second paragraph on page 181 to say (spec changes identical to 
       EXT_texture_array):
       
       "...  If a texture that is currently bound to one of the targets
       TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP,
       TEXTURE_1D_ARRAY_EXT, or TEXTURE_2D_ARRAY_EXT is deleted, it is as
       though BindTexture had been executed with the same target and texture
       zero. ..."

       Change second paragraph on page 182 to say (spec changes identical to 
       EXT_texture_array):
       
       "The texture object name space, including the initial one-, two-, and
       three dimensional, cube map, and one- and two-dimensional array texture
       objects, is shared among all texture units. ..."


    -- Section 3.8.14 "Depth Texture Comparison Modes" in "Texture Comparison Modes"

       Change second through fourth paragraphs (page 188) to say:

       "Let D_t be the depth texture value, in the range [0, 1].  For
       texture lookups from one- and two-dimensional, rectangle, and
       one-dimensional array targets, let R be the interpolated <r>
       texture coordinate, clamped to the range [0, 1].  For texture lookups
       from two-dimensional array texture targets, let R be the interpolated
       <q> texture coordinate, clamped to the range [0, 1].  Then the
       effective texture value L_t, I_t, or A_t is computed as follows:

       If the value of TEXTURE_COMPARE_MODE is NONE, then

          r = Dt

       If the value of TEXTURE_COMPARE_MODE is
       COMPARE_REF_DEPTH_TO_TEXTURE_EXT), then r depends on the texture
       comparison function as shown in table 3.27."

    -- Section 3.8.15 "Texture Application"

       Change the first paragraph (page 189) to say:

       "Texturing is enabled or disabled using the generic Enable and Disable
       commands, respectively, with the symbolic constants TEXTURE_1D,
       TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP, TEXTURE_1D_ARRAY_EXT, or
       TEXTURE_2D_ARRAY_EXT to enable one-, two-, three-dimensional, cube
       map, one-dimensional array, or two-dimensional array texture,
       respectively.  If both two- and one-dimensional textures are enabled,
       the two-dimensional texture is used.  If the three-dimensional and
       either of the two- or one-dimensional textures is enabled, the
       three-dimensional texture is used.  If the cube map texture and any of
       the three-, two-, or one-dimensional textures is enabled, then cube map
       texturing is used.  If one-dimensional array texture is enabled and any
       of cube map, three-, two-, or one-dimensional textures is enabled, 
       one-dimensional array texturing is used.  If two-dimensional array
       texture is enabled and any of cube map, three-, two-, one-dimensional
       textures or one-dimensional array texture is enabled, two-dimensional
       array texturing is used..."

    -- Section 3.11.2 of ARB_fragment_program (Fragment Program Grammar and Restrictions):

       (mostly add to existing grammar rules)

       <optionName>           ::= "MESA_texture_array"

       <texTarget>            ::= "1D"
                               | "2D"
                               | "3D"
                               | "CUBE"
                               | "RECT"
                               | <arrayTarget> (if program option is present)
                               | <shadowTarget> (if program option is present)

       <arrayTarget>          ::= "ARRAY1D"
                               | "ARRAY2D"

       <shadowTarget>         ::= "SHADOW1D"
                               | "SHADOW2D"
                               | "SHADOWRECT"
                               | <shadowArrayTarget> (if program option is present)

       <shadowArrayTarget>    ::= "SHADOWARRAY1D"
                               | "SHADOWARRAY2D"


    -- Add Section 3.11.4.5.4 "Texture Stack Option"

       "If a fragment program specifies the "MESA_texture_array" program
       option, the <texTarget> rule is modified to add the texture targets
       ARRAY1D and ARRAY2D (See Section 3.11.2)."

    -- Section 3.11.6 "Fragment Program Texture Instruction Set"

       (replace 1st and 2nd paragraphs with the following paragraphs)

       "The first three texture instructions described below specify the
       mapping of 4-tuple input vectors to 4-tuple output vectors.
       The sampling of the texture works as described in section 3.8,
       except that texture environments and texture functions are not
       applicable, and the texture enables hierarchy is replaced by explicit
       references to the desired texture target (i.e., 1D, 2D, 3D, cube map,
       rectangle, ARRAY1D, ARRAY2D).  These texture instructions specify
       how the 4-tuple is mapped into the coordinates used for sampling.  The
       following function is used to describe the texture sampling in the
       descriptions below: 

         vec4 TextureSample(vec4 coord, float lodBias, int texImageUnit,
                            enum texTarget);

       Note that not all four components of the texture coordinates <coord>
       are used by all texture targets.  Component usage for each <texTarget>
       is defined in table X.

                                                        coordinates used
         texTarget          Texture Type               s t r  layer  shadow
         ----------------   ---------------------      -----  -----  ------
         1D                 TEXTURE_1D                 x - -    -      -
         2D                 TEXTURE_2D                 x y -    -      -
         3D                 TEXTURE_3D                 x y z    -      -
         CUBE               TEXTURE_CUBE_MAP           x y z    -      -
         RECT               TEXTURE_RECTANGLE_ARB      x y -    -      -
         ARRAY1D            TEXTURE_1D_ARRAY_EXT       x - -    y      -
         ARRAY2D            TEXTURE_2D_ARRAY_EXT       x y -    z      -
         SHADOW1D           TEXTURE_1D                 x - -    -      z
         SHADOW2D           TEXTURE_2D                 x y -    -      z
         SHADOWRECT         TEXTURE_RECTANGLE_ARB      x y -    -      z
         SHADOWARRAY1D      TEXTURE_1D_ARRAY_EXT       x - -    y      z
         SHADOWARRAY2D      TEXTURE_2D_ARRAY_EXT       x y -    z      w

         Table X:  Texture types accessed for each of the <texTarget>, and
         coordinate mappings.  The "coordinates used" column indicate the
         input values used for each coordinate of the texture lookup, the
         layer selector for array textures, and the reference value for
         texture comparisons."

    -- Section 3.11.6.2 "TXP: Project coordinate and map to color"
    
       Add to the end of the section:
       
       "A program will fail to load if the TXP instruction is used in
       conjunction with the SHADOWARRAY2D target."

Additions to Chapter 4 of the OpenGL 2.0 Specification (Per-Fragment Operations)

    -- Section 4.4.2.3 "Attaching Texture Images to a Framebuffer"

       Add to the end of the section (spec changes identical to
       EXT_texture_array):

       "The command

         void FramebufferTextureLayerEXT(enum target, enum attachment,
                                         uint texture, int level, int layer);

       operates identically to FramebufferTexture3DEXT, except that it
       attaches a single layer of a three-dimensional texture or a one- or
       two-dimensional array texture.  <layer> is an integer indicating the
       layer number, and is treated identically to the <zoffset> parameter in
       FramebufferTexture3DEXT.  The error INVALID_VALUE is generated if
       <layer> is negative.  The error INVALID_OPERATION is generated if
       <texture> is non-zero and is not the name of a three dimensional
       texture or one- or two-dimensional array texture.  Unlike
       FramebufferTexture3D, no <textarget> parameter is accepted.

       If <texture> is non-zero and the command does not result in an error,
       the framebuffer attachment state corresponding to <attachment> is
       updated as in the other FramebufferTexture commands, except that
       FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT is set to <layer>."

    -- Section 4.4.4.1 "Framebuffer Attachment Completeness"

      Add to the end of the list of completeness rules (spec changes
      identical to EXT_texture_array):

        "* If FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT is TEXTURE and
           FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT names a one- or 
           two-dimensional array texture, then
           FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT must be smaller than the
           number of layers in the texture."

Additions to Chapter 5 of the OpenGL 2.0 Specification (Special Functions)

    -- Section 5.4 "Display Lists"

       Change the first paragraph on page 242 to say (spec changes
       identical to EXT_texture_array):

       "TexImage3D, TexImage2D, TexImage1D, Histogram, and ColorTable are
       executed immediately when called with the corresponding proxy arguments
       PROXY_TEXTURE_3D or PROXY_TEXTURE_2D_ARRAY_EXT; PROXY_TEXTURE_2D,
       PROXY_TEXTURE_CUBE_MAP, or PROXY_TEXTURE_1D_ARRAY_EXT;
       PROXY_TEXTURE_1D; PROXY_HISTOGRAM; and PROXY_COLOR_TABLE,
       PROXY_POST_CONVOLUTION_COLOR_TABLE, or
       PROXY_POST_COLOR_MATRIX_COLOR_TABLE."

Additions to Chapter 6 of the OpenGL 2.0 Specification (State and State Requests)

    -- Section 6.1.3 "Enumerated Queries"

       Add after the line beginning "If the value of
       FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT is TEXTURE" (spec changes
       identical to EXT_texture_array):

       "If <pname> is FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER_EXT and the
       texture object named FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT is a
       three-dimensional texture or a one- or two-dimensional array texture,
       then <params> will contain the number of texture layer attached to the
       attachment point.  Otherwise, <params> will contain the value zero."

    -- Section 6.1.4 "Texture Queries"
    
       Change the first three paragraphs (page 248) to say (spec changes
       identical to EXT_texture_array):

       "The command

         void GetTexImage(enum tex, int lod, enum format,
                          enum type, void *img);

       is used to obtain texture images. It is somewhat different from the
       other get commands; tex is a symbolic value indicating which texture
       (or texture face in the case of a cube map texture target name) is to
       be obtained.  TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, TEXTURE_1D_ARRAY_EXT,
       and TEXTURE_2D_ARRAY_EXT indicate a one-, two-, or three-dimensional
       texture, or one- or two-dimensional array texture, respectively.
       TEXTURE_CUBE_MAP_POSITIVE_X, ...

       GetTexImage obtains... from the first image to the last for
       three-dimensional textures.  One- and two-dimensional array textures
       are treated as two- and three-dimensional images, respectively, where
       the layers are treated as rows or images.  These groups are then...

       For three-dimensional and two-dimensional array textures, pixel storage
       operations are applied as if the image were two-dimensional, except
       that the additional pixel storage state values PACK_IMAGE_HEIGHT and
       PACK_SKIP_IMAGES are applied. ..."

Additions to Appendix A of the OpenGL 2.0 Specification (Invariance)

    None

Additions to the AGL/GLX/WGL Specifications

    None

GLX Protocol

    None

Dependencies on ARB_fragment_program

    If ARB_fragment_program is not supported, the changes to section 3.11
    should be ignored.

Dependencies on EXT_framebuffer_object

    If EXT_framebuffer_object is not supported, the changes to section
    3.8.8 ("Manual Mipmap Generation"), 4.4.2.3, and 6.1.3 should be ignored.

Dependencies on EXT_texture_compression_s3tc and NV_texture_compression_vtc

    (Identical dependency as EXT_texture_array.)

    S3TC texture compression is supported for two-dimensional array textures.
    When <target> is TEXTURE_2D_ARRAY_EXT, each layer is stored independently
    as a compressed two-dimensional textures.  When specifying or querying
    compressed images using one of the S3TC formats, the images are provided
    and/or returned as a series of two-dimensional textures stored
    consecutively in memory, with the layer closest to zero specified first.
    For array textures, images are not arranged in 4x4x4 or 4x4x2 blocks as in
    the three-dimensional compression format provided in the
    EXT_texture_compression_vtc extension.  Pixel store parameters, including
    those specific to three-dimensional images, are ignored when compressed
    image data are provided or returned, as in the
    EXT_texture_compression_s3tc extension.

    S3TC compression is not supported for one-dimensional texture targets in
    EXT_texture_compression_s3tc, and is not supported for one-dimensional
    array textures in this extension.  If compressed one-dimensional arrays
    are needed, use a two-dimensional texture with a height of one.

    This extension allows the use of the four S3TC internal format types in
    TexImage3D, CompressedTexImage3D, and CompressedTexSubImage3D calls.

Errors

    None

New State

    (add to table 6.15, p. 276)

                                                     Initial
    Get Value                     Type   Get Command  Value Description           Sec.    Attribute
    ----------------------------  -----  -----------  ----- --------------------  ------  ---------
    TEXTURE_BINDING_1D_ARRAY_EXT  2*xZ+  GetIntegerv    0   texture object bound  3.8.12  texture
                                                            to TEXTURE_1D_ARRAY
    TEXTURE_BINDING_2D_ARRAY_EXT  2*xZ+  GetIntegerv    0   texture object bound  3.8.12  texture
                                                            to TEXTURE_2D_ARRAY


New Implementation Dependent State

    (add to Table 6.32, p. 293)

                                                    Minimum
    Get Value                     Type  Get Command  Value  Description         Sec.  Attribute
    ----------------------------  ----  ----------- ------- ------------------  ----- ---------
    MAX_TEXTURE_ARRAY_LAYERS_EXT   Z+   GetIntegerv   64    maximum number of   3.8.1     -
                                                            layers for texture
                                                            arrays

Issues

    (1) Is "texture stack" a good name for this functionality?

        NO.  The name is changed to "array texture" to match the
        nomenclature used by GL_EXT_texture_array.

    (2) Should the R texture coordinate be treated as normalized or
    un-normalized?  If it were un-normalized, floor(R) could be thought
    of as a direct index into the array texture.  This may be more
    convenient for applications.

        RESOLVED.  All texture coordinates are normalized.  The issue of
        un-normalized texture coordinates has been discussed in the ARB
        before and should be left for a layered extension.

        RE-RESOLVED.  The R coordinate is un-normalized.  Accessing an array
        using [0, layers-1] coordinates is much more natural.

    (3) How does LOD selection work for stacked textures?

        RESOLVED.  For 2D array textures the R coordinate is ignored, and
        the LOD selection equations for 2D textures are used.  For 1D
        array textures the T coordinate is ignored, and the LOD selection
        equations for 1D textures are used.  The expected usage is in a
        fragment program with an explicit LOD selection.

    (4) What is the maximum size of a 2D array texture?  Is it the same
    as for a 3D texture, or should a new query be added?  How about for 1D
    array textures?

        RESOLVED.  A new query is added.

    (5) How are array textures exposed in GLSL?
    
        RESOLVED.  Use GL_EXT_texture_array.
        
    (6) Should a 1D array texture also be exposed?

        RESOLVED.  For orthogonality, yes.

    (7) How are stacked textures attached to framebuffer objects?

        RESOLVED.  Layers of both one- and two-dimensional array textures
        are attached using FreambufferTextureLayerEXT.  Once attached, the
        array texture layer behaves exactly as either a one- or
        two-dimensional texture.

    (8) How is this extension related to GL_EXT_texture_array?
    
        This extension adapats GL_MESAX_texture_stack to the notation,
        indexing, and FBO access of GL_EXT_texture_array.  This extension
        replaces the GLSL support of GL_EXT_texture_array with
        GL_ARB_fragment_program support.

        Assembly program support is also provided by GL_NV_gpu_program4.
        GL_NV_gpu_program4 also adds support for other features that are
        specific to Nvidia hardware, while this extension adds only support
        for array textures.

        Much of text of this extension that has changed since
        GL_MESAX_texture_stack comes directly from either
        GL_EXT_texture_array or GL_NV_gpu_program4.

Revision History

    ||2005/11/15||0.1||idr||Initial draft MESAX version.||
    ||2005/12/07||0.2||idr||Added framebuffer object interactions.||
    ||2005/12/12||0.3||idr||Updated fragment program interactions.||
    ||2007/05/16||0.4||idr||Converted to MESA_texture_array.  Brought in line with EXT_texture_array and NV_gpu_program4.||
