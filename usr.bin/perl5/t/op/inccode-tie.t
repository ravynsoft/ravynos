#!./perl

# Calls all tests in op/inccode.t after tying @INC first.

use Tie::Array;
my @orig_INC = @INC;
tie @INC, 'Tie::StdArray';
@INC = @orig_INC;
for my $file ('./op/inccode.t', './t/op/inccode.t', ':op:inccode.t') {
    if (-r $file) {
	do $file; die $@ if $@;
	exit;
    }
}
die "Cannot find ./op/inccode.t or ./t/op/inccode.t\n";
