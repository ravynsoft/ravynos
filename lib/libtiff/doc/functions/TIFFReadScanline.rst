TIFFReadScanline
================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFReadScanline(TIFF* tif, tdata_t buf, uint32_t row, tsample_t sample)

Description
-----------

Read the data for the specified row into the (user supplied) data buffer
*buf*.  The data are returned decompressed and, in the native byte- and
bit-ordering, but are otherwise packed (see further below). The buffer
must be large enough to hold an entire scanline of data. Applications
should call the routine :c:func:`TIFFScanlineSize` to find out the size
(in bytes) of a scanline buffer.
The *row* parameter is always used by :c:func:`TIFFReadScanline`; the
*sample* parameter is used only if data are organized in separate planes
(``PlanarConfiguration`` = 2).

Notes
-----

The library attempts to hide bit- and byte-ordering differences between the
image and the native machine by converting data to the native machine order.
Bit reversal is done if the ``FillOrder`` tag is opposite to the native
machine bit order. 16- and 32-bit samples are automatically byte-swapped if
the file was written with a byte order opposite to the native machine byte
order,

In C++ the *sample* parameter defaults to 0.

Return values
-------------

:c:func:`TIFFReadScanline` returns -1 if it detects an error; otherwise 1 is
returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``"Compression algorithm does not support random access"``:

  Data was requested in a non-sequential order from a file that uses a
  compression algorithm and that has ``RowsPerStrip`` greater than one.
  That is, data in the image is stored in a compressed form, and with multiple
  rows packed into a strip. In this case, the library does not support random
  access to the data. The data should either be accessed sequentially, or the
  file should be converted so that each strip is made up of one row of data.

Bugs
----

Reading subsampled YCbCR data does not work correctly because, for
``PlanarConfiguration`` = 2, the size of a scanline is not calculated on a
per-sample basis, and for ``PlanarConfiguration`` = 1, the library does not
unpack the block-interleaved samples; use the strip- and
tile-based interfaces to read these formats.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadEncodedStrip` (3tiff),
:doc:`TIFFReadRawStrip` (3tiff),
:doc:`libtiff` (3tiff)
