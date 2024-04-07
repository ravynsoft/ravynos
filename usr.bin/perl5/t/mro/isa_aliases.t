#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 13;

@Foogh::ISA = "Bar";
*Phoogh::ISA = *Foogh::ISA;
@Foogh::ISA = "Baz";

ok 'Foogh'->isa("Baz"),
 'isa after another stash has claimed the @ISA via glob assignment';
ok 'Phoogh'->isa("Baz"),
 'isa on the stash that claimed the @ISA via glob assignment';
ok !Foogh->isa("Bar"),
 '!isa when another stash has claimed the @ISA via glob assignment';
ok !Phoogh->isa("Bar"),
 '!isa on the stash that claimed the @ISA via glob assignment';

@Foogh::ISA = "Bar";
*Foogh::ISA = ["Baz"];

ok 'Foogh'->isa("Baz"),
 'isa after glob-to-ref assignment when *ISA is shared';
ok 'Phoogh'->isa("Baz"),
 'isa after glob-to-ref assignment on another stash when *ISA is shared';
ok !Foogh->isa("Bar"),
 '!isa after glob-to-ref assignment when *ISA is shared';
ok !Phoogh->isa("Bar"),
 '!isa after glob-to-ref assignment on another stash when *ISA is shared';

@Foo::ISA = "Bar";
*Phoo::ISA = \@Foo::ISA;
@Foo::ISA = "Baz";

ok 'Foo'->isa("Baz"),
 'isa after another stash has claimed the @ISA via ref-to-glob assignment';
ok 'Phoo'->isa("Baz"),
 'isa on the stash that claimed the @ISA via ref-to-glob assignment';
ok !Foo->isa("Bar"),
 '!isa when another stash has claimed the @ISA via ref-to-glob assignment';
ok !Phoo->isa("Bar"),
 '!isa on the stash that claimed the @ISA via ref-to-glob assignment';

*Fooo::ISA = *Baro::ISA;
@Fooo::ISA = "Bazo";
sub Bazo::ook { "Baz" }
sub L::ook { "See" }
Baro->ook;
local *Fooo::ISA = ["L"];
is 'Baro'->ook, 'See', 'localised *ISA=$ref assignment';
