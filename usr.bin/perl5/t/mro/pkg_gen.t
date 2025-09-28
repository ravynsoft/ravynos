#!./perl

use strict;
use warnings;

chdir 't' if -d 't';
require q(./test.pl); plan(tests => 7);

require mro;

{
    package Foo;
    our @ISA = qw//;
}

ok(!mro::get_pkg_gen('ReallyDoesNotExist'),
    "pkg_gen 0 for non-existent pkg");

my $f_gen = mro::get_pkg_gen('Foo');
ok($f_gen > 0, 'Foo pkg_gen > 0');

{
    no warnings 'once';
    *Foo::foo_func = sub { 123 };
}
my $new_f_gen = mro::get_pkg_gen('Foo');
ok($new_f_gen > $f_gen, 'Foo pkg_gen incs for methods');
$f_gen = $new_f_gen;

@Foo::ISA = qw/Bar/;
$new_f_gen = mro::get_pkg_gen('Foo');
ok($new_f_gen > $f_gen, 'Foo pkg_gen incs for @ISA');

undef %Foo::;
is(mro::get_pkg_gen('Foo'), 1, "pkg_gen 1 for undef %Pkg::");

delete $::{"Foo::"};
is(mro::get_pkg_gen('Foo'), 0, 'pkg_gen 0 for delete $::{Pkg::}');

delete $::{"Quux::"};
push @Quux::ISA, "Woot"; # should not segfault
ok(1, "No segfault on modification of ISA in a deleted stash");
