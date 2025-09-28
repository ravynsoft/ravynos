TIFFOpen
========

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: TIFF* TIFFOpen(const char* filename, const char* mode)

.. c:function:: TIFF* TIFFOpenW(const wchar_t* name, const char* mode)

.. c:function:: TIFF* TIFFFdOpen(const int fd, const char* filename, const char*mode)

.. c:function:: TIFF* TIFFOpenExt(const char* filename, const char* mode, TIFFOpenOptions* opts)

.. c:function:: TIFF* TIFFOpenWExt(const wchar_t* name, const char* mode, TIFFOpenOptions* opts)

.. c:function:: TIFF* TIFFFdOpenExt(const int fd, const char* filename, const char*mode, TIFFOpenOptions* opts)

.. c:function:: const char * TIFFSetFileName(TIFF* tif)

.. c:function:: int TIFFSetFileno(TIFF* tif, int fd)

.. c:function:: int TIFFSetMode(TIFF* tif, int mode)

.. c:type:: tsize_t (*TIFFReadWriteProc)(thandle_t, tdata_t, tsize_t)
.. c:type:: toff_t (*TIFFSeekProc)(thandle_t, toff_t, int)
.. c:type:: int (*TIFFCloseProc)(thandle_t)
.. c:type:: toff_t (*TIFFSizeProc)(thandle_t)
.. c:type:: int (*TIFFMapFileProc)(thandle_t, tdata_t*, toff_t*)
.. c:type:: void (*TIFFUnmapFileProc)(thandle_t, tdata_t, toff_t)

.. c:function:: TIFF* TIFFClientOpen(const char* filename, const char* mode, thandle_t clientdata, TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc, TIFFSeekProc seekproc, TIFFCloseProc closeproc, TIFFSizeProc sizeproc, TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc)

.. c:function:: TIFF* TIFFClientOpenExt(const char* filename, const char* mode, thandle_t clientdata, TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc, TIFFSeekProc seekproc, TIFFCloseProc closeproc, TIFFSizeProc sizeproc, TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc, TIFFOpenOptions* opts)

.. c:function:: thandle_t TIFFClientdata(TIFF* tif)

.. c:function:: thandle_t TIFFSetClientdata(TIFF* tif, thandle_t newvalue)

Description
-----------

:c:func:`TIFFOpen` opens a TIFF file whose name is *filename*
and returns a handle to be used in subsequent calls to routines in
:program:`libtiff`.  If the open operation fails, then
:c:macro:`NULL` (0) is returned.  The *mode* parameter specifies if
the file is to be opened for reading (``r``) or (``r+``), writing (``w``), or
appending (``a``) and, optionally, whether to override certain
default aspects of library operation (see below Options_).

The *mode* (``r``) opens only an **existing** file for reading and (``r+``)
for reading and writing.
When a file is opened for appending, existing data will not
be touched; instead new data will be written as additional subfiles.
If an existing file is opened for writing, all previous data is
overwritten.

If a file is opened for reading, the first TIFF directory in the file
is automatically read. 
If a file is opened for writing or appending, a default directory
is automatically created for writing subsequent data.
This directory has all the default values specified in TIFF Revision 6.0:

* ``BitsPerSample`` = 1,
* ``ThreshHolding`` = "bilevel art scan"
* ``FillOrder`` = 1 (most significant bit of each data byte is filled first)
* ``Orientation`` = 1 (the 0th row represents the visual top of the image,
  and the 0th column represents the visual left hand side),
* ``SamplesPerPixel`` = 1,
* ``RowsPerStrip`` = ∞,
* ``ResolutionUnit`` = 2 (inches), and
* ``Compression`` = 1 (no compression).

To alter these values, or to define values for additional fields,
:c:func:`TIFFSetField` must be used.

:c:func:`TIFFOpenW` opens a TIFF file with a Unicode filename, for read/writing.

:c:func:`TIFFFdOpen` is like :c:func:`TIFFOpen` except that it opens a
TIFF file given an open file descriptor *fd*.
The file's name and mode must reflect that of the open descriptor.
Even for write-only mode, ``libtiff`` needs read permissions because
some of its functions need to read back the partially written TIFF file.
The object associated with the file descriptor **must support random access**.
In order to close a TIFF file opened with :c:func:`TIFFFdOpen`
first :c:func:`TIFFCleanup` should be called to free the internal
TIFF structure without closing the file handle and afterwards the
file should be closed using its file descriptor *fd*.

:c:func:`TIFFOpenExt` (added in libtiff 4.5) is like :c:func:`TIFFOpen`,
but options, such as re-entrant error and warning handlers may be passed
with the *opts* argument. The *opts* argument may be NULL. 
Refer to :doc:`TIFFOpenOptions` for allocating and filling the *opts* argument
parameters. The allocated memory for :c:type:`TIFFOpenOptions`
can be released straight after successful execution of the related
"TIFFOpenExt" functions.

:c:func:`TIFFOpenWExt` (added in libtiff 4.5) is like :c:func:`TIFFOpenExt`,
but opens a TIFF file with a Unicode filename.

:c:func:`TIFFFdOpenExt` (added in libtiff 4.5) is like :c:func:`TIFFFdOpen`,
but options, such as re-entrant error and warning handlers may be passed
with the *opts* argument. The *opts* argument may be NULL. 
Refer to :doc:`TIFFOpenOptions` for filling the *opts* argument.

:c:func:`TIFFSetFileName` sets the file name in the tif-structure
and returns the old file name.

:c:func:`TIFFSetFileno` overwrites a copy of the open file's I/O descriptor,
that was saved when the TIFF file was first opened,
and returns the previous value. See note below.

:c:func:`TIFFSetMode` sets the ``libtiff`` open mode in the tif-structure
and returns the old mode.

:c:func:`TIFFClientOpen` is like :c:func:`TIFFOpen` except that the caller
supplies a collection of functions that the library will use to do UNIX-like
I/O operations.  The *readproc* and *writeproc* functions are called to read
and write data at the current file position.
*seekproc* is called to change the current file position à la :c:func:`lseek` (2).
*closeproc* is invoked to release any resources associated with an open file.
*sizeproc* is invoked to obtain the size in bytes of a file.
*mapproc* and *unmapproc* are called to map and unmap a file's contents in
memory; c.f. :c:func:`mmap` (2) and :c:func:`munmap` (2).
The *clientdata* parameter is an opaque "handle" passed to the client-specified
routines passed as parameters to :c:func:`TIFFClientOpen`.

:c:func:`TIFFClientOpenExt` (added in libtiff 4.5) is like :c:func:`TIFFClientOpen`,
but options argument *opts* like for :c:func:`TIFFOpenExt` can be passed.

:c:func:`TIFFClientdata` returns open file's clientdata handle,
which is the real open file's I/O descriptor used by ``libtiff``.
Note: Within tif_unix.c this handle is converted into an integer file descriptor.

:c:func:`TIFFSetClientdata` sets open file's clientdata, and return previous value.
The clientdata is used as open file's I/O descriptor within ``libtiff``.

.. note::
  *clientdata* is used as file descriptor or handle of the opened TIFF file within
  `libtif`, whereas the file descriptor *fd* (changeable by :c:func:`TIFFSetFileno`)
  is only set once to the value of *clientdata* converted to an integer
  (in tif_win32.c as well as in tif_unix.c).
  When updating the file's clientdata with :c:func:`TIFFSetClientdata`,
  the *fd* value is **not** updated.

Options
-------

The open mode parameter can include the following flags in
addition to the ``r``, ``r+``, ``w``, and ``a`` flags.
Note however that option flags must follow the read-write-append
specification.

Note 2: Also for ``w`` the file will be opened with *read access* rights
because ``libtiff`` needs to read back the partially written TIFF file
for some of its functions.


``l``:

  When creating a new file force information be written with
  Little-Endian byte order (but see below).
  By default the library will create new files using the native
  CPU byte order.

``b``:

  When creating a new file force information be written with
  Big-Endian byte order (but see below).
  By default the library will create new files using the native
  CPU byte order.

``L``:

  Force image data that is read or written to be treated with
  bits filled from Least Significant Bit (LSB) to
  Most Significant Bit (MSB).
  Note that this is the opposite to the way the library has
  worked from its inception.

``B``:

  Force image data that is read or written to be treated with
  bits filled from Most Significant Bit (MSB) to
  Least Significant Bit (LSB); this is the default.

``H``:

  Force image data that is read or written to be treated with
  bits filled in the same order as the native
  CPU.

``M``:

  Enable the use of memory-mapped files for images opened read-only.
  If the underlying system does not support memory-mapped files
  or if the specific image being opened cannot be memory-mapped
  then the library will fallback to using the normal system interface
  for reading information.
  By default the library will attempt to use memory-mapped files.

``m``:

  Disable the use of memory-mapped files.

``C``:

  Enable the use of "strip chopping" when reading images
  that are comprised of a single strip or tile of uncompressed data.
  Strip chopping is a mechanism by which the library will automatically
  convert the single-strip image to multiple strips,
  each of which has about 8 Kilobytes of data.
  This facility can be useful in reducing the amount of memory used
  to read an image because the library normally reads each strip
  in its entirety.
  Strip chopping does however alter the apparent contents of the
  image because when an image is divided into multiple strips it
  looks as though the underlying file contains multiple separate
  strips.
  Finally, note that default handling of strip chopping is a compile-time
  configuration parameter.
  The default behaviour, for backwards compatibility, is to enable
  strip chopping.

``c``:

  Disable the use of strip chopping when reading images.

``h``:

  Read TIFF header only, do not load the first image directory. That could be
  useful in case of the broken first directory. We can open the file and proceed
  to the other directories.

``4``:

  ClassicTIFF for creating a file (default)

``8``:

  BigTIFF for creating a file.

``D``:

  Enable use of deferred strip/tile offset/bytecount array loading. They will
  be loaded the first time they are accessed to. This loading will be done in
  its entirety unless the O flag is also specified.

``O``:

  On-demand loading of values of the strip/tile offset/bytecount arrays, limited
  to the requested strip/tile, instead of whole array loading (implies ``D``)

Byte order
----------

The TIFF specification (**all versions**) states that compliant readers
"must be capable of reading images written in either byte order" .
Nonetheless some software that claims to support the reading of
TIFF images is incapable of reading images in anything but the native
CPU byte order on which the software was written.
(Especially notorious are applications written to run on Intel-based machines.)
By default the library will create new files with the native
byte-order of the CPU on which the application is run.
This ensures optimal performance and is portable to any application
that conforms to the TIFF specification.
To force the library to use a specific byte-order when creating
a new file the ``b`` and ``l`` option flags may be included in
the call to open a file; for example, ``wb`` or ``wl``.

Return values
-------------

Upon successful completion
:c:func:`TIFFOpen`, :c:func:`TIFFFdOpen`, and :c:func:`TIFFClientOpen`
return a TIFF pointer.  Otherwise, :c:macro:`NULL` is returned.

Diagnostics
-----------


All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

``"%s": Bad mode``:

  The specified *mode* parameter was not one of ``r`` (read), ``w`` (write),
  or ``a`` (append).

``"%s: Cannot open"``:

  :c:func:`TIFFOpen` was unable to open the specified filename for read/writing.

``"Cannot read TIFF header"``:

  An error occurred while attempting to read the header information.

``"Error writing TIFF header"``:

  An error occurred while writing the default header information
  for a new file.

``"Not a TIFF file, bad magic number %d (0x%x)"``:

  The magic number in the header was not (hex)
  0x4d4d or (hex) 0x4949.

``"Not a TIFF file, bad version number %d (0x%x)"``:

  The version field in the header was not 42 (decimal).

``"Cannot append to file that has opposite byte ordering"``:

  A file with a byte ordering opposite to the native byte
  ordering of the current machine was opened for appending (``a``).
  This is a limitation of the library.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFClose` (3tiff),
:doc:`TIFFStrileQuery`,
:doc:`TIFFOpenOptions`