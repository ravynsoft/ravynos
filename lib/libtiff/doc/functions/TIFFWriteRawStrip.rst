TIFFWriteRawStrip
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFWriteRawStrip(TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size)

Description
-----------

Append *size* bytes of raw data to the specified strip.

Notes
-----

The strip number must be valid according to the current settings of the
``ImageLength`` and ``RowsPerStrip`` tags.
An image may be dynamically grown by increasing the value of
``ImageLength`` prior to each call to :c:func:`TIFFWriteRawStrip`.

Return values
-------------

-1 is returned if an error occurred.  Otherwise, the value of *size* is
returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``%s: File not open for writing``:

  The file was opened for reading, not writing.

``Can not write scanlines to a tiled image``:

  The image is assumed to be organized in tiles because the
  ``TileWidth`` and ``TileLength`` tags have been set with
  :c:func:`TIFFSetField`.

``%s: Must set "ImageWidth" before writing data``:

  The image's width has not be set before the first write.
  See :c:func:`TIFFSetField` for information on how to do this.

``%s: Must set "PlanarConfiguration" before writing data``:

  The organization of data has not be defined before the first write.
  See :c:func:`TIFFSetField` for information on how to do this.

``%s: No space for strip arrays"``:

  There was not enough space for the arrays that hold strip
  offsets and byte counts.

``%s: Strip %d out of range, max %d``:

  The specified strip is not a valid strip according to the
  currently specified image dimensions.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFWriteEncodedStrip` (3tiff),
:doc:`TIFFWriteScanline` (3tiff),
:doc:`libtiff` (3tiff)
