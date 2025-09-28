#!perl

use strict;
use warnings;

use Test::More tests => 18;

use XS::APItest qw(with_vars);

my $foo = "A"; my $rfoo = \$foo;
my $bar = "B"; my $rbar = \$bar;
my $baz = "C"; my $rbaz = \$baz;

with_vars foo bar baz {
    is $foo, 1;
    is $$rfoo, "A";
    isnt \$foo, $rfoo;

    is $bar, 2;
    is $$rbar, "B";
    isnt \$bar, $rbar;

    is $baz, 3;
    is $$rbaz, "C";
    isnt \$baz, $rbaz;
}

is $foo, "A";
is \$foo, $rfoo;

is $bar, "B";
is \$bar, $rbar;

is $baz, "C";
is \$baz, $rbaz;

with_vars x {
    is $x, 1;
}

is eval('$x++'), undef;
like $@, qr/explicit package name/;
