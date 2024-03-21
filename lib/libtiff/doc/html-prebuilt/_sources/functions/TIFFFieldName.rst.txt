TIFFFieldName
=============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: const char* TIFFFieldName(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldName` returns the textual name for a TIFF field.
*fip* is a field information pointer previously returned by
:c:func:`TIFFFindField`, :c:func:`TIFFFieldWithTag`,
or :c:func:`TIFFFieldWithName`.

Return values
-------------

:c:func:`TIFFFieldName` returns a constant C string.

See also
--------

:doc:`TIFFFieldDataType` (3tiff),
:doc:`TIFFFieldPassCount` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldReadCount` (3tiff),
:doc:`TIFFFieldTag` (3tiff),
:doc:`TIFFFieldWriteCount` (3tiff),
:doc:`libtiff` (3tiff)
