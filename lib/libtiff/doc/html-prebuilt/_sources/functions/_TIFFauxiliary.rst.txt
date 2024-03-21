_TIFFauxiliary
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: uint32_t _TIFFClampDoubleToUInt32(double val)

.. c:function:: uint32_t _TIFFMultiply32(TIFF* tif, uint32_t first, uint32_t second, const char* where)

.. c:function:: uint64_t _TIFFMultiply64(TIFF* tif, uint64_t first, uint64_t second, const char* where)

Description
-----------

:c:func:`_TIFFClampDoubleToUInt32` clamps double values into the range
of :c:type:`uint32_t` (i.e. 0 .. 0xFFFFFFFF)

:c:func:`_TIFFMultiply32` and :c:func:`_TIFFMultiply64` checks for
an integer overflow of the multiplication result and return the multiplication
result or `0` if an overflow would happen.
The string `where` is printed in the error message in case an overflow
happens and can be used to indicate where the function was called.

See also
--------

:doc:`libtiff` (3tiff),
