# Maintainer's Guide

## Working with the test suite

Most of the tests are contained in the `runtest` executable which
generally reads test cases from the `test` directory and compares output
to files in the `result` directory.

You can simply add new test cases and run `runtest -u` to update the
results. If you debug test failures, it's also useful to execute
`runtest -u` and then `git diff result` to get a diff between actual and
expected results. You can restore the original results by running
`git restore result` and `git clean -xd result`.

## Generated files

The documentation and other generated files can be rebuilt by running

    make -C doc rebuild

This requires `xsltproc`, the DocBook stylesheets in your XML Catalog
and the libxml2 Python bindings to be installed, so it's best done on a
Linux system. On Debian/Ubuntu, try

    apt install xsltproc python3-libxml2 docbook-xsl docbook-xml

doc/apibuild.py generates doc/libxml2-api.xml which is used to generate

- API documentation with XSLT stylesheets
- testapi.c with gentest.py
- Python bindings with python/generator.py

Man pages and HTML documentation for xmllint and xmlcatalog are
generated with xsltproc and DocBook stylesheets.

## Making a release

### Rebuild generated files and documentation

See above for details and run `make -C doc rebuild`.

Look for new warning messages and inspect changes for correctness
before committing.

### Update the NEWS file

You can get started by running

    git log --format='- %s (%an)' [previous-release-tag]..

### Bump the version number

Edit the version number in `configure.ac` if you haven't done so already.

### Build the tarball

I'd recommend to build the tarball by running

    make distcheck

which performs some useful checks as well.

### Upload the tarball

Follow the instructions at
<https://wiki.gnome.org/MaintainersCorner/Releasing>:

    scp libxml2-[version].tar.xz master.gnome.org:
    ssh master.gnome.org ftpadmin install libxml2-[version].tar.xz

### Tag the release

Create an annotated tag and push it:

    git tag -a [version] -m 'Release [version]'
    git push origin [version]

### Create a GitLab release

Create a new GitLab release on
<https://gitlab.gnome.org/GNOME/libxml2/-/releases>.

### Announce the release

Announce the release by sending an email to the mailing list at
xml@gnome.org.

## Updating the CI Docker image

Note that the CI image is used for libxslt as well. Run the following
commands with the Dockerfile in the .gitlab-ci directory:

    docker login registry.gitlab.gnome.org
    docker build -t registry.gitlab.gnome.org/gnome/libxml2 - \
        < .gitlab-ci/Dockerfile
    docker push registry.gitlab.gnome.org/gnome/libxml2

