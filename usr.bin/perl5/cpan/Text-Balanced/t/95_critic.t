#!perl
#===============================================================================
#
# t/95_critic.t
#
# DESCRIPTION
#   Test script to check Perl::Critic conformance.
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
        require Test::Perl::Critic;
        Test::Perl::Critic->import(-profile => '');
        1;
    };

    if (not $ok) {
        plan skip_all => 'Test::Perl::Critic required to test with Perl::Critic';
    }
    else {
        all_critic_ok('.');
    }
}

#===============================================================================
