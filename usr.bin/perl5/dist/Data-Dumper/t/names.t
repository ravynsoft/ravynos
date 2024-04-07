#!./perl -w

use strict;
use warnings;

use Carp;
use Data::Dumper;
use Test::More tests => 15;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

my ($a, $b, $obj);
my (@names);
my (@newnames, $objagain, %newnames);
my $dumpstr;
$a = 'alpha';
$b = 'beta';

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
@names = $obj->Names;
is_deeply(\@names, [qw(a b)], "Names() returned expected list");

@newnames = ( qw| gamma delta | );
$objagain = $obj->Names(\@newnames);
is($objagain, $obj, "Names returned same object");
is_deeply($objagain->{names}, \@newnames,
    "Able to use Names() to set names to be dumped");

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
%newnames = ( gamma => 'delta', epsilon => 'zeta' );
eval { @names = $obj->Names(\%newnames); };
like($@, qr/Argument to Names, if provided, must be array ref/,
    "Got expected error message: bad argument to Names()");

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
@newnames = ( qw| gamma delta epsilon | );
$objagain = $obj->Names(\@newnames);
is($objagain, $obj, "Names returned same object");
is_deeply($objagain->{names}, \@newnames,
    "Able to use Names() to set names to be dumped");
$dumpstr = _dumptostr($obj);
like($dumpstr, qr/gamma/s, "Got first name expected");
like($dumpstr, qr/delta/s, "Got first name expected");
unlike($dumpstr, qr/epsilon/s, "Did not get name which was not expected");

$obj = Data::Dumper->new([$a,$b], [qw(a b)]);
@newnames = ( qw| gamma | );
$objagain = $obj->Names(\@newnames);
is($objagain, $obj, "Names returned same object");
is_deeply($objagain->{names}, \@newnames,
    "Able to use Names() to set names to be dumped");
$dumpstr = _dumptostr($obj);
like($dumpstr, qr/gamma/s, "Got name expected");
unlike($dumpstr, qr/delta/s, "Did not get name which was not expected");
unlike($dumpstr, qr/epsilon/s, "Did not get name which was not expected");
like($dumpstr, qr/\$VAR2/s, "Got default name");

