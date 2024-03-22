Downloading and Unpacking
=========================

Downloading
-----------

You can download the released versions of Mesa via
`HTTPS <https://archive.mesa3d.org/>`__ or
`FTP <ftp://ftp.freedesktop.org/pub/mesa/>`__.

Our release tarballs are GPG-signed, and the public keys are available
here: `release-maintainers-keys.asc <release-maintainers-keys.asc>`__.

Starting with the first release of 2017, Mesa's version scheme is
year-based. Filenames are in the form ``mesa-Y.N.P.tar.gz``, where ``Y``
is the year (two digits), ``N`` is an incremental number (starting at 0)
and ``P`` is the patch number (0 for the first release, 1 for the first
patch after that).

When a new release is coming, release candidates (betas) may be found in
the same directory, and are recognizable by the
``mesa-Y.N.P-rcX.tar.gz`` filename.

Unpacking
---------

Mesa releases are available in two formats: ``.tar.xz`` and ``.tar.gz``.

To unpack the tarball:

.. code-block:: console

      tar xf mesa-Y.N.P.tar.xz

or

.. code-block:: console

      tar xf mesa-Y.N.P.tar.gz

Contents
--------

Proceed to the :doc:`compilation and installation
instructions <install>`.

Demos, GLUT, and GLU
--------------------

A package of SGI's GLU library is available
`here <https://archive.mesa3d.org/glu/>`__

A package of Mark Kilgard's GLUT library is available
`here <https://archive.mesa3d.org/glut/>`__

The Mesa demos collection is available
`here <https://archive.mesa3d.org/demos/>`__

In the past, GLUT, GLU and the Mesa demos were released in conjunction
with Mesa releases. But since GLUT, GLU and the demos change
infrequently, they were split off into their own Git repositories:
`GLUT <https://gitlab.freedesktop.org/mesa/glut>`__,
`GLU <https://gitlab.freedesktop.org/mesa/glu>`__ and
`Demos <https://gitlab.freedesktop.org/mesa/demos>`__,
