#!/usr/bin/perl -w
use strict;

use Test::More;

plan 'no_plan';

# Returns a list presented to it, but also returns a single
# undef if given a list of a single undef.  This mimics the
# behaviour of many user-defined subs and built-ins (eg: open) that
# always return undef regardless of context.
#
# We also do an 'empty return' if no arguments are passed.  This
# mimics the PBP guideline for returning nothing.

sub list_mirror {
    return undef if (@_ == 1 and not defined $_[0]);
    return if not @_;
    return @_;

}

### autodie clobbering tests ###

eval {
    list_mirror();
};

is($@, "", "No autodie, no fatality");

eval {
    use autodie qw(list_mirror);
    list_mirror();
};

ok($@, "Autodie fatality for empty return in void context");

eval {
    list_mirror();
};

is($@, "", "No autodie, no fatality (after autodie used)");

eval {
    use autodie qw(list_mirror);
    list_mirror(undef);
};

ok($@, "Autodie fatality for undef return in void context");

eval {
    use autodie qw(list_mirror);
    my @list = list_mirror();
};

ok($@,"Autodie fatality for empty list return");

eval {
    use autodie qw(list_mirror);
    my @list = list_mirror(undef);
};

ok($@,"Autodie fatality for undef list return");

eval {
    use autodie qw(list_mirror);
    my @list = list_mirror("tada");
};

ok(! $@,"No Autodie fatality for defined list return");

eval {
    use autodie qw(list_mirror);
    my $single = list_mirror("tada");
};

ok(! $@,"No Autodie fatality for defined scalar return");

eval {
    use autodie qw(list_mirror);
    my $single = list_mirror(undef);
};

ok($@,"Autodie fatality for undefined scalar return");
