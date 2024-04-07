#!./perl -T

use strict;
use warnings;

use Config;
use Test::More;
use Scalar::Util qw(tainted);

if (exists($Config{taint_support}) && not $Config{taint_support}) {
    plan skip_all => "your perl was built without taint support";
}
else {
    plan tests => 5;
}


ok( !tainted(1), 'constant number');

my $var = 2;

ok( !tainted($var), 'known variable');

ok( tainted($^X), 'interpreter variable');

$var = $^X;
ok( tainted($var), 'copy of interpreter variable');

{
    package Tainted;
    sub TIESCALAR { bless {} }
    sub FETCH { $^X }
}

tie my $tiedvar, 'Tainted';
ok( tainted($tiedvar), 'for magic variables');
