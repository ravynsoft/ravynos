# -*- mode: perl; -*-

# Test for memory leaks.

# XXX TODO: This test file doesn't actually seem to work! If you remove
# the sv_2mortal() in the XS file, it still happily passes all tests...

use strict;
use Test::More tests => 22;

use Math::BigInt::FastCalc;

#############################################################################
package Math::BigInt::FastCalc::LeakCheck;

use Math::BigInt::FastCalc;
our @ISA = qw< Math::BigInt::FastCalc >;

my $destroyed = 0;
sub DESTROY { $destroyed++; }

#############################################################################
package main;

for my $method (qw(_zero _one _two _ten))
  {
  $destroyed = 0;
    {
    my $num = Math::BigInt::FastCalc::LeakCheck->$method();
    bless $num, "Math::BigInt::FastCalc::LeakCheck";
    }
  is ($destroyed, 1, "$method does not leak memory");
  }

my $num = Math::BigInt::FastCalc->_zero();
for my $method (qw(_is_zero _is_one _is_two _is_ten _num))
  {
  $destroyed = 0;
    {
    my $rc = Math::BigInt::FastCalc->$method($num);
    bless \$rc, "Math::BigInt::FastCalc::LeakCheck";
    }
  is ($destroyed, 1, "$method does not leak memory");
  }

my $num_10 = Math::BigInt::FastCalc->_ten();
my $num_2 = Math::BigInt::FastCalc->_two();

my $num_long   = Math::BigInt::FastCalc->_new("1234567890");
my $num_long_2 = Math::BigInt::FastCalc->_new("12345678900987654321");

is (Math::BigInt::FastCalc->_str($num_long), "1234567890");
is (Math::BigInt::FastCalc->_str($num_long_2), "12345678900987654321");

# to hit all possible code branches
_test_acmp($num, $num);
_test_acmp($num_10, $num_10);
_test_acmp($num, $num_10);
_test_acmp($num_10, $num);
_test_acmp($num, $num_2);
_test_acmp($num_2, $num);
_test_acmp($num_long, $num);
_test_acmp($num, $num_long);
_test_acmp($num_long, $num_long);
_test_acmp($num_long, $num_long_2);
_test_acmp($num_long_2, $num_long);

sub _test_acmp
  {
  my ($n1,$n2) = @_;

  $destroyed = 0;
    {
    my $rc = Math::BigInt::FastCalc->_acmp($n1,$n2);
    bless \$rc, "Math::BigInt::FastCalc::LeakCheck";
    }
  my $n_1 = Math::BigInt::FastCalc->_str($n1);
  my $n_2 = Math::BigInt::FastCalc->_str($n2);
  is ($destroyed, 1, "_acmp($n_1,$n_2) does not leak memory");
  }
