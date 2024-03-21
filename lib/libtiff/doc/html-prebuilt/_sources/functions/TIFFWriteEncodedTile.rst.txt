TIFFWriteEncodedTile
====================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFWriteEncodedTile(TIFF* tif, ttile_t tile, tdata_t buf, tsize_t size)

Description
-----------

Compress *size* bytes of raw data from *buf* and **append** the result
to the end of the specified tile. Note that the value of *tile* is a
"raw tile number".  That is, the caller must take into account whether
or not the data are organized in separate planes
(``PlanarConfiguration`` = 2).
:c:func:`TIFFComputeTile` automatically does this when converting an
(x,y,z,sample) coordinate quadruple to a tile number.

Notes
-----

The library writes encoded data using the native machine byte order.
Correctly implemented TIFF readers are expected to do any necessary
byte-swapping to correctly process image data with ``BitsPerSample``
greater than 8.

Return values
-------------

-1 is returned if an error was encountered. Otherwise, the value of
*size* is returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``%s: File not open for writing``:

  The file was opened for reading, not writing.

``Can not write tiles to a stripped image``:

  The image is assumed to be organized in strips because neither of the
  ``TileWidth`` or ``TileLength`` tags have been set with
  :c:func:`TIFFSetField`.

``%s: Must set "ImageWidth" before writing data``:

  The image's width has not been set before the first write. See
  :c:func:`TIFFSetField` for information on how to do this.

``%s: Must set "PlanarConfiguration" before writing data``:

  The organization of data has not be defined before the first write.
  See :c:func:`TIFFSetField` for information on how to do this.

``%s: No space for tile arrays"``:

  There was not enough space for the arrays that hold tile offsets and
  byte counts.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFWriteTile` (3tiff),
:doc:`TIFFWriteRawTile` (3tiff),
:doc:`libtiff` (3tiff)
