package main;
use strict;
use warnings;

use constant NOFILE1 => 'this_file_had_better_not_exist';
use constant NOFILE2 => NOFILE1 . '2';
use constant NOFILE3 => NOFILE1 . '3';

# Calls open, while still in the main package.  This shouldn't
# be autodying.
sub leak_test {
    return open(my $fh, '<', $_[0]);
}

# This rename shouldn't be autodying, either.
sub leak_test_rename {
    return rename($_[0], $_[1]);
}

# These are used by core-trampoline-slurp.t
sub slurp_leak_unlink {
    unlink(NOFILE1, NOFILE2, NOFILE3);
}

sub slurp_leak_open {
    open(1,2,3,4,5);
}

package autodie_test_module;

# This should be calling CORE::open
sub your_open {
    return open(my $fh, '<', $_[0]);
}

# This should be calling CORE::rename
sub your_rename {
    return rename($_[0], $_[1]);
}

sub your_dying_rename {
    use autodie qw(rename);
    return rename($_[0], $_[1]);
}

1;
