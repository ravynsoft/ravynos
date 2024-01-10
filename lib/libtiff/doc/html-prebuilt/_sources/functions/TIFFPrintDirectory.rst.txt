TIFFPrintDirectory
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: void TIFFPrintDirectory(TIFF* tif, FILE* fd, long flags)

Description
-----------

:c:func:`TIFFPrintDirectory` prints a description of the current directory
in the specified TIFF file to the standard I/O output stream *fd*.
The *flags* parameter is used to control the "level of detail"
of the printed information; it is a bitwise-or of the flags defined in
:file:`tiffio.h`:


=================================  =====  ===========================
Name                               Value  Description
=================================  =====  ===========================
:c:macro:`TIFFPRINT_NONE`          0x0    no extra info
:c:macro:`TIFFPRINT_STRIPS`        0x1    strips/tiles info
:c:macro:`TIFFPRINT_CURVES`        0x2    color/gray response curves
:c:macro:`TIFFPRINT_COLORMAP`      0x4    colormap
:c:macro:`TIFFPRINT_JPEGQTABLES`   0x100  JPEG Q matrices
:c:macro:`TIFFPRINT_JPEGACTABLES`  0x200  JPEG AC tables
:c:macro:`TIFFPRINT_JPEGDCTABLES`  0x200  JPEG DC tables
=================================  =====  ===========================

Notes
-----

In C++ the *flags* parameter defaults to 0.

Return values
-------------

None.

Diagnostics
-----------

None.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadDirectory` (3tiff),
:doc:`TIFFSetDirectory` (3tiff)
