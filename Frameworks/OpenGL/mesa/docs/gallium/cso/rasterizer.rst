.. _rasterizer:

Rasterizer
==========

The rasterizer state controls the rendering of points, lines and triangles.
Attributes include polygon culling state, line width, line stipple,
multisample state, scissoring and flat/smooth shading.

Linkage
-------

clamp_vertex_color
   If set, TGSI_SEMANTIC_COLOR registers are clamped to the [0, 1] range after
   the execution of the vertex shader, before being passed to the geometry
   shader or fragment shader.

   OpenGL: glClampColor(GL_CLAMP_VERTEX_COLOR) in GL 3.0 or
   :ext:`GL_ARB_color_buffer_float`

   D3D11: seems always disabled

   Note the PIPE_CAP_VERTEX_COLOR_CLAMPED query indicates whether or not the
   driver supports this control.  If it's not supported, gallium frontends may
   have to insert extra clamping code.


clamp_fragment_color
   Controls whether TGSI_SEMANTIC_COLOR outputs of the fragment shader
   are clamped to [0, 1].

   OpenGL: glClampColor(GL_CLAMP_FRAGMENT_COLOR) in GL 3.0 or
   :ext:`GL_ARB_color_buffer_float`

   D3D11: seems always disabled

   Note the PIPE_CAP_FRAGMENT_COLOR_CLAMPED query indicates whether or not the
   driver supports this control.  If it's not supported, gallium frontends may
   have to insert extra clamping code.


Shading
-------

flatshade
   If set, the provoking vertex of each polygon is used to determine the color
   of the entire polygon.  If not set, fragment colors will be interpolated
   between the vertex colors.

   The actual interpolated shading algorithm is obviously
   implementation-dependent, but will usually be Gouraud for most hardware.

   .. note::

      This is separate from the fragment shader input attributes
      CONSTANT, LINEAR and PERSPECTIVE. The flatshade state is needed at
      clipping time to determine how to set the color of new vertices.

      :ref:`Draw` can implement flat shading by copying the provoking vertex
      color to all the other vertices in the primitive.

flatshade_first
   Whether the first vertex should be the provoking vertex, for most primitives.
   If not set, the last vertex is the provoking vertex.

   There are a few important exceptions to the specification of this rule.

   * ``PIPE_PRIMITIVE_POLYGON``: The provoking vertex is always the first
     vertex. If the caller wishes to change the provoking vertex, they merely
     need to rotate the vertices themselves.
   * ``PIPE_PRIMITIVE_QUAD``, ``PIPE_PRIMITIVE_QUAD_STRIP``: The option only has
     an effect if ``PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION`` is true.
     If it is not, the provoking vertex is always the last vertex.
   * ``PIPE_PRIMITIVE_TRIANGLE_FAN``: When set, the provoking vertex is the
     second vertex, not the first. This permits each segment of the fan to have
     a different color.

Polygons
--------

light_twoside
   If set, there are per-vertex back-facing colors.  The hardware
   (perhaps assisted by :ref:`Draw`) should be set up to use this state
   along with the front/back information to set the final vertex colors
   prior to rasterization.

   The frontface vertex shader color output is marked with TGSI semantic
   COLOR[0], and backface COLOR[1].

front_ccw
    Indicates whether the window order of front-facing polygons is
    counter-clockwise (TRUE) or clockwise (FALSE).

cull_mode
    Indicates which faces of polygons to cull, either PIPE_FACE_NONE
    (cull no polygons), PIPE_FACE_FRONT (cull front-facing polygons),
    PIPE_FACE_BACK (cull back-facing polygons), or
    PIPE_FACE_FRONT_AND_BACK (cull all polygons).

fill_front
    Indicates how to fill front-facing polygons, either
    PIPE_POLYGON_MODE_FILL, PIPE_POLYGON_MODE_LINE or
    PIPE_POLYGON_MODE_POINT.
fill_back
    Indicates how to fill back-facing polygons, either
    PIPE_POLYGON_MODE_FILL, PIPE_POLYGON_MODE_LINE or
    PIPE_POLYGON_MODE_POINT.

poly_stipple_enable
    Whether polygon stippling is enabled.
poly_smooth
    Controls OpenGL-style polygon smoothing/antialiasing

offset_point
    If set, point-filled polygons will have polygon offset factors applied
offset_line
    If set, line-filled polygons will have polygon offset factors applied
offset_tri
    If set, filled polygons will have polygon offset factors applied

offset_units
    Specifies the polygon offset bias
offset_units_unscaled
    Specifies the unit of the polygon offset bias. If false, use the
    GL/D3D1X behavior. If true, offset_units is a floating point offset
    which isn't scaled (D3D9). Note that GL/D3D1X behavior has different
    formula whether the depth buffer is unorm or float, which is not
    the case for D3D9.
offset_scale
    Specifies the polygon offset scale
offset_clamp
    Upper (if > 0) or lower (if < 0) bound on the polygon offset result



Lines
-----

line_width
    The width of lines.
line_smooth
    Whether lines should be smoothed. Line smoothing is simply anti-aliasing.
line_stipple_enable
    Whether line stippling is enabled.
line_stipple_pattern
    16-bit bitfield of on/off flags, used to pattern the line stipple.
line_stipple_factor
    When drawing a stippled line, each bit in the stipple pattern is
    repeated N times, where N = line_stipple_factor + 1.
line_last_pixel
    Controls whether the last pixel in a line is drawn or not.  OpenGL
    omits the last pixel to avoid double-drawing pixels at the ends of lines
    when drawing connected lines.


Points
------

sprite_coord_enable
   The effect of this state depends on PIPE_CAP_TGSI_TEXCOORD !

   Controls automatic texture coordinate generation for rendering sprite points.

   If PIPE_CAP_TGSI_TEXCOORD is false:
   When bit k in the sprite_coord_enable bitfield is set, then generic
   input k to the fragment shader will get an automatically computed
   texture coordinate.

   If PIPE_CAP_TGSI_TEXCOORD is true:
   The bitfield refers to inputs with TEXCOORD semantic instead of generic inputs.

   The texture coordinate will be of the form (s, t, 0, 1) where s varies
   from 0 to 1 from left to right while t varies from 0 to 1 according to
   the state of 'sprite_coord_mode' (see below).

   If any bit is set, then point_smooth MUST be disabled (there are no
   round sprites) and point_quad_rasterization MUST be true (sprites are
   always rasterized as quads).  Any mismatch between these states should
   be considered a bug in the gallium frontend.

   This feature is implemented in the :ref:`Draw` module but may also be
   implemented natively by GPUs or implemented with a geometry shader.


sprite_coord_mode
   Specifies how the value for each shader output should be computed when drawing
   point sprites. For PIPE_SPRITE_COORD_LOWER_LEFT, the lower-left vertex will
   have coordinates (0,0,0,1). For PIPE_SPRITE_COORD_UPPER_LEFT, the upper-left
   vertex will have coordinates (0,0,0,1).
   This state is used by :ref:`Draw` to generate texcoords.


point_quad_rasterization
   Determines if points should be rasterized according to quad or point
   rasterization rules.

   (Legacy-only) OpenGL actually has quite different rasterization rules
   for points and point sprites - hence this indicates if points should be
   rasterized as points or according to point sprite (which decomposes them
   into quads, basically) rules. Newer GL versions no longer support the old
   point rules at all.

   Additionally Direct3D will always use quad rasterization rules for
   points, regardless of whether point sprites are enabled or not.

   If this state is enabled, point smoothing and antialiasing are
   disabled. If it is disabled, point sprite coordinates are not
   generated.

   .. note::

      Some renderers always internally translate points into quads; this state
      still affects those renderers by overriding other rasterization state.

point_tri_clip
    Determines if clipping of points should happen after they are converted
    to "rectangles" (required by d3d) or before (required by OpenGL, though
    this rule is ignored by some IHVs).
    It is not valid to set this to enabled but have point_quad_rasterization
    disabled.
point_smooth
    Whether points should be smoothed. Point smoothing turns rectangular
    points into circles or ovals.
point_size_per_vertex
    Whether the vertex shader is expected to have a point size output.
    Undefined behavior is permitted if there is disagreement between
    this flag and the actual bound shader.
point_size
    The size of points, if not specified per-vertex.



Other Members
-------------

scissor
    Whether the scissor test is enabled.

multisample
    Whether :term:`MSAA` is enabled.

half_pixel_center
    When true, the rasterizer should use (0.5, 0.5) pixel centers for
    determining pixel ownership (e.g, OpenGL, D3D10 and higher)::

           0 0.5 1
        0  +-----+
           |     |
       0.5 |  X  |
           |     |
        1  +-----+

    When false, the rasterizer should use (0, 0) pixel centers for determining
    pixel ownership (e.g., D3D9 or earlier)::

         -0.5 0 0.5
      -0.5 +-----+
           |     |
        0  |  X  |
           |     |
       0.5 +-----+

bottom_edge_rule
    Determines what happens when a pixel sample lies precisely on a triangle
    edge.

    When true, a pixel sample is considered to lie inside of a triangle if it
    lies on the *bottom edge* or *left edge* (e.g., OpenGL drawables)::

        0                    x
      0 +--------------------->
        |
        |  +-------------+
        |  |             |
        |  |             |
        |  |             |
        |  +=============+
        |
      y V

    When false, a pixel sample is considered to lie inside of a triangle if it
    lies on the *top edge* or *left edge* (e.g., OpenGL FBOs, D3D)::

        0                    x
      0 +--------------------->
        |
        |  +=============+
        |  |             |
        |  |             |
        |  |             |
        |  +-------------+
        |
      y V

    Where:
     - a *top edge* is an edge that is horizontal and is above the other edges;
     - a *bottom edge* is an edge that is horizontal and is below the other
       edges;
     - a *left edge* is an edge that is not horizontal and is on the left side of
       the triangle.

    .. note::

        Actually all graphics APIs use a top-left rasterization rule for pixel
        ownership, but their notion of top varies with the axis origin (which
        can be either at y = 0 or at y = height).  Gallium instead always
        assumes that top is always at y=0.

    See also:
     - https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules
     - https://learn.microsoft.com/en-us/windows/win32/direct3d9/rasterization-rules

clip_halfz
    When true clip space in the z axis goes from [0..1] (D3D).  When false
    [-1, 1] (GL)

depth_clip_near
    When false, the near depth clipping plane of the view volume is disabled.
depth_clip_far
    When false, the far depth clipping plane of the view volume is disabled.
depth_clamp
    Whether the depth value will be clamped to the interval defined by the
    near and far depth range at the per-pixel level, after polygon offset has
    been applied and before depth testing. Note that a clamp to [0,1] according
    to GL rules should always happen even if this is disabled.

clip_plane_enable
    For each k in [0, PIPE_MAX_CLIP_PLANES), if bit k of this field is set,
    clipping half-space k is enabled, if it is clear, it is disabled.
    The clipping half-spaces are defined either by the user clip planes in
    ``pipe_clip_state``, or by the clip distance outputs of the shader stage
    preceding the fragment shader.
    If any clip distance output is written, those half-spaces for which no
    clip distance is written count as disabled; i.e. user clip planes and
    shader clip distances cannot be mixed, and clip distances take precedence.

conservative_raster_mode
    The conservative rasterization mode.  For PIPE_CONSERVATIVE_RASTER_OFF,
    conservative rasterization is disabled.  For PIPE_CONSERVATIVE_RASTER_POST_SNAP
    or PIPE_CONSERVATIVE_RASTER_PRE_SNAP, conservative rasterization is nabled.
    When conservative rasterization is enabled, the polygon smooth, line mooth,
    point smooth and line stipple settings are ignored.
    With the post-snap mode, unlike the pre-snap mode, fragments are never
    generated for degenerate primitives.  Degenerate primitives, when rasterized,
    are considered back-facing and the vertex attributes and depth are that of
    the provoking vertex.
    If the post-snap mode is used with an unsupported primitive, the pre-snap
    mode is used, if supported.  Behavior is similar for the pre-snap mode.
    If the pre-snap mode is used, fragments are generated with respect to the primitive
    before vertex snapping.

conservative_raster_dilate
    The amount of dilation during conservative rasterization.

subpixel_precision_x
    A bias added to the horizontal subpixel precision during conservative rasterization.
subpixel_precision_y
    A bias added to the vertical subpixel precision during conservative rasterization.
