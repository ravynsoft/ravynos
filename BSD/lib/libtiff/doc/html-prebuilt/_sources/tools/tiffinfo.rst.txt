tiffinfo
========

.. program:: tiffinfo

Synopsis
--------

**tiffinfo** [ *options* ] *input.tif* …

Description
-----------

:program:`tiffinfo` displays information about files created according
to the Tag Image File Format, Revision 6.0. By default, the contents of
each TIFF directory in each file are displayed, with the value of each
tag shown symbolically (where sensible).

Options
-------

.. option:: -c

  Display the colormap and color/gray response curves, if present.

.. option:: -D

  In addition to displaying the directory tags,
  read and decompress all the data in each image (but not display it).

.. option:: -d

  In addition to displaying the directory tags,
  print each byte of decompressed data in hexadecimal.

.. option:: -j

  Display any JPEG-related tags that are present.

.. option:: -o

  Set the initial TIFF directory according to the specified file offset.
  The file offset may be specified using the usual C-style syntax;
  i.e. a leading ``0x`` for hexadecimal and a leading ``0`` for octal.

.. option:: -s

  Display the offsets and byte counts for each data strip in a directory.

.. option:: -z

  Enable strip chopping when reading image data.

.. option:: -#

  Set the initial TIFF directory to *#*.

See also
--------

:doc:`tiffdump`,
:doc:`/functions/libtiff`
