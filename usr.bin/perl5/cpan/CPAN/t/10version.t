# -*- Mode: cperl; coding: utf-8; -*-

use strict;
use CPAN::Version;
use vars qw($D $N);

# for debugging uncomment the next two lines
# use CPAN;
# $CPAN::DEBUG = 16384;

while (<DATA>) {
  next if tr/.// > 1 && $]<5.006; # multidot tests are not for pre-5.6.0
  last if /^__END__$/;
  chomp;
  s/\s*#.*//;
  push @$D, [ split ];
}

$N = scalar @$D;
print "1..$N\n";

my $has_sort_versions = eval { require Sort::Versions; 1 };
my $has_versionpm = eval q{ use version 0.7203; 1 };
my $has_perl_versionpm = eval { require Perl::Version; 1 };
while (@$D) {
  my($l,$r,$exp) = @{shift @$D};
  my $res = CPAN::Version->vcmp($l,$r);
  if ($res != $exp){
    print "# l[$l]r[$r]exp[$exp]res[$res]\n";
    print "not ";
  }
  my @other = ();
  if ($has_sort_versions) {
    if (Sort::Versions::versioncmp($l,$r) != $res) {
      push @other, sprintf "SV: %d", Sort::Versions::versioncmp($l,$r);
    }
  }
  if ($has_versionpm) {
    local $^W;
    my $vpack = "version"; # hide the name from 5.004
    my $vres = eval { $vpack->new($l) cmp $vpack->new($r); };
    if ($@) {
      push @other, "v.pm: $@";
    } elsif ($vres != $res) {
      push @other, sprintf "v.pm: %d", $vres;
    }
  }
  if ($has_perl_versionpm) {
    local $^W;
    my $vpack = "Perl::Version"; # hide the name from 5.004
    my $vres = eval { $vpack->new($l) cmp $vpack->new($r); };
    if ($@) {
      push @other, "PV: $@";
    } elsif ($vres != $res) {
      push @other, sprintf "PV: %d", $vres;
    }
  }
  my $other = @other ? " (".join("; ", @other).")" : "";
  printf "ok %2d # %12s %12s %3d%s\n", $N-@$D, $l, $r, $res, $other;
  die "Panic" if CPAN::Version->vgt($l,$r) && CPAN::Version->vlt($l,$r);
}

__END__
0 0 0
1 0 1
0 1 -1
1 1 0
1.1 0.0a 1
1.1a 0.0 1
1.2.3 1.1.1 1
v1.2.3 v1.1.1 1
v1.2.3 v1.2.1 1
v1.2.3 v1.2.11 -1
v2.4 2.004000 -1
v2.4 2.004 0
1.2.3 1.2.11 -1
1.9 1.10 1
VERSION VERSION 0
0.02 undef 1
1.57_00 1.57 1
1.5700 1.57 1
1.57_01 1.57 1
1.88_51 1.8801 1
1.8_8_5_1 1.8801 1
0.2.10 0.2 -1
20000000.00 19990108 1
1.00 0.96 1
0.7.2 0.7 -1
0.7.02 0.7 -1
0.07.02 0.7 -1
1.3a5 1.3 1
undef 1.00 -1
v1.0 undef 1
v0.2.4 0.24 -1
v1.0.22 122 -1
1.0.22 122 -1
5.00556 v5.5.560 0
5.005056 v5.5.56 0
5.00557 v5.5.560 1
5.00056 v5.0.561 -1
0.0.2 0.000002 0
1.0.3 1.000003 0
1.0.1 1.000001 0
0.0.1 0.000001 0
0.01.04 0.001004 0
0.05.18 0.005018 0
4.08.00 4.008000 0
0.001.004 0.001004 0
0.005.018 0.005018 0
4.008.000 4.008000 0
4.008.000 4.008 1
v4.8 4.008 0
v4.8.0 4.008000 0
v1.99.1_1 1.98 -1
v2.3999 v2.4000 -1
v2.3999 2.004000 1
v2.3999 2.999999 1
v2.1000 2.999999 1
0123 123 -1
v2.005 2.005 0
v1.0 1.0 0
v1.0 1.000 0
v1.0 1.000000 0
__END__

# Local Variables:
# mode: cperl
# cperl-indent-level: 2
# End:
