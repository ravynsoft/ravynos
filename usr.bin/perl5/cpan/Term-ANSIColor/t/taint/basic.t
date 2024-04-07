#!/usr/bin/perl -T
#
# Check that Term::ANSIColor untaints generated constants.
#
# It's possible that the name of the constant function that we're calling
# could be tained (such as by loading the name of the constant function from
# an environment variable).  Term::ANSIColor does the work to untaint it; be
# sure that the taint flag is properly cleared.
#
# Copyright 2012, 2020 Russ Allbery <rra@cpan.org>
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use Test::More tests => 4;

# Load the module.
BEGIN {
    delete $ENV{ANSI_COLORS_ALIASES};
    delete $ENV{ANSI_COLORS_DISABLED};
    delete $ENV{NO_COLOR};
    use_ok('Term::ANSIColor', qw(:pushpop));
}

# Generate a tainted constant name.  PATH is always tainted, and tainting is
# sticky, so we can prepend the name to whatever PATH holds and then chop it
# off again.
my $constant = substr('BOLD' . $ENV{PATH}, 0, length('BOLD'));

# Using that as a constant should now work without any tainting problems.
## no critic (TestingAndDebugging::ProhibitNoStrict)
{
    no strict 'refs';
    is(&{$constant}(), "\e[1m", 'Constant subs are not tainted');
    is(BOLD(),         "\e[1m", '...and we can call the sub again');
    ok(defined(&Term::ANSIColor::BOLD), '...and it is now defined');
}
