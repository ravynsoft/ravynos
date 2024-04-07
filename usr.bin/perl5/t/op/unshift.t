#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
}

plan(19);

@array = (1, 2, 3);

{
    no warnings 'syntax';
    $count3 = unshift (@array);
}
is(join(' ',@array), '1 2 3', 'unshift null');
cmp_ok($count3, '==', 3, 'unshift count == 3');


$count3_2 = unshift (@array, ());
is(join(' ',@array), '1 2 3', 'unshift null empty');
cmp_ok($count3_2, '==', 3, 'unshift count == 3 again');

$count4 = unshift (@array, 0);
is(join(' ',@array), '0 1 2 3', 'unshift singleton list');
cmp_ok($count4, '==', 4, 'unshift count == 4');

$count7 = unshift (@array, 3, 2, 1);
is(join(' ',@array), '3 2 1 0 1 2 3', 'unshift list');
cmp_ok($count7, '==', 7, 'unshift count == 7');

@list = (5, 4);
$count9 = unshift (@array, @list);
is(join(' ',@array), '5 4 3 2 1 0 1 2 3', 'unshift array');
cmp_ok($count9, '==', 9, 'unshift count == 9');


@list = (7);
@list2 = (6);
$count11 = unshift (@array, @list, @list2);
is(join(' ',@array), '7 6 5 4 3 2 1 0 1 2 3', 'unshift arrays');
cmp_ok($count11, '==', 11, 'unshift count == 11');

# ignoring counts
@alpha = ('y', 'z');

{
    no warnings 'syntax';
    unshift (@alpha);
}
is(join(' ',@alpha), 'y z', 'void unshift null');

unshift (@alpha, ());
is(join(' ',@alpha), 'y z', 'void unshift null empty');

unshift (@alpha, 'x');
is(join(' ',@alpha), 'x y z', 'void unshift singleton list');

unshift (@alpha, 'u', 'v', 'w');
is(join(' ',@alpha), 'u v w x y z', 'void unshift list');

@bet = ('s', 't');
unshift (@alpha, @bet);
is(join(' ',@alpha), 's t u v w x y z', 'void unshift array');

@bet = ('q');
@gimel = ('r');
unshift (@alpha, @bet, @gimel);
is(join(' ',@alpha), 'q r s t u v w x y z', 'void unshift arrays');

# See RT#131000
{
    local $@;
    my @readonly_array = 10..11;
    Internals::SvREADONLY(@readonly_array, 1);
    eval { unshift @readonly_array, () };
    like $@, qr/^Modification of a read-only value/,
        "croak when unshifting onto readonly array";
}
