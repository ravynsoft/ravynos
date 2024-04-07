#!/usr/bin/perl -w
BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        chdir '../lib/parent';
        @INC = '..';
    }
}

use strict;
use Test::More tests => 2;
use lib 't/lib';

our $got_here;

my $res = eval q{
    package MyTest;

    use parent 'ReturnsFalse';

    $main::got_here++
};
my $error = $@;

is $got_here, undef, "The block did not run to its end.";
like $error, q{/^ReturnsFalse.pm did not return a true value at /}, "A module that returns a false value raises an error";
