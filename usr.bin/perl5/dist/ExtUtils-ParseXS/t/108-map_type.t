#!/usr/bin/perl
use strict;
use warnings;
use Test::More qw(no_plan); # tests =>  7;
use ExtUtils::ParseXS::Utilities qw(
    map_type
);

#print "\t" . map_type($self->{ret_type}, 'RETVAL', $self->{hiertype}) . ";\n"
#print "\t" . map_type($var_type, $var_name, $self->{hiertype});
#print "\t" . map_type($var_type, undef, $self->{hiertype});

pass("Passed all tests in $0");
