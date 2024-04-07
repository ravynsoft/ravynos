#!./perl

# Call fold_grind with /l and a UTF-8 Turkic locale

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

my $turkic_locale = find_utf8_turkic_locale();
skip_all "Couldn't find a UTF-8 turkic locale" unless defined $turkic_locale;

my $current_locale = POSIX::setlocale( &POSIX::LC_CTYPE, $turkic_locale) // "";
skip_all "Couldn't set locale to $turkic_locale"
                                        unless $current_locale eq $turkic_locale;

$::TEST_CHUNK = 'T';

do './re/fold_grind.pl';
print STDERR "$@\n" if $@;
print STDERR "$!\n" if $!;

1;

#
# ex: set ts=8 sts=4 sw=4 et:
#
