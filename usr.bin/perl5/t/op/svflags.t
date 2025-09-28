#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all("need B, need full perl") if is_miniperl();
}

# Tests the new documented mechanism for determining the original type
# of an SV.

plan tests => 16;
use strict;
use B qw(svref_2object SVf_IOK SVf_NOK SVf_POK);

my $x = 10;
my $xobj = svref_2object(\$x);
is($xobj->FLAGS & (SVf_IOK | SVf_POK), SVf_IOK, "correct base flags on IV");

my $y = $x . "";

is($xobj->FLAGS & (SVf_IOK | SVf_POK), SVf_IOK, "POK not set on IV used as string");

$x = 1.0;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "correct base flags on NV");

$y = $x . "";

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on NV used as string");

my $z = $x;
$x = $z;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on copy of NV used as string");

$x = "Inf" + 0;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "correct base flags on Inf NV");

$y = $x . "";

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on Inf NV used as string");

$z = $x;
$x = $z;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on copy of Inf NV used as string");

$x = "-Inf" + 0;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "correct base flags on -Inf NV");

$y = $x . "";

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on -Inf NV used as string");

$z = $x;
$x = $z;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on copy of -Inf NV used as string");

{
    local $^W = 0;
    $x  = "NaN" + 0;
}

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "correct base flags on NaN NV");

$y = $x . "";

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on NaN NV used as string");

$z = $x;
$x = $z;

is($xobj->FLAGS & (SVf_NOK | SVf_POK), SVf_NOK, "POK not set on copy of NaN NV used as string");

$x = "10";
is($xobj->FLAGS & (SVf_IOK | SVf_POK), SVf_POK, "correct base flags on PV");

$y = $x + 10;

is($xobj->FLAGS & (SVf_IOK | SVf_POK), (SVf_IOK | SVf_POK), "POK still set on PV used as number");
