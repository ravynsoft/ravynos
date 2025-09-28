#!/usr/bin/perl
#
# Check for obsolete strings in source files.
#
# Examine all source files in a distribution for obsolete strings and report
# on files that fail this check.  This catches various transitions I want to
# do globally in all my packages, like changing my personal URLs to https.
#
# The canonical version of this file is maintained in the rra-c-util package,
# which can be found at <https://www.eyrie.org/~eagle/software/rra-c-util/>.
#
# Copyright 2016, 2018-2021 Russ Allbery <eagle@eyrie.org>
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

# Bad patterns to search for.
my @BAD_REGEXES = (qr{ http:// \S+ [.]eyrie[.]org }xms);
my @BAD_STRINGS = qw(rra@stanford.edu RRA_MAINTAINER_TESTS);

# File or directory names to always skip.
my %SKIP = map { $_ => 1 } qw(
    .git .pc Changes _build blib changelog cover_db obsolete-strings.t
);

# Only run this test during automated testing, since failure doesn't indicate
# any user-noticable flaw in the package itself.
skip_unless_automated('Obsolete strings tests');

# Scan files for bad URL patterns.  This is meant to be run as the wanted
# function from File::Find.
sub check_file {
    my $filename = $_;

    # Ignore and prune any skipped files.  Ignore directories and binaries.
    if ($SKIP{$filename}) {
        $File::Find::prune = 1;
        return;
    }
    return if -d $filename;
    return if !-T $filename;

    # Scan the file.
    open(my $fh, '<', $filename) or BAIL_OUT("Cannot open $File::Find::name");
    while (defined(my $line = <$fh>)) {
        for my $regex (@BAD_REGEXES) {
            if ($line =~ $regex) {
                ok(0, "$File::Find::name contains $regex");
                close($fh) or BAIL_OUT("Cannot close $File::Find::name");
                return;
            }
        }
        for my $string (@BAD_STRINGS) {
            if (index($line, $string) != -1) {
                ok(0, "$File::Find::name contains $string");
                close($fh) or BAIL_OUT("Cannot close $File::Find::name");
                return;
            }
        }
    }
    close($fh) or BAIL_OUT("Cannot close $File::Find::name");
    ok(1, $File::Find::name);
    return;
}

# Use File::Find to scan all files from the top of the directory.
find(\&check_file, q{.});
done_testing();
