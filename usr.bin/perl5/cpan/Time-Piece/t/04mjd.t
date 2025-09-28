use Test;
BEGIN { plan tests => 12 }
# Test the calculation of (modified) Julian date
use Time::Piece;

# First a lookup table of epoch and MJD
# Use 3 sig fig in MJD (hence the use of strings)
# This will not work on systems that use a different reference
# epoch to unix time. To be more general we should use strptime
# to parse the reference date.
my %mjd = (
          951827696  => '51603.524', # 2000-02-29T12:34:56UT
          1000011    => '40598.574', # 1970-01-12T13:46:51UT
          1021605703 => '52411.140', # 2002-05-17T03:21:43UT
          1121605703 => '53568.547', # 2005-07-17T13:08:23UT
          1011590000 => '52295.218', # 2002-01-21T05:13:20UT
          1011605703 => '52295.399', # 2002-01-21T09:35:03
         );

# Now loop over each MJD
for my $time (keys %mjd) {

  # First check using GMT
  my $tp = gmtime( $time );
  ok(sprintf("%.3f",$tp->mjd),$mjd{$time});

  # Now localtime should give the same answer for MJD
  # since MJD is always referred to as UT
  $tp = localtime( $time );
  ok(sprintf("%.3f",$tp->mjd),$mjd{$time});

}

