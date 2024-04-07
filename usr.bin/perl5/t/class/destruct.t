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

# A legacy-perl class to act as a test helper
package DestructionNotify {
    sub new { my $pkg = shift; bless [ @_ ], $pkg }
    sub DESTROY { my $self = shift; ${ $self->[0] } .= $self->[1] }
}

{
    my $destroyed;
    my $notifier = DestructionNotify->new( \$destroyed, 1 );
    undef $notifier;
    $destroyed or
        BAIL_OUT('DestructionNotify does not work');
}

{
    my $destroyed;

    class Test1 {
        field $x;
        method x { return $x; }
        ADJUST {
            $x = DestructionNotify->new( \$destroyed, "x" );
        }

        field $y;
        field $z;
        ADJUST {
            # These in the "wrong" order just to prove to ourselves that it
            # doesn't matter
            $z = DestructionNotify->new( \$destroyed, "z" );
            $y = DestructionNotify->new( \$destroyed, "y" );
        }
    }

    my $obj = Test1->new;
    ok(!$destroyed, 'Destruction notify not yet triggered');

    refcount_is $obj, 1, 'Object has one reference';

    # one in $obj, one stack temporary here
    refcount_is $obj->x, 2, 'DestructionNotify has two references';

    undef $obj;
    is($destroyed, "zyx", 'Destruction notify triggered by object destruction in the correct order');
}

done_testing;
