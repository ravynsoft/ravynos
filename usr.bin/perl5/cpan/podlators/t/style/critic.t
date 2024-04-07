#!/usr/bin/perl
#
# Check for perlcritic errors in all code.
#
# If author tests are enabled, check all Perl code in blib/lib, examples, usr,
# t, and Build.PL for problems uncovered by perlcritic, ignoring template
# files, junk, and any files explicitly configured to be ignored.
#
# Written by Russ Allbery <eagle@eyrie.org>
# Copyright 2019-2022 Russ Allbery <eagle@eyrie.org>
# Copyright 2013-2014
#     The Board of Trustees of the Leland Stanford Junior University
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

use 5.010;
use strict;
use warnings;

use lib 't/lib';

use Test::RRA qw(skip_unless_author use_prereq);
use Test::RRA::Config qw(@CRITIC_IGNORE);

use Test::More;

# Skip tests unless we're running author tests since this test is too
# sensitive to the exact version of Perl::Critic to be generally useful.
skip_unless_author('Coding style tests');

# Load prerequisite modules.
use_prereq('Perl::Critic::Utils');
use_prereq('Test::Perl::Critic');

# Force the embedded Perl::Tidy check to use the correct configuration.
local $ENV{PERLTIDY} = 't/data/perltidyrc';

# Import the configuration file.
Test::Perl::Critic->import(-profile => 't/data/perlcriticrc');

# By default, Test::Perl::Critic only checks blib.  We also want to check t,
# Build.PL, and examples.
my @files = Perl::Critic::Utils::all_perl_files('blib');
if (!@files) {
    @files = Perl::Critic::Utils::all_perl_files('lib');
}
if (-e 'Build.PL') {
    push(@files, 'Build.PL');
}
for my $dir (qw(examples usr t)) {
    if (-d $dir) {
        push(@files, Perl::Critic::Utils::all_perl_files($dir));
    }
}

# Strip out Autoconf templates or left-over perltidy files.
@files = grep { !m{ [.](?:in|tdy) }xms } @files;

# Strip out ignored files.
my %ignore = map { $_ => 1 } @CRITIC_IGNORE;
@files = grep { !$ignore{$_} } @files;

# Declare a plan now that we know what we're testing.
plan tests => scalar @files;

# Run the actual tests.
for my $file (@files) {
    critic_ok($file);
}
