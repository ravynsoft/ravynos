#!./perl -w

use strict;
use Test::More;

require Fcntl;

# SEEK_SET intentionally included to test the skip functionality.
foreach my $symbol (qw(SEEK_SET O_BINARY S_ENFMT)) {
    my $full_name = "Fcntl::$symbol";
    if (defined eval $full_name) {
	foreach my $code ($full_name, "$full_name()") {
	    my $value = eval $code;
	    like ($value, qr/^[0-9]+$/, "$code is defined on this system");
	}
    } else {
	foreach my $code ($full_name, "$full_name()") {
	    my $value = eval $code;
	    like ($@,
		  qr/^Your vendor has not defined Fcntl macro $symbol, used at \(eval [0-9]+\) line 1\n\z/,
		  "Expected error message for $symbol, not defined on this system");
	}
    }
}

my $value = eval 'Fcntl::S_ISPIE()';
is($value, undef, "Fcntl::S_ISPIE isn't valid");
like ($@,
      qr/^S_ISPIE is not a valid Fcntl macro at \(eval [0-9]+\) line 1\n\z/,
      "Expected error message for S_ISPIE");

done_testing();
