#!perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

use strict;
use utf8;
use open qw( :utf8 :std );

plan 'no_plan';

# package klonk doesn't have a stash.

package 캎oẃ;
use mro 'c3';

# No parents

package urḲḵｋ;
use mro 'c3';

# 1 parent
@urḲḵｋ::ISA = 'kഌoんḰ';

package к;
use mro 'c3';

# 2 parents
@urḲḵｋ::ISA = ('kഌoんḰ', '캎oẃ');

package ṭ화ckэ;
use mro 'c3';

# No parents, has @ISA
@ṭ화ckэ::ISA = ();

package Źzzzዟᑉ;
use mro 'c3';

@Źzzzዟᑉ::ISA = ('ṭ화ckэ', '캎oẃ');

package Ẁ함Ｍ;
use mro 'c3';

@Ẁ함Ｍ::ISA = ('캎oẃ', 'ṭ화ckэ');

package main;

my %expect =
    (
     kഌoんḰ => [qw(kഌoんḰ)],
     urḲḵｋ => [qw(urḲḵｋ kഌoんḰ 캎oẃ)],
     캎oẃ => [qw(캎oẃ)],
     к => [qw(к)],
     ṭ화ckэ => [qw(ṭ화ckэ)],
     Źzzzዟᑉ => [qw(Źzzzዟᑉ ṭ화ckэ 캎oẃ)],
     Ẁ함Ｍ => [qw(Ẁ함Ｍ 캎oẃ ṭ화ckэ)],
    );

foreach my $package (qw(kഌoんḰ urḲḵｋ 캎oẃ к ṭ화ckэ Źzzzዟᑉ Ẁ함Ｍ)) {
    my $ref = bless [], $package;
    my $isa = $expect{$package};
    is("@{mro::get_linear_isa($package)}", "@$isa", "\@ISA for $package");

    foreach my $class ($package, @$isa, 'UNIVERSAL') {
	object_ok($ref, $class, $package);
    }
}
