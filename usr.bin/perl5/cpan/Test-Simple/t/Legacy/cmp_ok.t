#!/usr/bin/perl -w

use strict;
use warnings;

use lib 't/lib';

require Test::Simple::Catch;
my($out, $err) = Test::Simple::Catch::caught();
local $ENV{HARNESS_ACTIVE} = 0;

require Test::Builder;
my $TB = Test::Builder->create;
$TB->level(0);

sub try_cmp_ok {
    my($left, $cmp, $right, $error) = @_;
    
    my %expect;
    if( $error ) {
        $expect{ok} = 0;
        $expect{error} = $error;
    }
    else {
        $expect{ok}    = eval "\$left $cmp \$right";
        $expect{error} = $@;
        $expect{error} =~ s/ at .*\n?//;
    }

    local $Test::Builder::Level = $Test::Builder::Level + 1;

    my $ok;
    eval { $ok = cmp_ok($left, $cmp, $right, "cmp_ok"); };

    $TB->is_num(!!$ok, !!$expect{ok}, "  right return");
    
    my $diag = $err->read;

    if ($@) {
        $diag = $@;
        $diag =~ s/ at .*\n?//;
    }

    if( !$ok and $expect{error} ) {
        $diag =~ s/^# //mg;
        $TB->like( $diag, qr/\Q$expect{error}\E/, "  expected error" );
    }
    elsif( $ok ) {
        $TB->is_eq( $diag, '', "  passed without diagnostic" );
    }
    else {
        $TB->ok(1, "  failed without diagnostic");
    }
}


use Test::More;
Test::More->builder->no_ending(1);

require MyOverload;
my $cmp = Overloaded::Compare->new("foo", 42);
my $ify = Overloaded::Ify->new("bar", 23);
my $part = Overloaded::Partial->new('baz', 0);

my @Tests = (
    [1, '==', 1],
    [1, '==', 2],
    ["a", "eq", "b"],
    ["a", "eq", "a"],
    [1, "+", 1],
    [1, "-", 1],

    [$cmp, '==', 42],
    [$cmp, 'eq', "foo"],
    [$ify, 'eq', "bar"],
    [$ify, "==", 23],

    [$part, '!=', 0, 'expected: anything else'],

    [1, "=", 0,  "= is not a valid comparison operator in cmp_ok()"],
    [1, "+=", 0, "+= is not a valid comparison operator in cmp_ok()"],
);

plan tests => scalar @Tests;
$TB->plan(tests => @Tests * 2);

for my $test (@Tests) {
    try_cmp_ok(@$test);
}
