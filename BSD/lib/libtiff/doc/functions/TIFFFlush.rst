TIFFFlush
=========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFFlush(TIFF* tif)

.. c:function:: int TIFFFlushData(TIFF* tif)

Description
-----------

:c:func:`TIFFFlush` causes any pending writes for the specified file
(including writes for the current directory) to be done. In normal
operation this call is never needed—the library automatically does
any flushing required.

:c:func:`TIFFFlushData` flushes any pending image data for the specified
file to be written out; directory-related data are not flushed. In normal
operation this call is never needed—the library automatically does any
flushing required.

Return values
-------------

0 is returned if an error is encountered, otherwise 1 is returned.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFWriteEncodedStrip` (3tiff),
:doc:`TIFFWriteEncodedTile` (3tiff),
:doc:`TIFFWriteRawStrip` (3tiff),
:doc:`TIFFWriteRawTile` (3tiff),
:doc:`TIFFWriteScanline` (3tiff),
:doc:`TIFFWriteTile` (3tiff),
:doc:`libtiff` (3tiff)
