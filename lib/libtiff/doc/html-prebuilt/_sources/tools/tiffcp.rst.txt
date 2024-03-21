tiffcp
======

.. program:: tiffcp

Synopsis
--------

**tiffcp** [ *options* ] *src1.tif* … *srcN.tif* *dst.tif*

Description
-----------
:program:`tiffcp` combines one or more files created according
to the Tag Image File Format, Revision 6.0 into a single TIFF file.
Because the output file may be compressed using a different
algorithm than the input files, :program:`tiffcp` is most often
used to convert between different compression schemes.

By default, :program:`tiffcp` will copy all the understood tags in a
TIFF directory of an input file to the associated directory in the output file.

:program:`tiffcp` can be used to reorganize the storage characteristics of data
in a file, but it is explicitly intended to not alter or convert
the image data content in any way.

Options
-------

.. option:: -a

  Append to an existing output file instead of overwriting it.

.. option:: -b image

  subtract the following monochrome image from all others
  processed.  This can be used to remove a noise bias
  from a set of images.  This bias image is typically an
  image of noise the camera saw with its shutter closed.

.. option:: -B

  Force output to be written with Big-Endian byte order.
  This option only has an effect when the output file is created or
  overwritten and not when it is appended to.

.. option:: -C

  Suppress the use of "strip chopping" when reading images
  that have a single strip/tile of uncompressed data.

.. option:: -c

  Specify the compression to use for data written to the output file:
  :command:`-c none` for no compression,
  :command:`-c packbits` for PackBits compression,
  :command:`-c lzw` for Lempel-Ziv & Welch compression,
  :command:`-c zip` for Deflate compression,
  :command:`-c lzma` for LZMA2 compression,
  :command:`-c jpeg` for baseline JPEG compression,
  :command:`-c g3` for CCITT Group 3 (T.4) compression,
  :command:`-c g4` for CCITT Group 4 (T.6) compression, or
  :command:`-c sgilog` for SGILOG compression.

  By default :program:`tiffcp` will compress data according to the
  value of the ``Compression`` tag found in the source file.

  The CCITT Group 3 and Group 4 compression algorithms can only
  be used with bilevel data.

  Group 3 compression can be specified together with several
  T.4-specific options:

  * ``1d`` for 1-dimensional encoding,
  * ``2d`` for 2-dimensional encoding, and
  * ``fill`` to force each encoded scanline to be zero-filled so that the
    terminating EOL code lies on a byte boundary.

  Group 3-specific options are specified by appending a ``:``-separated
  list to the ``g3`` option; e.g. :command:`-c g3:2d:fill`
  to get 2D-encoded data with byte-aligned EOL codes.

  LZW, Deflate and LZMA2 compression can be specified together with a 
  ``predictor`` value. A predictor value of 2 causes each scanline of the output image to
  undergo horizontal differencing before it is encoded; a value of 1 forces each
  scanline to be encoded without differencing. A value 3 is for floating point
  predictor which you can use if the encoded data are in floating point format.
  LZW-specific options are specified by appending a ``:``-separated list to the
  ``lzw`` option; e.g. :command:`-c lzw:2` for LZW compression with horizontal differencing.

  Deflate and LZMA2 encoders support various compression levels (or encoder presets) set as
  character ``p`` and a preset number. ``p1`` is the fastest one with the worst
  compression ratio and ``p9`` is the slowest but with the best possible ratio;
  e.g. :command:`-c zip:3:p9` for
  Deflate encoding with maximum compression level and floating point predictor.

  For the Deflate codec, and in a libtiff build with libdeflate enabled, ``p12`` is
  actually the maximum level.

  For the Deflate codec, and in a libtiff build with libdeflate enabled, ``s0`` can be used to
  require zlib to be used, and ``s1`` for libdeflate (defaults to libdeflate when
  it is available).

.. option:: -f fillorder

  Specify the bit fill order to use in writing output data.  By default, :program:`tiffcp`
  will create a new file with the same fill order as the original.  Specifying :command:`\-f lsb2msb`
  will force data to be written with the ``FillOrder`` tag set to ``LSB2MSB``, while
  :command:`\-f msb2lsb` will force data to be written with the ``FillOrder`` tag set to
  ``MSB2LSB``.

.. option:: -i

  Ignore non-fatal read errors and continue processing of the input file.

.. option:: -l

  Specify the length of a tile (in pixels).

  :program:`tiffcp` attempts to set the tile dimensions so
  that no more than 8 kilobytes of data appear in a tile.

.. option:: -L

  Force output to be written with Little-Endian byte order.
  This option only has an effect when the output file is created or
  overwritten and not when it is appended to.

.. option:: -M

  Suppress the use of memory-mapped files when reading images.

.. option:: -o offset

  Set initial directory offset.

.. option:: -p

  Specify the planar configuration to use in writing image data
  that has one 8-bit sample per pixel. By default, :program:`tiffcp`
  will create a new file with the same planar configuration as
  the original.  Specifying :command:`\-p contig`
  will force data to be written with multi-sample data packed
  together, while :command:`-p separate`
  will force samples to be written in separate planes.

.. option:: -r

  Specify the number of rows (scanlines) in each strip of data
  written to the output file.  By default (or when value **0**
  is specified), :program:`tiffcp` attempts to set the rows/strip
  that no more than 8 kilobytes of data appear in a strip. If you specify
  special value **-1** it will results in infinite number of the rows per
  strip. The entire image will be the one strip in that case.

.. option:: -s

  Force the output file to be written with data organized in strips
  (rather than tiles).

.. option:: -t

  Force the output file to be written with data organized in tiles (rather than
  strips). options can be used to force the resultant image to be written as
  strips or tiles of data, respectively.

.. option:: -w

  Specify the width of a tile (in pixels). :program::`tiffcp`
  attempts to set the tile dimensions so that no more than 8 kilobytes of data
  appear in a tile.

.. option:: -x

  Force the output file to be written with ``PAGENUMBER`` value in sequence.

.. option:: -8

  Write BigTIFF instead of classic TIFF format.

.. option:: -,= character

  substitute *character* for ``,``
  in parsing image directory indices
  in files.  This is necessary if filenames contain commas.
  Note that ``-,=``
  with whitespace immediately following will disable
  the special meaning of the ``,`` entirely.  See examples.

.. option:: -m size

  Set maximum memory allocation size (in MiB). The default is 256MiB.
  Set to 0 to disable the limit.

Examples
--------

The following concatenates two files and writes the result using LZW encoding:

.. highlight:: shell

::

    tiffcp -c lzw a.tif b.tif result.tif

To convert a G3 1d-encoded TIFF to a single strip of G4-encoded data the following might be used:

::

    tiffcp -c g4 -r 10000 g3.tif g4.tif

(1000 is just a number that is larger than the number of rows in
the source file.)

To extract a selected set of images from a multi-image TIFF file, the file
name may be immediately followed by a ``,`` separated list of image directory
indices.  The first image is always in directory 0.  Thus, to copy the 1st and
3rd images of image file :file:`album.tif` to :file:`result.tif`:

::

    tiffcp album.tif,0,2 result.tif

A trailing comma denotes remaining images in sequence.  The following command
will copy all image with except the first one:

::

    tiffcp album.tif,1, result.tif

Given file :file:`CCD.tif` whose first image is a noise bias
followed by images which include that bias,
subtract the noise from all those images following it
(while decompressing) with the command:

::

    tiffcp -c none -b CCD.tif CCD.tif,1, result.tif

If the file above were named :file:`CCD,X.tif`, the ``-,=``
option would be required to correctly parse this filename with image numbers,
as follows:

::

    tiffcp -c none -,=% -b CCD,X.tif CCD,X%1%.tif result.tif

See also
--------

:doc:`tiffinfo` (1),
:doc:`tiffdump` (1),
:doc:`tiffsplit` (1),
:doc:`/functions/libtiff` (3tiff)
