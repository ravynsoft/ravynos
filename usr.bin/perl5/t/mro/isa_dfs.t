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

# No parents

package urkkk;

# 1 parent
@urkkk::ISA = 'klonk';

package kayo;

# 2 parents
@urkkk::ISA = ('klonk', 'kapow');

package thwacke;

# No parents, has @ISA
@thwacke::ISA = ();

package zzzzzwap;

@zzzzzwap::ISA = ('thwacke', 'kapow');

package whamm;

@whamm::ISA = ('kapow', 'thwacke');

package main;

require mro;

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
