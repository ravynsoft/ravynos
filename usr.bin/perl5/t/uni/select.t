#!./perl

#
# Tests whenever the return value of select(FH) is correctly encoded.
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );

plan( tests => 5 );

open DÙP, ">&", *STDERR;
open $dùp, ">&", *STDOUT;
open 둪,  ">&", *STDERR;
open $ᛞ웊, ">&", *STDOUT;

is select(DÙP), "main::STDOUT";
is select($dùp), "main::DÙP";

TODO: {
    local $TODO = "Scalar filehandles not yet clean";
    is select(둪), "main::dùp";
}

is select($ᛞ웊), "main::둪";
TODO: {
    local $TODO = "Scalar filehandles not yet clean";
    is select(STDOUT), "main::ᛞ웊";
}
