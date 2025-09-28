#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
}

plan(14);

@a = (1,2,3,4,5,6,7,8,9,10,11,12);
@b = ();
while ($_ = shift(@a)) {
    if ($x = /4/../8/) { $z = $x; push @b, $x + 0; }
    $y .= /1/../2/;
}
is(join("*", @b), "1*2*3*4*5");

is($z, '5E0');

is($y, '12E0123E0');

@a = ('a','b','c','d','e','f','g');

{
local $.;

open(of,'harness') or die "Can't open harness: $!";
while (<of>) {
    (3 .. 5) && ($foo .= $_);
}
$x = ($foo =~ y/\n/\n/);

is($x, 3);

$x = 3.14;
ok(($x...$x) eq "1");

{
    # coredump reported in bug 20001018.008 (#4474)
    readline(UNKNOWN);
    $. = 1;
    $x = 1..10;
    ok(1);
}

}

ok(!defined $.);

use warnings;
my $warn='';
$SIG{__WARN__} = sub { $warn .= join '', @_ };

ok(scalar(0..2));

like($warn, qr/uninitialized/);
$warn = '';

$x = "foo".."bar";

ok((() = ($warn =~ /isn't numeric/g)) == 2);
$warn = '';

$. = 15;
ok(scalar(15..0));

push @_, \scalar(0..0) for 1,2;
isnt $_[0], $_[1], '\scalar($a..$b) gives a different scalar each time';

# This evil little example from ticket #122829 abused the fact that each
# recursion level maintained its own flip-flip state.  The following com-
# ment describes how it *used* to work.

# This routine maintains multiple flip-flop states, each with its own
# numeric ID, starting from 1.  Pass the ID as the argument.
sub f {
    my $depth = shift() - 1;
    return f($depth) if $depth;
    return /3/../5/;
}
{
    my $accumulator;
    for(1..20) {
        if (f(1)) {
            my $outer = $_;
            for(1..10){
                $accumulator .= "$outer $_\n" if f(2);
            }
        }
    }
    is $accumulator, <<EOT, 'recursion shares state';
3 1
3 2
3 3
3 4
3 5
13 1
13 2
13 3
13 4
13 5
EOT
}

# Void context gives parenthesized lhs scalar context
no warnings 'void';
sub c { $context = qw[ void scalar list ][wantarray + defined wantarray] }
(c())x34;
is $context, 'scalar', '(...)x... in void context';
