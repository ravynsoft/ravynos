TIFFStrileQuery
===============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: uint64_t TIFFGetStrileByteCount(TIFF* tif, uint32_t strile);

.. c:function:: uint64_t TIFFGetStrileOffset(TIFF* tif, uint32_t strile);

.. c:function:: uint64_t TIFFGetStrileByteCountWithErr(TIFF* tif, uint32_t strile, int *pbErr);

.. c:function:: uint64_t TIFFGetStrileOffsetWithErr(TIFF* tif, uint32_t strile, int *pbErr);

Description
-----------

Make defer strile offset/bytecount loading available at runtime
and add per-strile offset/bytecount loading capabilities. Part of
this commit makes the behaviour that was previously met when ``libtiff``
was compiled with ``-DDEFER_STRILE_LOAD`` available for default builds.

When specifying the new ``D`` (Deferred) :c:func:`TIFFOpen` flag,
the loading of strile offset/bytecount is defered.
In that mode, the ``StripOffsets`` / ``StripByteCounts`` or
``TileOffsets`` / ``TileByteCounts`` arrays are only loaded when first
accessed. This can speed-up the opening of files stored on the network
when just metadata retrieval is needed.

Another addition is the capability of loading only the values of
the offset/bytecount of the strile of interest instead of the
whole array. This is enabled with the new ``O`` (Ondemand) flag of
:c:func:`TIFFOpen` (which implies ``D``).

The public :c:func:`TIFFGetStrileOffset`, :c:func:`TIFFGetStrileOffsetWithErr`,
:c:func:`TIFFGetStrileByteCount` and :c:func:`TIFFGetStrileByteCountWithErr`
functions have been added to API.
They are of particular interest when using sparse files (with
``offset == bytecount == 0``) and you want to detect if a strile is
present or not without decompressing the data, or updating an
existing sparse file.

:c:func:`TIFFGetStrileByteCount` returns the value of the TileByteCounts /
StripByteCounts array for the specified tile/strile.

:c:func:`TIFFGetStrileByteCountWithErr` additionally provides *pbErr*
as an *int* pointer to an error return variable,
which is set to "0" for successful return or to "1" for an error return.

:c:func:`TIFFGetStrileOffset` returns the value of the TileOffsets /
StripOffsets array for the specified tile/strile.

:c:func:`TIFFGetStrileOffsetWithErr` additionally provides *pbErr*
as an *int* pointer to an error return variable,
which is set to "0" for successful return or to "1" for an error return.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen`  (3tiff),
:doc:`TIFFDeferStrileArrayWriting` (3tiff)

