#!perl
#===============================================================================
#
# t/99_pod_coverage.t
#
# DESCRIPTION
#   Test script to check POD coverage.
#
# COPYRIGHT
#   Copyright (C) 2015 Steve Hay.  All rights reserved.
#
# LICENCE
#   This script is free software; you can redistribute it and/or modify it under
#   the same terms as Perl itself, i.e. under the terms of either the GNU
#   General Public License or the Artistic License, as specified in the LICENCE
#   file.
#
#===============================================================================

use 5.008001;

use strict;
use warnings;

use Test::More;

#===============================================================================
# MAIN PROGRAM
#===============================================================================

MAIN: {
    plan skip_all => 'Author testing only' unless $ENV{AUTHOR_TESTING};

    my $ok = eval {
        require Test::Pod::Coverage;
        Test::Pod::Coverage->import();
        1;
    };

    if (not $ok) {
        plan skip_all => 'Test::Pod::Coverage required to test POD coverage';
    }
    elsif ($Test::Pod::Coverage::VERSION < 0.08) {
        plan skip_all => 'Test::Pod::Coverage 0.08 or higher required to test POD coverage';
    }
    else {
        all_pod_coverage_ok();
    }
}

#===============================================================================
