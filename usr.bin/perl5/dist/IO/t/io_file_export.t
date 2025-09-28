#!./perl -w

# This script checks that IO::File exports the SEEK* constants if
# IO::Seekable is loaded first, which was temporarily broken during 5.14
# code freeze. See [perl #88486].

BEGIN{
    require($ENV{PERL_CORE} ? "../../t/test.pl" : "./t/test.pl");
    plan(tests => 3);
}

use IO::Seekable (); # import nothing
use IO::File;        # import defaults

# No strict!
cmp_ok SEEK_END, 'ne', "SEEK_END", 'SEEK_END';
cmp_ok SEEK_SET, 'ne', "SEEK_SET", 'SEEK_SET';
cmp_ok SEEK_CUR, 'ne', "SEEK_CUR", 'SEEK_CUR';
