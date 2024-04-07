package AutoLoad;

use strict;
use warnings;

require Exporter;

our @ISA = qw/Exporter/;
our @EXPORT = qw/func1/;

sub func1 {
    return 1;
}

sub func2 {
    return 1;
}

1;
