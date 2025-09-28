#!./perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

chdir 't' if -d 't';
require q(./test.pl); plan(tests => 7);

require mro;

{
    package ᕘ;
    our @ISA = qw//;
}

ok(!mro::get_pkg_gen('레알ឭ되s놑Eξsᴛ'),
    "pkg_gen 0 for non-existent pkg");

my $f_gen = mro::get_pkg_gen('ᕘ');
ok($f_gen > 0, 'ᕘ pkg_gen > 0');

{
    no warnings 'once';
    *ᕘ::ᕘ_Ƒ운ℭ = sub { 123 };
}
my $new_f_gen = mro::get_pkg_gen('ᕘ');
ok($new_f_gen > $f_gen, 'ᕘ pkg_gen incs for methods');
$f_gen = $new_f_gen;

@ᕘ::ISA = qw/Bar/;
$new_f_gen = mro::get_pkg_gen('ᕘ');
ok($new_f_gen > $f_gen, 'ᕘ pkg_gen incs for @ISA');

undef %ᕘ::;
is(mro::get_pkg_gen('ᕘ'), 1, "pkg_gen 1 for undef %Pkg::");

delete $::{"ᕘ::"};
is(mro::get_pkg_gen('ᕘ'), 0, 'pkg_gen 0 for delete $::{Pkg::}');

delete $::{"ㄑଊｘ::"};
push @ㄑଊｘ::ISA, "Woot"; # should not segfault
ok(1, "No segfault on modification of ISA in a deleted stash");
