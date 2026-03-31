libtiff
=======

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. highlight:: shell

::

    cc file.c -ltiff

Description
-----------

:program:`libtiff` is a library for reading and writing data files encoded with the
*"Tag Image File"* format, Revision 6.0 (or revision 5.0 or revision 4.0). This file
format is suitable for archiving multi-color and monochromatic image data.

The library supports several compression algorithms, as indicated by the
``Compression`` field, including:
no compression (1),
CCITT 1D Huffman compression (2),
CCITT Group 3 Facsimile compression (3),
CCITT Group 4 Facsimile compression (4),
Lempel-Ziv & Welch compression (5),
baseline JPEG compression (7),
word-aligned 1D Huffman compression (32771),
PackBits compression (32773).
In addition, several nonstandard compression algorithms are supported: the
4-bit compression algorithm used by the
*ThunderScan* program (32809) (decompression only),
NeXT's 2-bit compression algorithm (32766) (decompression only),
an experimental LZ-style algorithm known as Deflate (32946),
and an experimental CIE LogLuv compression scheme designed
for images with high dynamic range (32845 for LogL and 32845 for LogLuv).
Directory information may be in either little- or big-endian byte order; byte
swapping is automatically done by the library. Data bit ordering may be either
Most Significant Bit (``MSB``) to Least Significant Bit (``LSB``) or
LSB to MSB.
Finally, the library does not support files in which the
``BitsPerSample`` ,
``Compression`` ,
``MinSampleValue`` ,
or
``MaxSampleValue``
fields are defined differently on a per-sample basis
(in Rev. 6.0 the
``Compression``
tag is not defined on a per-sample basis, so this is immaterial).

Data types
----------

The library makes extensive use of C typedefs to promote portability.
Two sets of typedefs are used, one for communication with clients
of the library and one for internal data structures and parsing of the
TIFF format.
The following typedefs are exposed to users either through function
definitions or through parameters passed through the varargs interfaces.

.. highlight:: c

::

    typedef uint32_t ttag_t;    // directory tag
    typedef uint32_t tdir_t;    // directory index
    typedef uint16_t tsample_t; // sample number
    typedef uint32_t tstrip_t;  // strip number
    typedef uint32_t ttile_t;   // tile number
    typedef int64_t tmsize_t;   // signed size type (int32_t on 32-bit platforms)
    typedef tmsize_t tsize_t;   // i/o size in bytes
    typedef void* tdata_t;      // image data ref
    typedef void* thandle_t;    // client data handle
    typedef uint64_t toff_t;    // file offset

Note that
:c:type:`tstrip_t`,
:c:type:`ttile_t`,
and
:c:type:`tsize_t`
are constrained to be no more than 32-bit quantities by 32-bit fields they are
stored in in the
TIFF
image.
Likewise
:c:type:`tsample_t`
is limited by the 16-bit field used to store the
``SamplesPerPixel``
tag.

:c:type:`tdir_t`
constrains the maximum number of
IFDs
that may appear in an image and may be an arbitrary size (w/o penalty).
Starting with libtiff 4.5.0, tdir_t is a 32-bit unsigned integer. Previously,
it was a 16-bit unsigned integer.

:c:type:`ttag_t`
must be either int, unsigned int, pointer, or double because the library uses
a varargs interface and
C restricts the type of the parameter before an ellipsis to be a promoted type.
:c:type:`toff_t`
is defined as :c:type:`uint64_t` because TIFF file offsets are (unsigned) 32-bit
quantities, and BigTIFF file offsets are unsigned 64-bit quantities.
A signed value is used because some interfaces return -1 on
error. Finally, note that user-specified data references are passed as opaque
handles and only cast at the lowest layers where their type is presumed.

.. TODO: Check why this toff_t was switched to unsigned and update description.

.. _List_of_routines:

List of routines
----------------

The following routines are part of the library. Consult specific manual pages
for details on their operation; on most systems doing :command:`man function-name`
will work.

.. list-table:: *Libtiff functions*
    :widths: 5 20
    :header-rows: 1

    * - Name
      - Description
    * - :c:func:`TIFFAccessTagMethods`
      -  provides read/write access to the TIFFTagMethods within the TIFF structure
         to application code without giving access to the private TIFF structure
    * - :c:func:`TIFFCheckpointDirectory`
      - writes the current state of the directory
    * - :c:func:`TIFFCheckTile`
      - very x,y,z,sample is within image
    * - :c:func:`TIFFCIELabToRGBInit`
      - initialize CIE L*a*b* 1976 to RGB conversion state
    * - :c:func:`TIFFCIELabToXYZ`
      - perform CIE L*a*b* 1976 to CIE XYZ conversion
    * - :c:func:`TIFFCleanup`
      - auxiliary function to free the TIFF structure
    * - :c:func:`TIFFClientdata`
      - return open file's clientdata handle, which represents
        the file descriptor used within ``libtiff``.
    * - :c:func:`TIFFClientOpen`
      - open a file for reading or writing
    * - :c:func:`TIFFClientOpenExt`
      - open a file for reading or writing with options,
        such as re-entrant error and warning handlers may be passed
    * - :c:func:`TIFFClose`
      - close a previously opened TIFF file
    * - :c:func:`TIFFComputeStrip`
      - return strip containing y,sample
    * - :c:func:`TIFFComputeTile`
      - return tile containing x,y,z,sample
    * - :c:func:`TIFFCreateCustomDirectory`
      - setup for a *custom* directory in a open TIFF file
    * - :c:func:`TIFFCreateDirectory`
      - setup for a directory in a open TIFF file
    * - :c:func:`TIFFCreateEXIFDirectory`
      - setup for a *EXIF* custom directory in a open TIFF file within a TIFF tag
    * - :c:func:`TIFFCreateGPSDirectory`
      - setup for a *GPS* custom directory in a open TIFF file within a TIFF tag
    * - :c:func:`TIFFCurrentDirectory`
      - return index of current directory
    * - :c:func:`TIFFCurrentDirOffset`
      - return file offset of the current directory (instead of an index)
    * - :c:func:`TIFFCurrentRow`
      - return index of current scanline
    * - :c:func:`TIFFCurrentStrip`
      - return index of current strip
    * - :c:func:`TIFFCurrentTile`
      - return index of current tile
    * - :c:func:`TIFFDataWidth`
      - return the size of TIFF data types
    * - :c:func:`TIFFDefaultStripSize`
      - return number of rows for a reasonable-sized strip according to the
        current settings of the ImageWidth, BitsPerSample and SamplesPerPixel,
        tags and any compression-specific requirements
    * - :c:func:`TIFFDefaultTileSize`
      - return pixel width and height of a reasonable-sized tile;
        suitable for setting up the TileWidth and TileLength tags
    * - :c:func:`TIFFDeferStrileArrayWriting`
      - is an advanced writing function to control when/where the
        [Strip/Tile][Offsets/ByteCounts] arrays are written into the file,
        and must be used in a particular sequence together with
        TIFFForceStrileArrayWriting() (see description)
    * - :c:func:`TIFFError`
      - library-wide error handling function printing to ``stderr``
    * - :c:func:`TIFFErrorExt`
      - user-specific library-wide error handling function that can be passed
        a file handle, which is set to the open TIFF file within ``libtiff``
    * - :c:func:`TIFFErrorExtR`
      - user-specific re-entrant library error handling function,
        to which its TIFF structure is passed
        containing the pointer to a user-specific data object
    * - :c:func:`TIFFFdOpen`
      - open a file for reading or writing
    * - :c:func:`TIFFFdOpenExt`
      - open a file for reading or writing with options,
        such as re-entrant error and warning handlers may be passed
    * - :c:func:`TIFFFieldDataType`
      - get data type from field information
    * - :c:func:`TIFFFieldIsAnonymous`
      - returns if field was unknown to ``libtiff`` and has been auto-registered
    * - :c:func:`TIFFFieldName`
      - get field name from field information
    * - :c:func:`TIFFFieldPassCount`
      - get whether to pass a value count to Get/SetField
    * - :c:func:`TIFFFieldReadCount`
      - get number of values to be read from field
    * - :c:func:`TIFFFieldSetGetCountSize`
      - returns size of ``count`` parameter of :c:func:`TIFFSetField` and
        :c:func:`TIFFGetField`
    * - :c:func:`TIFFFieldSetGetSize`
      - return data size in bytes of the field data type used for ``libtiff``
        internal storage.
    * - :c:func:`TIFFFieldTag`
      - get tag value from field information
    * - :c:func:`TIFFFieldWithName`
      - get field information given field name
    * - :c:func:`TIFFFieldWithTag`
      - get field information given tag
    * - :c:func:`TIFFFieldWriteCount`
      - get number of values to be written to field
    * - :c:func:`TIFFFileName`
      - return name of open file
    * - :c:func:`TIFFFileno`
      - return open file descriptor
    * - :c:func:`TIFFFindCODEC`
      - find standard codec for the specific scheme
    * - :c:func:`TIFFFindField`
      - get field information given tag and data type
    * - :c:func:`TIFFFlush`
      - flush all pending writes
    * - :c:func:`TIFFFlushData`
      - flush pending data writes
    * - :c:func:`TIFFForceStrileArrayWriting`
      - is an advanced writing function that writes the
        [Strip/Tile][Offsets/ByteCounts] arrays at the end of the file (see description)
    * - :c:func:`TIFFFreeDirectory`
      - release storage associated with a directory
    * - :c:func:`TIFFGetBitRevTable`
      - return bit reversal table
    * - :c:func:`TIFFGetClientInfo`
      - returns a pointer to the data of the named entry in the clientinfo-list
    * - :c:func:`TIFFGetCloseProc`
      - returns a pointer to file close method
    * - :c:func:`TIFFGetConfiguredCODECs`
      - gets list of configured codecs, both built-in and registered by user
    * - :c:func:`TIFFGetField`
      - return tag value in current directory
    * - :c:func:`TIFFGetFieldDefaulted`
      - return tag value in current directory with default value set if the
        value is not already set and a default is defined
    * - :c:func:`TIFFGetMapFileProc`
      - returns a pointer to memory mapping method
    * - :c:func:`TIFFGetMode`
      - return open file mode
    * - :c:func:`TIFFGetReadProc`
      - returns a pointer to file read method
    * - :c:func:`TIFFGetSeekProc`
      - returns a pointer to file seek method
    * - :c:func:`TIFFGetSizeProc`
      - returns a pointer to file size requesting method
    * - :c:func:`TIFFGetStrileByteCount`
      - return value of the TileByteCounts/StripByteCounts array for the
        specified tile/strile
    * - :c:func:`TIFFGetStrileByteCountWithErr`
      - same as `TIFFGetStrileByteCount()` and additionally provides an error return
    * - :c:func:`TIFFGetStrileOffset`
      - return value of the TileOffsets/StripOffsets array for the specified tile/strile
    * - :c:func:`TIFFGetStrileOffsetWithErr`
      - same as `TIFFGetStrileOffset()` and additionally provides an error return
    * - :c:func:`TIFFGetTagListCount`
      - return number of entries in the custom tag list
    * - :c:func:`TIFFGetTagListEntry`
      - return tag number of the (n.th - 1) entry within the custom tag list
    * - :c:func:`TIFFGetUnmapFileProc`
      - returns a pointer to memory unmapping method
    * - :c:func:`TIFFGetVersion`
      - return library version string
    * - :c:func:`TIFFGetWriteProc`
      - returns a pointer to file write method
    * - :c:func:`TIFFIsBigEndian`
      - returns a non-zero value if the file is BigEndian and zero if the file is
        LittleEndian
    * - :c:func:`TIFFIsBigTIFF`
      - returns a non-zero value if the file is in BigTIFF style
    * - :c:func:`TIFFIsByteSwapped`
      - return true if image data is byte-swapped
    * - :c:func:`TIFFIsCODECConfigured`
      - check, whether we have working codec
    * - :c:func:`TIFFIsMSB2LSB`
      - return true if image data is being returned with bit 0 as the most significant bit
    * - :c:func:`TIFFIsTiled`
      - return true if image data is tiled
    * - :c:func:`TIFFIsUpSampled`
      - returns a non-zero value if image data returned through the read interface
        Routines is being up-sampled
    * - :c:func:`TIFFLastDirectory`
      - returns a non-zero value if the current directory is the last directory
        in the file; otherwise zero is returned
    * - :c:func:`TIFFMergeFieldInfo`
      - adds application defined TIFF tags to the list of known ``libtiff`` tags
    * - :c:func:`TIFFNumberOfDirectories`
      - return number of directories in a file
    * - :c:func:`TIFFNumberOfStrips`
      - return number of strips in an image
    * - :c:func:`TIFFNumberOfTiles`
      - return number of tiles in an image
    * - :c:func:`TIFFOpen`
      - open a file for reading or writing
    * - :c:func:`TIFFOpenExt`
      - open a file for reading or writing  with options,
        such as re-entrant error and warning handlers may be passed
    * - :c:func:`TIFFOpenW`
      - opens a TIFF file with a Unicode filename, for read/writing
    * - :c:func:`TIFFOpenWExt`
      - opens a TIFF file with a Unicode filename, for read/writing
        with options, such as re-entrant error and warning handlers may be passed
    * - :c:func:`TIFFOpenOptionsAlloc`
      - allocates memory for :c:type:`TIFFOpenOptions` opaque structure
    * - :c:func:`TIFFOpenOptionsFree`
      - releases the allocated memory for :c:type:`TIFFOpenOptions`
    * - :c:func:`TIFFOpenOptionsSetMaxSingleMemAlloc`
      - limits the maximum single memory allocation within ``libtiff``
    * - :c:func:`TIFFOpenOptionsSetErrorHandlerExtR`
      - setup of a user-specific and per-TIFF handle (re-entrant) error handler
    * - :c:func:`TIFFOpenOptionsSetWarningHandlerExtR`
      - setup of a user-specific and per-TIFF handle (re-entrant) warning handler
    * - :c:func:`TIFFPrintDirectory`
      - print description of the current directory
    * - :c:func:`TIFFRasterScanlineSize`
      - returns the size in bytes of a complete decoded and packed raster scanline
    * - :c:func:`TIFFRasterScanlineSize64`
      - return size as :c:type:`uint64_t`
    * - :c:func:`TIFFRawStripSize`
      - return number of bytes in a raw strip
    * - :c:func:`TIFFRawStripSize64`
      - return number of bytes in a raw strip as :c:type:`uint64_t`
    * - :c:func:`TIFFReadBufferSetup`
      - specify i/o buffer for reading
    * - :c:func:`TIFFReadCustomDirectory`
      - read the custom directory from the given offset
        and set the context of the TIFF-handle tif to that custom directory
    * - :c:func:`TIFFReadDirectory`
      - read the next directory
    * - :c:func:`TIFFReadEncodedStrip`
      - read and decode a strip of data
    * - :c:func:`TIFFReadEncodedTile`
      - read and decode a tile of data
    * - :c:func:`TIFFReadEXIFDirectory`
      - read the EXIF directory from the given offset
        and set the context of the TIFF-handle tif to that EXIF directory
    * - :c:func:`TIFFReadFromUserBuffer`
      - replaces the use of :c:func:`TIFFReadEncodedStrip` / :c:func:`TIFFReadEncodedTile`
        when the user can provide the buffer for the input data
    * - :c:func:`TIFFReadGPSDirectory`
      - read the GPS directory from the given offset
        and set the context of the TIFF-handle tif to that GPS directory
    * - :c:func:`TIFFReadRawStrip`
      - read a raw strip of data
    * - :c:func:`TIFFReadRawTile`
      - read a raw tile of data
    * - :c:func:`TIFFReadRGBAImage`
      - read an image into a fixed format raster
    * - :c:func:`TIFFReadRGBAImageOriented`
      - works like :c:func:`TIFFReadRGBAImage` except that the user can specify
        the raster origin position
    * - :c:func:`TIFFReadRGBAStrip`
      - reads a single strip of a strip-based image into memory, storing the
        result in the user supplied RGBA raster
    * - :c:func:`TIFFReadRGBAStripExt`
      - same as :c:func:`TIFFReadRGBAStrip` but providing the parameter `stop_on_error`
    * - :c:func:`TIFFReadRGBATile`
      - reads a single tile of a tile-based image into memory, storing the
        result in the user supplied RGBA raster
    * - :c:func:`TIFFReadRGBATileExt`
      - same as :c:func:`TIFFReadRGBATile` but providing the parameter `stop_on_error`
    * - :c:func:`TIFFReadScanline`
      - read and decode a row of data
    * - :c:func:`TIFFReadTile`
      - read and decode a tile of data
    * - :c:func:`TIFFRegisterCODEC`
      - override standard codec for the specific scheme
    * - :c:func:`TIFFReverseBits`
      - reverse bits in an array of bytes
    * - :c:func:`TIFFRewriteDirectory`
      - operates similarly to :c:func:`TIFFWriteDirectory`, but can be called
        with directories previously read or written that already have an established
        location in the file and places it at the end of the file
    * - :c:func:`TIFFRGBAImageBegin`
      - setup decoder state for TIFFRGBAImageGet
    * - :c:func:`TIFFRGBAImageEnd`
      - release TIFFRGBAImage decoder state
    * - :c:func:`TIFFRGBAImageGet`
      - read and decode an image
    * - :c:func:`TIFFRGBAImageOK`
      - is image readable by TIFFRGBAImageGet
    * - :c:func:`TIFFScanlineSize`
      - return size of a scanline
    * - :c:func:`TIFFScanlineSize64`
      - return size of a scanline as :c:type:`uint64_t`
    * - :c:func:`TIFFSetClientdata`
      - set open file's clientdata (file descriptor/handle),
        and return previous value
    * - :c:func:`TIFFSetClientInfo`
      - adds or replaces an entry in the clientinfo-list
    * - :c:func:`TIFFSetCompressionScheme`
      - set compression scheme
    * - :c:func:`TIFFSetDirectory`
      - set the current directory
    * - :c:func:`TIFFSetErrorHandler`
      - set error handler function
    * - :c:func:`TIFFSetErrorHandlerExt`
      - set error handler function with a file handle as parameter
    * - :c:func:`TIFFSetField`
      - set a tag's value in the current directory
    * - :c:func:`TIFFSetFileName`
      - sets the file name in the TIFF-structure and returns the old file name
    * - :c:func:`TIFFSetFileno`
      - overwrites a copy of the open file's I/O descriptor, and return previous value
        (refer to detailed description)
    * - :c:func:`TIFFSetMode`
      - sets the ``libtiff`` open mode in the TIFF-structure and returns the old mode
    * - :c:func:`TIFFSetSubDirectory`
      - set the current directory
    * - :c:func:`TIFFSetTagExtender`
      - is used to register the merge function for user defined tags as an
        extender callback with ``libtiff``
    * - :c:func:`TIFFSetupStrips`
      - setup  or reset strip parameters and strip array memory
    * - :c:func:`TIFFSetWarningHandler`
      - set warning handler function
    * - :c:func:`TIFFSetWarningHandlerExt`
      - set warning handler function with a file handle as parameter
    * - :c:func:`TIFFSetWriteOffset`
      - set current write offset
    * - :c:func:`TIFFStripSize`
      - return size of a strip
    * - :c:func:`TIFFStripSize64`
      - return equivalent size for a strip of data as :c:type:`uint64_t`
    * - :c:func:`TIFFSwabArrayOfDouble`
      - swap bytes of an array of doubles
    * - :c:func:`TIFFSwabArrayOfFloat`
      - swap bytes of an array of floats
    * - :c:func:`TIFFSwabArrayOfLong`
      - swap bytes of an array of longs
    * - :c:func:`TIFFSwabArrayOfLong8`
      - swap bytes of an array of uint64_t
    * - :c:func:`TIFFSwabArrayOfShort`
      - swap bytes of an array of shorts
    * - :c:func:`TIFFSwabArrayOfTriples`
      - swap the first and third byte of each triple within an array of bytes
    * - :c:func:`TIFFSwabDouble`
      - swap bytes of double
    * - :c:func:`TIFFSwabFloat`
      - swap bytes of float
    * - :c:func:`TIFFSwabLong`
      - swap bytes of long
    * - :c:func:`TIFFSwabLong8`
      - swap bytes of long long (uint64_t)
    * - :c:func:`TIFFSwabShort`
      - swap bytes of short
    * - :c:func:`TIFFTileRowSize`
      - return size of a row in a tile
    * - :c:func:`TIFFTileRowSize64`
      - return size of a row in a tile as :c:type:`uint64_t`
    * - :c:func:`TIFFTileSize`
      - return size of a tile
    * - :c:func:`TIFFTileSize64`
      - return size of a tile as :c:type:`uint64_t`
    * - :c:func:`TIFFUnlinkDirectory`
      - unlink the specified directory from the directory chain
    * - :c:func:`TIFFUnRegisterCODEC`
      - unregisters the codec
    * - :c:func:`TIFFUnsetField`
      - clear the contents of the field in the internal structure
    * - :c:func:`TIFFVGetField`
      - return tag value in current directory
    * - :c:func:`TIFFVGetFieldDefaulted`
      - return tag value in current directory
    * - :c:func:`TIFFVSetField`
      - set a tag's value in the current directory
    * - :c:func:`TIFFVStripSize`
      - return number of bytes in a strip
    * - :c:func:`TIFFVStripSize64`
      - return number of bytes in a strip with *nrows* rows of data as :c:type:`uint64_t`
    * - :c:func:`TIFFVTileSize`
      - returns the number of bytes in a row-aligned tile with *nrows* of data
    * - :c:func:`TIFFVTileSize64`
      - returns the number of bytes in a row-aligned tile with *nrows* of data
        a :c:type:`uint64_t` number
    * - :c:func:`TIFFWarning`
      - library-wide warning handling function printing to ``stderr``
    * - :c:func:`TIFFWarningExt`
      - user-specific library-wide warning handling function that can be passed
        a file handle, which is set to the open TIFF file within ``libtiff``
    * - :c:func:`TIFFWarningExtR`
      - user-specific re-entrant library warning handling function,
        to which its TIFF structure is passed
        containing the pointer to a user-specific data object
    * - :c:func:`TIFFWriteBufferSetup`
      - sets up the data buffer used to write raw (encoded) data to a file
    * - :c:func:`TIFFWriteCheck`
      - verify file is writable and that the directory information is setup properly
    * - :c:func:`TIFFWriteCustomDirectory`
      - write the current custom directory (also EXIF or GPS) to file
    * - :c:func:`TIFFWriteDirectory`
      - write the current directory
    * - :c:func:`TIFFWriteEncodedStrip`
      - compress and write a strip of data
    * - :c:func:`TIFFWriteEncodedTile`
      - compress and write a tile of data
    * - :c:func:`TIFFWriteRawStrip`
      - write a raw strip of data
    * - :c:func:`TIFFWriteRawTile`
      - write a raw tile of data
    * - :c:func:`TIFFWriteScanline`
      - write a scanline of data
    * - :c:func:`TIFFWriteTile`
      - compress and write a tile of data
    * - :c:func:`TIFFXYZToRGB`
      - perform CIE XYZ to RGB conversion
    * - :c:func:`TIFFYCbCrtoRGB`
      - perform YCbCr to RGB conversion
    * - :c:func:`TIFFYCbCrToRGBInit`
      - initialize YCbCr to RGB conversion state

.. list-table:: *Libtiff auxillary functions*
    :widths: 5 20
    :header-rows: 1

    * - Name
      - Description
    * - :c:func:`_TIFFCheckMalloc`
      - checking for integer overflow before dynamically allocate memory buffer
    * - :c:func:`_TIFFCheckRealloc`
      - checking for integer overflow before dynamically reallocate memory buffer
    * - :c:func:`_TIFFClampDoubleToUInt32`
      - clamps double values into the range of :c:type:`uint32_t` (i.e. 0 .. 0xFFFFFFFF)
    * - :c:func:`_TIFFfree`
      - free memory buffer
    * - :c:func:`_TIFFGetExifFields`
      - return a pointer to the ``libtiff`` internal definition list of the EXIF tags
    * - :c:func:`_TIFFGetGpsFields`
      - return a pointer to the ``libtiff`` internal definition list of the GPS tags
    * - :c:func:`_TIFFmalloc`
      - dynamically allocate memory buffer
    * - :c:func:`_TIFFmemcmp`
      - compare contents of the memory buffers
    * - :c:func:`_TIFFmemcpy`
      - copy contents of the one buffer to another
    * - :c:func:`_TIFFmemset`
      - fill memory buffer with a constant byte
    * - :c:func:`_TIFFMultiply32`
      - checks for an integer overflow of the multiplication result of `uint32_t` and
        return the multiplication result or `0` if an overflow would happen
    * - :c:func:`_TIFFMultiply64`
      - checks for an integer overflow of the multiplication result of `uint64_t` and
        return the multiplication result or `0` if an overflow would happen
    * - :c:func:`_TIFFrealloc`
      - dynamically reallocate memory buffer
    * - :c:func:`_TIFFRewriteField`
      - Rewrite a field in the directory on disk without regard
        to updating the TIFF directory structure in memory


Tag usage
---------

For a table of TIFF tags recognized by the library refer to
:doc:`/specification/coverage`.

"Pseudo tags"
-------------

In addition to the normal TIFF
tags the library supports a collection of
tags whose values lie in a range outside the valid range of TIFF
tags. These tags are termed *pseudo-tags*
and are used to control various codec-specific functions within the library.
The table below summarizes the defined pseudo-tags.

.. list-table:: libtiff supported tags
    :widths: 10 2 2 15
    :header-rows: 1

    * - Tag name
      - Codec
      - R/W
      - Library Use/Notes

    * - :c:macro:`TIFFTAG_FAXMODE`
      - G3
      - R/W
      - general codec operation
    * - :c:macro:`TIFFTAG_FAXFILLFUNC`
      - G3/G4
      - R/W
      - bitmap fill function
    * - :c:macro:`TIFFTAG_JPEGQUALITY`
      - JPEG
      - R/W
      - compression quality control
    * - :c:macro:`TIFFTAG_JPEGCOLORMODE`
      - JPEG
      - R/W
      - control colorspace conversions
    * - :c:macro:`TIFFTAG_JPEGTABLESMODE`
      - JPEG
      - R/W
      - control contents of ``JPEGTables`` tag
    * - :c:macro:`TIFFTAG_ZIPQUALITY`
      - Deflate
      - R/W
      - compression quality level
    * - :c:macro:`TIFFTAG_PIXARLOGDATAFMT`
      - PixarLog
      - R/W
      - user data format
    * - :c:macro:`TIFFTAG_PIXARLOGQUALITY`
      - PixarLog
      - R/W
      - compression quality level
    * - :c:macro:`TIFFTAG_SGILOGDATAFMT`
      - SGILog
      - R/W
      - user data format

:c:macro:`TIFFTAG_FAXMODE`:

  Control the operation of the Group 3 codec.
  Possible values (independent bits that can be combined by
  or'ing them together) are:

  :c:macro:`FAXMODE_CLASSIC`:

    (enable old-style format in which the ``RTC``
    is written at the end of the last strip),

  :c:macro:`FAXMODE_NORTC`:

    (opposite of :c:macro:`FAXMODE_CLASSIC`; also called
    :c:macro:`FAXMODE_CLASSF`),

  :c:macro:`FAXMODE_NOEOL`:

    (do not write ``EOL`` codes at the start of each row of data),

  :c:macro:`FAXMODE_BYTEALIGN`:

    (align each encoded row to an 8-bit boundary),

  :c:macro:`FAXMODE_WORDALIGN`:

    (align each encoded row to an 16-bit boundary),

  The default value is dependent on the compression scheme; this
  pseudo-tag is used by the various G3 and G4 codecs to share code.

:c:macro:`TIFFTAG_FAXFILLFUNC`:

  Control the function used to convert arrays of black and white
  runs to packed bit arrays.
  This hook can be used to image decoded scanlines in multi-bit
  depth rasters (e.g. for display in colormap mode)
  or for other purposes.
  The default value is a pointer to a builtin function that images
  packed bilevel data.

:c:macro:`TIFFTAG_IPTCNEWSPHOTO`:

  Tag contains image metadata per the IPTC newsphoto spec: Headline,
  captioning, credit, etc... Used by most wire services.

:c:macro:`TIFFTAG_PHOTOSHOP`:

  Tag contains Photoshop captioning information and metadata. Photoshop
  uses in parallel and redundantly alongside :c:macro:`IPTCNEWSPHOTO` information.

:c:macro:`TIFFTAG_JPEGQUALITY`:

  Control the compression quality level used in the baseline algorithm.
  Note that quality levels are in the range 0-100 with a default value of 75.

:c:macro:`TIFFTAG_JPEGCOLORMODE`:

  Control whether or not conversion is done between
  RGB and YCbCr colorspaces.
  Possible values are:

  :c:macro:`JPEGCOLORMODE_RAW`:

    (do not convert), and

  :c:macro:`JPEGCOLORMODE_RGB`:

    (convert to/from RGB)

  The default value is :c:macro:`JPEGCOLORMODE_RAW`.

:c:macro:`TIFFTAG_JPEGTABLESMODE`:

  Control the information written in the ``JPEGTables`` tag.
  Possible values (independent bits that can be combined by
  or'ing them together) are:

  :c:macro:`JPEGTABLESMODE_QUANT`:

    (include quantization tables), and

  :c:macro:`JPEGTABLESMODE_HUFF`:

    (include Huffman encoding tables).

  The default value is :c:expr:`JPEGTABLESMODE_QUANT|JPEGTABLESMODE_HUFF`.

:c:macro:`TIFFTAG_ZIPQUALITY`:

  Control the compression technique used by the Deflate codec.
  Quality levels are in the range 1-9 with larger numbers yielding better
  compression at the cost of more computation.
  The default quality level is 6 which yields a good time-space tradeoff.

:c:macro:`TIFFTAG_PIXARLOGDATAFMT`:

  Control the format of user data passed *in*
  to the PixarLog codec when encoding and passed
  *out* from when decoding. Possible values are:

  :c:macro:`PIXARLOGDATAFMT_8BIT`:

    for 8-bit unsigned pixels,

  :c:macro:`PIXARLOGDATAFMT_8BITABGR`:

    for 8-bit unsigned ABGR-ordered pixels,

  :c:macro:`PIXARLOGDATAFMT_11BITLOG`:

    for 11-bit log-encoded raw data,

  :c:macro:`PIXARLOGDATAFMT_12BITPICIO`:

    for 12-bit PICIO-compatible data,

  :c:macro:`PIXARLOGDATAFMT_16BIT`:

    for 16-bit signed samples, and

  :c:macro:`PIXARLOGDATAFMT_FLOAT`:

    for 32-bit IEEE floating point samples.

:c:macro:`TIFFTAG_PIXARLOGQUALITY`:

  Control the compression technique used by the PixarLog codec.
  This value is treated identically to :c:macro:`TIFFTAG_ZIPQUALITY`; see the
  above description.

:c:macro:`TIFFTAG_SGILOGDATAFMT`:

  Control the format of client data passed *in*
  to the SGILog codec when encoding and passed
  *out* from when decoding.  Possible values are:

  :c:macro:`SGILOGDATAFMT_FLTXYZ`:

    for converting between LogLuv and 32-bit IEEE floating valued XYZ pixels,

  :c:macro:`SGILOGDATAFMT_16BITLUV`:

    for 16-bit encoded Luv pixels,

  :c:macro:`SGILOGDATAFMT_32BITRAW`:
  :c:macro:`SGILOGDATAFMT_24BITRAW`:

     for no conversion of data,

  :c:macro:`SGILOGDATAFMT_8BITRGB`:

    for returning 8-bit RGB data (valid only when decoding LogLuv-encoded data),

  :c:macro:`SGILOGDATAFMT_FLTY`:

    for converting between LogL and 32-bit IEEE floating valued Y pixels,

  :c:macro:`SGILOGDATAFMT_16BITL`:

    for 16-bit encoded L pixels, and

  :c:macro:`SGILOGDATAFMT_8BITGRY`:

    for returning 8-bit greyscale data
    (valid only when decoding LogL-encoded data).

Diagnostics
-----------

All error messages are directed through the :c:func:`TIFFErrorExtR` routine.
By default messages are directed to ``stderr`` in the form:
``module: message\n``.
Warning messages are likewise directed through the
:c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`/tools/tiffdump`,
:doc:`/tools/tiffcp`,
:doc:`/tools/tiffinfo`,
:doc:`/tools/tiffsplit`,

**"Tag Image File Format Specification *Revision 6.0*"**,
an Aldus Technical Memorandum.

**"The Spirit of TIFF Class F"** ,
an appendix to the TIFF 5.0 specification prepared by Cygnet Technologies.

Bugs
----

* The library does not support multi-sample images
  where some samples have different bits/sample.

* The library does not support random access to compressed data
  that is organized with more than one row per tile or strip.
