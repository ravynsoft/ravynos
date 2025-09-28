TIFFsize
========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: tsize_t TIFFRasterScanlineSize(TIFF* tif)

.. c:function:: uint64_t TIFFRasterScanlineSize64(TIFF* tif)

.. c:function:: tsize_t TIFFScanlineSize(TIFF* tif)

.. c:function:: uint64_t TIFFScanlineSize64(TIFF* tif)

Description
-----------

:c:func:`TIFFScanlineSize` returns the size in bytes of a row of data as
it would be returned in a call to :c:func:`TIFFReadScanline`, or as it
would be expected in a call to :c:func:`TIFFWriteScanline`.
Note that this number may be 1/samples-per-pixel if data is
stored as separate planes.
The `ScanlineSize` in case of YCbCrSubsampling is defined as the
strip size divided by the strip height, i.e. the size of a pack of vertical
subsampling lines divided by vertical subsampling. It should thus make
sense when multiplied by a multiple of vertical subsampling.
:c:func:`TIFFScanlineSize64` returns the size as :c:type:`uint64_t`.

:c:func:`TIFFRasterScanlineSize` returns the size in bytes of a complete
decoded and packed raster scanline. Note that this value may be different
from the value returned by :c:func:`TIFFScanlineSize` if data is stored
as separate planes.
:c:func:`TIFFRasterScanlineSize64` returns the size as :c:type:`uint64_t`.

Diagnostics
-----------

None.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFReadScanline` (3tiff),
:doc:`libtiff` (3tiff)
