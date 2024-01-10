TIFFbuffer
==========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>


.. c:function:: int TIFFReadBufferSetup(TIFF* tif, tdata_t buffer, tsize_t size)
.. c:function:: int TIFFWriteBufferSetup(TIFF* tif, tdata_t buffer, tsize_t size)

Description
-----------

The following routines are provided for client-control of the I/O buffers
used by the library. Applications need never use these routines; they are
provided only for "intelligent clients" that wish to optimize memory usage
and/or eliminate potential copy operations that can occur when working with
images that have data stored without compression.

:c:func:`TIFFReadBufferSetup` sets up the data buffer used to read raw (encoded)
data from a file. If the specified pointer is :c:macro:`NULL` (zero), then a
buffer of the appropriate size is allocated. Otherwise the caller must guarantee
that the buffer is large enough to hold any individual strip of raw data.
:c:func:`TIFFReadBufferSetup` returns a non-zero value if the setup was successful
and zero otherwise.

:c:func:`TIFFWriteBufferSetup` sets up the data buffer used to write raw (encoded)
data to a file. If the specified *size* is -1, then the buffer size is selected to
hold a complete tile or strip, or at least 8 kilobytes, whichever is greater. If
the specified *buffer* is :c:macro:`NULL` (zero), then a buffer of the appropriate
size is dynamically allocated.
:c:func:`TIFFWriteBufferSetup` returns a non-zero value if the setup was successful
and zero otherwise.

Diagnostics
-----------

``%s: No space for data buffer at scanline %ld``:

  :c:func:`TIFFReadBufferSetup` was unable to dynamically allocate space
  for a data buffer.

``%s: No space for output buffer``:

  :c:func:`TIFFWriteBufferSetup` was unable to dynamically allocate space
  for a data buffer.

See also
--------

:doc:`libtiff` (3tiff)
