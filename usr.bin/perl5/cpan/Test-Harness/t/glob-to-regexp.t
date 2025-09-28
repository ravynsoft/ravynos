#!/usr/bin/perl -w

use strict;
use warnings;

use Test::More;

require TAP::Parser::Scheduler;

my @tests;
while (<DATA>) {
    my ( $glob, $pattern, $name ) = /^(\S+)\t+(\S+)(?:\t+(.*))?$/;
    die "'$_'" unless $pattern;
    push @tests, [ $glob, $pattern, $name ];
}

plan tests => scalar @tests;

for (@tests) {
    my ( $glob, $pattern, $name ) = @$_;
    is( TAP::Parser::Scheduler->_glob_to_regexp($glob), $pattern,
        defined $name ? "$glob  -- $name" : $glob
    );
}
__DATA__
Pie			Pie
*.t			[^/]*\.t
**.t			.*?\.t
A?B			A[^/]B
*/*.t			[^/]*\/[^/]*\.t
A,B			A\,B				, outside {} not special
{A,B}			(?:A|B)
A{B}C			A(?:B)C
A{B,C}D			A(?:B|C)D
A{B,C,D}E{F,G,H}I,J	A(?:B|C|D)E(?:F|G|H)I\,J
{Perl,Rules}		(?:Perl|Rules)
A}B			A\}B				Bare } corner case
A{B,C}D}E		A(?:B|C)D\}E
},A{B,C}D},E		\}\,A(?:B|C)D\}\,E
{A{1,2},D{3,4}}		(?:A(?:1|2)|D(?:3|4))
{A,{B,C},D}		(?:A|(?:B|C)|D)
A{B,C\}D,E\,F}G		A(?:B|C\}D|E\,F)G
A\\B			A\\B
A(B)C			A\(B\)C
1{A(B)C,D|E}2		1(?:A\(B\)C|D\|E)2
