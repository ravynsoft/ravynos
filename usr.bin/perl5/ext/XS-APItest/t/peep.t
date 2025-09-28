#!perl

use strict;
use warnings;
use Test::More tests => 6;

use XS::APItest;

my $record = XS::APItest::peep_record;
my $rrecord = XS::APItest::rpeep_record;

# our peep got called and remembered the string constant
XS::APItest::peep_enable;
eval q[my $foo = q/affe/];
XS::APItest::peep_disable;

is(scalar @{ $record }, 1);
is(scalar @{ $rrecord }, 1);
is($record->[0], 'affe');
is($rrecord->[0], 'affe');


# A deep-enough nesting of conditionals defeats the deferring mechanism
# and triggers recursion. Note that this test is sensitive to the details
# rpeep: the main thing it is testing is that rpeep is called more than
# peep, and that all branches are covered; the order of branch calling is
# less important.

my $code =  q[my ($a,$b); $a =];
$code .= qq{ \$b ? "foo$_" :} for (1..10);
$code .= qq{ "foo11" };
XS::APItest::peep_enable;
eval $code;
XS::APItest::peep_disable;

is_deeply($record,  [ "foo11" ]);
is_deeply($rrecord, [
    qw(foo1 foo2 foo3 foo4 foo5 foo6 foo10 foo9 foo8 foo7 foo11) ]);
