#!./perl

use strict;
use warnings;
no warnings 'once';

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './loc_tools.pl';
}

skip_all "No locales" unless locales_enabled('LC_CTYPE');

my $current_locale = POSIX::setlocale( &POSIX::LC_CTYPE, "C") // "";
skip_all "Couldn't set locale to C" unless $current_locale eq 'C';

use locale;

# Some implementations don't have the 128-255 range characters all
# mean nothing under the C locale (an example being VMS).  This is
# legal, but since we don't know what the right answers should be,
# skip the locale tests in that situation.
for my $i (128 .. 255) {
    my $char = chr(utf8::unicode_to_native($i));
    skip_all "C locale doesn't behave as expected" if uc($char) ne $char
                                                   || lc($char) ne $char;
}

$::TEST_CHUNK = 'l';

do './re/fold_grind.pl';
print STDERR "$@\n" if $@;
print STDERR "$!\n" if $!;

1;

#
# ex: set ts=8 sts=4 sw=4 et:
#
