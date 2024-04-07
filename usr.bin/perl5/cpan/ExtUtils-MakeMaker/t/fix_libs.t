#!/usr/bin/perl -w

# Unit test the code which fixes up $self->{LIBS}

BEGIN {
    chdir 't' if -d 't';
}

use strict;
use warnings;
use lib './lib';
use Test::More 'no_plan';

use ExtUtils::MakeMaker;

my @tests = (
        # arg           # want
    [   undef,          ['']    ],
    [   "foo",          ['foo'] ],
    [   [],             ['']    ],
    [   ["foo"],        ['foo'] ],
    [   [1, 2, 3],      [1, 2, 3] ],
    [   [0],            [0]     ],
    [   [''],           ['']    ],
    [   "  ",           ['  ']  ],
);

for my $test (@tests) {
    my($arg, $want) = @$test;

    my $display = defined $arg ? $arg : "undef";
    is_deeply( MM->_fix_libs($arg), $want, "fix_libs($display)" );
}
