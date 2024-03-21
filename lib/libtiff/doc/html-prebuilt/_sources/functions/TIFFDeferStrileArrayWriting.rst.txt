TIFFDeferStrileArrayWriting
===========================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFDeferStrileArrayWriting(TIFF* tif)

.. c:function:: int TIFFForceStrileArrayWriting(TIFF* tif)

Description
-----------

:c:func:`TIFFDeferStrileArrayWriting` is an advanced writing function
that must be used in a particular sequence, and generally together
with  :c:func:`TIFFForceStrileArrayWriting`, to achieve its intended
effect. Their aim is to control when and where the
``StripOffsets`` / ``StripByteCounts`` or ``TileOffsets`` / ``TileByteCounts``
arrays are written into the file.

The purpose of this is to generate 'cloud-optimized geotiff' files where
the first KB of the file only contain the IFD entries without the potentially
large strile arrays. Those are written afterwards.

More precisely, when :c:func:`TIFFWriteCheck` is called, the tag entries for
those arrays will be written with type = count = offset = 0 as a temporary value.

Its effect is only valid for the current directory, and before
:c:func:`TIFFWriteDirectory` is first called, and  will be reset
when changing directory.

The typical sequence of calls is:

.. highlight:: c

::

 TIFFOpen()
 /* or TIFFCreateDirectory(tif) */
 /* Set fields with calls to TIFFSetField(tif, ...) */
 TIFFDeferStrileArrayWriting(tif)
 TIFFWriteCheck(tif, ...)
 TIFFWriteDirectory(tif)
 /* ... potentially create other directories and come back to the above directory */
 TIFFForceStrileArrayWriting(tif) /* emit the arrays at the end of file */

Returns
-------

1 in case of success, 0 otherwise.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`libtiff` (3tiff)
