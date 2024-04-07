#!./perl

$| = 1;

use strict;
use Test::More;

plan(skip_all => "skipped for VMS") if $^O eq 'VMS';
plan(tests => 12);

use Env  qw(@FOO);
use vars qw(@BAR);

sub array_equal
{
    my ($a, $b) = @_;
    return 0 unless scalar(@$a) == scalar(@$b);
    for my $i (0..scalar(@$a) - 1) {
	return 0 unless $a->[$i] eq $b->[$i];
    }
    return 1;
}

@FOO = qw(a B c);
@BAR = qw(a B c);
is_deeply(\@FOO, \@BAR, "Assignment");

$FOO[1] = 'b';
$BAR[1] = 'b';
is_deeply(\@FOO, \@BAR, "Storing");

$#FOO = 0;
$#BAR = 0;
is_deeply(\@FOO, \@BAR, "Truncation");

push @FOO, 'b', 'c';
push @BAR, 'b', 'c';
is_deeply(\@FOO, \@BAR, "Push");

pop @FOO;
pop @BAR;
is_deeply(\@FOO, \@BAR, "Pop");

shift @FOO;
shift @BAR;
is_deeply(\@FOO, \@BAR, "Shift");

push @FOO, 'c';
push @BAR, 'c';
is_deeply(\@FOO, \@BAR, "Push");

unshift @FOO, 'a';
unshift @BAR, 'a';
is_deeply(\@FOO, \@BAR, "Unshift");

@FOO = reverse @FOO;
@BAR = reverse @BAR;
is_deeply(\@FOO, \@BAR, "Reverse");

@FOO = sort @FOO;
@BAR = sort @BAR;
is_deeply(\@FOO, \@BAR, "Sort");

splice @FOO, 1, 1, 'B';
splice @BAR, 1, 1, 'B';
is_deeply(\@FOO, \@BAR, "Splice");

my $foo = $ENV{FOO};
() = splice @FOO, 0, 0;
is $ENV{FOO}, $foo, 'Splice in list context';
