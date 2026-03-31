TIFFCreateDirectory
===================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFCreateDirectory(TIFF* tif)

.. c:function:: int TIFFFreeDirectory(TIFF* tif)

.. c:function:: int TIFFUnlinkDirectory(TIFF* tif, tdir_t dirn)

Description
-----------

The following routines create or release a directory.

:c:func:`TIFFCreateDirectory` setup for a directory in a open TIFF file.
The newly created directory will not exist on the file till
:c:func:`TIFFWriteDirectory`, :c:func:`TIFFCheckpointDirectory`,
:c:func:`TIFFFlush` or :c:func:`TIFFClose` is called.
:c:func:`TIFFWriteDirectory` also creates a new directory,
frees the ``*tif`` structure and sets up a new one.

:c:func:`TIFFFreeDirectory` releases allocated storage associated with a
directory, especially custom-fields.
However, the main part of the directory is not touched. Routine
:c:func:`TIFFCleanup` calls :c:func:`TIFFFreeDirectory` to release
the directory part of the `tif` structure.

:c:func:`TIFFUnlinkDirectory` unlink the specified directory from the
directory chain.
The parameter *dirn* specifies the subfile/directory
as an integer number, with the first directory numbered one (1).
This is different to :c:func:`TIFFSetDirectory` or :c:func:`TIFFCurrentDirectory` where the first
directory starts with zero (0).

Directory query functions :c:func:`TIFFCurrentDirectory`,
:c:func:`TIFFCurrentDirOffset`, :c:func:`TIFFLastDirectory` and
:c:func:`TIFFNumberOfDirectories` retrieve information about directories
in an open TIFF file. Be aware that until a directory is
not written to file AND read back, the query functions won't retrieve
the correct information!

Notes
-----

Be aware:

- that until a directory is not written to file AND read back, the
  query functions won't retrieve the correct information!
- that the newly created directory will not exist on the file till
  :c:func:`TIFFWriteDirectory`, :c:func:`TIFFCheckpointDirectory`,
  :c:func:`TIFFFlush` or :c:func:`TIFFClose` has been called.
- that :c:func:`TIFFCreateDirectory` and :c:func:`TIFFWriteDirectory`
  create a new directory, free the ``*tif`` structure and set up a new one.
- that unlike :c:func:`TIFFWriteDirectory`, :c:func:`TIFFCheckpointDirectory`
  does not free up the directory data structures in memory.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFCustomDirectory` (3tiff),
:doc:`TIFFquery` (3tiff),
:doc:`TIFFSetDirectory` (3tiff),
:doc:`TIFFWriteDirectory` (3tiff)
