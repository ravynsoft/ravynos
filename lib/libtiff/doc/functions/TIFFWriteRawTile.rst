TIFFWriteRawTile
================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFWriteRawTile(TIFF* tif, ttile_t tile, tdata_t buf, tsize_tsize)

Description
-----------

Append *size* bytes of raw data to the specified tile.

Return values
-------------

-1 is returned if an error occurred. Otherwise, the value of *size* is
returned.

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

  The image's width has not be set before the first write.
  See :c:func:`TIFFSetField` for information on how to do this.

``%s: Must set "PlanarConfiguration" before writing data``:

  The organization of data has not be defined before the first write.
  See :c:func:`TIFFSetField` for information on how to do this.

``%s: No space for tile arrays``:

  There was not enough space for the arrays that hold tile offsets and
  byte counts.

``%s: Specified tile %d out of range, max %d``:

  The specified tile is not valid according to the currently specified
  image dimensions.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFWriteEncodedTile` (3tiff),
:doc:`TIFFWriteScanline` (3tiff),
:doc:`libtiff` (3tiff)
