TIFFFieldPassCount
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFFieldPassCount(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldPassCount` returns true (nonzero) if
:c:func:`TIFFGetField` and :c:func:`TIFFSetField`
expect a :c:var:`count` value to be passed before the actual data pointer.

:c:var:`fip` is a field information pointer previously returned by
:c:func:`TIFFFindField`,
:c:func:`TIFFFieldWithTag`,
:c:func:`TIFFFieldWithName`.

When a :c:var:`count` is required, it will be of type :c:type:`uint32_t`
if :c:func:`TIFFFieldReadCount` reports :c:macro:`TIFF_VARIABLE2`,
and of type :c:type:`uint16_t` otherwise.  Use :c:func:`TIFFFieldWriteCount`
for :c:func:`TIFFSetField`, respectively. (This distinction is
critical for use of :c:func:`TIFFGetField`, but normally not so for
use of :c:func:`TIFFSetField`.)

An alternative function for the :c:var:`count` value determination
is :c:func:`TIFFFieldSetGetCountSize`.

Return values
-------------

:c:func:`TIFFFieldPassCount` returns an integer that is always 1 (true)
or 0 (false).

See also
--------

:doc:`TIFFFieldDataType` (3tiff),
:doc:`TIFFFieldName` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldReadCount` (3tiff),
:doc:`TIFFFieldTag` (3tiff),
:doc:`TIFFFieldWriteCount` (3tiff),
:doc:`libtiff`
