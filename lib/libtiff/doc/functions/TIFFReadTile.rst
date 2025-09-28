TIFFReadTile
============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFReadTile(TIFF* tif, tdata_t buf, uint32_t x, uint32_t y, uint32_t z, tsample_t sample)

Description
-----------

Return the data for the tile *containing* the specified coordinates. The
data placed in *buf* are returned decompressed and, typically, in the
native byte- and bit-ordering, but are otherwise packed (see further
below). The buffer must be large enough to hold an entire tile of data.
Applications should call the routine :c:func:`TIFFTileSize` to find out
the size (in bytes) of a tile buffer. The *x* and *y* parameters are
always used by :c:func:`TIFFReadTile`.  The *z* parameter is used if the
image is deeper than 1 slice (``ImageDepth`` > 1).  The *sample*
parameter is used only if data are organized in separate planes (
``PlanarConfiguration`` = 2).

Notes
-----

The library attempts to hide bit- and byte-ordering differences between
the image and the native machine by converting data to the native machine
order.  Bit reversal is done if the ``FillOrder`` tag is opposite to the
native machine bit order. 16- and 32-bit samples are automatically
byte-swapped if the file was written with a byte order opposite to the
native machine byte order.

Return values
-------------

:c:func:`TIFFReadTile` returns -1 if it detects an error; otherwise the
number of bytes in the decoded tile is returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFtile` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadEncodedTile` (3tiff),
:doc:`TIFFReadRawTile` (3tiff),
:doc:`libtiff` (3tiff)
