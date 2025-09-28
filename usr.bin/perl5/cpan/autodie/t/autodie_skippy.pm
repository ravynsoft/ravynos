package autodie_skippy;
use strict;
use warnings;
use autodie;
use parent qw(autodie::skip);

# This should skip upwards to the caller.

sub fail_open {
    open(my $fh, '<', 'this_file_had_better_not_exist');
}

package autodie_unskippy;
use autodie;

# This should not skip upwards.

sub fail_open {
    open(my $fh, '<', 'this_file_had_better_not_exist');
}

1;
