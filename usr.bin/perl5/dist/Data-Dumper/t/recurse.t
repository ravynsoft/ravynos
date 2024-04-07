#!perl

# Test the Maxrecurse option

use strict;
use warnings;

use Test::More tests => 32;
use Data::Dumper;

SKIP: {
    skip "no XS available", 16
      if $Data::Dumper::Useperl;
    local $Data::Dumper::Useperl = 1;
    test_recursion();
}

test_recursion();

sub test_recursion {
    my $pp = $Data::Dumper::Useperl ? "pure perl" : "XS";
    $Data::Dumper::Purity = 1; # make sure this has no effect
    $Data::Dumper::Indent = 0;
    $Data::Dumper::Maxrecurse = 1;
    is(eval { Dumper([]) }, '$VAR1 = [];', "$pp: maxrecurse 1, []");
    is(eval { Dumper([[]]) }, undef, "$pp: maxrecurse 1, [[]]");
    ok($@, "exception thrown");
    is(eval { Dumper({}) }, '$VAR1 = {};', "$pp: maxrecurse 1, {}");
    is(eval { Dumper({ a => 1 }) }, q($VAR1 = {'a' => 1};),
       "$pp: maxrecurse 1, { a => 1 }");
    is(eval { Dumper({ a => {} }) }, undef, "$pp: maxrecurse 1, { a => {} }");
    ok($@, "exception thrown");
    is(eval { Dumper(\1) }, "\$VAR1 = \\1;", "$pp: maxrecurse 1, \\1");
    is(eval { Dumper(\\1) }, undef, "$pp: maxrecurse 1, \\1");
    ok($@, "exception thrown");
    $Data::Dumper::Maxrecurse = 3;
    is(eval { Dumper(\1) }, "\$VAR1 = \\1;", "$pp: maxrecurse 3, \\1");
    is(eval { Dumper(\(my $s = {})) }, "\$VAR1 = \\{};", "$pp: maxrecurse 3, \\{}");
    is(eval { Dumper(\(my $s = { a => [] })) }, "\$VAR1 = \\{'a' => []};",
       "$pp: maxrecurse 3, \\{ a => [] }");
    is(eval { Dumper(\(my $s = { a => [{}] })) }, undef,
       "$pp: maxrecurse 3, \\{ a => [{}] }");
    ok($@, "exception thrown");
    $Data::Dumper::Maxrecurse = 0;
    is(eval { Dumper([[[[[]]]]]) }, q($VAR1 = [[[[[]]]]];),
       "$pp: check Maxrecurse doesn't set limit to 0 recursion");
}
