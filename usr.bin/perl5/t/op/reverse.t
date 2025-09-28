#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 25;

is(reverse("abc"), "cba", 'simple reverse');

$_ = "foobar";
is(reverse(), "raboof", 'reverse of the default variable');

{
    my @a = ("foo", "bar");
    my @b = reverse @a;

    is($b[0], $a[1], 'array reversal moved second element to first');
    is($b[1], $a[0], 'array reversal moved first element to second');
}

{
    my @a = (1, 2, 3, 4);
    @a = reverse @a;
    is("@a", "4 3 2 1", 'four element array reversed');

    delete $a[1];
    @a = reverse @a;
    ok(!exists $a[2], 'array reversed with deleted second element');
    is($a[0] . $a[1] . $a[3], '124', 'remaining elements ok after delete and reverse');

    @a = (5, 6, 7, 8, 9);
    @a = reverse @a;
    is("@a", "9 8 7 6 5", 'five element array reversed');

    delete $a[3];
    @a = reverse @a;
    ok(!exists $a[1], 'five element array reversed with deleted fourth element');
    is($a[0] . $a[2] . $a[3] . $a[4], '5789', 'remaining elements ok after delete and reverse');

    delete $a[2];
    @a = reverse @a;
    ok(!exists $a[2] && !exists $a[3], 'test position of two deleted elements after reversal');
    is($a[0] . $a[1] . $a[4], '985', 'check value of remaining elements');

    my @empty;
    @empty = reverse @empty;
    is("@empty", "", 'reversed empty array is still empty');
}

use Tie::Array;

{
    tie my @a, 'Tie::StdArray';

    @a = (1, 2, 3, 4);
    @a = reverse @a;
    is("@a", "4 3 2 1", 'tie array reversal');

    delete $a[1];
    @a = reverse @a;
    ok(!exists $a[2], 'deleted element position ok after reversal of tie array');
    is($a[0] . $a[1] . $a[3], '124', 'remaining elements ok after delete and reversal for tie array');

    @a = (5, 6, 7, 8, 9);
    @a = reverse @a;
    is("@a", "9 8 7 6 5", 'five element tie array reversal');

    delete $a[3];
    @a = reverse @a;
    ok(!exists $a[1], 'deleted element position ok after tie array reversal');
    is($a[0] . $a[2] . $a[3] . $a[4], '5789', 'remaining elements ok after tie array delete and reversal');

    delete $a[2];
    @a = reverse @a;
    ok(!exists $a[2] && !exists $a[3], 'two deleted element positions ok after tie array reversal');
    is($a[0] . $a[1] . $a[4], '985', 'remaining elements ok after two deletes and reversals');

    tie my @empty, "Tie::StdArray";
    @empty = reverse @empty;
    is(scalar(@empty), 0, 'reversed tie array still empty after reversal');
}

{
    # Unicode.

    my $a = "\x{263A}\x{263A}x\x{263A}y\x{263A}";
    my $b = scalar reverse($a);
    my $c = scalar reverse($b);
    is($a, $c, 'Unicode string double reversal matches original');
}

{
    # https://github.com/Perl/perl5/issues/17737
    # utf8 length magic handling
    # threw an assertion failure with -DDEBUGGING
    my @x;
    push @x, length reverse for split "-", "\x{100}--0";
    is($x[1], 0, "check set magic being called to clear length magic");
}

# [perl #132544] stack pointer used to go wild when nullary reverse
# required extending the stack
for(0..1000){()=(0..$_,scalar reverse )}
pass "extending the stack without crashing";
