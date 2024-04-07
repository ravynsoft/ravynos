package NoExporter;

use strict;
our $VERSION = 1.02;

sub import {
    shift;
    die "NoExporter exports nothing.  You asked for: @_" if @_;
}

1;

