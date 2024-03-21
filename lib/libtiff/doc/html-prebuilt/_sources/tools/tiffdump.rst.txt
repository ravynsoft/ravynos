tiffdump
========

.. program:: tiffdump

Synopsis
--------

**tiffdump** [ *options* ] *name* …

Description
-----------

:program:`tiffdump` displays directory information from files created according
to the Tag Image File Format, Revision 6.0.
The header of each TIFF file (magic number, version, and first directory offset)
is displayed, followed by the tag contents of each directory in the file.
For each tag, the name, data type, count, and value(s) is displayed.
When the symbolic name for a tag or data type is known, the symbolic
name is displayed followed by it's numeric (decimal) value.
Tag values are displayed enclosed in ``<>`` characters immediately
preceded by the value of the count field.
For example, an ``ImageWidth``
tag might be displayed as ``ImageWidth (256) SHORT (3) 1<800>``.

:program:`tiffdump` is particularly useful for investigating the contents of
TIFF files that :program:`libtiff` does not understand.

Options
-------

.. option:: -h

  Force numeric data to be printed in hexadecimal rather than the
  default decimal.

.. option:: -m items

  Change the number of indirect data items that are printed. By default, this
  will be 24.

.. option:: -o offset

  Dump the contents of the IFD at the a particular file offset.
  The file offset may be specified using the usual C-style syntax;
  i.e. a leading ``0x`` for hexadecimal and a leading ``0`` for octal.

See also
--------

:doc:`tiffinfo` (1),
:doc:`/functions/libtiff` (3tiff)
