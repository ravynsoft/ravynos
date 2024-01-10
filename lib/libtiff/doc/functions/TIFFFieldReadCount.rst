TIFFFieldReadCount
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFFieldReadCount(const TIFFField* fip)

Description
-----------

:c:func:`TIFFFieldReadCount` returns the number of values available to be read
from the specified TIFF field; that is, the number of arguments that should be
supplied to :c:func:`TIFFGetField`.  For most field types this is a small
positive integer, typically 1 or 2, but there are some special values:

* :c:macro:`TIFF_VARIABLE` ``= -1`` indicates that a variable number of
  values is possible; then, a :c:type:`uint16_t` *count* argument and a
  pointer *data* argument must be supplied to :c:func:`TIFFGetField`.
* :c:macro:`TIFF_VARIABLE2` ``= -3`` is the same as :c:macro:`TIFF_VARIABLE`
  except that the *count* argument must have type :c:type:`uint32_t`.
* :c:macro:`TIFF_SPP` ``= -2`` indicates that the number of arguments is
  equal to the image's number of samples per pixel.

*fip* is a field information pointer previously returned by
:c:func:`TIFFFindField`, :c:func:`TIFFFieldWithTag`, or
:c:func:`TIFFFieldWithName`.

Return values
-------------

:c:func:`TIFFFieldReadCount` returns an integer.

See also
--------

:doc:`TIFFFieldDataType` (3tiff),
:doc:`TIFFFieldName` (3tiff),
:doc:`TIFFFieldPassCount` (3tiff),
:doc:`TIFFFieldQuery` (3tiff),
:doc:`TIFFFieldTag` (3tiff),
:doc:`TIFFFieldWriteCount` (3tiff),
:doc:`libtiff` (3tiff)
