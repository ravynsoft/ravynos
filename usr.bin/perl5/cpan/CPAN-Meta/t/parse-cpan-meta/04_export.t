#!/usr/bin/perl

delete $ENV{PERL_YAML_BACKEND};
delete $ENV{PERL_JSON_BACKEND};

# Testing of basic document structures

use strict;
BEGIN {
	$|  = 1;
	$^W = 1;
}

use Test::More tests => 4;
use Parse::CPAN::Meta;



ok not(defined &main::Load), 'Load is not exported';
ok not(defined &main::Dump), 'Dump is not exported';
ok not(defined &main::LoadFile), 'LoadFile is not exported';
ok not(defined &main::DumpFile), 'DumpFile is not exported';
