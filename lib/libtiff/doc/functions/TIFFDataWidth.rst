TIFFDataWidth
=============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFDataWidth(TIFFDataType type)

Description
-----------

:c:func:`TIFFDataWidth` returns the size of *type* within TIFF file in bytes.
Currently following data types are supported:

* :c:macro:`TIFF_BYTE`
* :c:macro:`TIFF_ASCII`
* :c:macro:`TIFF_SBYTE`
* :c:macro:`TIFF_UNDEFINED`
* :c:macro:`TIFF_SHORT`
* :c:macro:`TIFF_SSHORT`
* :c:macro:`TIFF_LONG`
* :c:macro:`TIFF_SLONG`
* :c:macro:`TIFF_FLOAT`
* :c:macro:`TIFF_IFD`
* :c:macro:`TIFF_RATIONAL`
* :c:macro:`TIFF_SRATIONAL`
* :c:macro:`TIFF_DOUBLE`
* :c:macro:`TIFF_LONG8`
* :c:macro:`TIFF_SLONG8`
* :c:macro:`TIFF_IFD8`

Return values
-------------

:c:func:`TIFFDataWidth` returns a number of bytes occupied by the item
of given type within the TIFF file. 0 returned when unknown data type supplied.

See also
--------

:doc:`libtiff` (3tiff)
