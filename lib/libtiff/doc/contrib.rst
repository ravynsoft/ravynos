Contributed TIFF Software
=========================

.. image:: images/smallliz.jpg
    :width: 108
    :alt: smallliz

The :file:`contrib` directory has contributed software that
uses the TIFF library or which is associated with the library
(typically glue and guidance for ports to non-UNIX platforms, or tools that
aren't directly TIFF related).

.. list-table:: Contributed software
    :widths: 5 20
    :header-rows: 1

    * - Location
      - Description

    * - :file:`contrib/dbs`
      - various tools from Dan & Chris Sears, including a simple X-based viewer

    * - :file:`contrib/ras`
      - two programs by Patrick Naughton for converting
        between Sun rasterfile format and TIFF (these
        require :file:`libpixrect.a`, as opposed to the one in
        tools that doesn't)

    * - :file:`contrib/mac-mpw`

        :file:`contrib/mac-cw`
      - scripts and files from Niles Ritter for building
        the library and tools under Macintosh/MPW C and
        code warrior.

    * - :file:`contrib/acorn`
      - scripts and files from Peter Greenham for building
        the library and tools on an Acorn RISC OS system.

    * - :file:`contrib/win32`
      - scripts and files from Scott Wagner for building
        the library under Windows NT and Windows 95. (The makefile.vc in the
        libtiff/libtiff directory may be sufficient for most users.)

    * - :file:`contrib/win_dib`
      - two separate implementations of TIFF to DIB code suitable for any Win32
        platform.  Contributed by Mark James, and Philippe Tenenhaus.

    * - :file:`contrib/ojpeg`
      - Patch for IJG JPEG library related to support for some Old JPEG in TIFF files.
        Contributed by Scott Marovich.

    * - :file:`contrib/dosdjgpp`
      - scripts and files from Alexander Lehmann for building
        the library under MSDOS with the DJGPP v2 compiler.

    * - :file:`contrib/tags`
      - scripts and files from Niles Ritter for adding private
        tag support at runtime, without changing libtiff.

    * - :file:`contrib/mfs`
      - code from Mike Johnson to read+write images in memory
        without modifying the library

    * - :file:`contrib/pds`
      - various routines from Conrad Poelman; a TIFF image iterator and
        code to support "private sub-directories"

    * - :file:`contrib/iptcutil`
      - A utility by `Bill Radcliffe <billr@corbis.com>`_ to
        convert an extracted IPTC Newsphoto caption from a binary blob to
        ASCII text, and vice versa. IPTC binary blobs can be extracted from
        images via the `ImageMagick <http://www.ImageMagick.org/>`_ convert(1)
        utility.

    * - :file:`contrib/addtiffo`
      - A utility (and supporting subroutine) for building
        one or more reduce resolution
        overviews to an existing TIFF file.  Supplied by
        `Frank Warmerdam <http://pobox.com/~warmerdam>`_.

    * - :file:`contrib/stream`
      - A class (TiffStream) for accessing TIFF files through a C++ stream
        interface.  Supplied by `Avi Bleiweiss <avi@shutterfly.com>`_.


Questions regarding these packages are usually best directed toward
their authors.
