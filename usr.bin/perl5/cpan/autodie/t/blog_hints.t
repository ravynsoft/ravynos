#!/usr/bin/perl -w
use strict;
use warnings;
use Test::More 'no_plan';

use FindBin;
use lib "$FindBin::Bin/lib";

use Some::Module qw(some_sub);
use my::autodie qw(! some_sub);

eval { some_sub() };

isnt("$@", "", "some_sub should die in void/scalar context");

isa_ok($@, 'autodie::exception');
is($@->context, 'scalar');
is($@->function, 'Some::Module::some_sub');
like("$@", qr/can't be called in scalar context/);

my @returns = eval { some_sub(0); };
is($@, "", "Good call to some_sub");
is_deeply(\@returns, [1,2,3], "Returns unmolested");

@returns = eval { some_sub(1) };

isnt("$@","");
is($@->return->[0], undef);
is($@->return->[1], 'Insufficient credit');
like("$@", qr/Insufficient credit/);
