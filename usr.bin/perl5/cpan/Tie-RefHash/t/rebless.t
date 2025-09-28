#!/usr/bin/perl -T -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;

use Tie::RefHash;

{
  package Moose;
  sub new { bless { }, shift };

  package Elk;
  our @ISA = "Moose";
}

$\ = "\n";
print "1..2";

my $obj = Moose->new;

tie my %hash, "Tie::RefHash";

$hash{$obj} = "magic";

print ( ( $hash{$obj} eq "magic" ) ? "" : "not ", "ok - keyed before rebless" );

bless $obj, "Elk";

print ( ( $hash{$obj} eq "magic" ) ? "" : "not ", "ok - still the same");
