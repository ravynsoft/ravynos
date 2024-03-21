TIFFFieldDataType
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: TIFFDataType TIFFFieldDataType(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldDataType` returns the data type stored in a TIFF field.
*fip* is a field information pointer previously returned by
:c:func:`TIFFFindField`, :c:func:`TIFFFieldWithTag`,
or :c:func:`TIFFFieldWithName`.

Return values
-------------

:c:func:`TIFFFieldDataType` returns a member of the enum type
:c:type:`TIFFDataType`.

See also
--------

:doc:`TIFFFieldName` (3tiff),
:doc:`TIFFFieldPassCount` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldReadCount` (3tiff),
:doc:`TIFFFieldTag` (3tiff),
:doc:`TIFFFieldWriteCount` (3tiff),
:doc:`libtiff` (3tiff)
