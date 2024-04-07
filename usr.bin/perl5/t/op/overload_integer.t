#!./perl

BEGIN {
    chdir 't' if -d 't';
    push @INC, '../lib';
    require './test.pl';
}

use strict;
use warnings;

plan tests => 2;

package Foo;

use overload; 

sub import
{
    overload::constant 'integer' => sub { return shift };
}

package main;

BEGIN { $INC{'Foo.pm'} = "/lib/Foo.pm" }

use Foo;

my $result = eval "5+6";
my $error = $@;
$result //= '';

is ($error, '', "No exception was thrown with an overload::constant 'integer' inside an eval.");
is ($result, 11, "Correct solution");

