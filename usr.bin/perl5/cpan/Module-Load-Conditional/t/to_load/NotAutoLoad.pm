package NotAutoLoad;

use strict;
use warnings;

require Exporter;

our @ISA = qw/Exporter/;
our @EXPORT = qw/func3/;

sub func3 {
    return 1;
}

sub func4 {
    return 1;
}

1;
