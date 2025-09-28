#!/usr/bin/perl
use strict;
use warnings;

# tests for generating embedded typemaps

use Test::More tests => 1;
use ExtUtils::Typemaps;

SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_string(string => <<HERE);
char* T_PV
HERE
  is($map->as_embedded_typemap(), <<'HERE', "Embedded typemap as expected");
TYPEMAP: <<END_TYPEMAP;
TYPEMAP
char*	T_PV

END_TYPEMAP
HERE
}


