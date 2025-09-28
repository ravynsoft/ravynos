#!perl -w

# Check that require doesn't leave the handle it uses open, if it happens that
# the handle it opens gets file descriptor 0. RT #37033.

chdir 't' if -d 't';
require './test.pl';
set_up_inc( 'lib' );

use strict;

sub test_require {
    my ($state, $want) = @_;
    delete $INC{'test_use_14937.pm'};
    open my $fh, '<', 'README' or die "Can't open README: $!";
    my $fileno = fileno $fh;
    if (defined $want) {
	is($fileno, $want,
	   "file handle has correct numeric file descriptor $state");
    } else {
	like($fileno, qr/\A\d+\z/,
	     "file handle has a numeric file descriptor $state");
    }
    close $fh or die;

    is($INC{'test_use_14937.pm'}, undef, "test_use_14937 isn't loaded $state");
    require test_use_14937;
    isnt($INC{'test_use_14937.pm'}, undef, "test_use_14937 is loaded $state");

    open $fh, '<', 'README' or die "Can't open README: $!";
    is(fileno $fh, $fileno,
       "file handle has the same numeric file descriptor $state");
    close $fh or die;
}

is(fileno STDIN, 0, 'STDIN is open on file descriptor 0');
test_require('(STDIN open)');

close STDIN or die "Can't close STDIN: $!";

is(fileno STDIN, undef, 'STDIN is closed');
test_require('(STDIN closed)', 0);

done_testing();
