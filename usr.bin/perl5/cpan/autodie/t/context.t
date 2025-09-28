#!/usr/bin/perl -w
use strict;

use Test::More;

plan 'no_plan';

sub list_return {
    return if @_;
    return qw(foo bar baz);
}

sub list_return2 {
    return if @_;
    return qw(foo bar baz);
}

# Returns a list presented to it, but also returns a single
# undef if given a list of a single undef.  This mimics the
# behaviour of many user-defined subs and built-ins (eg: open) that
# always return undef regardless of context.

sub list_mirror {
    return undef if (@_ == 1 and not defined $_[0]);
    return @_;

}

use Fatal qw(list_return);
use Fatal qw(:void list_return2);

TODO: {

    # Clobbering context was documented as a bug in the original
    # Fatal, so we'll still consider it a bug here.

    local $TODO = "Fatal clobbers context, just like it always has.";

    my @list = list_return();

    is_deeply(\@list,[qw(foo bar baz)],'fatal sub works in list context');
}

eval {
    my @line = list_return(1);  # Should die
};

ok($@,"List return fatalised");

### Tests where we've fatalised our function with :void ###

my @list2 = list_return2();

is_deeply(\@list2,[qw(foo bar baz)],'fatal sub works in list context');

eval {
    my @line = list_return2(1);  # Shouldn't die
};

ok(! $@,"void List return fatalised survives when non-void");

eval {
    list_return2(1);
};

ok($@,"void List return fatalised");
