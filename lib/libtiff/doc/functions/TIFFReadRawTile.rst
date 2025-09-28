TIFFReadRawTile
===============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFReadRawTile(TIFF* tif, ttile_t tile, tdata_t buf, tsize_t size)

Description
-----------

Read the contents of the specified tile into the (user supplied) data buffer.
Note that the value of *tile* is a "raw tile number". That is, the caller
must take into account whether or not the data is organized in separate planes
(``PlanarConfiguration`` = 2).
:c:func:`TIFFComputeTile` automatically does this when converting an
(x,y,z,sample) coordinate quadruple to a tile number. To read a full tile
of data the data buffer should typically be at least as large as the value
returned by :c:func:`TIFFTileSize`.

Return values
-------------

The actual number of bytes of data that were placed in *buf* is returned;
:c:func:`TIFFReadRawTile` returns -1 if an error was encountered.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadEncodedTile` (3tiff),
:doc:`TIFFReadTile` (3tiff),
:doc:`TIFFtile` (3tiff),
:doc:`libtiff` (3tiff)
