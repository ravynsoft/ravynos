TIFF File Format Specification
==============================

.. image:: ../images/jim.gif
    :width: 139
    :alt: jim

A copy of the 6.0 specification is available from Adobe at
`<http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf>`_
or from the libtiff
ftp site at `<https://download.osgeo.org/libtiff/doc/TIFF6.pdf>`_.

Draft :doc:`technote2` covers problems
with the TIFF 6.0 design for embedding JPEG-compressed data in TIFF, and 
describes an alternative.

Other Adobe information on TIFF can be retrieved from
`<http://partners.adobe.com/public/developer/tiff/index.html>`_.

Joris Van Damme maintains a list of known tags and their descriptions and
definitions. It is available online at
`<http://www.awaresystems.be/imaging/tiff/tifftags.html>`_.

There is a FAQ, related both to TIFF format and libtiff library:
`<http://www.awaresystems.be/imaging/tiff/faq.html>`_.

A design for a TIFF variation supporting files larger than 4GB is detailed in :doc:`bigtiff`.

The LibTIFF coverage of the TIFF 6.0 specification is detailed in :doc:`coverage`.

.. toctree::
    :maxdepth: 1
    :titlesonly:

    technote2
    bigtiff
    coverage
    coverage-bigtiff
