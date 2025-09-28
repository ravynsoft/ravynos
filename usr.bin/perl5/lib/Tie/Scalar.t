#!./perl

BEGIN {
	chdir 't' if -d 't';
	@INC = '../lib';
}

# this must come before main, or tests will fail
package TieTest;

use Tie::Scalar;
our @ISA = qw( Tie::Scalar );

sub new { 'Fooled you.' }

package main;

our $flag;
use Test::More;

use_ok( 'Tie::Scalar' );

# these are "abstract virtual" parent methods
for my $method (qw( TIESCALAR FETCH STORE )) {
	eval { Tie::Scalar->$method() };
	like( $@, qr/doesn't define a $method/, "croaks on inherited $method()" );
}

# the default value is undef
my $scalar = Tie::StdScalar->TIESCALAR();
is( $$scalar, undef, 'used TIESCALAR, default value is still undef' );

# Tie::StdScalar redirects to TIESCALAR
$scalar = Tie::StdScalar->new();
is( $$scalar, undef, 'used new(), default value is still undef' );

# this approach should work as well
tie $scalar, 'Tie::StdScalar';
is( $$scalar, undef, 'tied a scalar, default value is undef' );

# first set, then read
$scalar = 'fetch me';
is( $scalar, 'fetch me', 'STORE() and FETCH() verified with one test!' );

# test DESTROY with an object that signals its destruction
{
	my $scalar = 'foo';
	tie $scalar, 'Tie::StdScalar', DestroyAction->new();
	ok( $scalar, 'tied once more' );
	is( $flag, undef, 'destroy flag not set' );
}

# $scalar out of scope, Tie::StdScalar::DESTROY() called, DestroyAction set flag
is( $flag, 1, 'and DESTROY() works' );

# we want some noise, and some way to capture it
use warnings;
my $warn;
local $SIG{__WARN__} = sub {
	$warn = $_[0];
};

# Tie::Scalar::TIEHANDLE should find and call TieTest::new and complain
is( tie( my $foo, 'TieTest'), 'Fooled you.', 'delegated to new()' );
like( $warn, qr/WARNING: calling TieTest->new/, 'caught warning fine' );

package DestroyAction;

sub new {
	bless( \(my $self), $_[0] );
}

sub DESTROY {
	$main::flag = 1;
}


#
# Bug #72878: don't recurse forever if both new and TIESCALAR are missing.
#
package main;

@NoMethods::ISA = qw [Tie::Scalar];

{
    #
    # Without the fix for #72878, the code runs forever.
    # Trap this, and die if with an appropriate message if this happens.
    #
    local $SIG {__WARN__} = sub {
        die "Called NoMethods->new"
             if $_ [0] =~ /^WARNING: calling NoMethods->new/;
    };

    eval {tie my $foo => "NoMethods";};

    like $@ =>
        qr /\QNoMethods must define either a TIESCALAR() or a new() method/,
        "croaks if both new() and TIESCALAR() are missing";
};

#
# Don't croak on missing new/TIESCALAR if you're inheriting one.
#
my $called1 = 0;
my $called2 = 0;

sub HasMethod1::new {$called1 ++}
   @HasMethod1::ISA        = qw [Tie::Scalar];
   @InheritHasMethod1::ISA = qw [HasMethod1];

sub HasMethod2::TIESCALAR {$called2 ++}
   @HasMethod2::ISA        = qw [Tie::Scalar];
   @InheritHasMethod2::ISA = qw [HasMethod2];

my $r1 = eval {tie my $foo => "InheritHasMethod1"; 1};
my $r2 = eval {tie my $foo => "InheritHasMethod2"; 1};

ok $r1 && $called1, "inheriting new() does not croak";
ok $r2 && $called2, "inheriting TIESCALAR() does not croak";

done_testing();
