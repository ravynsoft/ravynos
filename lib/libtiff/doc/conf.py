# -*- coding: utf-8 -*-
# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

# Substitutions from external build system.
srcdir = None
builddir = None
ext_source_branch = None
ext_source_user = None


# -- Project information -----------------------------------------------------

project = 'LibTIFF'
copyright = '1988-2022, LibTIFF contributors'
author = 'LibTIFF contributors'

# The full version, including alpha/beta/rc tags
release = 'UNDEFINED'
version = 'UNDEFINED'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.extlinks'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

extlinks = {
    # GitLab links
    'issue' : ('https://gitlab.com/libtiff/libtiff/-/issues/%s', 'issue #%s'),
    'merge' : ('https://gitlab.com/libtiff/libtiff/-/merge_requests/%s', 'merge request #%s'),
    'commit' : ('https://gitlab.com/libtiff/libtiff/-/commit/%s', 'commit %s'),
    'branch' : ('https://gitlab.com/libtiff/libtiff/-/tree/%s', 'branch %s'),
    'tag' : ('https://gitlab.com/libtiff/libtiff/-/tags/%s', 'tag %s'),

    # Old Bugzilla
    'bugzilla' : ('http://bugzilla.maptools.org/show_bug.cgi?id=%s', 'MapTools bugzilla #%s'),
    'bugzilla-rs' : ('http://bugzilla.remotesensing.org/show_bug.cgi?id=%s', 'Remote Sensing bugzilla #%s [no longer available]'),

    # GDAL
    'gdal-trac' : ('http://trac.osgeo.org/gdal/ticket/%s', 'GDAL trac #%s'),
    'oss-fuzz' : ('https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=%s', 'OSS-Fuzz #%s'),

    # Security
    'cve' : ('https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-%s', 'CVE-%s'),
}


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinxdoc'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_extra_path = []

# -- Options for manual page output --------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('functions/libtiff', 'libtiff', 'introduction to libtiff, a library for reading and writing TIFF files', author, '3tiff'),

    ('tools/tiffcp', 'tiffcp', 'copy (and possibly convert) a TIFF file', author, 1),
    ('tools/tiffdump', 'tiffdump', 'print verbatim information about TIFF files', author, 1),
    ('tools/tiffinfo', 'tiffinfo', 'print information about TIFF files', author, 1),
    ('tools/tiffset', 'tiffset', 'set or unset a field in a TIFF header', author, 1),
    ('tools/tiffsplit', 'tiffsplit', 'split a multi-image TIFF into single-image TIFF files', author, 1),

    ('functions/_TIFFauxiliary', '_TIFFauxiliary', 'auxiliary functions', author, '3tiff'),
    ('functions/_TIFFRewriteField', '_TIFFRewriteField', 'rewrite a field in the directory on disk', author, '3tiff'),
    ('functions/TIFFAccessTagMethods', 'TIFFAccessTagMethods', 'provides read/write access to the TIFFTagMethods', author, '3tiff'),
    ('functions/TIFFbuffer', 'TIFFbuffer', 'I/O buffering control routines', author, '3tiff'),
    ('functions/TIFFClientInfo', 'TIFFClientInfo', 'provides a method to hand over user defined data from one routine to another', author, '3tiff'),
    ('functions/TIFFClose', 'TIFFClose', 'close a previously opened TIFF file', author, '3tiff'),
    ('functions/TIFFcodec', 'TIFFcodec', 'codec-related utility routines', author, '3tiff'),
    ('functions/TIFFcolor', 'TIFFcolor', 'color conversion routines', author, '3tiff'),
    ('functions/TIFFCreateDirectory', 'TIFFCreateDirectory', 'routines to create a directory and retrieve information about directories', author, '3tiff'),
    ('functions/TIFFCustomDirectory', 'TIFFCustomDirectory', 'routines to create a custom directory', author, '3tiff'),
    ('functions/TIFFCustomTagList', 'TIFFCustomTagList', 'returns information about the custom tag list', author, '3tiff'),
    ('functions/TIFFDataWidth', 'TIFFDataWidth', 'get the size of TIFF data types', author, '3tiff'),
    ('functions/TIFFDeferStrileArrayWriting', 'TIFFDeferStrileArrayWriting', 'defer strile array writing', author, '3tiff'),
    ('functions/TIFFError', 'TIFFError', 'library error handling interface', author, '3tiff'),
    ('functions/TIFFFieldDataType', 'TIFFFieldDataType', 'get TIFF data type from field information', author, '3tiff'),
    ('functions/TIFFFieldName', 'TIFFFieldName', 'get TIFF field name from field information', author, '3tiff'),
    ('functions/TIFFFieldPassCount', 'TIFFFieldPassCount', 'get whether to pass a count to TIFFGetField/TIFFSetField', author, '3tiff'),
    ('functions/TIFFFieldQuery', 'TIFFFieldQuery', 'routines to query TIFF field information', author, '3tiff'),
    ('functions/TIFFFieldReadCount', 'TIFFFieldReadCount', 'get number of values to be read from field', author, '3tiff'),
    ('functions/TIFFFieldTag', 'TIFFFieldTag', 'get TIFF field tag value from field information', author, '3tiff'),
    ('functions/TIFFFieldWriteCount', 'TIFFFieldWriteCount', 'get number of values to be written to field', author, '3tiff'),
    ('functions/TIFFFlush', 'TIFFFlush', 'flush pending writes to an open TIFF file', author, '3tiff'),
    ('functions/TIFFGetField', 'TIFFGetField', 'get the value(s) of a tag in an open TIFF file', author, '3tiff'),
    ('functions/TIFFmemory', 'TIFFmemory', 'memory management-related functions for use with TIFF files', author, '3tiff'),
    ('functions/TIFFMergeFieldInfo', 'TIFFMergeFieldInfo', 'add application-defined TIFF tags to the list of known libtiff tags', author, '3tiff'),
    ('functions/TIFFOpen', 'TIFFOpen', 'open a TIFF file for reading or writing', author, '3tiff'),
    ('functions/TIFFPrintDirectory', 'TIFFPrintDirectory', 'print a description of a TIFF directory', author, '3tiff'),
    ('functions/TIFFProcFunctions', 'TIFFProcFunctions', 'set TIFF processing functions', author, '3tiff'),
    ('functions/TIFFquery', 'TIFFquery', 'query routines', author, '3tiff'),
    ('functions/TIFFReadDirectory', 'TIFFReadDirectory', 'get the contents of the next directory in an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadEncodedStrip', 'TIFFReadEncodedStrip', 'read and decode a strip of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadEncodedTile', 'TIFFReadEncodedTile', 'read and decode a tile of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadFromUserBuffer', 'TIFFReadFromUserBuffer', 'decode data using an user defined buffer', author, '3tiff'),
    ('functions/TIFFReadRawStrip', 'TIFFReadRawStrip', 'return the undecoded contents of a strip of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadRawTile', 'TIFFReadRawTile', 'return an undecoded tile of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadRGBAImage', 'TIFFReadRGBAImage', 'read and decode an image into a fixed-format raster', author, '3tiff'),
    ('functions/TIFFReadRGBAStrip', 'TIFFReadRGBAStrip', 'read and decode an image strip into a fixed-format raster', author, '3tiff'),
    ('functions/TIFFReadRGBATile', 'TIFFReadRGBATile', 'read and decode an image tile into a fixed-format raster', author, '3tiff'),
    ('functions/TIFFReadScanline', 'TIFFReadScanline', 'read and decode a scanline of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFReadTile', 'TIFFReadTile', 'read and decode a tile of data from an open TIFF file', author, '3tiff'),
    ('functions/TIFFRGBAImage', 'TIFFRGBAImage', 'read and decode an image into a raster', author, '3tiff'),
    ('functions/TIFFSetDirectory', 'TIFFSetDirectory', 'set the current directory for an open TIFF file', author, '3tiff'),
    ('functions/TIFFSetField', 'TIFFSetField', 'set the value(s) of a tag in a TIFF file open for writing', author, '3tiff'),
    ('functions/TIFFSetTagExtender', 'TIFFSetTagExtender', 'register the merge function for user defined tags as an extender callback with libtiff', author, '3tiff'),
    ('functions/TIFFsize', 'TIFFsize', 'return the size of various items associated with an open TIFF file', author, '3tiff'),
    ('functions/TIFFStrileQuery', 'TIFFStrileQuery', 'get strile byte count and offset', author, '3tiff'),
    ('functions/TIFFstrip', 'TIFFstrip', 'strip-related utility routines', author, '3tiff'),
    ('functions/TIFFswab', 'TIFFswab', 'byte- and bit-swapping routines', author, '3tiff'),
    ('functions/TIFFtile', 'TIFFtile', 'tile-related utility routines', author, '3tiff'),
    ('functions/TIFFWarning', 'TIFFWarning', 'library warning interface', author, '3tiff'),
    ('functions/TIFFWriteDirectory', 'TIFFWriteDirectory', 'write the current directory in an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteEncodedStrip', 'TIFFWriteEncodedStrip', 'compress and write a strip of data to an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteEncodedTile', 'TIFFWriteEncodedTile', 'compress and write a tile of data to an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteRawStrip', 'TIFFWriteRawStrip', 'write a strip of raw data to an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteRawTile', 'TIFFWriteRawTile', 'write a tile of raw data to an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteScanline', 'TIFFWriteScanline', 'write a scanline to an open TIFF file', author, '3tiff'),
    ('functions/TIFFWriteTile', 'TIFFWriteTile', 'encode and write a tile of data to an open TIFF file', author, '3tiff'),
]

# If true, show URL addresses after external links.
man_show_urls = True
