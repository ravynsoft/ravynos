This directory contains the source code for the Perl extension
VMS::Stdio, which provides access from Perl to VMS-specific
stdio functions.  For more specific documentation of its
function, please see the pod section of Stdio.pm.

===> Installation

This extension, like most Perl extensions, should be installed
by copying the files in this directory to a location *outside*
the Perl distribution tree, and then saying

    $ perl Makefile.PL  ! Build Descrip.MMS for this extension
    $ MMK               ! Build the extension
    $ MMK test          ! Run its regression tests
    $ MMK install       ! Install required files in public Perl tree


===> Revision History

1.0  29-Nov-1994  Charles Bailey  bailey@genetics.upenn.edu
     original version - vmsfopen
1.1  09-Mar-1995  Charles Bailey  bailey@genetics.upenn.edu
     changed calling sequence to return FH/undef - like POSIX::open
     added fgetname and tmpnam
2.0  28-Feb-1996  Charles Bailey  bailey@genetics.upenn.edu
     major rewrite for Perl 5.002: name changed to VMS::Stdio,
     new functions added, and prototypes incorporated
2.1  24-Mar-1998  Charles Bailey  bailey@newman.upenn.edu
     Added writeof()
     Removed old VMs::stdio compatibility interface
