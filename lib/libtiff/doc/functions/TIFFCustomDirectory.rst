TIFFCustomDirectory
===================

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:function:: int TIFFCreateCustomDirectory(TIFF* tif, const TIFFFieldArray* infoarray)

.. c:function:: int TIFFCreateEXIFDirectory(TIFF* tif)

.. c:function:: int TIFFCreateGPSDirectory(TIFF* tif)

.. c:function:: int TIFFWriteCustomDirectory(TIFF* tif,  uint64 *pdiroff)

.. c:function:: int TIFFReadCustomDirectory(TIFF* tif, toff_t diroff, const TIFFFieldArray* infoarray)

.. c:function:: int TIFFReadEXIFDirectory(TIFF* tif, toff_t diroff)

.. c:function:: int TIFFReadGPSDirectory(TIFF* tif, toff_t diroff)

.. c:function:: const TIFFFieldArray* _TIFFGetExifFields(void)

.. c:function:: const TIFFFieldArray* _TIFFGetGpsFields(void)

Description
-----------

The following routines create a custom directory and retrieve information
about directories in an open TIFF file.

:c:func:`TIFFCreateCustomDirectory`, :c:func:`TIFFCreateEXIFDirectory`,
:c:func:`TIFFCreateGPSDirectory` will setup a custom directory or one
of the predefined EXIF or GPS directories and set the context of the
TIFF-handle ``tif`` to that custom directory for functions
like :c:func:`TIFFSetField`.

:c:func:`TIFFWriteCustomDirectory` will write the contents of the
current custom directory to the file and return the offset to that
directory in :c:var:`pdiroff`. That offset has to be written to the main-IFD:

.. highlight:: c

::

         /* Go back to the first directory, and add the EXIFIFD pointer. */
        TIFFSetDirectory(tif, 0);
        TIFFSetField(tif, TIFFTAG_EXIFIFD, pdiroff);


:c:func:`TIFFReadCustomDirectory` will read the custom directory from the
arbitrary offset into the :c:var:`infoarray` and sets the context of the
TIFF-handle :c:var:`tif` to that custom directory for functions like
:c:func:`TIFFReadField`. The :c:type:`TIFFFieldArray` :c:var:`infoarray`
has to be according the layout of the custom directory. For the predefined
EXIF and GPS directories, the relevant :c:type:`TIFFFieldArray` definitions
are hidden within the functions :c:func:`TIFFReadEXIFDirectory` and
:c:func:`TIFFReadGPSDirectory` The code is very similar to :c:func:`TIFFReadDirectory`.
The offset to the custom directory diroff has to be read from the
relative TIFF tag first.

:c:func:`_TIFFGetExifFields` and :c:func:`_TIFFGetGpsFields`  will
return a pointer to the ``libtiff`` internal definition list of the
EXIF and GPS tags, respectively.

Notes
-----

Be aware

- that until a directory is not written to file AND read back, the query
  functions won't retrieve the correct information!
- that the newly created directory will not exist on the file till
  :c:func:`TIFFWriteDirectory`, :c:func:`TIFFCheckpointDirectory`,
  :c:func:`TIFFFlush` or :c:func:`TIFFClose` has been called.
- that :c:func:`TIFFCreateDirectory` and :c:func:`TIFFWriteDirectory`
  create a new directory, free the ``*tif`` structure and set up a new one.
- that unlike :c:func:`TIFFWriteDirectory`, :c:func:`TIFFCheckpointDirectory`
  does not free up the directory data structures in memory.
- that LibTiff does not support custom directory chains
  (NextIFD pointing to another IFD).
  NextIFD of custom directories is always set to zero
  and should be zero when reading.

Unfortunately to create or read custom directories with predefined fields
it is necessary to include the private tif_dir.h. However, for EXIF and
GPS directories, which have a predefined schema within ``libtiff``, this
is not necessary. There are some test programmes that briefly demonstrate
the creation and reading of EXIF, GPS and custom directories.
See test/custom_dir.c and test/custom_dir_EXIF_231.c

Hints and detailed instructions
-------------------------------

Writing TIFF files with more than one directory (IFD) is not easy because
some side effects need to be known.

The main point here is that there can only be one ``tif`` structure in
the main memory for a file, which can only hold the tags of one directory
at a time. It is useless to work with two different tiffOut1, tiffOut2
pointers, because there is only ONE TIFF object (TIFF directory) within
the ``libtiff``. If you want to address a second directory in the file,
the tags of the current directory must first be saved in the file,
otherwise they will be lost (overwritten or mixed). Then the ``tif``
structure in the main memory must be tidied up, otherwise the old tags
will beincluded in the new directory.
This can be done either by creating a new, empty ``tif`` structure or by
reading in an directory previously saved in the file.

A sequence to handle a second (or third) TIFF directory - in this case
the GPS IFD - is as follows:

1) Create TIFF file
2) Complete the "normal" metadata
3) Set the tag for the custom directory with a “dummy” value in order to
   get the tag reserved. The final value will be inserted lateron. This
   prevents the main directory from being written to the file again at an
   additional area, leaving the first memory area unused:

.. highlight:: c

::

    TIFFSetField(tiffOut, TIFFTAG_GPSIFD, dir_offset);

4) Save current TIFF-directory to file. Otherwise, it will be lost.
   Remember also the number of the current directory:

.. highlight:: c

::

    TIFFWriteDirectory(tiffOut);
    dirNum = TIFFCurrentDirectory(tiffOut);

5) Create the second TIFF-directory (e.g. custom directory) and set its
   fields. The TIFFFieldArray infoarray has to be specified beforehand
   somewhere in your private include files.
   An example is given for the EXIF directory in tif_dirinfo.c

.. highlight:: c

::

    TIFFCreateCustomDirectory(tiffOut, infoarray);        /* for a real custom directory */
    /* or alternatively, use GPS or EXIF with pre-defined TIFFFieldArray IFD field structure */
    TIFFCreateGPSDirectory(tiffOut);
    TIFFSetField(tiffOut, GPSTAG_VERSIONID, gpsVersion);  /* set fields of the custom directory */

Be aware that every :c:func:`TIFFCreateDirectory` or :c:func:`TIFFWriteDirectory`
apparently frees the ``*tif`` structure and sets up a new one!

6) Write custom directory to file. The offset to that directory in the file
   is returned in :c:var:`dir_offset`.

.. highlight:: c

::

    TIFFWriteCustomDirectory(tiffOut, &dir_offset);

7) Reload the first directory (i.e. the original TIFF directory).
   Apparently, this reads the data back from file.

.. highlight:: c

::

    TIFFSetDirectory(tiffOut, dirNum);

8) Set the correct offset value of the custom directory IFD tag and save
   that changes to file.

.. highlight:: c

::

    TIFFSetField(tiffOut, TIFFTAG_GPSIFD, dir_offset);
    TIFFWriteDirectory(tiffOut);

RETURN VALUES
-------------

1 is returned when the contents are successfully written to the file.
Otherwise, 0 is returned if an error was encountered when writing the
directory contents.

Diagnostics
-----------

All error messages are directed to the :c:func:`TIFFErrorExtR` routine.
Likewise, warning messages are directed to the :c:func:`TIFFWarningExtR` routine.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFCreateDirectory` (3tiff),
:doc:`TIFFquery` (3tiff),
:doc:`TIFFSetDirectory` (3tiff),
:doc:`TIFFWriteDirectory` (3tiff)

