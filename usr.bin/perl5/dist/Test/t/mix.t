# -*-perl-*-
use strict;
use Test qw(:DEFAULT $TESTOUT $TESTERR $ntest);

### This test is crafted in such a way as to prevent Test::Harness from
### seeing the todo tests, otherwise you get people sending in bug reports
### about Test.pm having "UNEXPECTEDLY SUCCEEDED" tests.

open F, ">", "mix";
$TESTOUT = *F{IO};
$TESTERR = *F{IO};

plan tests => 4, todo => [2,3];

# line 15
ok(sub { 
       my $r = 0;
       for (my $x=0; $x < 10; $x++) {
	   $r += $x*($r+1);
       }
       $r
   }, 3628799);

ok(0);
ok(1);

skip(1,0);

close F;
$TESTOUT = *STDOUT{IO};
$TESTERR = *STDERR{IO};
$ntest = 1;

open F, "<", "mix";
my $out = join '', <F>;
close F;
unlink "mix";

my $expect = <<"EXPECT";
1..4 todo 2 3;
ok 1
not ok 2
# Failed test 2 in $0 at line 23 *TODO*
ok 3 # ($0 at line 24 TODO?!)
ok 4 # skip
EXPECT


sub commentless {
  my $in = $_[0];
  $in =~ s/^#[^\n]*\n//mg;
  $in =~ s/\n#[^\n]*$//mg;
  return $in;
}


print "1..1\n";
ok( commentless($out), commentless($expect) );
