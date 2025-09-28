#!/usr/bin/perl

use Test::More tests => 3;
use Test::Builder::Tester;

is(line_num(),6,"normal line num");
is(line_num(-1),6,"line number minus one");
is(line_num(+2),10,"line number plus two");
