#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

my $truevar  = (5 == 5);
my $falsevar = (5 == 6);

cmp_ok($truevar, '==', 1);
cmp_ok($truevar, 'eq', "1");

cmp_ok($falsevar, '==', 0);
cmp_ok($falsevar, 'eq', "");

{
    # Check that boolean COW string buffer is safe to copy into new SVs and
    # doesn't get corrupted by inplace mutations
    my $x = $truevar;
    $x =~ s/1/t/;

    cmp_ok($x, 'eq', "t");
    cmp_ok($truevar, 'eq', "1");

    my $y = $truevar;
    substr($y, 0, 1, "T");

    cmp_ok($y, 'eq', "T");
    cmp_ok($truevar, 'eq', "1");
}

done_testing();
