#!./perl

use strict;
use warnings;

sub freeze_at_begin {
    my ($var) = @_;

    return $var =~ m{$var}o;
}

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';

    freeze_at_begin('frozen');
}

plan tests => 2;

ok( !freeze_at_begin('not'),   "/o done at begin is preserved and a new string does not match" );
ok( freeze_at_begin('frozen'), "/o done at begin is preserved and the original string matches" );

1;

#
# ex: set ts=8 sts=4 sw=4 et:
#
