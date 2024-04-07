#!./perl -w

# Original by slaven@rezic.de, modified by jhi and matt.w.johnson@gmail.com
#
# Adapted from Porting/cmpVERSION.pl by Abigail
# Changes folded back into that by Nicholas
#
# If some modules fail this, you need a version bump (_001, etc.)
# AND you should probably also nudge the upstream maintainer for
# example by filing a bug, with a patch attached and linking to
# the core change.
#
# This test script works by finding the last non-RC tagged commit,
# which it assumes was the last release, then for each module:
# if it has changed since that commit, but its version number is still the
# same as that commit, report it.
#
# There's also a module exclusion list in Porting/cmpVERSION.pl.

BEGIN {
    @INC = '..' if -f '../TestInit.pm';
    @INC = '.'  if -f  './TestInit.pm';
}
use TestInit qw(T A); # T is chdir to the top level, A makes paths absolute
use strict;

require './t/test.pl';
my $source = find_git_or_skip('all');
chdir $source or die "Can't chdir to $source: $!";

system "$^X Porting/cmpVERSION.pl --tap";
