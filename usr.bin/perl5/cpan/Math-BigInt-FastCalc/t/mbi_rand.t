# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

my $count = 128;

plan(($^O eq 'os390') ? (skip_all => 'takes too long on os390')
                      : (tests => $count*4));

use Math::BigInt only => 'FastCalc';

my $length = 128;

# If you get a failure here, please re-run the test with the printed seed
# value as input "perl t/mbi_rand.t seed" and send me the output

my $seed = @ARGV == 1 ? $ARGV[0] : int(rand(1165537));
#diag("    seed: $seed\n");
srand($seed);

my $_base_len;
my @_base_len;

#diag("     lib: ", Math::BigInt->config('lib'));
if (Math::BigInt->config('lib') =~ /::Calc/) {
    $_base_len = Math::BigInt::Calc->_base_len();
    @_base_len = Math::BigInt::Calc->_base_len();
    #diag("base len: $_base_len (scalar context)");
    #diag("base len: @_base_len (list contex)");
}

my ($A, $B, $A_str, $B_str, $AdivB, $AmodB, $A_len, $B_len);
my $two = Math::BigInt->new(2);
for (my $i = 0; $i < $count; $i++) {
    #diag("");

    # length of A and B
    $A_len = int(rand($length) + 1);
    $B_len = int(rand($length) + 1);
    $A_str = '';
    $B_str = '';

    # We create the numbers from "patterns", e.g. get a random number and a
    # random count and string them together. This means things like
    # "100000999999999999911122222222" are much more likely. If we just strung
    # together digits, we would end up with "1272398823211223" etc. It also
    # means that we get more frequently equal numbers or other special cases.

    while (length($A_str) < $A_len) {
        $A_str .= int(rand(100)) x int(rand(16));
    }
    while (length($B_str) < $B_len) {
        $B_str .= int(rand(100)) x int(rand(16));
    }

    $A_str =~ s/^0+(?=\d)//;
    $B_str =~ s/^0+(?=\d)//;
    #diag("      As: $A_str");
    #diag("      Bs: $B_str");
    $A = Math::BigInt->new($A_str);
    $B = Math::BigInt->new($B_str);
    #diag("       A: $A");
    #diag("       B: $B");

  SKIP: {
        skip '$A and/or $B are zero.', 4 if $A->is_zero() || $B->is_zero();

        # check that int(A / B) * B + A % B == A holds for all inputs

        # $X = ($A / $B) * $B + 2 * ($A % $B) - ($A % $B);

        ($AdivB, $AmodB) = $A->copy()->bdiv($B);

        #diag("   A / B: $AdivB");
        #diag("   A % B: $AmodB");

        is($AdivB * $B + $two * $AmodB - $AmodB, $A_str,
           "AdivB * B + 2 * AmodB - AmodB == A");

        if (is($AdivB * $B / $B, $AdivB, "AdivB * B / B == AdivB")) {
            if (Math::BigInt->config('lib') =~ /::Calc/) {
                #diag("AdivB->[-1]: ", $AdivB->{value}->[-1]);
                #diag("    B->[-1]: ", $B->{value}->[-1]);
            }
        }

        # swap 'em and try this, too
        # $X = ($B/$A)*$A + $B % $A;
        ($AdivB, $AmodB) = $B->copy()->bdiv($A);
        # print "check: $AdivB $AmodB";

        is($AdivB * $A + $two * $AmodB - $AmodB, $B_str,
           "AdivB * A + 2 * AmodB - AmodB == B");

        is($AdivB * $A / $A, $AdivB, "AdivB * A / A == AdivB");
    }
}
