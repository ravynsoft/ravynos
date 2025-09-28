#!./perl

BEGIN {
	$ENV{FOO} = "foo";
	$ENV{BAR} = "bar";
}

use strict;
use Test::More tests => 2;
use Env qw(FOO $BAR);

$FOO .= "/bar";
$BAR .= "/baz";

is($FOO, 'foo/bar');
is($BAR, 'bar/baz');
