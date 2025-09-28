#!/usr/bin/perl
use strict;
use warnings;
use Test::More;
use ExtUtils::Typemaps;

my @tests = (
  [' *  ** ', '***'],
  [' *     ** ', '***'],
  [' *     ** foobar  *   ', '*** foobar *'],
  ['unsigned int', 'unsigned int'],
  ['std::vector<int>', 'std::vector<int>'],
  ['std::vector< unsigned int >', 'std::vector<unsigned int>'],
  ['std::vector< vector<unsigned int> >', 'std::vector<vector<unsigned int> >'],
  ['std::map< map <unsigned int, int>, int>', 'std::map<map<unsigned int, int>, int>'],
);

plan tests => scalar(@tests);

foreach my $test (@tests) {
  is(ExtUtils::Typemaps::tidy_type($test->[0]), $test->[1], "Tidying '$test->[0]'");
}

