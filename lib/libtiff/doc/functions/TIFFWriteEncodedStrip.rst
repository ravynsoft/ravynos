TIFFWriteEncodedStrip
=====================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFWriteEncodedStrip(TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size)

Description
-----------

Compress *size* bytes of raw data from *buf* and write the result to the
specified strip; replacing any previously written data. Note that the
value of *strip* is a "raw strip number".  That is, the caller must take
into account whether or not the data are organized in separate planes
(``PlanarConfiguration`` = 2).

Notes
-----

The library writes encoded data using the native machine byte order.
Correctly implemented TIFF readers are expected to do any necessary
byte-swapping to correctly process image data with ``BitsPerSample``
greater than 8.

The strip number must be valid according to the current settings of the
``ImageLength`` and ``RowsPerStrip`` tags.
An image may be dynamically grown by increasing the value of
``ImageLength`` prior to each call to ``TIFFWriteEncodedStrip``.

Return values
-------------

-1 is returned if an error was encountered. Otherwise, the value of
*size* is returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``%s: File not open for writing``:

  The file was opened for reading, not writing.

``Can not write scanlines to a tiled image``:

  The image is assumed to be organized in tiles because the
  ``TileWidth`` and ``TileLength`` tags have been set with
  :c:func:`TIFFSetField`.

``%s: Must set "ImageWidth" before writing data\fP``:

  The image's width has not be set before the first write. See
  :c:func:`TIFFSetField` for information on how to do this.

``%s: Must set "PlanarConfiguration" before writing data``:

  The organization of data has not be defined before the first
  write. See :c:func:`TIFFSetField` for information on how to do
  this.

``%s: No space for strip arrays"``:

  There was not enough space for the arrays that hold strip offsets and
  byte counts.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFWriteScanline` (3tiff),
:doc:`TIFFWriteRawStrip` (3tiff),
:doc:`libtiff` (3tiff)
