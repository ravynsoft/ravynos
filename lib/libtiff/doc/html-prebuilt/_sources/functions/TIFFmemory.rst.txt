TIFFmemory
==========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tdata_t _TIFFmalloc(tsize_t size)

.. c:function:: tdata_t _TIFFrealloc(tdata_t buffer, tsize_t size)

.. c:function:: void _TIFFfree(tdata_t buffer)

.. c:function:: void _TIFFmemset(tdata_t s, int c, tsize_t n)

.. c:function:: void _TIFFmemcpy(tdata_t dest, const tdata_t src, tsize_t n)

.. c:function:: int _TIFFmemcmp(const tdata_t s1, const tdata_ts2, tsize_t n)

.. c:function:: void* _TIFFCheckMalloc(TIFF* tif, tmsize_t nmemb, tmsize_t elem_size, const char* what)

.. c:function:: void* _TIFFCheckRealloc(TIFF* tif, void* buffer, tmsize_t nmemb, tmsize_t elem_size, const char* what)

Description
-----------

These routines are provided for writing portable software that uses
:program:`libtiff`; they hide any memory-management related issues, such as
dealing with segmented architectures found on 16-bit machines.

:c:func:`_TIFFmalloc` and :c:func:`_TIFFrealloc` are used to dynamically
allocate and reallocate memory used by :program:`libtiff`; such as memory
passed into the I/O routines. Memory allocated through these interfaces is
released back to the system using the :c:func:`_TIFFfree` routine.

Memory allocated through one of the above interfaces can be set to a known
value using :c:func:`_TIFFmemset`, copied to another memory location using
:c:func:`_TIFFmemcpy`, or compared for equality using :c:func:`_TIFFmemcmp`.
These routines conform to the equivalent C routines:
:c:func:`memset`, :c:func:`memcpy`, :c:func:`memcmp`, respectively.

:c:func:`_TIFFCheckMalloc` and :c:func:`_TIFFCheckRealloc` are checking for
integer overflow before calling :c:func:`_TIFFmalloc` and :c:func:`_TIFFrealloc`,
respectively.

Diagnostics
-----------

None.

See also
--------

malloc (3),
memory (3),
:doc:`libtiff` (3tiff)
