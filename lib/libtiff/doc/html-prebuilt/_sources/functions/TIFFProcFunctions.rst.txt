TIFFProcFunctions
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: TIFFCloseProc TIFFGetCloseProc(TIFF* tif)

.. c:function:: TIFFMapFileProc TIFFGetMapFileProc(TIFF* tif)

.. c:function:: TIFFReadWriteProc TIFFGetReadProc(TIFF* tif)

.. c:function:: TIFFSeekProc TIFFGetSeekProc(TIFF* tif)

.. c:function:: TIFFSizeProc TIFFGetSizeProc(TIFF* tif)

.. c:function:: TIFFUnmapFileProc TIFFGetUnmapFileProc(TIFF* tif)

.. c:function:: TIFFReadWriteProc TIFFGetWriteProc(TIFF* tif)

Description
-----------

.. TODO: Explain or link to explanation of procedure handling.

The following routines return ?????? an open TIFF file.

:c:func:`TIFFGetCloseProc` returns a pointer to file close method.

:c:func:`TIFFGetMapFileProc` returns a pointer to memory mapping method.

:c:func:`TIFFGetReadProc` returns a pointer to file read method.

:c:func:`TIFFGetSeekProc` returns a pointer to file seek method.

:c:func:`TIFFGetSizeProc` returns a pointer to file size requesting method.

:c:func:`TIFFGetUnmapFileProc` returns a pointer to memory unmapping method.

:c:func:`TIFFGetWriteProc` returns a pointer to file write method.

Diagnostics
-----------

None.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen` (3tiff)
