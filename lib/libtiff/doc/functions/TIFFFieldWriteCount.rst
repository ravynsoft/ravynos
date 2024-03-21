TIFFFieldWriteCount
===================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFFieldWriteCount(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldWriteCount` returns the number of values to be written
into the specified TIFF field; that is, the number of arguments that should
be supplied to :c:func:`TIFFSetField`. For most field types this is a small
positive integer, typically 1 or 2, but there are some special values:

* :c:macro:`TIFF_VARIABLE` indicates that a variable number of values is
  possible; then, a :c:type:`uint16_t` *count* argument and a pointer *data*
  argument must be supplied to :c:func:`TIFFSetField`.
* :c:macro:`TIFF_VARIABLE2` is the same as :c:macro:`TIFF_VARIABLE` except
  that the *count* argument must have type :c:expr:`uint32_t`. (On most
  modern machines, this makes no practical difference, and the *count*
  argument can simply be an :c:expr:`int` in either case.)
* :c:type:`TIFF_SPP` indicates that the number of arguments must be equal
  to the image's number of samples per pixel.

*fip* is a field information pointer previously returned by
:c:func:`TIFFFindField`, :c:func:`TIFFFieldWithTag`, or
:c:func:`TIFFFieldWithName`.

For most field types, :c:func:`TIFFFieldWriteCount` returns the same value as
:c:func:`TIFFFieldReadCount`, but there are some exceptions.

Return values
-------------

:c:func:`TIFFFieldWriteCount` returns an integer.

See also
--------

:doc:`TIFFFieldDataType` (3tiff),
:doc:`TIFFFieldName` (3tiff),
:doc:`TIFFFieldPassCount` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldReadCount` (3tiff),
:doc:`TIFFFieldTag` (3tiff),
:doc:`libtiff` (3tiff)
