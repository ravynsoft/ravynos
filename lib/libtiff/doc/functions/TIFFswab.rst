TIFFswab
========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: const unsigned char* TIFFGetBitRevTable(int reversed)

.. c:function:: void TIFFReverseBits(u_char* data, unsigned long nbytes)

.. c:function:: void TIFFSwabShort(uint16_t* data)

.. c:function:: void TIFFSwabLong(uint32_t* data)

.. c:function:: void TIFFSwabLong8(uint64_t* lp)

.. c:function:: void TIFFSwabFloat(float* fp)

.. c:function:: void TIFFSwabDouble(double *dp)

.. c:function:: void TIFFSwabArrayOfShort(uint16_t* wp, tmsize_t n)

.. c:function:: void TIFFSwabArrayOfTriples(uint8_t* tp, tmsize_t n)

.. c:function:: void TIFFSwabArrayOfLong(uint32_t* lp, tmsize_t n)

.. c:function:: void TIFFSwabArrayOfLong8(uint64_t* lp, tmsize_t n)

.. c:function:: void TIFFSwabArrayOfFloat(float* fp, tmsize_t n)

.. c:function:: void TIFFSwabArrayOfDouble(double* dp, tmsize_t n)

Description
-----------

The following routines are used by the library to swap 16-, 32- and 64-bit
data and to reverse the order of bits in bytes.

:c:func:`TIFFSwabShort` and :c:func:`TIFFSwabLong` and :c:func:`TIFFSwabFloat`
swap the bytes in a single 16- and 32-bit item, respectively.

:c:func:`TIFFSwabLong8` and :c:func:`TIFFSwabDouble`
swap the bytes in a single 64-bit item.

:c:func:`TIFFSwabArrayOfTriples` swap the first and the third byte of
each triple (three bytes) within the byte array. The second byte of each
triple stays untouched.

:c:func:`TIFFSwabArrayOfShort` and  :c:func:`TIFFSwabArrayOfLong`,
:c:func:`TIFFSwabArrayOfFloat` swap the bytes in an array of 16- and 32-bit
items, respectively.

:c:func:`TIFFSwabArrayOfLong8` and :c:func:`TIFFSwabArrayOfDouble`
swap the bytes in an array of 64-bit items.

:c:func:`TIFFReverseBits` replaces each byte in *data* with the
equivalent bit-reversed value. This operation is performed with a
lookup table, which is returned using the :c:func:`TIFFGetBitRevTable`
function.  The *reversed* parameter specifies which table should be
returned. Supply *1* if you want bit reversal table. Supply *0* to get
the table that do not reverse bit values. It is a lookup table that can
be used as an "identity function"; i.e. :c:expr:`TIFFNoBitRevTable[n] == n`.

Diagnostics
-----------

None.

See also
--------

:doc:`libtiff` (3tiff)
