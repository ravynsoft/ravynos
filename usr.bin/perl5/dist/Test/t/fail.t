# -*-perl-*-
use strict;
our $Expect;
use Test qw($TESTOUT $TESTERR $ntest ok skip plan); 
plan tests => 14;

open F, ">fails";
$TESTOUT = *F{IO};
$TESTERR = *F{IO};

my $r=0;
{
    # Shut up deprecated usage warning.
    local $^W = 0;
    $r |= skip(0,0);
}
$r |= ok(0);
$r |= ok(0,1);
$r |= ok(sub { 1+1 }, 3);
$r |= ok(sub { 1+1 }, sub { 2 * 0});

my @list = (0,0);
$r |= ok @list, 1, "\@list=".join(',',@list);
$r |= ok @list, 1, sub { "\@list=".join ',',@list };
$r |= ok 'segmentation fault', '/bongo/';

for (1..2) { $r |= ok(0); }

$r |= ok(1, undef);
$r |= ok(undef, 1);

ok($r); # (failure==success :-)

close F;
$TESTOUT = *STDOUT{IO};
$TESTERR = *STDERR{IO};
$ntest = 1;

open F, "fails";
my $O;
while (<F>) { $O .= $_; }
close F;
unlink "fails";

ok join(' ', map { m/(\d+)/; $1 } grep /^not ok/, split /\n+/, $O),
    join(' ', 1..13);

my @got = split /not ok \d+\n/, $O;
shift @got;

$Expect =~ s/\n+$//;
my @expect = split /\n\n/, $Expect;


sub commentless {
  my $in = $_[0];
  $in =~ s/^#[^\n]*\n//mg;
  $in =~ s/\n#[^\n]*$//mg;
  return $in;
}


for (my $x=0; $x < @got; $x++) {
    ok commentless($got[$x]), commentless($expect[$x]."\n");
}


BEGIN {
    $Expect = <<"EXPECT";
# Failed test 1 in $0 at line 15

# Failed test 2 in $0 at line 17

# Test 3 got: '0' ($0 at line 18)
#   Expected: '1'

# Test 4 got: '2' ($0 at line 19)
#   Expected: '3'

# Test 5 got: '2' ($0 at line 20)
#   Expected: '0'

# Test 6 got: '2' ($0 at line 23)
#   Expected: '1' (\@list=0,0)

# Test 7 got: '2' ($0 at line 24)
#   Expected: '1' (\@list=0,0)

# Test 8 got: 'segmentation fault' ($0 at line 25)
#   Expected: qr{bongo}

# Failed test 9 in $0 at line 27

# Failed test 10 in $0 at line 27 fail #2

# Failed test 11 in $0 at line 29

# Test 12 got: <UNDEF> ($0 at line 30)
#    Expected: '1'

# Failed test 13 in $0 at line 32
EXPECT

}
