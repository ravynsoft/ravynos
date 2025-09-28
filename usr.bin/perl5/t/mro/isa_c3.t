#!perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

use strict;

plan 'no_plan';

# package klonk doesn't have a stash.

package kapow;
use mro 'c3';

# No parents

package urkkk;
use mro 'c3';

# 1 parent
@urkkk::ISA = 'klonk';

package kayo;
use mro 'c3';

# 2 parents
@urkkk::ISA = ('klonk', 'kapow');

package thwacke;
use mro 'c3';

# No parents, has @ISA
@thwacke::ISA = ();

package zzzzzwap;
use mro 'c3';

@zzzzzwap::ISA = ('thwacke', 'kapow');

package whamm;
use mro 'c3';

@whamm::ISA = ('kapow', 'thwacke');

package main;

my %expect =
    (
     klonk => [qw(klonk)],
     urkkk => [qw(urkkk klonk kapow)],
     kapow => [qw(kapow)],
     kayo => [qw(kayo)],
     thwacke => [qw(thwacke)],
     zzzzzwap => [qw(zzzzzwap thwacke kapow)],
     whamm => [qw(whamm kapow thwacke)],
    );

foreach my $package (qw(klonk urkkk kapow kayo thwacke zzzzzwap whamm)) {
    my $ref = bless [], $package;
    my $isa = $expect{$package};
    is("@{mro::get_linear_isa($package)}", "@$isa", "\@ISA for $package");

    foreach my $class ($package, @$isa, 'UNIVERSAL') {
	object_ok($ref, $class, $package);
    }
}

package _119433 {
    use mro 'c3';
    no warnings 'uninitialized';
    $#_119433::ISA++;
    ::pass "no crash when ISA contains nonexistent elements";
}
