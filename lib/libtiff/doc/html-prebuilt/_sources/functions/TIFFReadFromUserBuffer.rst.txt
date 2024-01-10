TIFFReadFromUserBuffer
======================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFReadFromUserBuffer(TIFF* tif, uint32_t strile, void* inbuf, tmsize_t insize, void* outbuf, tmsize_t outsize)

Description
-----------

Use the provided input buffer (`inbuf`, `insize`) and decompress it
into (`outbuf`, `outsize`). This function replaces the use of
:c:func:`TIFFReadEncodedStrip` / :c:func:`TIFFReadEncodedTile`
when the user can provide the buffer for the input data, for example when
he wants to avoid ``libtiff`` to read the strile offset/count values from the
``StripOffsets`` / ``StripByteCounts`` or ``TileOffsets`` /
``TileByteCounts`` arrays. `inbuf` content must be writable
(if bit reversal is needed).


Return values
-------------

Returns 1 in case of success, 0 otherwise.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadRawStrip` (3tiff),
:doc:`TIFFReadScanline` (3tiff),
:doc:`TIFFReadEncodedStrip` (3tiff),
:doc:`TIFFReadEncodedTile` (3tiff),
:doc:`libtiff` (3tiff),
