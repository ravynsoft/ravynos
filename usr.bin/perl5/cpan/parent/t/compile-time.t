#!/usr/bin/perl -w

use strict;
use Test::More tests => 3;

{
    package MyParent;
    sub exclaim { "I CAN HAS PERL?" }
}

{
    package Child;
    use parent -norequire, 'MyParent';
}

my $obj = {};
bless $obj, 'Child';
isa_ok $obj, 'MyParent', 'Inheritance';
can_ok $obj, 'exclaim';
is $obj->exclaim, "I CAN HAS PERL?", 'Inheritance is set up correctly';

