Name

    MESA_packed_depth_stencil

Name Strings

    GL_MESA_packed_depth_stencil

Contact

    Keith Whitwell, VA Linux Systems Inc. (keithw 'at' valinux.com)
    Brian Paul, VA Linux Systems Inc. (brianp 'at' valinux.com)

Status

    Obsolete.

Version


Number

    ???

Dependencies

    EXT_abgr affects the definition of this extension
    SGIS_texture4D affects the definition of this extension
    EXT_cmyka affects the definition of this extension
    ARB_packed_pixels affects the definition of this extension

Overview
	
    Provides a mechanism for DrawPixels and ReadPixels to efficiently
    transfer depth and stencil image data.  Specifically, we defined new
    packed pixel formats and types which pack both stencil and depth
    into one value.

Issues:

    1. Is this the right way to distinguish between 24/8 and 8/24
       pixel formats?  Should we instead provide both:
	
       GL_DEPTH_STENCIL_MESA
       GL_STENCIL_DEPTH_MESA

       And perhaps just use GL_UNSIGNED_INT, GL_UNSIGNED_SHORT ?

    2. If not, is it correct to use _REV to indicate that stencil
       preceeds depth in the 1_15 and 8_24 formats?

    3. Do we really want the GL_UNSIGNED_SHORT formats?  

	
New Procedures and Functions

    None.

New Tokens
    
    Accepted by the <format> parameter of ReadPixels and DrawPixels:

	GL_DEPTH_STENCIL_MESA		0x8750

    Accepted by the <type> parameter of ReadPixels and DrawPixels:

	GL_UNSIGNED_INT_24_8_MESA	0x8751
	GL_UNSIGNED_INT_8_24_REV_MESA	0x8752
	GL_UNSIGNED_SHORT_15_1_MESA	0x8753
	GL_UNSIGNED_SHORT_1_15_REV_MESA	0x8754

Additions to Chapter 2 of the 1.1 Specification (OpenGL Operation)

    None

Additions to Chapter 3 of the 1.1 Specification (Rasterization)

    One entry is added to table 3.5 (DrawPixels and ReadPixels formats).
    The new table is:

			Target
	Format Name	Buffer	Element Meaning and Order
	-----------	------	-------------------------
	COLOR_INDEX	Color	Color index
	STENCIL_INDEX	Stencil Stencil index
	DEPTH_COMPONENT Depth	Depth component
	RED		Color	R component
	GREEN		Color	G component
	BLUE		Color	B component
	ALPHA		Color	A component
	RGB		Color	R, G, B components
	RGBA		Color	R, G, B, A components
	BGRA	        Color   B, G, R, A components
	ABGR_EXT	Color	A, B, G, R components
	CMYK_EXT	Color	Cyan, Magenta, Yellow, Black components
	CMYKA_EXT	Color	Cyan, Magenta, Yellow, Black, A components
	LUMINANCE	Color	Luminance component
	LUMINANCE_ALPHA Color	Luminance, A components
	DEPTH_STENCIL   Depth,  Depth component, stencil index.
			Stencil

	Table 3.5: DrawPixels and ReadPixels formats.  The third column
	gives a description of and the number and order of elements in a
	group.

    Add to the description of packed pixel formats:

	<type> Parameter		Data	of	  Matching
	Token Name			Type	Elements  Pixel Formats
	----------------		----	--------  -------------

	UNSIGNED_BYTE_3_3_2	      ubyte   3	      RGB
	UNSIGNED_BYTE_2_3_3_REV       ubyte   3	      RGB
	UNSIGNED_SHORT_5_6_5	      ushort  3	      RGB
	UNSIGNED_SHORT_5_6_5_REV      ushort  3	      RGB
	UNSIGNED_SHORT_4_4_4_4	      ushort  4	      RGBA,BGRA,ABGR_EXT,CMYK_EXT
	UNSIGNED_SHORT_4_4_4_4_REV    ushort  4	      RGBA,BGRA
	UNSIGNED_SHORT_5_5_5_1	      ushort  4	      RGBA,BGRA,ABGR_EXT,CMYK_EXT
	UNSIGNED_SHORT_1_5_5_5_REV    ushort  4	      RGBA,BGRA
	UNSIGNED_INT_8_8_8_8	      uint    4	      RGBA,BGRA,ABGR_EXT,CMYK_EXT
	UNSIGNED_INT_8_8_8_8_REV      uint    4	      RGBA,BGRA
	UNSIGNED_INT_10_10_10_2       uint    4	      RGBA,BGRA,ABGR_EXT,CMYK_EXT
	UNSIGNED_INT_2_10_10_10_REV   uint    4	      RGBA,BGRA
	UNSIGNED_SHORT_15_1_MESA      ushort  2       DEPTH_STENCIL_MESA
	UNSIGNED_SHORT_1_15_REV_MESA  ushort  2       DEPTH_STENCIL_MESA
	UNSIGNED_SHORT_24_8_MESA      ushort  2       DEPTH_STENCIL_MESA
	UNSIGNED_SHORT_8_24_REV_MESA  ushort  2       DEPTH_STENCIL_MESA

	UNSIGNED_INT_8_24:

	     31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10	9  8  7  6  5  4  3  2	1  0
	    +-----------------------+-----------------------------------------------------------------------+
	    |			    |			     			    			    |
	    +-----------------------+-----------------------------------------------------------------------+

		    first					second		    
		    element					element		    


	UNSIGNED_INT_24_8:

	     31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10	9  8  7  6  5  4  3  2	1  0
	    +----------------------------------------------------------------------+------------------------+
	    |			    			     			   | 			    |
	    +----------------------------------------------------------------------+------------------------+

		    first								  second		    
		    element								  element		    

	UNSIGNED_SHORT_15_1:

	      15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
	    +-----------------------------------------------------------+---+
	    |					    			|   |
	    +-----------------------------------------------------------+---+

	    		first						second	
			element						element	


	UNSIGNED_SHORT_1_15_REV:

	      15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
	    +---+-----------------------------------------------------------+
	    |	|		     					    |
	    +---+-----------------------------------------------------------+

	    second		    first
	    element		    element

    The assignment of elements to fields in the packed pixel is as
    described in the table below:

			   First       Second	Third	    Fourth
	Format		   Element     Element	Element     Element
	------		   -------     -------	-------     -------
	RGB		   red	       green	blue
	RGBA		   red	       green	blue	    alpha
	BGRA		   blue	       green	red	    alpha
	ABGR_EXT	   alpha       blue	green	    red
	CMYK_EXT	   cyan	       magenta	yellow	    black
	DEPTH_STENCIL_MESA depth       stencil

Additions to Chapter 4 of the 1.1 Specification (Per-Fragment Operations
and the Frame Buffer)

    The new format is added to the discussion of Obtaining Pixels from the
    Framebuffer.  It should read " If the <format> is one of RED, GREEN,
    BLUE, ALPHA, RGB, RGBA, ABGR_EXT, LUMINANCE, or LUMINANCE_ALPHA, and
    the GL is in color index mode, then the color index is obtained."

    The new format is added to the discussion of Index Lookup.  It should
    read "If <format> is one of RED, GREEN, BLUE, ALPHA, RGB, RGBA,
    ABGR_EXT, LUMINANCE, or LUMINANCE_ALPHA, then the index is used to
    reference 4 tables of color components: PIXEL_MAP_I_TO_R,
    PIXEL_MAP_I_TO_G, PIXEL_MAP_I_TO_B, and PIXEL_MAP_I_TO_A."


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

    None

Revision History

    Version 1.0 - 23 Sep 2000
        Keith's original version.

    Version 1.1 - 3 Nov 2000
        Brian's edits, assigned values to new enums.

