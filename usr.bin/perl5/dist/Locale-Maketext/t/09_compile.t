#!perl -T

use strict;
use warnings;

use Test::More tests => 2;

use Scalar::Util qw(tainted);
use Locale::Maketext;

my @ENV_values = map { !/^PERL/ && defined($ENV{$_}) && !ref($ENV{$_}) && $ENV{$_} ? $ENV{$_} : () } sort keys %ENV;
die "No %ENV vars to test?" if !@ENV_values;

my ($tainted_value)= @ENV_values;
$tainted_value =~ s/([\[\]])/~$1/g;

# If ${^TAINT} is not set despite -T, this perl doesn't have taint support
ok(!${^TAINT} || tainted($tainted_value), "\$tainted_value is tainted")
    or die("Could not find tainted value to use for testing (maybe fix the test?)");

my $result = Locale::Maketext::_compile("hello [_1]", $tainted_value);

pass("_compile does not hang on tainted values");

