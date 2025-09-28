#!/usr/bin/perl
#
# Tests for the automatic determination of the manual page title if not
# specified via options to pod2man or the Pod::Man constructor.
#
# Copyright 2015-2016, 2018-2019 Russ Allbery <rra@cpan.org>
#
# This program is free software; you may redistribute it and/or modify it
# under the same terms as Perl itself.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.008;
use strict;
use warnings;

use File::Spec;
use IO::File;
use Test::More tests => 5;

BEGIN {
    use_ok('Pod::Man');
}

# Create a parser and set it up with an input source.  There isn't a way to do
# this in Pod::Simple without actually parsing the document, so send the
# output to a string that we'll ignore.
my $path = File::Spec->catfile('t', 'data', 'basic.pod');
my $handle = IO::File->new($path, 'r');
my $parser = Pod::Man->new(errors => 'pod');
my $output;
$parser->output_string(\$output);
$parser->parse_file($handle);

# Check the results of devise_title for this.  We should get back STDIN and
# not report an error.
my ($name, $section) = $parser->devise_title;
is($name, 'STDIN', 'devise_title uses STDIN for file handle input');
ok(!$parser->errors_seen, '...and no errors were seen');

# Now check handling of a simple file name with no parent directory, which
# simulates a POD file at the top of a distribution.  In podlators 4.06, this
# produced an erroneous warning.  (That wouldn't actually fail this test, but
# I'd see it during development, which is good enough and doesn't require
# anything too complicated.)
$parser->source_filename('Foo.pm');
($name, $section) = $parser->devise_title;
is($name, 'Foo', 'devise_title with a simple module name');
is($section, 3, '...and the correct section');
