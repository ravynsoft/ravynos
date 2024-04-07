#!./perl -w

BEGIN {
    chdir "t" if -d "t";
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
}

# Test srand.

use strict;

plan(tests => 10);

# Generate a load of random numbers.
# int() avoids possible floating point error.
sub mk_rand { map int rand 10000, 1..100; }


# Check that rand() is deterministic.
srand(1138);
my @first_run  = mk_rand;

srand(1138);
my @second_run = mk_rand;

ok( eq_array(\@first_run, \@second_run),  'srand(), same arg, same rands' );


# Check that different seeds provide different random numbers
srand(31337);
@first_run  = mk_rand;

srand(1138);
@second_run = mk_rand;

ok( !eq_array(\@first_run, \@second_run),
                                 'srand(), different arg, different rands' );


# Check that srand() isn't affected by $_
{   
    local $_ = 42;
    srand();
    @first_run  = mk_rand;

    srand(42);
    @second_run = mk_rand;

    ok( !eq_array(\@first_run, \@second_run),
                       'srand(), no arg, not affected by $_');
}

# This test checks whether Perl called srand for you.
{
    local $ENV{PERL_RAND_SEED};
    @first_run  = `$^X -le "print int rand 100 for 1..100"`;
    sleep(1); # in case our srand() is too time-dependent
    @second_run = `$^X -le "print int rand 100 for 1..100"`;
}

ok( !eq_array(\@first_run, \@second_run), 'srand() called automatically');

# check srand's return value
my $seed = srand(1764);
is( $seed, 1764, "return value" );

$seed = srand(0);
ok( $seed, "true return value for srand(0)");
cmp_ok( $seed, '==', 0, "numeric 0 return value for srand(0)");

{
    my @warnings;
    my $b;
    {
	local $SIG{__WARN__} = sub {
	    push @warnings, "@_";
	    warn @_;
	};
	$b = $seed + 0;
    }
    is( $b, 0, "Quacks like a zero");
    is( "@warnings", "", "Does not warn");
}

# [perl #40605]
{
    use warnings;
    my $w = '';
    local $SIG{__WARN__} = sub { $w .= $_[0] };
    srand(2**100);
    like($w, qr/^Integer overflow in srand at /, "got a warning");
}
