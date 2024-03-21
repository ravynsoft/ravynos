_TIFFRewriteField
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int _TIFFRewriteField(TIFF* tif, uint16_t tag, TIFFDataType in_datatype, tmsize_t count, void* data)

Description
-----------

:c:func:`_TIFFRewriteField`
Rewrite a field in the directory on disk without regard to updating the
TIFF directory structure in memory.  Currently only supported for field
that already exist in the on-disk directory.
Mainly used for updating stripoffset / stripbytecount values after
the directory is already on disk.

Return values
-------------

Returns zero on failure, and one on success.

See also
--------

:doc:`libtiff` (3tiff)
