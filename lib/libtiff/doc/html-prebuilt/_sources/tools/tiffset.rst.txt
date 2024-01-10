tiffset
========

.. program:: tiffset

Synopsis
--------

**tiffset** [ *options* ] *filename.tif*

Description
-----------

:program:`tiffset` sets the value of a TIFF header to a specified value
or removes an existing setting.

Options
-------

.. option:: -d dirnumber

  Change the current directory (starting at 0).

.. option:: -s tagnumber [ count ] value …

  Set the value of the named tag to the value or values specified.

.. option:: -sd diroffset

  Change the current directory by offset.

.. option:: -sf tagnumber filename

  Set the value of the tag to the contents of filename.  This option is
  supported for ASCII tags only.

.. option:: -u tagnumber

  Unset the tag.

Examples
--------

The following example sets the image description tag (270) of :file:`a.tif` to
the contents of the file :file:`descrip`:

.. highlight:: shell

::

    tiffset -sf 270 descrip a.tif

The following example sets the artist tag (315) of :file:`a.tif` to the string
``Anonymous``:

::

    tiffset -s 315 Anonymous a.tif


This example sets the resolution of the file :file:`a.tif` to 300 dpi:

::

    tiffset -s 296 2 a.tif
    tiffset -s 282 300.0 a.tif
    tiffset -s 283 300.0 a.tif

Set the photometric interpretation of the third page of :file:`a.tif` to
min-is-black (ie. inverts it):

::

    tiffset -d 2 -s 262 1 a.tif

See also
--------

:doc:`tiffdump` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`/functions/libtiff` (3tiff)
