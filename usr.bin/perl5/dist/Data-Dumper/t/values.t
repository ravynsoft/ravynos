#!./perl -w

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 4;

my ($a, $b, $obj);
my (@values, @names);
my (@newvalues, $objagain, %newvalues);
$a = 'alpha';
$b = 'beta';

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
@values = $obj->Values;
is_deeply(\@values, [$a,$b], "Values() returned expected list");

@newvalues = ( qw| gamma delta epsilon | );
$objagain = $obj->Values(\@newvalues);
is($objagain, $obj, "Values returned same object");
is_deeply($objagain->{todump}, \@newvalues,
    "Able to use Values() to set values to be dumped");

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
%newvalues = ( gamma => 'delta', epsilon => 'zeta' );
eval { @values = $obj->Values(\%newvalues); };
like($@, qr/Argument to Values, if provided, must be array ref/,
    "Got expected error message: bad argument to Values()");


