#!/usr/bin/env perl -w

# Test isa_ok() and can_ok() in test.pl

use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
    push @INC, ".";
    require 'test.pl';
}

require Test::More;

can_ok('Test::More', qw(require_ok use_ok ok is isnt like skip can_ok
                        pass fail eq_array eq_hash eq_set));
can_ok(bless({}, "Test::More"), qw(require_ok use_ok ok is isnt like skip 
                                   can_ok pass fail eq_array eq_hash eq_set));


isa_ok(bless([], "Foo"), "Foo");
isa_ok([], 'ARRAY');
isa_ok(\42, 'SCALAR');
{
    local %Bar::;
    local @Foo::ISA = 'Bar';
    isa_ok( "Foo", "Bar" );
}


# can_ok() & isa_ok should call can() & isa() on the given object, not 
# just class, in case of custom can()
{
       local *Foo::can;
       local *Foo::isa;
       *Foo::can = sub { $_[0]->[0] };
       *Foo::isa = sub { $_[0]->[0] };
       my $foo = bless([0], 'Foo');
       ok( ! $foo->can('bar') );
       ok( ! $foo->isa('bar') );
       $foo->[0] = 1;
       can_ok( $foo, 'blah');
       isa_ok( $foo, 'blah');
}


note "object/class_ok"; {
    {
        package Child;
        our @ISA = qw(Parent);
    }

    {
        package Parent;
        sub new { bless {}, shift }
    }

    # Unfortunately we can't usefully test the failure case without
    # significantly modifying test.pl
    class_ok "Child", "Parent";
    class_ok "Parent", "Parent";
    object_ok( Parent->new, "Parent" );
    object_ok( Child->new, "Parent" );
}

done_testing;
