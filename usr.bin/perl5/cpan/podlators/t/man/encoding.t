#!/usr/bin/perl
#
# Encoding tests for Pod::Man.
#
# This test uses a single test file with UTF-8 characters and escapes and
# processes it with different encoding configurations for Pod::Man, comparing
# it with pre-generated and hand-checked output files.
#
# The primary purpose of these test files is for portability testing on
# different operating systems, but this test ensures that they remain accurate
# for any changes to Pod::Man.  It doubles as a test that the preamble is
# emitted correctly.
#
# Copyright 2022 Russ Allbery <rra@cpan.org>
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
use Test::Podlators qw(slurp);

BEGIN {
    use_ok('Pod::Man');
}

# Force the timestamp on the input file since it will otherwise depend on the
# checkout.
local $ENV{SOURCE_DATE_EPOCH} = 1664146047;

# Get the path to the input and output files.
my $input = File::Spec->catfile('t', 'data', 'man', 'encoding.pod');
#<<<
my %output = (
    groff => File::Spec->catfile('t', 'data', 'man', 'encoding.groff'),
    roff  => File::Spec->catfile('t', 'data', 'man', 'encoding.roff'),
    utf8  => File::Spec->catfile('t', 'data', 'man', 'encoding.utf8'),
);
#>>>

# For each encoding, load the input, generate the output, and check that the
# output matches.
for my $encoding (sort(keys(%output))) {
    my $parser = Pod::Man->new(
        encoding => $encoding,
        center => 'podlators',
        release => 'testing',
    );
    my $got;
    $parser->output_string(\$got);
    $parser->parse_file($input);

    # Strip off the version line.
    $got =~ s{ ^ [^\n]+ Automatically [ ] generated [ ] by [^\n]+ \n }{}xms;

    # Check the output.  If it doesn't match, save the erroneous output in a
    # file for later inspection.
    my $expected = slurp($output{$encoding});
    if (!ok($got eq $expected, "encoding.pod output with $encoding")) {
        my $tmpdir = File::Spec->catdir('t', 'tmp');
        if (!-d $tmpdir) {
            mkdir($tmpdir, 0777);
        }
        my $outfile = File::Spec->catfile('t', 'tmp', "encoding$$.$encoding");
        open(my $output, '>', $outfile)
          or BAIL_OUT("cannot create $outfile for failed output: $!");
        print {$output} $got
          or BAIL_OUT("cannot write failed output to $outfile: $!");
        close($output)
          or BAIL_OUT("cannot write failed output to $outfile: $!");
        diag("Non-matching output left in $outfile");
    }
}
