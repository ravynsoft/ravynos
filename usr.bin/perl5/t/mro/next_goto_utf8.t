#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );
require q(./test.pl); plan(tests => 4);

use mro;

{
    package PṞoxᚤ;
    our @ISA = qw//;
    sub next_prxᔬ { goto &next::method }
    sub maybe_prxᔬ { goto &maybe::next::method }
    sub can_prxᔬ { goto &next::can }

    package Ⱦ밧ᶟ;
    our @ISA = qw//;
    sub ᕗ { 42 }
    sub Ƚ { 24 }
    # বẔ doesn't exist intentionally
    sub ʠঊₓ { 242 }

    package ᵗ톺;
    our @ISA = qw/Ⱦ밧ᶟ/;
    sub ᕗ { shift->PṞoxᚤ::next_prxᔬ() }
    sub Ƚ { shift->PṞoxᚤ::maybe_prxᔬ() }
    sub বẔ { shift->PṞoxᚤ::maybe_prxᔬ() }
    sub ʠঊₓ { shift->PṞoxᚤ::can_prxᔬ()->() }
}

is(ᵗ톺->ᕗ, 42, 'proxy next::method via goto');
is(ᵗ톺->Ƚ, 24, 'proxy maybe::next::method via goto');
ok(!ᵗ톺->বẔ, 'proxy maybe::next::method via goto with no method');
is(ᵗ톺->ʠঊₓ, 242, 'proxy next::can via goto');
