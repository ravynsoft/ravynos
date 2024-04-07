package Some::Module;
use strict;
use warnings;
use Exporter 5.57 'import';

our @EXPORT_OK = qw(some_sub);

# This is an example of a subroutine that returns (undef, $msg)
# to signal failure.

sub some_sub {
    my ($arg) = @_;

    if ($arg) {
        return (undef, "Insufficient credit");
    }

    return (1,2,3);
}

1;
