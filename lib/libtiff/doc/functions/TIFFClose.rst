TIFFClose
==========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: void TIFFClose(TIFF* tif)

.. c:function:: void TIFFCleanup(TIFF* tif)

Description
-----------

:c:func:`TIFFClose` closes a file that was previously opened with
:c:func:`TIFFOpen`.  Any buffered data are flushed to the file, including
the contents of the current directory (if modified); and all resources
are reclaimed. :c:func:`TIFFClose` calls :c:func:`TIFFCleanup` and then
the associated function to close the file handle.

:c:func:`TIFFCleanup` is an auxiliary function to free the TIFF structure.
The given structure will be completely freed, so you should save opened file
handle and pointer to the close procedure in external variables before
calling :c:func:`TIFFCleanup`, if you will need these ones to close the file.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen`  (3tiff),
:doc:`TIFFError` (3tiff)
