TIFFAccessTagMethods
====================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: TIFFTagMethods *TIFFAccessTagMethods(TIFF* tif)

Description
-----------

This provides read/write access to the TIFFTagMethods within the TIFF
structure to application code without giving access to the private TIFF
structure.

Diagnostics
-----------

None

See also
--------

:doc:`libtiff` (3tiff)
