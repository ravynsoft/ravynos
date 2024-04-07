#!/usr/bin/perl

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};

# Testing of basic document structures

use strict;
BEGIN {
	$|  = 1;
	$^W = 1;
}

use Test::More tests => 2;
use Parse::CPAN::Meta ();

my $one = <<'END_YAML';
---
- foo
END_YAML

my $two = <<'END_YAML';
---
- foo
---
- bar
END_YAML

my $one_scalar_tiny = Parse::CPAN::Meta::Load( $one );
my $two_scalar_tiny = Parse::CPAN::Meta::Load( $two );

is_deeply( $one_scalar_tiny, [ 'foo' ], 'one: Parsed correctly' );
is_deeply( $two_scalar_tiny, [ 'bar' ], 'two: Parsed correctly' );
