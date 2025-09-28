#!/usr/bin/perl
#
# Test Perl code for strict, warnings, and syntax.
#
# The canonical version of this file is maintained in the rra-c-util package,
# which can be found at <https://www.eyrie.org/~eagle/software/rra-c-util/>.
#
# Written by Russ Allbery <eagle@eyrie.org>
# Copyright 2016, 2018-2021 Russ Allbery <eagle@eyrie.org>
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

# Skip for normal user installs since this doesn't affect functionality.
skip_unless_automated('Strictness tests');

# Load prerequisite modules.  At least 0.25 is needed to recognize that having
# use 5.012 or later automatically implies use strict.
use_prereq('Test::Strict', '0.25');

# Directories to exclude from checks.
my %EXCLUDE = map { $_ => 1 } qw(.git blib);

# Determine whether we want to check the given file or top-level directory.
# Assume that the only interesting files at the top level are directories or
# files ending in *.PL.
#
# $file - Name of the file or directory
#
# Returns: 1 if it should be checked, undef otherwise.
sub should_check {
    my ($file) = @_;
    return if $EXCLUDE{$file};
    return 1 if -d $file;
    return 1 if $file =~ m{ [.] PL \z }xms;
    return;
}

# Test::Strict (as of 0.47) doesn't have a way of excluding whole directories
# from all_perl_files_ok and doesn't exclude .git, which results in false
# positives if there are Perl files unpacked under .git (which is often the
# case when using dgit).  We therefore can't just point it at the root of the
# module distribution and instead have to manually construct a list of
# interesting files.
opendir(my $rootdir, File::Spec->curdir)
  or die "$0: cannot open current directory: $!\n";
my @files = File::Spec->no_upwards(readdir($rootdir));
closedir($rootdir) or die "$0: cannot close current directory: $!\n";
my @to_check = grep { should_check($_) } @files;

# Test the files and top-level directories we found, including checking for
# use warnings.
$Test::Strict::TEST_WARNINGS = 1;
all_perl_files_ok(@to_check);

# Hack to suppress "used only once" warnings.
END { $Test::Strict::TEST_WARNINGS = 0 }
