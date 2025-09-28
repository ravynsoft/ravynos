#!./perl

print "1..14\n";

# Tests bug #22977.  Test case from Dave Mitchell.
sub f ($);
sub f ($) {
my $test = $_[0];
write;
format STDOUT =
ok @<<<<<<<
$test
.
}

f(1);
f(2);

# A bug caused by the fix for #22977/50528
sub foo {
  sub bar {
    # Fill the pad with alphabet soup, to give the closed-over variable a
    # high padoffset (more likely to trigger the bug and crash).
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    my $x;
    format STDOUT2 =
@<<<<<<
"ok 3".$x # $x is not available, but this should not crash
.
  }
}
*STDOUT = *STDOUT2{FORMAT};
undef *bar;
write;

# A regression introduced in 5.10; format cloning would close over the
# variables in the currently-running sub (the main CV in this test) if the
# outer sub were an inactive closure.
sub baz {
  my $a;
  sub {
    $a;
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t)}
    my $x;
    format STDOUT3 =
@<<<<<<<<<<<<<<<<<<<<<<<<<
defined $x ? "not ok 4 - $x" : "ok 4"
.
  }
}
*STDOUT = *STDOUT3{FORMAT};
{
  local $^W = 1;
  my $w;
  local $SIG{__WARN__} = sub { $w = shift };
  write;
  print "not " unless $w =~ /^Variable "\$x" is not available at/;
  print "ok 5 - closure var not available when outer sub is inactive\n";
}

# Formats inside closures should close over the topmost clone of the outer
# sub on the call stack.
# Tests will be out of sequence if the wrong sub is used.
sub make_closure {
  my $arg = shift;
  sub {
    shift == 0 and &$next(1), return;
    my $x = "ok $arg";
    format STDOUT4 =
@<<<<<<<
$x
.
    sub { write }->(); # separate sub, so as not to rely on it being the
  }                    # currently-running sub
}
*STDOUT = *STDOUT4{FORMAT};
$clo1 = make_closure 6;
$clo2 = make_closure 7;
$next = $clo1;
&$clo2(0);
$next = $clo2;
&$clo1(0);

# Cloning a format whose outside has been undefined
sub x {
    {my ($a,$b,$c,$d,$e,$f,$g,$h,$i,$j,$k,$l,$m,$n,$o,$p,$q,$r,$s,$t,$u)}
    my $z;
    format STDOUT6 =
@<<<<<<<<<<<<<<<<<<<<<<<<<
defined $z ? "not ok 8 - $z" : "ok 8"
.
}
undef &x;
*STDOUT = *STDOUT6{FORMAT};
{
  local $^W = 1;
  my $w;
  local $SIG{__WARN__} = sub { $w = shift };
  write;
  print "not " unless $w =~ /^Variable "\$z" is not available at/;
  print "ok 9 - closure var not available when outer sub is undefined\n";
}

format STDOUT7 =
@<<<<<<<<<<<<<<<<<<<<<<<<<<<
do { my $x = "ok 10 - closure inside format"; sub { $x }->() }
.
*STDOUT = *STDOUT7{FORMAT};
write;

$testn = 12;
format STDOUT8 =
@<<<< - recursive formats
do { my $t = "ok " . $testn--; write if $t =~ 12; $t}
.
*STDOUT = *STDOUT8{FORMAT};
write;

sub _13 {
    my $x;
format STDOUT13 =
@* - formats closing over redefined subs (got @*)
ref \$x eq 'SCALAR' ? "ok 13" : "not ok 13", ref \$x;
.
}
undef &_13;
eval 'sub _13 { my @x; write }';
*STDOUT = *STDOUT13{FORMAT};
_13();

# This is a variation of bug #22977, which crashes or fails an assertion
# up to 5.16.
# Keep this test last if you want test numbers to be sane.
BEGIN { \&END }
END {
  my $test = "ok 14";
  *STDOUT = *STDOUT5{FORMAT};
  write;
  format STDOUT5 =
@<<<<<<<
$test
.
}
