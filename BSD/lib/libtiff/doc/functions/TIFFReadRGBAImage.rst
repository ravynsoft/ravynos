TIFFReadRGBAImage
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:macro:: TIFFGetR(abgr)

    :c:expr:`((abgr) & 0xff)`

.. c:macro:: TIFFGetG(abgr)

    :c:expr:`(((abgr) >> 8) & 0xff)`

.. c:macro:: TIFFGetB(abgr)

    :c:expr:`(((abgr) >> 16) & 0xff)`

.. c:macro:: TIFFGetA(abgr)

    :c:expr:`(((abgr) >> 24) & 0xff)`

.. c:function:: int TIFFReadRGBAImage(TIFF* tif, uint32_t width, uint32_t height, uint32_t* raster, int stopOnError)

.. c:function:: int TIFFReadRGBAImageOriented(TIFF* tif, uint32_t width, uint32_t height, uint32_t * raster, int orientation, int stopOnError)

Description
-----------

:c:func:`TIFFReadRGBAImage` reads a strip- or tile-based image into memory,
storing the result in the user supplied *raster*.
The raster is assumed to be an array of *width* × *height* 32-bit entries,
where *width* must be less than or equal to the width of the image
(*height* may be any non-zero size).
If the raster dimensions are smaller than the image, the image data is
cropped to the raster bounds.
If the raster height is greater than that of the image, then the image data
are placed in the lower part of the raster.
(Note that the raster is assume to be organized such that the pixel
at location (*x*, *y*) is *raster* [ *y* × *width* + *x* ];
with the raster origin in the lower-left hand corner.)

:c:func:`TIFFReadRGBAImageOriented` works like :c:func:`TIFFReadRGBAImage`
except that the user can specify the raster origin position with the
*orientation* parameter. Four orientations are supported:

* :c:macro:`ORIENTATION_TOPLEFT`: origin in top-left corner,
* :c:macro:`ORIENTATION_TOPRIGHT`: origin in top-right corner,
* :c:macro:`ORIENTATION_BOTLEFT`: origin in bottom-left corner
* :c:macro:`ORIENTATION_BOTRIGHT`: origin in bottom-right corner.

If you choose :c:macro:`ORIENTATION_BOTLEFT`, the result will be the same
as returned by the :c:func:`TIFFReadRGBAImage`.

Raster pixels are 8-bit packed red, green, blue, alpha samples.
The macros :c:macro:`TIFFGetR`, :c:macro:`TIFFGetG`, :c:macro:`TIFFGetB`,
and :c:macro:`TIFFGetA` should be used to access individual samples.
Images without Associated Alpha matting information have a constant
Alpha of 1.0 (255).

:c:func:`TIFFReadRGBAImage` converts non-8-bit images by scaling sample
values.  Palette, grayscale, bilevel, CMYK, and YCbCr images are
converted to RGB transparently.
Raster pixels are returned uncorrected by any colorimetry information
present in the directory.

The parameter *stopOnError* specifies how to act if an error is
encountered while reading the image. If *stopOnError* is non-zero, then
an error will terminate the operation; otherwise :c:func:`TIFFReadRGBAImage`
will continue processing data until all the possible data in the
image have been requested.

Notes
-----

In C++ the *stopOnError* parameter defaults to 0.

``SamplesPerPixel`` must be either 1, 2, 4, 8, or 16 bits.
Colorimetric samples/pixel must be either 1, 3, or 4 (i.e.
``SamplesPerPixel`` minus ``ExtraSamples``).

Palettte image colormaps that appear to be incorrectly written
as 8-bit values are automatically scaled to 16-bits.

:c:func:`IFFReadRGBAImage` is just a wrapper around the more general
:doc:`TIFFRGBAImage` facilities.

Return values
-------------

1 is returned if the image was successfully read and converted.
Otherwise, 0 is returned if an error was encountered and
*stopOnError* is zero.

.. TODO: Specify, what the return value is if an error occurs and stopOnError was non-zero.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``"Sorry, can not handle %d-bit pictures"``:

  The image had ``BitsPerSample`` other than 1, 2, 4, 8, or 16.

``"Sorry, can not handle %d-channel images"``:

  The image had ``SamplesPerPixel`` other than 1, 3, or 4.

``Missing needed "PhotometricInterpretation" tag``:

  The image did not have a tag that describes how to display
  the data.

``No "PhotometricInterpretation" tag, assuming RGB``:

  The image was missing a tag that describes how to display it,
  but because it has 3 or 4 samples/pixel, it is assumed to be
  RGB.

``No "PhotometricInterpretation" tag, assuming min-is-black``:

  The image was missing a tag that describes how to display it,
  but because it has 1 sample/pixel, it is assumed to be a grayscale
  or bilevel image.

``"No space for photometric conversion table"``:

  There was insufficient memory for a table used to convert
  image samples to 8-bit RGB.

``Missing required "Colormap" tag``:

  A Palette image did not have a required ``Colormap`` tag.

``"No space for tile buffer"``:

  There was insufficient memory to allocate an i/o buffer.

``"No space for strip buffer"``:

  There was insufficient memory to allocate an i/o buffer.

``"Can not handle format"``:

  The image has a format (combination of ``BitsPerSample``,
  ``SamplesPerPixel``, and ``PhotometricInterpretation``)
  that :c:func:`TIFFReadRGBAImage` can not handle.

``"No space for B&W mapping table"``:

  There was insufficient memory to allocate a table used to map
  grayscale data to RGB.

``"No space for Palette mapping table"``:

  There was insufficient memory to allocate a table used to map
  data to 8-bit RGB.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFRGBAImage` (3tiff),
:doc:`TIFFReadRGBAStrip` (3tiff),
:doc:`TIFFReadRGBATile` (3tiff),
:doc:`libtiff` (3tiff)
