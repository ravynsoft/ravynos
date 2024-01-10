TIFFReadRawStrip
================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFReadRawStrip(TIFF* tif, tstrip_t strip, tdata_t buf, tsize_t size)

Description
-----------

Read the contents of the specified strip into the (user supplied) data buffer.
Note that the value of *strip* is a "raw strip number". That is, the caller
must take into account whether or not the data is organized in separate planes
(``PlanarConfiguration`` = 2).
To read a full strip of data the data buffer should typically be at least as
large as the number returned by :c:func:`TIFFStripSize`.

Return values
-------------

The actual number of bytes of data that were placed in *buf* is returned;
:c:func:`TIFFReadRawStrip` returns -1 if an error was encountered.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadEncodedStrip` (3tiff),
:doc:`TIFFReadScanline` (3tiff),
:doc:`TIFFstrip` (3tiff),
:doc:`libtiff` (3tiff)
