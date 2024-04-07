#!/usr/bin/perl
#
# Test the parse_from_filehandle method.
#
# This backward compatibility interface is not provided by Pod::Simple, so
# Pod::Man and Pod::Text had to implement it directly.  Test to be sure it's
# working properly.
#
# Copyright 2006, 2009, 2012, 2014-2016, 2018-2019 Russ Allbery <rra@cpan.org>
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
use Test::More tests => 4;
use Test::Podlators qw(read_snippet slurp);

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

# Load the tests.
my $man_data_ref = read_snippet('man/cpp');
my $text_data_ref = read_snippet('text/cpp');

# Write the POD source to a temporary file for the input file handle.
my $infile = File::Spec->catfile('t', 'tmp', "tmp$$.pod");
open(my $input, '>', $infile) or BAIL_OUT("cannot create $infile: $!");
print {$input} $man_data_ref->{input}
  or BAIL_OUT("cannot write to $infile: $!");
close($input) or BAIL_OUT("cannot write to $infile: $!");

# Write the Pod::Man output to a file.
my $outfile = File::Spec->catfile('t', 'tmp', "tmp$$.man");
open($input, '<', $infile) or BAIL_OUT("cannot open $infile: $!");
open(my $output, '>', $outfile) or BAIL_OUT("cannot open $outfile: $!");
my $parser = Pod::Man->new;
$parser->parse_from_filehandle($input, $output);
close($input) or BAIL_OUT("cannot read from $infile: $!");
close($output) or BAIL_OUT("cannot write to $outfile: $!");

# Read the output back in and compare it.
my $got = slurp($outfile, 'man');
is($got, $man_data_ref->{output}, 'Pod::Man output');

# Clean up the temporary output file.
unlink($outfile);

# Now, do the same drill with Pod::Text.  Parse the input to a temporary file.
$outfile = File::Spec->catfile('t', 'tmp', "tmp$$.txt");
open($input, '<', $infile) or BAIL_OUT("cannot open $infile: $!");
open($output, '>', $outfile) or BAIL_OUT("cannot open $outfile: $!");
$parser = Pod::Text->new;
$parser->parse_from_filehandle($input, $output);
close($input) or BAIL_OUT("cannot read from $infile: $!");
close($output) or BAIL_OUT("cannot write to $outfile: $!");

# Read the output back in and compare it.  Pod::Text adds a trailing blank
# line that we need to strip out.
$got = slurp($outfile);
$got =~ s{ \n \s+ \z }{\n}xms;
is($got, $text_data_ref->{output}, 'Pod::Text output');

# Clean up temporary files.
unlink($infile, $outfile);
