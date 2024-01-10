TIFFWriteDirectory
==================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFWriteDirectory(TIFF* tif)

.. c:function:: int TIFFRewriteDirectory(TIFF* tif)

.. c:function:: int TIFFCheckpointDirectory(TIFF* tif)

.. c:function:: void TIFFSetWriteOffset(TIFF* tif, toff_t off)

.. c:function:: int TIFFWriteCheck(TIFF* tif, int tiles, const char* module)

Description
-----------

:c:func:`TIFFWriteDirectory` will write the contents of the current
directory (IFD) to the file and setup to create a new directory (IFD)
using :c:func:`TIFFCreateDirectory`.
Applications only need to call :c:func:`TIFFWriteDirectory`
when writing multiple subfiles (images) to a single TIFF file.
This is called "multi-page TIFF" or "multi-image TIFF"
(see :doc:`/multi_page`).
:c:func:`TIFFWriteDirectory` is automatically called by
:c:func:`TIFFClose` and :c:func:`TIFFFlush` to write a modified
directory if the file is open for writing.

The :c:func:`TIFFRewriteDirectory` function operates similarly to
:c:func:`TIFFWriteDirectory`, but can be called with directories
previously read or written that already have an established location
in the file.  It will rewrite the directory, but instead of placing it
at its old location (as :c:func:`TIFFWriteDirectory` would) it will
place them at the end of the file, correcting the pointer from the
preceding directory or file header to point to it's new location.  This
is particularly important in cases where the size of the directory and
pointed to data has grown, so it won't fit in the space available at the
old location.

The :c:func:`TIFFCheckpointDirectory` writes the current state of the
tiff directory into the file to make what is currently in the file
readable.  Unlike :c:func:`TIFFWriteDirectory`,
:c:func:`TIFFCheckpointDirectory` does not free up the directory data
structures in memory, so they can be updated (as strips/tiles are
written) and written again.  Reading such a partial file you will at
worst get a tiff read error for the first strip/tile encountered that
is incomplete, but you will at least get all the valid data in the file
before that.  When the file is complete, just use
:c:func:`TIFFWriteDirectory` as usual to finish it off cleanly.

The :c:func:`TIFFSetWriteOffset` sets the current write offset.
This should only be used to set the offset to a known previous location
(very carefully), or to 0 so that the next write gets appended to the end
of the file.

The :c:func:`TIFFWriteCheck`  verify file is writable and that the directory
information is setup properly.  In doing the latter we also "freeze"
the state of the directory so that important information is not changed.

Return values
-------------

1 is returned when the contents are successfully written to the file.
Otherwise, 0 is returned if an error was encountered when writing
the directory contents.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.

``"Error post-encoding before directory write"``:

  Before writing the contents of the current directory, any pending data are
  flushed. This message indicates that an error occurred while doing this.

``"Error flushing data before directory write"``:

  Before writing the contents of the current directory, any pending data are
  flushed. This message indicates that an error occurred while doing this.

``"Cannot write directory, out of space"``:

  There was not enough space to allocate a temporary area for the directory
  that was to be written.

``"Error writing directory count"``:

  A write error occurred when writing the count of fields in the directory.

``"Error writing directory contents"``:

  A write error occurred when writing the directory fields.

``"Error writing directory link"``:

  A write error occurred when writing the link to the next directory.

``Error writing data for field "%s"``:

  A write error occurred when writing indirect data for the specified field.

``"Error writing TIFF header"``:

  A write error occurred when re-writing header at the front of the file.

``"Error fetching directory count"``:

  A read error occurred when fetching the directory count field for
  a previous directory.
  This can occur when setting up a link to the directory that is being
  written.

``"Error fetching directory link"``:

  A read error occurred when fetching the directory link field for
  a previous directory.
  This can occur when setting up a link to the directory that is being
  written.

See also
--------

:doc:`TIFFquery` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`TIFFCreateDirectory` (3tiff),
:doc:`TIFFCustomDirectory` (3tiff),
:doc:`TIFFSetDirectory` (3tiff),
:doc:`TIFFReadDirectory` (3tiff),
:doc:`TIFFError` (3tiff),
:doc:`/multi_page`,
:doc:`libtiff` (3tiff)
