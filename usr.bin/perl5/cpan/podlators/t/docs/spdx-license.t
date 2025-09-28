#!/usr/bin/perl
#
# Check source files for SPDX-License-Identifier fields.
#
# Examine all source files in a distribution to check that they contain an
# SPDX-License-Identifier field.  This does not check the syntax or whether
# the identifiers are valid.
#
# The canonical version of this file is maintained in the rra-c-util package,
# which can be found at <https://www.eyrie.org/~eagle/software/rra-c-util/>.
#
# Copyright 2018-2021 Russ Allbery <eagle@eyrie.org>
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

use Test::RRA qw(skip_unless_automated);

use File::Find qw(find);
use Test::More;

# File name (the file without any directory component) and path patterns to
# skip for this check.
## no critic (RegularExpressions::ProhibitFixedStringMatches)
#<<<
my @IGNORE = (
    qr{ \A Build ( [.] (?!PL) .* )? \z }ixms,  # Generated file from Build.PL
    qr{ \A LICENSE \z }xms,                 # Generated file, no license itself
    qr{ \A (Changes|NEWS|THANKS) \z }xms,   # Package license should be fine
    qr{ \A TODO \z }xms,                    # Package license should be fine
    qr{ \A MANIFEST ( [.] .* )? \z }xms,    # Package license should be fine
    qr{ \A Makefile \z }xms,                # Generated file, no license itself
    qr{ \A (MY)? META [.] .* }xms,          # Generated file, no license itself
    qr{ [.] output \z }xms,                 # Test data
    qr{ pod2htm . [.] tmp \z }xms,          # Windows pod2html output
    qr{ ~ \z }xms,                          # Backup files
);
my @IGNORE_PATHS = (
    qr{ \A [.] / [.] git/ }xms,             # Version control files
    qr{ \A [.] / [.] pc/ }xms,              # quilt metadata files
    qr{ \A [.] /_build/ }xms,               # Module::Build metadata
    qr{ \A [.] /blib/ }xms,                 # Perl build system artifacts
    qr{ \A [.] /cover_db/ }xms,             # Artifacts from coverage testing
    qr{ \A [.] /debian/ }xms,               # Found in debian/* branches
    qr{ \A [.] /docs/metadata/ }xms,        # Package license should be fine
    qr{ \A [.] /README ( [.] .* )? \z }xms, # Package license should be fine
    qr{ \A [.] /share/ }xms,                # Package license should be fine
    qr{ \A [.] /t/data/generate/ }xms,      # Test metadata
    qr{ \A [.] /t/data/spin/ }xms,          # Test metadata
    qr{ \A [.] /t/data/update/ }xms,        # Test output
    qr{ \A [.] /t/data .* [.] json \z }xms, # Test metadata
);
#>>>
## use critic

# Only run this test during automated testing, since failure doesn't indicate
# any user-noticable flaw in the package itself.
skip_unless_automated('SPDX identifier tests');

# Check a single file for an occurrence of the string.
#
# $path - Path to the file
#
# Returns: undef
sub check_file {
    my $filename = $_;
    my $path = $File::Find::name;

    # Ignore files in the whitelist and binary files.
    for my $pattern (@IGNORE) {
        return if $filename =~ $pattern;
    }
    for my $pattern (@IGNORE_PATHS) {
        if ($path =~ $pattern) {
            $File::Find::prune = 1;
            return;
        }
    }
    return if -d $filename;
    return if !-T $filename;

    # Scan the file.
    my ($saw_legacy_notice, $saw_spdx, $skip_spdx);
    open(my $file, '<', $filename) or BAIL_OUT("Cannot open $path");
    while (defined(my $line = <$file>)) {
        if ($line =~ m{ \b See \s+ LICENSE \s+ for \s+ licensing }xms) {
            $saw_legacy_notice = 1;
        }
        if ($line =~ m{ \b SPDX-License-Identifier: \s+ \S+ }xms) {
            $saw_spdx = 1;
            last;
        }
        if ($line =~ m{ no \s SPDX-License-Identifier \s registered }xms) {
            $skip_spdx = 1;
            last;
        }
    }
    close($file) or BAIL_OUT("Cannot close $path");

    # If there is a legacy license notice, report a failure regardless of file
    # size.  Otherwise, skip files under 1KB.  They can be rolled up into the
    # overall project license and the license notice may be a substantial
    # portion of the file size.
    if ($saw_legacy_notice) {
        ok(!$saw_legacy_notice, "$path has legacy license notice");
    } else {
        ok($saw_spdx || $skip_spdx || -s $filename < 1024, $path);
    }
    return;
}

# Use File::Find to scan all files from the top of the directory.
find(\&check_file, q{.});
done_testing();
