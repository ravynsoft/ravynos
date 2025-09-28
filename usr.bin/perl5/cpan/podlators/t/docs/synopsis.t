#!/usr/bin/perl
#
# Check the SYNOPSIS section of the documentation for syntax errors.
#
# The canonical version of this file is maintained in the rra-c-util package,
# which can be found at <https://www.eyrie.org/~eagle/software/rra-c-util/>.
#
# Written by Russ Allbery <eagle@eyrie.org>
# Copyright 2019, 2021 Russ Allbery <eagle@eyrie.org>
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

use Test::RRA qw(skip_unless_automated use_prereq);

use File::Spec;
use Test::More;

# Skip for normal user installs since this doesn't affect functionality.
skip_unless_automated('Synopsis syntax tests');

# Load prerequisite modules.
use_prereq('Perl::Critic::Utils');
use_prereq('Test::Synopsis');

# Helper function that checks to see if a given path starts with blib/script.
# This is written a bit weirdly so that it's portable to Windows and VMS.
#
# $path - Path to a file
#
# Returns: True if the file doesn't start with blib/script, false otherwise.
sub in_blib_script {
    my ($path) = @_;
    my ($volume, $dir, $file) = File::Spec->splitpath($path);
    my @dir = File::Spec->splitdir($dir);
    return (scalar(@dir) < 2 || $dir[0] ne 'blib' || $dir[1] ne 'script');
}

# The default Test::Synopsis all_synopsis_ok() function requires that the
# module be in a lib directory.  Use Perl::Critic::Utils to find the modules
# in blib, or lib if it doesn't exist.  However, strip out anything in
# blib/script, since scripts use a different SYNOPSIS syntax.
my @files = Perl::Critic::Utils::all_perl_files('blib');
@files = grep { in_blib_script($_) } @files;
if (!@files) {
    @files = Perl::Critic::Utils::all_perl_files('lib');
}
plan tests => scalar @files;

# Run the actual tests.
for my $file (@files) {
    synopsis_ok($file);
}
