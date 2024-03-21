TIFFtile
========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: void TIFFDefaultTileSize(TIFF* tif, uint32_t* tw, uint32_t* th)

.. c:function:: tsize_t TIFFTileSize(TIFF* tif)

.. c:function:: uint64_t TIFFTileSize64(TIFF* tif)

.. c:function:: tsize_t TIFFTileRowSize(TIFF* tif)

.. c:function:: uint64_t TIFFTileRowSize64(TIFF* tif)

.. c:function:: tsize_t TIFFVTileSize(TIFF* tif, uint32_t nrows)

.. c:function:: uint64_t TIFFVTileSize64(TIFF* tif, uint32_t nrows)

.. c:function:: ttile_t TIFFComputeTile(TIFF* tif, uint32_t x, uint32_t y, uint32_t z, tsample_t sample)

.. c:function:: int TIFFCheckTile(TIFF* tif, uint32_t x, uint32_t y, uint32_t z, tsample_t sample)

.. c:function:: ttile_t TIFFNumberOfTiles(TIFF* tif)

Description
-----------

:c:func:`TIFFDefaultTileSize` returns the pixel width and height of a
reasonable-sized tile; suitable for setting up the ``TileWidth`` and
``TileLength`` tags.  If the *tw* and *th* values passed in are
non-zero, then they are adjusted to reflect any compression-specific
requirements. The returned width and height are constrained to be a
multiple of 16 pixels to conform with the TIFF specification.

:c:func:`TIFFTileSize` returns the equivalent size for a tile of data
as it would be returned in a call to :c:func:`TIFFReadTile` or as it
would be expected in a call to :c:func:`TIFFWriteTile`.
:c:func:`TIFFTileSize64` returns a :c:type:`uint64_t` number.
If an error occurs, 0 is returned.

:c:func:`TIFFVTileSize` returns the number of bytes in a row-aligned
tile with *nrows* of data.
:c:func:`TIFFVTileSize64` returns a :c:type:`uint64_t` number.
If an error occurs, 0 is returned.

:c:func:`TIFFTileRowSize` returns the number of bytes of a row of data
in a tile.
:c:func:`TIFFTileRowSize64` returns a :c:type:`uint64_t` number.
If an error occurs, 0 is returned.

:c:func:`TIFFComputeTile` returns the tile that contains the specified
coordinates. A valid tile is always returned; out-of-range coordinate
values are clamped to the bounds of the image. The *x* and *y*
parameters are always used in calculating a tile. The *z*
parameter is used if the image is deeper than 1 slice
(``ImageDepth`` > 1).
The *sample* parameter is used only if data are organized in separate
planes (``PlanarConfiguration`` = 2).

:c:func:`TIFFCheckTile` returns a non-zero value if the supplied
coordinates are within the bounds of the image and zero otherwise. The
*x* parameter is checked against the value of the ``ImageWidth`` tag.
The *y* parameter is checked against the value of the ``ImageLength``
tag. The *z* parameter is checked against the value of the
``ImageDepth`` tag (if defined). The *sample* parameter is checked
against the value of the ``SamplesPerPixel`` parameter if the data are
organized in separate planes.

:c:func:`TIFFNumberOfTiles` returns the number of tiles in the image.

Diagnostics
-----------

None.

See also
--------

:doc:`TIFFReadEncodedTile` (3tiff),
:doc:`TIFFReadRawTile` (3tiff),
:doc:`TIFFReadTile` (3tiff),
:doc:`TIFFWriteEncodedTile` (3tiff),
:doc:`TIFFWriteRawTile` (3tiff),
:doc:`TIFFWriteTile` (3tiff),
:doc:`libtiff` (3tiff)
