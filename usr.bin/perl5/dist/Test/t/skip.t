# -*-perl-*-
use strict;
use Test qw($TESTOUT $TESTERR $ntest plan ok skip); 
plan tests => 6;

open F, ">skips" or die "open skips: $!";
$TESTOUT = *F{IO};
$TESTERR = *F{IO};

skip(1, 0);  #should skip

my $skipped=1;
skip('hop', sub { $skipped = 0 });
skip(sub {'jump'}, sub { $skipped = 0 });
skip('skipping stones is more fun', sub { $skipped = 0 });

close F;

$TESTOUT = *STDOUT{IO};
$TESTERR = *STDERR{IO};
$ntest = 1;
open F, "skips" or die "open skips: $!";

ok $skipped, 1, 'not skipped?';

my @T = <F>;
chop @T;
my @expect = split /\n+/, join('',<DATA>);
ok @T, 4;
for (my $x=0; $x < @T; $x++) {
    ok $T[$x], $expect[$x];
}

END { close F; unlink "skips" }

__DATA__
ok 1 # skip

ok 2 # skip hop

ok 3 # skip jump

ok 4 # skip skipping stones is more fun
