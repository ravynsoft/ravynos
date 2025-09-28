# GNU Libtool

1. Introduction
===============

[GNU Libtool][libtool] is a generic library support script.
[Libtool][] hides the complexity of using shared libraries behind a
consistent, portable interface.

Libtool's home page is:

    http://www.gnu.org/software/libtool/libtool.html

See the file [NEWS][] for a description of recent changes to Libtool.

Please note that you can build GNU Libtool from this directory using a
vendor Make program as long as this is an official release tarball;
otherwise you will need GNU Make for sane VPATH support.  See the file
[INSTALL][] for complete generic instructions on how to build and install
Libtool.  Also, see the file [doc/notes.txt][notes] for some platform-
specific information.

See the info node (libtool)Tested Platforms. (or the file
[doc/PLATFORMS][platforms]) for a list of platforms that Libtool already
supports.

Please try it on all the platforms you have access to:

 * If it builds and passes the test suite (`gmake check`), please send
   a short note to the [libtool mailing list][libtool list] with a
   subject line including the string `[PLATFORM]`, and containing the
   details from the end of `./libtool --help` in the body.
 * Otherwise, see _Reporting Bugs_ below for how to help us fix any
   problems you discover.

To use Libtool, add the new generic library building commands to your
`Makefile`, `Makefile.in`, or `Makefile.am`.  See the documentation for
details.

[install]: http://git.savannah.gnu.org/cgit/libtool.git/tree/INSTALL
[libtool]: http://www.gnu.org/s/libtool
[libtool list]: mailto:libtool@gnu.org
[news]: http://git.savannah.gnu.org/cgit/libtool.git/tree/NEWS
[notes]: http://git.savannah.gnu.org/cgit/libtool.git/tree/doc/notes.texi
[platforms]: http://git.savannah.gnu.org/cgit/libtool.git/tree/doc/PLATFORMS


2. Reporting Bugs
=================

If this distribution doesn't work for you, before you report the
problem, at least try upgrading to the latest released version first,
and see whether the issue persists.  If you feel able, you can also
check whether the issue has been fixed in the development sources for
the next release (see _Obtaining the Latest Sources_ below).

Once you've determined that your bug is still not fixed in the latest
version, please send a full report to the libtool [bug mailing list][],
including:

  1. the information from the end of the help message given by
     `./libtool --help`, and the verbose output of any failed tests
     (see _The Test Suites_ immediately below);
  2. complete instructions for how to reproduce your bug, along with
     the results you were expecting, and how they differ from what you
     actually see;
  3. a workaround or full fix for the bug, if you have it;
  4. a copy of `tests/testsuite.log` if you are experiencing failures
     in the Autotest testsuite.
  5. new test cases for the testsuite that demonstrate the bug are
     especially welcome, and will help to ensure that future releases
     don't reintroduce the problem - if you're not able to write a
     complete testsuite case, a simple standalone shell script is
     usually good enough to help us write a test for you.

If you have any other suggestions, or if you wish to port Libtool to a
new platform, please send email to the [mailing list][libtool list].

Please note that if you send us an non-trivial code for inclusion in a
future release, we may ask you for a copyright assignment (for brief
details see the 'Copyright Assignment' section on our
[Contributing][contribute] webpage.

[bug mailing list]: mailto:bug-libtool@gnu.org
[contribute]: http://www.gnu.org/software/libtool/contribute.html


3. The Test Suite
=================

Libtool comes an integrated sets of tests to check that your build
is sane.  You can run like this, assuming that `gmake` refers to GNU
make:

    gmake check

The new, Autotest-driven testsuite is documented in:

    info Autoconf 'testsuite Invocation'

but simple help may also be obtained through:

    gmake check TESTSUITEFLAGS='--help'

For verbose output, add the flag '-v', for running only a subset of the
independent tests, merely specify them by number or by keyword, both of
which are displayed with the '--list' flag.  For example, the 'libtool'
keyword is used for the tests that exercise only this script.  So it is
possible to test an installed script, possibly from a different Libtool
release, with:

    gmake check \
        TESTSUITEFLAGS="-k libtool LIBTOOL=/path/to/libtool"

Some tests, like the one exercising `max_cmd_len` limits, make use of
this to invoke the testsuite recursively on a subset of tests.  For these
tests, the variable `INNER_TESTSUITEFLAGS` may be used.  It will be
expanded right after the `-k libtool`, without separating whitespace, so
that further limiting of the recursive set of tests is possible.  For
example, to run only the template tests within the `max_cmd_len`, use:

    gmake check TESTSUITEFLAGS="-v -x -k max_cmd_len \
                INNER_TESTSUITEFLAGS=',template -v -x'"

If you wish to report test failures to the libtool list, you need to
send the file `tests/testsuite.log` to the [bug mailing list][].


4. Obtaining the Latest Sources
===============================

* With the exception of ancient releases, all official GNU Libtool
  releases have a detached GPG signature file.  With this you can verify
  that the corresponding file (i.e. without the `.sig` suffix) is the
  same file that was released by the owner of it's GPG key ID.  First,
  be sure to download both the .sig file and the corresponding release,
  then run a command like this:

      gpg --verify libtool-x.y.z.tar.gz.sig

  If that command fails because you don't have the required public key,
  then run this command to import it:

      gpg --keyserver keys.gnupg.net --recv-keys 2983D606

  and then rerun the `gpg --verify` command.

* Official stable releases of GNU Libtool, along with these detached
  signature files are available from:

      ftp://ftp.gnu.org/gnu/libtool

  To reduce load on the main server, please use one of the mirrors
  listed at:

      http://www.gnu.org/order/ftp.html

* Alpha quality pre-releases of GNU Libtool, also with detached
  signature files are available from:

      ftp://alpha.gnu.org/gnu/libtool

  and some of the mirrors listed at:

      http://www.gnu.org/order/ftp.html

* The master libtool repository is stored in git.

  If you are a member of the savannah group for GNU Libtool, a writable
  copy of the libtool repository can be obtained by:

      git clone <savannah-user>@git.sv.gnu.org:/srv/git/libtool.git

  If you are behind a firewall that blocks the git protocol, you may
  find it useful to use

      git config --global url.http://git.sv.gnu.org/r/.insteadof \
        git://git.sv.gnu.org/

  to force git to transparently rewrite all savannah git references to
  use http.

  If you are not a member of the savannah group for GNU Libtool, you can
  still fetch a read-only copy with either:

      git clone git://git.sv.gnu.org/libtool.git

  or using the CVS pserver protocol:

      cvs -d:pserver:anonymous@pserver.git.sv.gnu.org:/srv/git/libtool.git \
          co -d libtool HEAD

* Before you can build from git, you need to bootstrap.  This requires:
  - Autoconf 2.64 or later
  - Automake 1.11.1 or later
  - Help2man 1.29 or later
  - Xz 4.999.8beta or later (from [tukaani.org](http://tukaani.org/xz))
  - Texinfo 4.8 or later
  - Any prerequisites of the above (such as m4, perl, tex)

  Note that these bootstrapping dependencies are much stricter than
  those required to use a destributed release for your own packages.
  After installation, GNU Libtool is designed to work either standalone,
  or optionally with:
  - Autoconf 2.59 or later
  - Automake 1.9.6 or later

* The `bootstrap` script sets up the source directory for you to hack.


5. Version Numbering
====================

People have complained that they find the version numbering scheme under
which libtool is released confusing... so we've changed it!

It works like this:

    <major-number>.<minor-number>

Releases with a **major-number** less than 1 were not yet feature
complete.  Releases with a **major-number** of 1 used the old numbering
scheme that everyone disliked so much.  Releases with a **major-number**
of 2 us the new scheme described here.  If libtool ever undergoes a
major rewrite or substantial restructuring, the **major-number** will be
incremented again.

If we make a patch release to fix bugs in a stable release, we use a
third number, so:

    2.4.2

If we make an alpha quality prerelease, we use a fourth number for the
number of changsets applied since the version it's based on:

    2.4.2.418

And finally, if you build an unreleased version it will have a short git
revision hash string in hexadecimal appended to all of that:

    2.4.2.418.3-30eaa

--
  Copyright (C) 2004-2010, 2015-2019, 2021-2022 Free Software
  Foundation, Inc.

  Written by Gary V. Vaughan, 2004

  This file is part of GNU Libtool.

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without warranty of any kind.


Local Variables:
mode: text
fill-column: 72
End:
vim:tw=72
