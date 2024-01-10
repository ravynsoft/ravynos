TIFF Tools Overview
===================

.. image:: images/quad.jpg
    :width: 144
    :alt: quad

Since LibTIFF version 4.6.0 only five tools are suported.
Two tools can be built as unsupported into directory
:file:`tools/unsupported`.
The source code of the other tools is moved to directory
:file:`archive/tools/`.
This source code still serves as a programming example
for the use of the TIFF library.

Manual pages
------------

.. toctree::
    :maxdepth: 1
    :titlesonly:

    tools/tiffcp
    tools/tiffdump
    tools/tiffinfo
    tools/tiffset
    tools/tiffsplit

Programs
--------


.. list-table:: Programs
    :widths: 5 20
    :header-rows: 1

    * - Tool
      - Description


    * - :doc:`tools/tiffcp`
      - Copy, concatenate, and convert TIFF images (e.g. switching from
        ``Compression=5`` to ``Compression=1``) 

    * - :doc:`tools/tiffdump`
      - Display the verbatim contents of the TIFF directory in a file
        (it's very useful for debugging bogus files that you may get from
        someone that claims they support TIFF)

    * - :doc:`tools/tiffinfo`
      - Display information about one or more TIFF files

    * - :doc:`tools/tiffset`
      - Set a field in a TIFF header

    * - :doc:`tools/tiffsplit`
      - Create one or more single-image files from a (possibly)
        multi-image file

Check out the manual pages for details about the above programs.
