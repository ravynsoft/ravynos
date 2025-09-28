#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 12);

=pod

This example is taken from: http://rt.cpan.org/Public/Bug/Display.html?id=20879

               ---     ---     ---
Level 5     8 | A | 9 | B | A | C |    (More General)
               ---     ---     ---       V
                  \     |     /          |
                   \    |    /           |
                    \   |   /            |
                     \  |  /             |
                       ---               |
Level 4             7 | D |              |
                       ---               |
                      /   \              |
                     /     \             |
                  ---       ---          |
Level 3        4 | G |   6 | E |         |
                  ---       ---          |
                   |         |           |
                   |         |           |
                  ---       ---          |
Level 2        3 | H |   5 | F |         |
                  ---       ---          |
                      \   /  |           |
                       \ /   |           |
                        \    |           |
                       / \   |           |
                      /   \  |           |
                  ---       ---          |
Level 1        1 | J |   2 | I |         |
                  ---       ---          |
                    \       /            |
                     \     /             |
                       ---               v
Level 0             0 | K |            (More Specialized)
                       ---


0123456789A
KJIHGFEDABC

=cut

{
    package Test::A; use mro 'c3';

    package Test::B; use mro 'c3';

    package Test::C; use mro 'c3';

    package Test::D; use mro 'c3';
    use base qw/Test::A Test::B Test::C/;

    package Test::E; use mro 'c3';
    use base qw/Test::D/;

    package Test::F; use mro 'c3';
    use base qw/Test::E/;
    sub testmeth { "wrong" }

    package Test::G; use mro 'c3';
    use base qw/Test::D/;

    package Test::H; use mro 'c3';
    use base qw/Test::G/;

    package Test::I; use mro 'c3';
    use base qw/Test::H Test::F/;
    sub testmeth { "right" }

    package Test::J; use mro 'c3';
    use base qw/Test::F/;

    package Test::K; use mro 'c3';
    use base qw/Test::J Test::I/;
    sub testmeth { shift->next::method }
}

ok(eq_array(
    mro::get_linear_isa('Test::A'),
    [ qw(Test::A) ]
), '... got the right C3 merge order for Test::A');

ok(eq_array(
    mro::get_linear_isa('Test::B'),
    [ qw(Test::B) ]
), '... got the right C3 merge order for Test::B');

ok(eq_array(
    mro::get_linear_isa('Test::C'),
    [ qw(Test::C) ]
), '... got the right C3 merge order for Test::C');

ok(eq_array(
    mro::get_linear_isa('Test::D'),
    [ qw(Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::D');

ok(eq_array(
    mro::get_linear_isa('Test::E'),
    [ qw(Test::E Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::E');

ok(eq_array(
    mro::get_linear_isa('Test::F'),
    [ qw(Test::F Test::E Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::F');

ok(eq_array(
    mro::get_linear_isa('Test::G'),
    [ qw(Test::G Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::G');

ok(eq_array(
    mro::get_linear_isa('Test::H'),
    [ qw(Test::H Test::G Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::H');

ok(eq_array(
    mro::get_linear_isa('Test::I'),
    [ qw(Test::I Test::H Test::G Test::F Test::E Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::I');

ok(eq_array(
    mro::get_linear_isa('Test::J'),
    [ qw(Test::J Test::F Test::E Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::J');

ok(eq_array(
    mro::get_linear_isa('Test::K'),
    [ qw(Test::K Test::J Test::I Test::H Test::G Test::F Test::E Test::D Test::A Test::B Test::C) ]
), '... got the right C3 merge order for Test::K');

is(Test::K->testmeth(), "right", 'next::method working ok');
