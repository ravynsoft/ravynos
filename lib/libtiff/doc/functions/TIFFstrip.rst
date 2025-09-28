TIFFstrip
=========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: uint32_t TIFFDefaultStripSize(TIFF* tif, uint32_t estimate)

.. c:function:: tmsize_t TIFFStripSize(TIFF* tif)

.. c:function:: uint64_t TIFFStripSize64(TIFF* tif)

.. c:function:: tmsize_t TIFFVStripSize(TIFF* tif, uint32_t nrows)

.. c:function:: uint64_t TIFFVStripSize64(TIFF* tif, uint32_t nrows)

.. c:function:: tmsize_t TIFFRawStripSize(TIFF* tif, uint32_t strip)

.. c:function:: uint64_t TIFFRawStripSize64(TIFF* tif, uint32_t strip)

.. c:function:: tstrip_t TIFFComputeStrip(TIFF* tif, uint32_t row, tsample_t sample)

.. c:function:: tstrip_t TIFFNumberOfStrips(TIFF* tif)

.. c:function:: int TIFFSetupStrips(TIFF* tif)

Description
-----------

:c:func:`TIFFDefaultStripSize` returns the number of rows for a
reasonable-sized strip according to the current settings of the
``ImageWidth``, ``BitsPerSample`` and ``SamplesPerPixel``,
tags and any compression-specific requirements. If the *estimate*
parameter, sf non-zero, then it is taken as an estimate of the desired strip
size and adjusted according to any compression-specific requirements. The
value returned by this function is typically used to define the
``RowsPerStrip`` tag. In lieu of any unusual requirements
``TIFFDefaultStripSize`` tries to create strips that have approximately
8 kilobytes of uncompressed data.

:c:func:`TIFFStripSize` returns the equivalent size for a strip of data
as it would be returned in a call to :c:func:`TIFFReadEncodedStrip`
or as it would be expected in a call to :c:func:`TIFFWriteEncodedStrip`.
If an error occurs, 0 is returned.

:c:func:`TIFFStripSize64` returns the equivalent size for a strip of data
as :c:type:`uint64_t`.
If an error occurs, 0 is returned.

:c:func:`TIFFVStripSize` returns the number of bytes in a strip with
*nrows* rows of data.
If an error occurs, 0 is returned.

:c:func:`TIFFVStripSize64` returns the number of bytes in a strip with
*nrows* rows of data as :c:type:`uint64_t`.
If an error occurs, 0 is returned.

:c:func:`TIFFRawStripSize` returns the number of bytes in a raw strip
(i.e. not decoded).
If an error occurs, 0xFFFFFFFF `(=(tmsize_t(-1))` is returned.

:c:func:`TIFFRawStripSize64` returns the number of bytes in a raw strip
as :c:type:`uint64_t`.
If an error occurs, 0xFFFFFFFF `(=(uint64_t(-1))` is returned.

:c:func:`TIFFComputeStrip` returns the strip that contains the specified
coordinates. A valid strip is always returned; out-of-range coordinate
values are clamped to the bounds of the image. The *row* parameter is
always used in calculating a strip. The *sample* parameter is used only
if data are organized in separate planes (``PlanarConfiguration`` = 2).

:c:func:`TIFFNumberOfStrips` returns the number of strips in the image.

:c:func:`TIFFSetupStrips` setup  or reset strip parameters and strip array memory.

Diagnostics
-----------

None.

See also
--------

:doc:`TIFFReadEncodedStrip` (3tiff),
:doc:`TIFFReadRawStrip` (3tiff),
:doc:`TIFFWriteEncodedStrip` (3tiff),
:doc:`TIFFWriteRawStrip` (3tiff),
:doc:`libtiff` (3tiff)
