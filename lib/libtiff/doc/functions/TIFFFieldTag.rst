TIFFFieldTag
============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: uint32_t TIFFFieldTag(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldTag` returns the numeric tag value for a TIFF field.
This can be compared to various constants exported by the :program:`libtiff`
header files, such as :c:macro:`TIFFTAG_IMAGEWIDTH`.

*fip* is a field information pointer previously returned by
:c:func:`TIFFFindField`, :c:func:`TIFFFieldWithTag`, or
:c:func:`TIFFFieldWithName`.

Return values
-------------

:c:func:`TIFFFieldTag` returns an integer tag value.

See also
--------

:doc:`TIFFFieldDataType` (3tiff),
:doc:`TIFFFieldName` (3tiff),
:doc:`TIFFFieldPassCount` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldReadCount` (3tiff),
:doc:`TIFFFieldWriteCount` (3tiff),
:doc:`libtiff` (3tiff)
