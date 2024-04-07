#!/usr/bin/perl
#
# Additional tests for Pod::Man heading generation.
#
# Copyright 2002, 2004, 2006, 2008-2009, 2012, 2015, 2018-2019, 2022
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

use Test::More tests => 11;
use Test::Podlators qw(read_test_data);

BEGIN {
    use_ok('Pod::Man');
}

# Loop through all the test data, generate output, and compare it to the
# desired output data.
my $testnum = 1;
while (defined(my $data = read_test_data(\*DATA, { options => 1 }))) {
    my $parser = Pod::Man->new(%{ $data->{options} });
    isa_ok($parser, 'Pod::Man', 'Parser object');

    # Run the parser, storing the output into a Perl variable.
    my $got;
    $parser->output_string(\$got);
    $parser->parse_string_document($data->{input});

    # Extract just the heading line.
    my ($heading) = ($got =~ m{^ ([.]TH [^\n]+ \n)}xms);

    # Compare the results.
    is($heading, $data->{output}, "Test $testnum");
    $testnum++;
}

# Below the marker are sets of options, the input data, and the corresponding
# expected .TH line from the man page.  The options and output are separated
# by lines containing only ###.

__DATA__

###
date 2009-01-17
release 1.0
###
=head1 NAME

test - Test man page
###
.TH STDIN 1 2009-01-17 1.0 "User Contributed Perl Documentation"
###

###
date 2009-01-17
name TEST
section 8
release 2.0-beta
###
=head1 NAME

test - Test man page
###
.TH TEST 8 2009-01-17 2.0-beta "User Contributed Perl Documentation"
###

###
date 2009-01-17
release 1.0
center Testing Documentation
###
=head1 NAME

test - Test man page
###
.TH STDIN 1 2009-01-17 1.0 "Testing Documentation"
###

###
date
release
center
###
=head1 NAME

test - Test man page
###
.TH STDIN 1 "" "" ""
###

###
date foo ""bar""
release "quoted"
section 4"
name "BAR
center Something
###
=head1 NAME

test - Test man page
###
.TH """BAR" "4""" "foo """"bar""""" """quoted""" Something
###
