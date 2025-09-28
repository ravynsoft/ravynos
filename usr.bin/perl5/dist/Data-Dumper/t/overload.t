#!./perl -w

use strict;
use warnings;

use Data::Dumper;

use Test::More tests => 4;

package Foo;
use overload '""' => 'as_string';

sub new { bless { foo => "bar" }, shift }
sub as_string { "%%%%" }

package main;

my $f = Foo->new;

isa_ok($f, 'Foo');
is("$f", '%%%%', 'String overloading works');

my $d = Dumper($f);

like($d, qr/bar/);
like($d, qr/Foo/);

