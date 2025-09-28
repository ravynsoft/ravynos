TIFFReadEncodedStrip
====================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tmsize_t TIFFReadEncodedStrip(TIFF* tif, uint32_t strip, void* buf, tmsize_t size)

Description
-----------

Read the specified strip of data and place up to *size* bytes of decompressed
information in the (user supplied) data buffer.

Notes
-----

The value of *strip* is a "raw strip number".  That is, the caller must take
into account whether or not the data are organized in separate planes
(``PlanarConfiguration`` = 2).
To read a full strip of data the data buffer should typically be at least as
large as the number returned by :c:func:`TIFFStripSize`.
If -1 is passed in the *size* parameter, the whole strip will be read. You
should be sure you have enough space allocated for the buffer.

The library attempts to hide bit- and byte-ordering differences between the
image and the native machine by converting data to the native machine order.
Bit reversal is done if the ``FillOrder`` tag is opposite to the native
machine bit order. 16- and 32-bit samples are automatically byte-swapped if
the file was written with a byte order opposite to the native machine byte
order.

Return values
-------------

The actual number of bytes of data that were placed in *buf* is returned;
:c:func:`TIFFReadEncodedStrip` returns -1 if an error was encountered.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadRawStrip` (3tiff),
:doc:`TIFFReadScanline` (3tiff),
:doc:`TIFFReadEncodedTile` (3tiff),
:doc:`libtiff` (3tiff)
