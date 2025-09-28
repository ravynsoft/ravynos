#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

use v5.36;
use feature 'class';
no warnings 'experimental::class';

# ADJUST
{
    my $adjusted;

    class Test1 {
        ADJUST { $adjusted .= "a" }
        ADJUST { $adjusted .= "b" }
    }

    Test1->new;
    is($adjusted, "ab", 'both ADJUST blocks run in order');
}

# $self in ADJUST
{
    my $self_in_ADJUST;

    class Test2 {
        ADJUST { $self_in_ADJUST = $self; }
    }

    my $obj = Test2->new;
    is($self_in_ADJUST, $obj, '$self is set correctly inside ADJUST blocks');
}

done_testing;
