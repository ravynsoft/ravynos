#!/usr/bin/perl -w

use strict;
use warnings;

use lib 't/lib';

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => 't', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

use Test::More tests => 1;
use ExtUtils::MakeMaker;

# dir_target() was typo'd as dir_targets()
can_ok('MM', 'dir_target');
