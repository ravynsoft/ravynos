TIFFSetField
============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>
    #include <stdarg.h>"

.. c:function:: int TIFFSetField(TIFF* tif, ttag_t tag, ...)

.. c:function:: int TIFFVSetField(TIFF* tif, ttag_t tag, va_list ap)

.. c:function:: int TIFFUnsetField(TIFF* tif, uint32_t tag)

Description
-----------

:c:func:`TIFFSetField` sets the value of a field or pseudo-tag in the
current directory associated with the open TIFF file *tif*.
(A *pseudo-tag* is a parameter that is used to control the operation of
the TIFF library but whose value is not read or written to the underlying
file.)  To set the value of a field the file must have been previously
opened for writing with :c:func:`TIFFOpen`.
Pseudo-tags can be set whether the file was opened for reading or writing.
The field is identified by *tag*, one of the values defined in the include
file :file:`tiff.h` (see also the table below).
The actual value is specified using a variable argument list, as
prescribed by the :file:`stdarg.h` interface.

:c:func:`TIFFVSetField` is functionally equivalent to :c:func:`TIFFSetField`
except that it takes a pointer to a variable argument list.
:c:func:`TIFFVSetField` is useful for writing routines that are layered on
top of the functionality provided by :c:func:`TIFFSetField`.

:c:func:`TIFFUnsetField` clears the contents of the field in the internal
structure. If it is a custom field, it is removed from the list of known tags.

The tags understood by :program:`libtiff`, the number of parameter values,
and the expected types for the parameter values are shown below.
The data types are:

.. list-table:: Tag data types
    :widths: 5 15
    :header-rows: 1

    * - Type
      - Description

    * - :c:expr:`char*`
      - a null-terminated string corresponding to the ``ASCII`` data type
    * - :c:expr:`uint16_t`
      - an unsigned 16-bit value
    * - :c:expr:`uint32_t`
      - an unsigned 32-bit value;
    * - :c:expr:`uint16_t*`
      - an array of unsigned 16-bit values.
    * - :c:expr:`void*`
      - an array of data values of unspecified type.

Consult the TIFF specification for information on the meaning of each tag.

  .. list-table:: Tag properties
    :widths: 5 3 5 10
    :header-rows: 1

    * - Tag name
      - Count
      - Types
      - Notes


    * - :c:macro:`TIFFTAG_ARTIST`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_BADFAXLINES`
      - 1
      - :c:expr:`uint32_t`
      -
    * - :c:macro:`TIFFTAG_BITSPERSAMPLE`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_CLEANFAXDATA`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_COLORMAP`
      - 3
      - :c:expr:`uint16_t*`
      - :c:expr:`1<<BitsPerSample` arrays
    * - :c:macro:`TIFFTAG_COMPRESSION`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_CONSECUTIVEBADFAXLINES`
      - 1
      - :c:expr:`uint32_t`
      -
    * - :c:macro:`TIFFTAG_COPYRIGHT`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_DATETIME`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_DOCUMENTNAME`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_DOTRANGE`
      - 2
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_EXTRASAMPLES`
      - 2
      - :c:expr:`uint16_t`, :c:expr:`uint16_t*`
      - † count, types array
    * - :c:macro:`TIFFTAG_FAXFILLFUNC`
      - 1
      - :c:expr:`TIFFFaxFillFunc`
      - G3/G4 compression pseudo-tag
    * - :c:macro:`TIFFTAG_FAXMODE`
      - 1
      - :c:expr:`int`
      - † G3/G4 compression pseudo-tag
    * - :c:macro:`TIFFTAG_FILLORDER`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_GROUP3OPTIONS`
      - 1
      - :c:expr:`uint32_t`
      - †
    * - :c:macro:`TIFFTAG_GROUP4OPTIONS`
      - 1
      - :c:expr:`uint32_t`
      - †
    * - :c:macro:`TIFFTAG_HALFTONEHINTS`
      - 2
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_HOSTCOMPUTER`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_ICCPROFILE`
      - 2
      - :c:expr:`uint32_t`, :c:expr:`void*`
      - count, profile data*
    * - :c:macro:`TIFFTAG_IMAGEDEPTH`
      - 1
      - :c:expr:`uint32_t`
      - †
    * - :c:macro:`TIFFTAG_IMAGEDESCRIPTION`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_IMAGELENGTH`
      - 1
      - :c:expr:`uint32_t`
      -
    * - :c:macro:`TIFFTAG_IMAGEWIDTH`
      - 1
      - :c:expr:`uint32_t`
      - †
    * - :c:macro:`TIFFTAG_INKNAMES`
      - 2
      - :c:expr:`uint16_t`, :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_INKSET`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_JPEGCOLORMODE`
      - 1
      - :c:expr:`int`
      - † JPEG pseudo-tag
    * - :c:macro:`TIFFTAG_JPEGQUALITY`
      - 1
      - :c:expr:`int`
      - JPEG pseudo-tag
    * - :c:macro:`TIFFTAG_JPEGTABLES`
      - 2
      - :c:expr:`uint32_t*`, :c:expr:`void*`
      - † count, tables
    * - :c:macro:`TIFFTAG_JPEGTABLESMODE`
      - 1
      - :c:expr:`int`
      - † JPEG pseudo-tag
    * - :c:macro:`TIFFTAG_MAKE`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_MATTEING`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_MAXSAMPLEVALUE`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_MINSAMPLEVALUE`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_MODEL`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_ORIENTATION`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_PAGENAME`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_PAGENUMBER`
      - 2
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_PHOTOMETRIC`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_PHOTOSHOP`
      - ?
      - :c:expr:`uint32_t`, :c:expr:`void*`
      - count, data
    * - :c:macro:`TIFFTAG_PLANARCONFIG`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_PREDICTOR`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_PRIMARYCHROMATICITIES`
      - 1
      - :c:expr:`float*`
      - 6-entry array
    * - :c:macro:`TIFFTAG_REFERENCEBLACKWHITE`
      - 1
      - :c:expr:`float*`
      - † 6-entry array
    * - :c:macro:`TIFFTAG_RESOLUTIONUNIT`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_RICHTIFFIPTC`
      - 2
      - :c:expr:`uint32_t`, :c:expr:`void*`
      - count, data
    * - :c:macro:`TIFFTAG_ROWSPERSTRIP`
      - 1
      - :c:expr:`uint32_t`
      - † must be > 0
    * - :c:macro:`TIFFTAG_SAMPLEFORMAT`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_SAMPLESPERPIXEL`
      - 1
      - :c:expr:`uint16_t`
      - † value must be ≤ 4
    * - :c:macro:`TIFFTAG_SMAXSAMPLEVALUE`
      - 1
      - :c:expr:`double`
      -
    * - :c:macro:`TIFFTAG_SMINSAMPLEVALUE`
      - 1
      - :c:expr:`double`
      -
    * - :c:macro:`TIFFTAG_SOFTWARE`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_STONITS`
      - 1
      - :c:expr:`double`
      - †
    * - :c:macro:`TIFFTAG_SUBFILETYPE`
      - 1
      - :c:expr:`uint32_t`
      -
    * - :c:macro:`TIFFTAG_SUBIFD`
      - 2
      - :c:expr:`uint16_t`, :c:expr:`uint64_t*`
      - count, offsets array
    * - :c:macro:`TIFFTAG_TARGETPRINTER`
      - 1
      - :c:expr:`char*`
      -
    * - :c:macro:`TIFFTAG_THRESHHOLDING`
      - 1
      - :c:expr:`uint16_t`
      -
    * - :c:macro:`TIFFTAG_TILEDEPTH`
      - 1
      - :c:expr:`uint32_t`
      - †
    * - :c:macro:`TIFFTAG_TILELENGTH`
      - 1
      - :c:expr:`uint32_t`
      - † must be a multiple of 8
    * - :c:macro:`TIFFTAG_TILEWIDTH`
      - 1
      - :c:expr:`uint32_t`
      - † must be a multiple of 8
    * - :c:macro:`TIFFTAG_TRANSFERFUNCTION`
      - 1 or 3‡
      - :c:expr:`uint16_t*`
      - :c:expr:`1<<BitsPerSample` entry arrays
    * - :c:macro:`TIFFTAG_WHITEPOINT`
      - 1
      - :c:expr:`float*`
      - 2-entry array
    * - :c:macro:`TIFFTAG_XMLPACKET`
      - 2
      - :c:expr:`uint32_t`, :c:expr:`void*`
      - count, data
    * - :c:macro:`TIFFTAG_XPOSITION`
      - 1
      - :c:expr:`float`
      -
    * - :c:macro:`TIFFTAG_XRESOLUTION`
      - 1
      - :c:expr:`float`
      -
    * - :c:macro:`TIFFTAG_YCBCRCOEFFICIENTS`
      - 1
      - :c:expr:`float*`
      - † 3-entry array
    * - :c:macro:`TIFFTAG_YCBCRPOSITIONING`
      - 1
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_YCBCRSAMPLING`
      - 2
      - :c:expr:`uint16_t`
      - †
    * - :c:macro:`TIFFTAG_YPOSITION`
      - 1
      - :c:expr:`float`
      -
    * - :c:macro:`TIFFTAG_YRESOLUTION`
      - 1
      - :c:expr:`float`
      -

†:
  Tag may not have its values changed once data is written.

‡:

  If ``SamplesPerPixel`` is one, then a single array is passed;
  otherwise three arrays should be passed.

\*:
  The contents of this field are quite complex.  See
  "The ICC Profile Format Specification",
  Annex B.3 "Embedding ICC Profiles in TIFF Files"
  (available at http://www.color.org) for an explanation.

Return values
-------------

1 is returned if the operation was successful.
Otherwise, 0 is returned if an error was detected.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``%s: Cannot modify tag "%s" while writing``:

  Data has already been written to the file, so the
  specified tag's value can not be changed.
  This restriction is applied to all tags that affect
  the format of written data.

``%d: Bad value for "%s"``:

  An invalid value was supplied for the named tag.

See also
--------

:doc:`TIFFOpen` (3tiff),
:doc:`TIFFGetField` (3tiff),
:doc:`TIFFSetDirectory` (3tiff),
:doc:`TIFFWriteDirectory` (3tiff),
:doc:`TIFFReadDirectory` (3tiff),
:doc:`libtiff` (3tiff)
