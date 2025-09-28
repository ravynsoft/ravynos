TIFFWriteTile
=============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

TIFFWriteTile \- encode and write a tile of data to an open TIFF file

.. c:function:: tsize_t TIFFWriteTile(TIFF* tif, tdata_t buf, uint32_t x, uint32_t y, uint32_t z, tsample_t sample)

Description
-----------

Write the data for the tile *containing* the specified coordinates. The
data in *buf* are (potentially) compressed, and written to the
indicated file, normally being appended to the end of the file. The
buffer must be contain an entire tile of data.  Applications should
call the routine :c:func:`TIFFTileSize` to find out the size (in bytes)
of a tile buffer. The *x* and *y* parameters are always used by
:c:func:`TIFFWriteTile`.  The *z* parameter is used if the image is
deeper than 1 slice (``ImageDepth`` > 1).
The *sample* parameter is used only if data are organized in separate
planes (``PlanarConfiguration`` = 2).

Return values
-------------

:c:func:`TIFFWriteTile` returns -1 if it detects an error; otherwise
the number of bytes in the tile is returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFtile` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadTile` (3tiff),
:doc:`TIFFWriteScanline` (3tiff),
:doc:`TIFFWriteEncodedTile` (3tiff),
:doc:`TIFFWriteRawTile` (3tiff),
:doc:`libtiff` (3tiff)
