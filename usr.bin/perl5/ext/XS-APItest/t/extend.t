#!perl
#
# Test stack expansion macros: EXTEND() etc, especially for edge cases
# where the count wraps to a native value or gets truncated.
#
# Some of these tests aren't really testing; they are however exercising
# edge cases, which other tools like ASAN may then detect problems with.
# In particular, test_EXTEND() does *(p+n) = NULL and *PL_stack_max = NULL
# before returning, to help such tools spot errors.
#
# Also, it doesn't test large but legal grow requests; only ridiculously
# large requests that are guaranteed to wrap.

use Test::More;
use Config;
use XS::APItest qw(test_EXTEND);

plan tests => 48;

my $uvsize   = $Config::Config{uvsize};   # sizeof(UV)
my $sizesize = $Config::Config{sizesize}; # sizeof(Size_t)

# The first arg to test_EXTEND() is the SP to use in EXTEND(), treated
# as an offset from PL_stack_max. So extend(-1, 1, $use_ss) shouldn't
# call Perl_stack_grow(), while   extend(-1, 2, $use_ss) should.
# Exercise offsets near to PL_stack_max to detect edge cases.
# Note that having the SP pointer beyond PL_stack_max is legal.

for my $offset (-1, 0, 1) {

    # treat N as either an IV or a SSize_t
    for my $use_ss (0, 1) {

        # test with N in range -1 .. 3; only the -1 should panic

        eval { test_EXTEND($offset, -1, $use_ss) };
        like $@, qr/panic: .*negative count/, "test_EXTEND($offset, -1, $use_ss)";

        for my $n (0,1,2,3) {
            eval { test_EXTEND($offset, $n, $use_ss) };
            is $@, "", "test_EXTEND($offset, $n, $use_ss)";
        }

        # some things can wrap if the int size is greater than the ptr size

        SKIP: {
            skip "Not small ptrs", 3 if $use_ss || $uvsize <= $sizesize;

            # 0xffff... wraps to -1
            eval { test_EXTEND($offset, (1 << 8*$sizesize)-1, $use_ss) };
            like $@, qr/panic: .*negative count/,
                        "test_EXTEND(-1, SIZE_MAX, $use_ss)";

            #  0x10000... truncates to zero;
            #  but the wrap-detection code converts it to -1 to force a panic
            eval { test_EXTEND($offset, 1 << 8*$sizesize, $use_ss) };
            like $@, qr/panic: .*negative count/,
                        "test_EXTEND(-1, SIZE_MAX+1, $use_ss)";

            #  0x1ffff... truncates and then wraps to -1
            eval { test_EXTEND($offset, (1 << (8*$sizesize+1))-1, $use_ss) };
            like $@, qr/panic: .*negative count/,
                        "test_EXTEND(-1, 2*SIZE_MAX-1, $use_ss)";


        }
    }
}
