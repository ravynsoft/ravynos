TIFFReadEncodedTile
===================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFReadEncodedTile(TIFF* tif, ttile_t tile, tdata_t buf, tsize_t size)

Description
-----------

Read the specified tile of data and place up to *size* bytes of decompressed
information in the (user supplied) data buffer.

Notes
-----

The value of *tile* is a "raw tile number". That is, the caller must take
into account whether or not the data are organized in separate planes
(``PlanarConfiguration`` = 2).
:c:func:`TIFFComputeTile` automatically does this when converting an
(x,y,z,sample) coordinate quadruple to a tile number. To read a full tile
of data the data buffer should be at least as large as the value returned by
:c:func:`TIFFTileSize`.

The library attempts to hide bit- and byte-ordering differences between the
image and the native machine by converting data to the native machine order.
Bit reversal is done if the ``FillOrder`` tag is opposite to the native
machine bit order. 16- and 32-bit samples are automatically byte-swapped if
the file was written with a byte order opposite to the native machine byte
order.

Return values
-------------

The actual number of bytes of data that were placed in *buf* is returned;
:c:func:`TIFFReadEncodedTile` returns -1 if an error was encountered.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadRawTile` (3tiff),
:doc:`TIFFReadTile` (3tiff),
:doc:`TIFFReadEncodedStrip` (3tiff),
:doc:`libtiff` (3tiff)
