tiffsplit
=========

.. program:: tiffsplit

Synopsis
--------

**tiffsplit** *src.tif* [ *prefix* ]

Description
-----------

:program:`tiffsplit` takes a multi-directory (page) TIFF
file and creates one or more single-directory (page) TIFF
files from it.
The output files are given names created by concatenating
a prefix, a lexically ordered suffix in the range [aaa--zzz],
the suffix :file:`.tif`
(e.g. :file:`xaaa.tif`, :file:`xaab.tif`, :file:`…`, :file:`xzzz.tif`).
If a prefix is not specified on the command line, the default prefix of
:file:`x` is used.

Options
-------

None.

Exit status
-----------

:program:`tiffsplit` exits with one of the following values:

0:

  Success

1:

  An error occurred either reading the input or writing results.

Bugs
----

Only a select set of "known tags" are copied when splitting.

See also
--------

:doc:`tiffcp` (1),
:doc:`tiffinfo` (1),
:doc:`/functions/libtiff` (3tiff),
