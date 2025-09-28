#!perl -w

# test the MULTICALL macros
# Note: as of Oct 2010, there are not yet comprehensive tests
# for these macros.

use warnings;
use strict;

use Test::More tests => 80;
use XS::APItest;


{
    my $sum = 0;
    sub add { $sum += $_++ }

    my @a = (1..3);
    XS::APItest::multicall_each \&add, @a;
    is($sum, 6, "sum okay");
    is($a[0], 2, "a[0] okay");
    is($a[1], 3, "a[1] okay");
    is($a[2], 4, "a[2] okay");
}

# [perl #78070]
# multicall using a sub that already has CvDEPTH > 1 caused sub
# to be prematurely freed

{
    my $destroyed = 0;
    sub REC::DESTROY { $destroyed = 1 }

    my $closure_var;
    {
	my $f = sub {
	    no warnings 'void';
	    $closure_var;
	    my $sub = shift;
	    if (defined $sub) {
		XS::APItest::multicall_each \&$sub, 1,2,3;
	    }
	};
	bless $f,  'REC';
	$f->($f);
	is($destroyed, 0, "f not yet destroyed");
    }
    is($destroyed, 1, "f now destroyed");

}

# [perl #115602]
# deep recursion realloced the CX stack, but the dMULTICALL local var
# 'cx' still pointed to the old one.
# This doesn't actually test the failure (I couldn't think of a way to
# get the failure to show at the perl level) but it allows valgrind or
# similar to spot any errors.

{
    sub rec { my $c = shift; rec($c-1) if $c > 0 };
    my @r = XS::APItest::multicall_each { rec(90) } 1,2,3;
    pass("recursion");
}



# Confirm that MULTICALL handles arg return correctly in the various
# contexts. Also check that lvalue subs are handled the same way, as
# these take different code paths.
# Whenever an explicit 'return' is used, it is followed by '1;' to avoid
# the return being optimised into a leavesub.
# Adding a 'for' loop pushes extra junk on the stack, which we want to
# avoid being interpreted as a return arg.

{
    package Ret;

    use XS::APItest qw(multicall_return G_VOID G_SCALAR G_LIST);

    # Helper function for the block that follows:
    # check that @$got matches what would be expected if a function returned
    # the items in @$args in $gimme context.

    sub gimme_check {
        my ($gimme, $got, $args, $desc) = @_;

        if ($gimme == G_VOID) {
            ::is (scalar @$got, 0, "G_VOID:   $desc");
        }
        elsif ($gimme == G_SCALAR) {
            ::is (scalar @$got, 1, "G_SCALAR: $desc: expect 1 arg");
            ::is ($got->[0], (@$args ? $args->[-1] : undef),
                        "G_SCALAR: $desc: correct arg");
        }
        else {
            ::is (join('-',@$got), join('-', @$args), "G_LIST:  $desc");
        }
    }

    for my $gimme (G_VOID, G_SCALAR, G_LIST) {
        my @a;

        # zero args

        @a = multicall_return {()} $gimme;
        gimme_check($gimme, \@a, [], "()");
        sub f1 :lvalue { () }
        @a = multicall_return \&f1, $gimme;
        gimme_check($gimme, \@a, [], "() lval");

        @a = multicall_return { return; 1 } $gimme;
        gimme_check($gimme, \@a, [], "return");
        sub f2 :lvalue { return; 1 }
        @a = multicall_return \&f2, $gimme;
        gimme_check($gimme, \@a, [], "return lval");


        @a = multicall_return { for (1,2) { return; 1 } } $gimme;
        gimme_check($gimme, \@a, [], "for-return");
        sub f3 :lvalue { for (1,2) { return; 1 } }
        @a = multicall_return \&f3, $gimme;
        gimme_check($gimme, \@a, [], "for-return lval");

        # one arg

        @a = multicall_return {"one"} $gimme;
        gimme_check($gimme, \@a, ["one"], "one arg");
        sub f4 :lvalue { "one" }
        @a = multicall_return \&f4, $gimme;
        gimme_check($gimme, \@a, ["one"], "one arg lval");

        @a = multicall_return { return "one"; 1} $gimme;
        gimme_check($gimme, \@a, ["one"], "return one arg");
        sub f5 :lvalue { return "one"; 1 }
        @a = multicall_return \&f5, $gimme;
        gimme_check($gimme, \@a, ["one"], "return one arg lval");

        @a = multicall_return { for (1,2) { return "one"; 1} } $gimme;
        gimme_check($gimme, \@a, ["one"], "for-return one arg");
        sub f6 :lvalue { for (1,2) { return "one"; 1 } }
        @a = multicall_return \&f6, $gimme;
        gimme_check($gimme, \@a, ["one"], "for-return one arg lval");

        # two args

        @a = multicall_return {"one", "two" } $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "two args");
        sub f7 :lvalue { "one", "two" }
        @a = multicall_return \&f7, $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "two args lval");

        @a = multicall_return { return "one", "two"; 1} $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "return two args");
        sub f8 :lvalue { return "one", "two"; 1 }
        @a = multicall_return \&f8, $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "return two args lval");

        @a = multicall_return { for (1,2) { return "one", "two"; 1} } $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "for-return two args");
        sub f9 :lvalue { for (1,2) { return "one", "two"; 1 } }
        @a = multicall_return \&f9, $gimme;
        gimme_check($gimme, \@a, ["one", "two"], "for-return two args lval");
    }

    # MULTICALL *shouldn't* clear savestack after each call

    sub f10 { my $x = 1; $x };
    my @a = XS::APItest::multicall_return \&f10, G_SCALAR;
    ::is($a[0], 1, "leave scope");
}
