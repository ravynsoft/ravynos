#!/usr/bin/perl
#
# Tests for backward compatibility with Pod::Parser.
#
# Copyright 2006, 2008-2009, 2012, 2015, 2018-2019 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use lib 't/lib';

use File::Spec;
use Test::More tests => 7;
use Test::Podlators qw(slurp);

# Ensure the modules load properly.
BEGIN {
    use_ok('Pod::Man');
    use_ok('Pod::Text');
}

# Create a temporary directory to use for output, but don't fail if it already
# exists.  If we failed to create it, we'll fail later on.  We unfortunately
# have to create files on disk to easily create file handles for testing.
my $tmpdir = File::Spec->catdir('t', 'tmp');
if (!-d $tmpdir) {
    mkdir($tmpdir, 0777);
}

# Create some test POD to use to test the -cutting option.
my $infile = File::Spec->catfile('t', 'tmp', "tmp$$.pod");
open(my $input, '>', $infile) or BAIL_OUT("cannot create $infile: $!");
print {$input} "Some random B<text>.\n"
  or BAIL_OUT("cannot write to $infile: $!");
close($input) or BAIL_OUT("cannot write to $infile: $!");

# Test the -cutting option with Pod::Man.
my $parser = Pod::Man->new;
isa_ok($parser, 'Pod::Man', 'Pod::Man parser object');
my $outfile = File::Spec->catfile('t', 'tmp', "tmp$$.man");
open(my $output, '>', $outfile) or BAIL_OUT("cannot open $outfile: $!");
$parser->parse_from_file({ -cutting => 0 }, $infile, $output);
close($output) or BAIL_OUT("cannot write to $outfile: $!");
my $got = slurp($outfile, 'man');
is($got, "Some random \\fBtext\\fR.\n", 'Pod::Man -cutting output');
unlink($outfile);

# Likewise for Pod::Text.
$parser = Pod::Text->new;
isa_ok($parser, 'Pod::Text', 'Pod::Text parser object');
$outfile = File::Spec->catfile('t', 'tmp', "tmp$$.txt");
open($output, '>', $outfile) or BAIL_OUT("cannot open $outfile: $!");
$parser->parse_from_file({ -cutting => 0 }, $infile, $output);
close($output) or BAIL_OUT("cannot write to $outfile: $!");
$got = slurp($outfile);
is($got, "    Some random text.\n\n", 'Pod::Text -cutting output');
unlink($outfile);

# Rewrite the input file to be fully valid POD since we won't use -cutting.
unlink($infile);
open($input, '>', $infile) or BAIL_OUT("cannot create $infile: $!");
print {$input} "=pod\n\nSome random B<text>.\n"
  or BAIL_OUT("cannot write to $infile: $!");
close($input) or BAIL_OUT("cannot write to $infile: $!");

# Now test the pod2text function with a single output.  This will send the
# results to standard output, so we need to redirect that to a file.
open($output, '>', $outfile) or BAIL_OUT("cannot open $outfile: $!");
open(my $save_stdout, '>&', STDOUT) or BAIL_OUT("cannot dup stdout: $!");
open(STDOUT, '>&', $output) or BAIL_OUT("cannot redirect stdout: $!");
pod2text($infile);
close($output) or BAIL_OUT("cannot write to $outfile: $!");
open(STDOUT, '>&', $save_stdout) or BAIL_OUT("cannot fix stdout: $!");
close($save_stdout) or BAIL_OUT("cannot close saved stdout: $!");
$got = slurp($outfile);
is($got, "    Some random text.\n\n", 'Pod::Text pod2text function');

# Clean up.
unlink($infile, $outfile);
