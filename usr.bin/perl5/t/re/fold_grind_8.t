#!./perl

# Call fold_grind with /l and a UTF-8 locale

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

# Look for a utf8 locale.
my $utf8_locale = find_utf8_ctype_locale();
skip_all "Couldn't find a UTF-8 locale" unless defined $utf8_locale;

my $current_locale = POSIX::setlocale( &POSIX::LC_CTYPE, $utf8_locale) // "";
skip_all "Couldn't set locale to $utf8_locale"
                                        unless $current_locale eq $utf8_locale;

$::TEST_CHUNK = 'L';

do './re/fold_grind.pl';
print STDERR "$@\n" if $@;
print STDERR "$!\n" if $!;

1;

#
# ex: set ts=8 sts=4 sw=4 et:
#
