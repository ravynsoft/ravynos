Name

    MESA_sprite_point

Name Strings

    GL_MESA_sprite_point

Contact

    Brian Paul, VA Linux Systems Inc. (brianp 'at' valinux.com)

Status

    Obsolete - see GL_ARB_point_sprite.

Version


Number

    ???

Dependencies

    GL_EXT_point_parameters effects the definition of this extension
    GL_ARB_multitexture effects the definition of this extension

Overview
	
    This extension modifies the way in which points are rendered,
    specifically when they're textured.  When SPRITE_POINT_MESA is enabled
    a point is rendered as if it were a quadrilateral with unique texture
    coordinates at each vertex.  This extension effectively turns points
    into sprites which may be rendered more easily and quickly than using
    conventional textured quadrilaterals.

    When using point size > 1 or attenuated points this extension is an
    effective way to render many small sprite images for particle systems
    or other effects.

Issues:

    1. How are the texture coordinates computed?

       The lower-left corner has texture coordinate (0,0,r,q).
       The lower-right, (1,0,r,q).  The upper-right, (1,1,r,q).
       The upper-left, (0,1,r,q).

    2. What about texgen and texture matrices?

       Texgen and the texture matrix have no effect on the point's s and t
       texture coordinates.  The r and q coordinates may have been computed
       by texgen or the texture matrix.  Note that with a 3D texture and/or
       texgen that the r coordinate could be used to select a slice in the
       3D texture.

    3. What about point smoothing?

       When point smoothing is enabled, a triangle fan could be rendered
       to approximate a circular point.  This could be problematic to
       define and implement so POINT_SMOOTH is ignored when drawing sprite
       points.

       Smoothed points can be approximated by using an appropriate texture
       images, alpha testing and blending.

       POLYGON_SMOOTH does effect the rendering of the quadrilateral, however.

    4. What about sprite rotation?

       There is none.  Sprite points are always rendered as window-aligned
       squares.  One could define rotated texture images if desired.  A 3D
       texture and appropriate texture r coordinates could be used to
       effectively specify image rotation per point.

    5. What about POLYGON_MODE?

       POLYGON_MODE does not effect the rasterization of the quadrilateral.

    6. What about POLYGON_CULL?

       TBD.  Polygon culling is normally specified and implemented in the
       transformation stage of OpenGL.  However, some rasterization hardware
       implements it later during triangle setup.

       Polygon culling wouldn't be useful for sprite points since the
       quadrilaterals are always defined in counter-clockwise order in
       window space.  For that reason, polygon culling should probably be
       ignored.

    7. Should sprite points be alpha-attenuated if their size is below the
       point parameter's threshold size?

    8. Should there be an advertisized maximum sprite point size?

       No.  Since we're rendering the point as a quadrilateral there's no
       need to limit the size.


New Procedures and Functions

    None.

New Tokens
    
    Accepted by the <pname> parameter of Enable, Disable, IsEnabled,
    GetIntegerv, GetBooleanv, GetFloatv and GetDoublev:

	SPRITE_POINT_MESA		0x????
        MAX_SPRITE_POINT_SIZE_MESA      0x????   (need this?)

Additions to Chapter 2 of the 1.1 Specification (OpenGL Operation)

    None

Additions to Chapter 3 of the 1.1 Specification (Rasterization)

    Section ???.

    When SPRITE_POINT_MESA is enabled points are rasterized as screen-
    aligned quadrilaterals.  If the four vertices of the quadrilateral
    are labeled A, B, C, and D, starting at the lower-left corner and moving
    counter-clockwise around the quadrilateral, then the vertex and
    texture coordinates are computed as follows:

      vertex   window coordinate       texture coordinate
        A      (x-r, y-r, z, w)          (0, 0, r, q)
        B      (x+r, y-r, z, w)          (1, 0, r, q)
        C      (x+r, y+r, z, w)          (1, 1, r, q)
        D      (x-r, y+r, z, w)          (0, 1, r, q)

    where x, y, z, w are the point's window coordinates, r and q are the
    point's 3rd and 4th texture coordinates  and r is half the point's
    size.  The other vertex attributes (such as the color and fog coordinate)
    are simply duplicated from the original point vertex.

    Point size may either be specified with PointSize or computed
    according to the EXT_point_parameters extension.

    The new texture coordinates are not effected by texgen or the texture
    matrix.  Note, however, that the texture r and q coordinates are passed
    unchanged and may have been computed with texgen and/or the texture
    matrix.

    If multiple texture units are present the same texture coordinate is
    used for all texture units.

    The point is then rendered as if it were a quadrilateral using the
    normal point sampling rules.  POLYGON_MODE does not effect the
    rasterization of the quadrilateral but POLYGON_SMOOTH does.

    POINT_SMOOTH has no effect when SPRITE_POINT_MESA is enabled.

Additions to Chapter 4 of the 1.1 Specification (Per-Fragment Operations
and the Frame Buffer)

    None.

Additions to Chapter 5 of the 1.1 Specification (Special Functions)

    None

Additions to Chapter 6 of the 1.1 Specification (State and State Requests)

    None

Additions to the GLX Specification

    None

GLX Protocol

    TBD

Errors

    None

New State

    Add boolean variable SPRITE_POINT_MESA to the point attribute group.

Revision History

    Version 1.0 - 4 Dec 2000
        Original draft.



