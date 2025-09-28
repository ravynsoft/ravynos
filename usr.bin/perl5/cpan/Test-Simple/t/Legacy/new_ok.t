#!/usr/bin/perl -w

use strict;

use Test::More tests => 13;

{
    package Bar;

    sub new {
        my $class = shift;
        return bless {@_}, $class;
    }


    package Foo;
    our @ISA = qw(Bar);
}

{
    my $obj = new_ok("Foo");
    is_deeply $obj, {};
    isa_ok $obj, "Foo";

    $obj = new_ok("Bar");
    is_deeply $obj, {};
    isa_ok $obj, "Bar";

    $obj = new_ok("Foo", [this => 42]);
    is_deeply $obj, { this => 42 };
    isa_ok $obj, "Foo";

    $obj = new_ok("Foo", [], "Foo");
    is_deeply $obj, {};
    isa_ok $obj, "Foo";
}

# And what if we give it nothing?
eval {
    new_ok();
};
is $@, sprintf "new_ok() must be given at least a class at %s line %d.\n", $0, __LINE__ - 2;
