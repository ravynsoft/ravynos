#!/usr/bin/perl
#
# Basic tests for podlators.
#
# This test case uses a single sample file and runs it through all available
# formatting modules, comparing the results to known-good output that's
# included with the package.  This provides a general sanity check that the
# modules are working properly.
#
# New regression tests and special cases should probably not be added to the
# sample input file, since updating all the output files is painful.  Instead,
# the machinery to run small POD snippets through the specific formatter being
# tested should probably be used instead.
#
# Copyright 2001-2002, 2004, 2006, 2009, 2012, 2014-2015, 2018-2019, 2022
#     Russ Allbery <rra@cpan.org>
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
use Test::More tests => 15;
use Test::Podlators qw(slurp);

# Check that all the modules can be loaded.
BEGIN {
    use_ok('Pod::Man');
    use_ok('Pod::Text');
    use_ok('Pod::Text::Color');
    use_ok('Pod::Text::Overstrike');
    use_ok('Pod::Text::Termcap');
}

# Flush output, since otherwise our diag messages come after other tests.
local $| = 1;

# Hard-code configuration for Term::Cap to get predictable results.
#<<<
local $ENV{COLUMNS}  = 80;
local $ENV{TERM}     = 'xterm';
local $ENV{TERMPATH} = File::Spec->catfile('t', 'data', 'termcap');
local $ENV{TERMCAP}  = 'xterm:co=#80:do=^J:md=\\E[1m:us=\\E[4m:me=\\E[m';
#>>>

# Find the source of the test file.
my $input = File::Spec->catfile('t', 'data', 'basic.pod');

# Map of translators to the file containing the formatted output to compare
# against.
#<<<
my %output = (
    'Pod::Man'              => File::Spec->catfile('t', 'data', 'basic.man'),
    'Pod::Text'             => File::Spec->catfile('t', 'data', 'basic.txt'),
    'Pod::Text::Color'      => File::Spec->catfile('t', 'data', 'basic.clr'),
    'Pod::Text::Overstrike' => File::Spec->catfile('t', 'data', 'basic.ovr'),
    'Pod::Text::Termcap'    => File::Spec->catfile('t', 'data', 'basic.cap'),
);
#>>>

# Walk through teach of the modules and format the sample file, checking to
# ensure the results match the pre-generated file.
for my $module (sort keys %output) {
    my $parser = $module->new();
    isa_ok($parser, $module, 'parser object');

    # Run the formatting module.  Store the output into a Perl variable
    # instead of a file.
    my $got;
    $parser->output_string(\$got);
    $parser->parse_file($input);

    # If the test module is Pod::Man, strip off the header.  This test does
    # not attempt to compare it, since it contains version numbers that
    # change.
    if ($module eq 'Pod::Man') {
        $got =~ s{ \A .* \n [.]nh \n }{}xms;
    }

    # Try to convert on EBCDIC boxes so that the test still works.
    if (ord('A') == 193 && $module eq 'Pod::Text::Termcap') {
        $got =~ tr{\033}{\047};
    }

    # Check the output.  If it doesn't match, save the erroneous output in a
    # file for later inspection.
    my $expected = slurp($output{$module});
    if (!ok($got eq $expected, "$module output is correct")) {
        my ($suffix) = ($output{$module} =~ m{ [.] ([^.]+) \z }xms);
        my $tmpdir = File::Spec->catdir('t', 'tmp');
        if (!-d $tmpdir) {
            mkdir($tmpdir, 0777);
        }
        my $outfile = File::Spec->catfile('t', 'tmp', "out$$.$suffix");
        open(my $output, '>', $outfile)
          or BAIL_OUT("cannot create $outfile for failed output: $!");
        print {$output} $got
          or BAIL_OUT("cannot write failed output to $outfile: $!");
        close($output)
          or BAIL_OUT("cannot write failed output to $outfile: $!");
        diag("Non-matching output left in $outfile");
    }
}
