#!/usr/bin/perl
use strict;
use warnings;

use Test::More tests => 2;
use ExtUtils::Typemaps;

SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  $map->add_inputmap(xstype => 'T_UV', code => '  $var = ($type)SvUV($arg);');
  is($map->as_string(), <<'HERE', "Simple typemap (with input and code including leading whitespace) matches expectations");
TYPEMAP
unsigned int	T_UV

INPUT
T_UV
  $var = ($type)SvUV($arg);
HERE
}


SCOPE: {
  my $map = ExtUtils::Typemaps->new();
  $map->add_typemap(ctype => 'unsigned int', xstype => 'T_UV');
  $map->add_inputmap(xstype => 'T_UV', code => "  \$var =\n(\$type)\n          SvUV(\$arg);");
  is($map->as_string(), <<'HERE', "Simple typemap (with input and multi-line code) matches expectations");
TYPEMAP
unsigned int	T_UV

INPUT
T_UV
  $var =
	($type)
          SvUV($arg);
HERE
}

