#!./perl

# Test || in weird situations.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}


package Countdown;

sub TIESCALAR {
  my $class = shift;
  my $instance = shift || undef;
  return bless \$instance => $class;
}

sub FETCH {
  print "# FETCH!  ${$_[0]}\n";
  return ${$_[0]}--;
}


package main;

plan( tests => 14 );


my ($a, $b, $c);

$! = 1;
$a = $!;
my $a_str = sprintf "%s", $a;
my $a_num = sprintf "%d", $a;

$c = $a || $b;

is($c, $a_str, "comparison of string equality");
is($c+0, $a_num, "comparison of numeric equality");   # force numeric context.

$a =~ /./g or die "Match failed for some reason"; # Make $a magic

$c = $a || $b;

is($c, $a_str, "comparison of string equality");
is($c+0, $a_num, "comparison of numeric equality");   # force numeric context.

my $val = 3;

$c = $val || $b;
is($c, 3, "|| short-circuited as expected");

tie $a, 'Countdown', $val;

$c = $a;
is($c, 3,       'Single FETCH on tied scalar');

$c = $a;
is($c, 2,       '   $tied = $var');

$c = $a || $b;

{
    local $TODO = 'Double FETCH';
    is($c, 1,   '   $tied || $var');
}

$y = " ";
for (pos $x || pos $y) {
    eval { $_++ };
}
is(pos($y) || $@, 1, "|| propagates lvaluish context to its rhs");

$x = "  ";
pos $x = 1;
for (pos $x || pos $y) {
    eval { $_++ };
}
is(pos($x) || $@, 2, "|| propagates lvaluish context to its lhs");

for ($h{k} || $h{l}) {}
ok(!exists $h{k},
  "|| does not propagate lvaluish cx to a subscript on its lhs");
ok(!exists $h{l},
  "|| does not propagate lvaluish cx to a subscript on its rhs");

my $aa, $bb, $cc;
$bb = 1;

my $res = 0;
# Well, really testing OP_DOR I guess
unless ($aa || $bb // $cc) {
	$res = 1;
}
is($res, 0, "res is 0 after mixed OR/DOR");

$res = 0;
unless ($aa // $bb || $cc) {
	$res = 1;
}
is($res, 0, "res is 0 after mixed DOR/OR");
