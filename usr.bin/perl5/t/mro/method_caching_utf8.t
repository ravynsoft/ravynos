#!./perl

BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require './test.pl';
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );
use strict;
use warnings;
no warnings 'redefine'; # we do a lot of this
no warnings 'prototype'; # we do a lot of this

{
    package MC텟ᵀ::Bࡎᶓ;
    sub ᕘ { return $_[1]+1 };

    package MC텟ᵀ::ድ리ᭉᛞ;
    our @ISA = qw/MC텟ᵀ::Bࡎᶓ/;

    package Ƒｏｏ; our @ƑＯＯ = qw//;
}

# These are various ways of re-defining MC텟ᵀ::Bࡎᶓ::ᕘ and checking whether the method is cached when it shouldn't be
my @testsubs = (
    sub { is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 1); },
    sub { eval 'sub MC텟ᵀ::Bࡎᶓ::ᕘ { return $_[1]+2 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 2); },
    sub { eval 'sub MC텟ᵀ::Bࡎᶓ::ᕘ($) { return $_[1]+3 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 3); },
    sub { eval 'sub MC텟ᵀ::Bࡎᶓ::ᕘ($) { 4 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 4); },
    sub { *MC텟ᵀ::Bࡎᶓ::ᕘ = sub { $_[1]+5 }; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 5); },
    sub { local *MC텟ᵀ::Bࡎᶓ::ᕘ = sub { $_[1]+6 }; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 6); },
    sub { is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 5); },
    sub { sub FFF { $_[1]+7 }; local *MC텟ᵀ::Bࡎᶓ::ᕘ = *FFF; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 7); },
    sub { is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 5); },
    sub { sub DḊƋ { $_[1]+8 }; *MC텟ᵀ::Bࡎᶓ::ᕘ = *DḊƋ; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 8); },
    sub { *ǎᔆɗＦ::앗ｄƑ = sub { $_[1]+9 }; *MC텟ᵀ::Bࡎᶓ::ᕘ = \&ǎᔆɗＦ::앗ｄƑ; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 9); },
    sub { undef *MC텟ᵀ::Bࡎᶓ::ᕘ; eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { eval "sub MC텟ᵀ::Bࡎᶓ::ᕘ($);"; *MC텟ᵀ::Bࡎᶓ::ᕘ = \&ǎᔆɗＦ::앗ｄƑ; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 9); },
    sub { *Xƴƶ = sub { $_[1]+10 }; ${MC텟ᵀ::Bࡎᶓ::}{ᕘ} = \&Xƴƶ; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 10); },
    sub { ${MC텟ᵀ::Bࡎᶓ::}{ᕘ} = sub { $_[1]+11 }; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 11); },

    sub { undef *MC텟ᵀ::Bࡎᶓ::ᕘ; eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MC텟ᵀ::Bࡎᶓ; sub ᕘ { $_[1]+12 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 12); },
    sub { eval 'package ᛎᛎᛎ; sub ᕘ { $_[1]+13 }'; *MC텟ᵀ::Bࡎᶓ::ᕘ = \&ᛎᛎᛎ::ᕘ; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 13); },
    sub { ${MC텟ᵀ::Bࡎᶓ::}{ᕘ} = sub { $_[1]+14 }; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 14); },
    # 5.8.8 fails this one
    sub { undef *{MC텟ᵀ::Bࡎᶓ::}; eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MC텟ᵀ::Bࡎᶓ; sub ᕘ { $_[1]+15 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 15); },
    sub { undef %{MC텟ᵀ::Bࡎᶓ::}; eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MC텟ᵀ::Bࡎᶓ; sub ᕘ { $_[1]+16 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 16); },
    sub { %{MC텟ᵀ::Bࡎᶓ::} = (); eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { eval 'package MC텟ᵀ::Bࡎᶓ; sub ᕘ { $_[1]+17 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 17); },
    # 5.8.8 fails this one too
#TODO: This fails due to the tokenizer not being clean, rather than mro.
    sub { *{MC텟ᵀ::Bࡎᶓ::} = *{Ƒｏｏ::}; eval { MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0) }; like($@, qr/locate object method/); },
    sub { *MC텟ᵀ::ድ리ᭉᛞ::ᕘ = \&MC텟ᵀ::Bࡎᶓ::ᕘ; eval { MC텟ᵀ::ድ리ᭉᛞ::ᕘ(0,0) }; ok(!$@); undef *MC텟ᵀ::ድ리ᭉᛞ::ᕘ },
    sub { eval 'package MC텟ᵀ::Bࡎᶓ; sub ᕘ { $_[1]+18 }'; is(MC텟ᵀ::ድ리ᭉᛞ->ᕘ(0), 18); },
);

plan(tests => scalar(@testsubs));

$_->() for (@testsubs);
