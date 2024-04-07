#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}
use strict;
use warnings;
our (@array, @r, $k, $v, $c);

plan tests => 65;

@array = qw(crunch zam bloop);

(@r) = each @array;
is (scalar @r, 2, "'each' on array returns index and value of next element");
is ($r[0], 0, "got expected index");
is ($r[1], 'crunch', "got expected value");
($k, $v) = each @array;
is ($k, 1, "got expected index of next element");
is ($v, 'zam', "got expected value of next element");
($k, $v) = each @array;
is ($k, 2, "got expected index of remaining element");
is ($v, 'bloop', "got expected value of remaining element");
(@r) = each @array;
is (scalar @r, 0,
    "no elements remaining to be iterated over in original array");

(@r) = each @array;
is (scalar @r, 2, "start second iteration over original array");
is ($r[0], 0, "got expected index");
is ($r[1], 'crunch', "got expected value");
($k) = each @array;
is ($k, 1, "got index when only index was assigned to variable");

my @lex_array = qw(PLOP SKLIZZORCH RATTLE);

(@r) = each @lex_array;
is (scalar @r, 2, "'each' on array returns index and value of next element");
is ($r[0], 0, "got expected index");
is ($r[1], 'PLOP', "got expected value");
($k, $v) = each @lex_array;
is ($k, 1, "got expected index of next element");
is ($v, 'SKLIZZORCH', "got expected value of next element");
($k) = each @lex_array;
is ($k, 2, "got expected index of remaining element");
(@r) = each @lex_array;
is (scalar @r, 0,
    "no elements remaining to be iterated over in original array");

my $ar = ['bacon'];

(@r) = each @$ar;
is (scalar @r, 2,
    "'each' on array inside reference returns index and value of next element");
is ($r[0], 0, "got expected index");
is ($r[1], 'bacon', "got expected value of array element inside reference");

(@r) = each @$ar;
is (scalar @r, 0,
    "no elements remaining to be iterated over in array inside reference");

is (each @$ar, 0, "scalar context 'each' on array returns expected index");
is (scalar each @$ar, undef,
    "no elements remaining to be iterated over; array reference case");

my @keys;
@keys = keys @array;
is ("@keys", "0 1 2",
    "'keys' on array in list context returns list of indices");

@keys = keys @lex_array;
is ("@keys", "0 1 2",
    "'keys' on another array in list context returns list of indices");

($k, $v) = each @array;
is ($k, 0, "got expected index");
is ($v, 'crunch', "got expected value");

@keys = keys @array;
is ("@keys", "0 1 2",
    "'keys' on array in list context returns list of indices");

($k, $v) = each @array;
is ($k, 0, "following 'keys', got expected index");
is ($v, 'crunch', "following 'keys', got expected value");



my @values;
@values = values @array;
is ("@values", "@array",
    "'values' on array returns list of values");

@values = values @lex_array;
is ("@values", "@lex_array",
    "'values' on another array returns list of values");

($k, $v) = each @array;
is ($k, 0, "following 'values', got expected index");
is ($v, 'crunch', "following 'values', got expected index");

@values = values @array;
is ("@values", "@array",
    "following 'values' and 'each', 'values' continues to return expected list of values");

($k, $v) = each @array;
is ($k, 0,
    "following 'values', 'each' and 'values', 'each' continues to return expected index");
is ($v, 'crunch',
    "following 'values', 'each' and 'values', 'each' continues to return expected value");

# reset
while (each @array) { }

# each(ARRAY) in the conditional loop
$c = 0;
while (($k, $v) = each @array) {
    is ($k, $c, "'each' on array in loop returns expected index '$c'");
    is ($v, $array[$k],
        "'each' on array in loop returns expected value '$array[$k]'");
    $c++;
}

# each(ARRAY) on scalar context in conditional loop
# should guarantee to be wrapped into defined() function.
# first return value will be 0 --> [#90888]
$c = 0;
$k = 0;
$v = 0;
while ($k = each @array) {
    is ($k, $v,
        "'each' on array in scalar context in loop returns expected index '$v'");
    $v++;
}

# each(ARRAY) in the conditional loop
$c = 0;
for (; ($k, $v) = each @array ;) {
    is ($k, $c,
        "'each' on array in list context in loop returns expected index '$c'");
    is ($v, $array[$k],
        "'each' on array in list context in loop returns expected value '$array[$k]'");
    $c++;
}

# each(ARRAY) on scalar context in conditional loop
# --> [#90888]
$c = 0;
$k = 0;
$v = 0;
for (; $k = each(@array) ;) {
    is ($k, $v,
        "'each' on array in scalar context in loop returns expected index '$v'");
    $v++;
}

# Reset the iterator when the array is cleared [RT #75596]
{
    my @a = 'a' .. 'c';
    my ($i, $v) = each @a;
    is ("$i-$v", '0-a', "got expected index and value");
    @a = 'A' .. 'C';
    ($i, $v) = each @a;
    is ("$i-$v", '0-A',
        "got expected new index and value after array gets new content");
}

# Check that the iterator is reset when localization ends
{
    @array = 'a' .. 'c';
    my ($i, $v) = each @array;
    is ("$i-$v", '0-a', "got expected index and value");
    {
        local @array = 'A' .. 'C';
        my ($i, $v) = each @array;
        is ("$i-$v", '0-A',
            "got expected new index and value after array is localized and gets new content");
        ($i, $v) = each @array;
        is ("$i-$v", '1-B',
            "got expected next index and value after array is localized and gets new content");
    }
    ($i, $v) = each @array;
    is ("$i-$v", '1-b',
         "got expected next index and value upon return to pre-localized array");
    # Explicit reset
    while (each @array) { }
}

my $a = 7;
*a = sub  { \@_ }->($a);
($a, $b) = each our @a;
is "$a $b", "0 7", 'each in list assignment';
$a = 7;
($a, $b) = (3, values @a);
is "$a $b", "3 7", 'values in list assignment';
