#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 1;

# Make sure that case-insensitive matching of any Latin1 chars don't load
# utf8.pm.  We assume that NULL won't force loading utf8.pm, and since it
# doesn't match any of the other chars, the regexec.c code would try to load
# a swash if it thought there was one.
"\0" =~ /[\001-\xFF]/i;

ok(! exists $INC{"utf8.pm"}, 'case insensitive matching of any Latin1 chars does not load utf8.pm');
