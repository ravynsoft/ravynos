#!/usr/bin/perl
use strict;
use warnings;

# This is a quickie script which I wrote to generate the .patch file for
# an arbitrary commit. It takes on sha1 as an argument, or saving that
# uses the sha1 associated to HEAD.
# It tries to find which of our primary branches the sha1 can be found on,
# and then prints to standard out something similar to what our rsync feed
# would produce for that situation. The main difference being, in that case
# we KNOW what branch we are on, and in this one we don't, and in that case
# the $tstamp field holds the time the snapshot was generated (so that multiple
# fetches will always have an increasing tstamp field), however in this case
# we use the commit date of the sha1.
#
# This is more or less intended to be used as a utility to generated .patch
# files for other processes, like gitweb and snapshots.
#
# The script assumes it is being run from a git WD.
#
# Yves

use lib "Porting";
use GitUtils qw(gen_dot_patch);
print gen_dot_patch(@ARGV), -t STDOUT ? "\n" : "";


