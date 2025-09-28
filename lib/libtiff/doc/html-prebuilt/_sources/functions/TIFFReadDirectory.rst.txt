TIFFReadDirectory
=================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFReadDirectory(TIFF* tif)

Description
-----------

Read the next directory in the specified file and make it the current
directory. Applications only need to call :c:func:`TIFFReadDirectory`
to read multiple subfiles in a single TIFF file—(the first directory
in a file is automatically read when :c:func:`TIFFOpen` is called.

Notes
-----

If the library is compiled with :c:macro:`STRIPCHOP_SUPPORT` enabled, then
images that have a single uncompressed strip or tile of data are
automatically treated as if they were made up of multiple strips or tiles of
approximately 8 kilobytes each. This operation is done only in-memory; it does
not alter the contents of the file. However, the construction of the "chopped
strips" is visible to the application through the number of strips [tiles]
returned by :c:func:`TIFFNumberOfStrips` [:c:func:`TIFFNumberOfTiles`].

Return values
-------------

If the next directory was successfully read, 1 is returned. Otherwise, 0 is
returned if an error was encountered, or if there are no more directories to
be read.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

``Seek error accessing TIFF directory``:

  An error occurred while positioning to the location of the
  directory.

``Wrong data type %d for field "%s"``:

  The tag entry in the directory had an incorrect data type.
  For example, an ``ImageDescription`` tag with a ``SHORT``
  data type.

``TIFF directory is missing required "%s" field``:

  The specified tag is required to be present by the TIFF
  5.0 specification, but is missing.
  The directory is (usually) unusable.

``%s: Rational with zero denominator``:

  A directory tag has a ``RATIONAL`` value whose denominator is zero.

``Incorrect count %d for field "%s" (%lu, expecting %lu); tag ignored``:

  The specified tag's count field is bad.
  For example, a count other than 1 for a ``SubFileType`` tag.

``Cannot handle different per-sample values for field "%s"``:

  The tag has ``SamplesPerPixel`` values and they are not all the same; e.g.
  ``BitsPerSample``.  The library is unable to handle images of this sort.

``Count mismatch for field "%s"; expecting %d, got %d``:

  The count field in a tag does not agree with the number expected by the
  library. This should never happen, so if it does, the library refuses to
  read the directory.

``Invalid TIFF directory; tags are not sorted in ascending order``:

  The directory tags are not properly sorted as specified
  in the TIFF 5.0 specification.  This error is not fatal.

``Ignoring unknown field with tag %d (0x%x)``:

  An unknown tag was encountered in the directory;
  the library ignores all such tags.

``TIFF directory is missing required "ImageLength" field``:

  The image violates the specification by not having a necessary field.
  There is no way for the library to recover from this error.

``TIFF directory is missing required "PlanarConfig" field``:

  The image violates the specification by not having a necessary field.
  There is no way for the library to recover from this error.

``TIFF directory is missing required "StripOffsets" field``:

  The image has multiple strips, but is missing the tag that
  specifies the file offset to each strip of data.
  There is no way for the library to recover from this error.

``TIFF directory is missing required "TileOffsets" field``:

  The image has multiple tiles, but is missing the tag that
  specifies the file offset to each tile of data.
  There is no way for the library to recover from this error.

``TIFF directory is missing required "StripByteCounts" field``:

  The image has multiple strips, but is missing the tag that
  specifies the size of each strip of data.
  There is no way for the library to recover from this error.

``TIFF directory is missing required "StripByteCounts" field, calculating from imagelength``:

  The image violates the specification by not having a necessary field.
  However, when the image is comprised of only one strip or tile, the
  library will estimate the missing value based on the file size.

``Bogus "StripByteCounts" field, ignoring and calculating from imagelength``:

  Certain vendors violate the specification by writing zero for
  the StripByteCounts tag when they want to leave the value
  unspecified.
  If the image has a single strip, the library will estimate
  the missing value based on the file size.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFCreateDirectory` (3tiff),
:doc:`TIFFCustomDirectory` (3tiff),
:doc:`TIFFquery` (3tiff),
:doc:`TIFFWriteDirectory` (3tiff),
:doc:`TIFFSetDirectory` (3tiff),
:doc:`/multi_page`,
:doc:`libtiff` (3tiff)
