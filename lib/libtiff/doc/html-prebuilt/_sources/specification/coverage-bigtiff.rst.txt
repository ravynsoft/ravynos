LibTIFF Coverage of the BigTIFF Specification
=============================================

Since LibTIFF version 4.0.0, the BigTIFF format specification has been implemented within LibTIFF.
BigTIFF is a backward-compatible extension to the `TIFF <http://www.awaresystems.be/imaging/tiff.html>`_
file format.

BigTIFF logically extends the original TIFF file format (referred to as 'ClassicTIFF' from now on).
The BigTIFF specification is the result of work by a variety of parties on the LibTIFF mailing list, including
the then current LibTIFF maintainers, Joris Van Damme and Adobe staff. The BigTIFF specification has not yet
been officially approved by the TIFF specification owner (Adobe), but implementation within LibTIFF
could accelerate that process.  For more information on the BigTIFF file format, we recommend AWare Systems'
`BigTIFF page <http://www.awaresystems.be/imaging/tiff/bigtiff.html>`_.

BigTIFF is especially useful for people and vendors that are confronted with very large images, and
still seek to use an open, simple, and extendable format. This requirement is frequently encountered in the
geospatial field, but also concerns large format scanners, medical imaging and other fields.

The LibTiff BigTIFF support was made possible by four sponsors funding the project. The programming work was
primarily done by `Joris Van Damme (AWare Systems) <http://www.awaresystems.be/index.html>`_.
The sponsors for the BigTIFF support work in LibTIFF, in alphabetical order:

.. list-table:: Sponsors
    :widths: 5 20
    :header-rows: 0

    * - .. image:: bigtiffpr_images/esri.png
            :width: 100%
            :alt: ESRI
      - `ESRI <http://www.esri.com/>`_ has been giving customers around the world the power to
        think and plan geographically since 1969.  As the leader in GIS, ESRI applies innovative technologies to help
        organizations create, analyze, and visualize information for more informed decisions.  Running on more than
        a million desktops and thousands of servers, ESRI applications are the foundation of the world's mapping and
        spatial analysis infrastructure.
    * - .. image:: bigtiffpr_images/leica.png
            :width: 100%
            :alt: Leica Geosystems Geospatial Imaging
      - `Leica Geosystems Geospatial Imaging <http://gi.leica-geosystems.com/default.aspx>`_ offers a
        range of workflow solutions for photogrammetry, mapping, remote sensing, catalog management and exploitation
        of geospatial imagery. Enterprise organizations use this imagery as the basis for generating information for
        both education and decision making processes. Those who use Leica Geosystems products every day trust them
        for their precision, seamless integration, interoperability and superior customer support.
    * - .. image:: bigtiffpr_images/safe.png
            :width: 100%
            :alt: Safe Software
      - `Safe Software <http://www.safe.com/>`_ is the maker of FME, a powerful spatial ETL (Extract,
        Transform and Load) tool that enables true data interoperability. FME manages the translation,
        transformation, integration and web-based distribution of geospatial data in 200 GIS, CAD, raster and
        database formats. Safe Software's FME technology is also embedded in numerous market-leading GIS and
        ETL applications.
    * - .. image:: bigtiffpr_images/weogeo.png
            :width: 100%
            :alt: WeoGeo
      - `WeoGeo <http://www.weogeo.com/>`_ is a web-based data management resource for the geospatial
        industry that allows the free market exchange of mapping related imagery products, featuring an innovative
        solution that efficiently manages digital mapping files of any size. With an intuitive user interface
        and the scalable power of Amazon Web Services (AWS), geospatial professionals can view, sort, search,
        and share complex, high volume maps quickly and effectively.
