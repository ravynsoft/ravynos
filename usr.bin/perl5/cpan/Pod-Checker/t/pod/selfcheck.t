#!/usr/bin/perl
use File::Basename;
use File::Spec;
use strict;
my $THISDIR;
BEGIN {
   $THISDIR = dirname $0;
   unshift @INC, $THISDIR;
   require "testpchk.pl";
   import TestPodChecker qw(testpodcheck);
}

# test that our POD is correct!
my $path = File::Spec->catfile($THISDIR,(File::Spec->updir()) x 2, 'lib', 'Pod', '*.pm');
print "THISDIR=$THISDIR PATH=$path\n";
my @pods = glob($path);
print "PODS=@pods\n";

print "1..",scalar(@pods),"\n";

my $errs = 0;
my $testnum = 1;
foreach my $pod (@pods) {
  my $out = File::Spec->catfile($THISDIR, basename($pod));
  $out =~ s{\.pm}{.OUT};
  my %options = ( -Out => $out );
  my $failmsg = testpodcheck(-In => $pod, -Out => $out, -Cmp => "$THISDIR/empty.xr");
  if($failmsg) {
    if(open(IN, "<$out")) {
      while(<IN>) {
        warn "podchecker: $_";
      }
      close(IN);
    } else {
      warn "Error: Cannot read output file $out: $!\n";
    }
    print "not ok $testnum\n";
    $errs++;
  } else {
    print "ok $testnum\n";
  }
  $testnum++;
}
exit( ($errs == 0) ? 0 : -1 )  unless $ENV{HARNESS_ACTIVE};

