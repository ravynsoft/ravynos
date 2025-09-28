#!/usr/bin/perl -w
BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        chdir '../lib/parent';
        @INC = '..';
    }
}

use strict;
use Test::More tests => 9;
use lib 't/lib';

{
    package Child;
    use parent 'Dummy';
}

{
    package Child2;
    require Dummy;
    use parent -norequire, 'Dummy::InlineChild';
}

{
    package Child3;
    use parent "Dummy'Outside";
}

my $obj = {};
bless $obj, 'Child';
isa_ok $obj, 'Dummy';
can_ok $obj, 'exclaim';
is $obj->exclaim, "I CAN FROM Dummy", 'Inheritance is set up correctly';

$obj = {};
bless $obj, 'Child2';
isa_ok $obj, 'Dummy::InlineChild';
can_ok $obj, 'exclaim';
is $obj->exclaim, "I CAN FROM Dummy::InlineChild", 'Inheritance is set up correctly for inlined classes';

$obj = {};
bless $obj, 'Child3';
isa_ok $obj, 'Dummy::Outside';
can_ok $obj, 'exclaim';
is $obj->exclaim, "I CAN FROM Dummy::Outside", "Inheritance is set up correctly for classes inherited from via '";

