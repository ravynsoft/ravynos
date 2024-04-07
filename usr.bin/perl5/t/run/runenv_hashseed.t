#!./perl

# Test that setting PERL_HASH_SEED and PERL_PERTURB_KEYS in different
# combinations works as expected, and that changing the values provided
# produces the expected results
#
# We do this by first executing Perl with a given PERL_PERTURB_KEYS
# mode, and then extract the randomly chosen PERL_HASH_SEED it ran under
# from its debug output which was printed to STDERR, and then use it for
# further tests. This allows the tests to be robust to the choice of hash
# function and seed sizes that might be in use in the perl being tested.
# We do not ask perl to output any keys on this run, as our subsequent
# runs will use different environment variables (specifically
# PERL_HASH_SEED) which will change any key order results we see.
#
# We then execute perl a further three times and ask perl to build a
# hash with a specific number of buckets and a specific set of keys. We
# then have perl print the raw keys to STDOUT.
#
# For two of these three runs we supply the same seed, and both of those
# times we supply the same perturb mode, but in different ways, once as
# a name and once as a digit. The debug output should be identical in
# both cases regardless of mode. For PERL_PERTURB_KEYS mode 0=NO, and
# 2=DETERMINISTIC the key order should match. For mode 1=RANDOM the key
# order should differ the vast majority of the time, however the test is
# probabilistic and occasionally may result in the same key order.
#
# The third run we supply a different seed, with a 1 bit difference, but
# with the same PERL_PERTURB_KEYS mode. In this case we expect the key
# order to differ for all three modes, but again the test is
# probabilistic and we may get the same key order in a small percentage
# of the times we try this.
#
# To address the probabilistic nature of these tests we run them
# multiple times and count how many times we get the same key order.
# Most times this should be zero, but occasionally it might be higher.
# Therefore we use a threshold $allowed_fails to determine how many
# times the key order may be unchanged before we consider the tests
# actually failed. We also use a largish number of keys in a hash with
# a large number of buckets, which means we produce a lot a large temp
# files as we test, so we aggressively clean them up as we go.


BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    require Config;
    Config->import;
}

skip_all_without_config('d_fork');
skip_all("NO_PERL_HASH_ENV or NO_PERL_HASH_SEED_DEBUG set")
    if $Config{ccflags} =~ /-DNO_PERL_HASH_ENV\b/
    || $Config{ccflags} =~ /-DNO_PERL_HASH_SEED_DEBUG\b/;
use strict;
use warnings;

# enable DEBUG_RUNENV if you want to see what is being returned
# by the executed perl.
sub my_runperl {
    my ($cmd_array, $perturb, $set_seed) = @_;
    my $opts_hash= {
        PERL_HASH_SEED_DEBUG => 1,
        PERL_PERTURB_KEYS => $perturb
    };
    $opts_hash->{PERL_HASH_SEED}= $set_seed if $set_seed;

    my ( $out, $err )
        = runperl_and_capture( $opts_hash, $cmd_array );
    my @err= split /\n/, $err;

    my $seed;
    my $mode_name;
    my $mode_digit;
    my @err_got_data;
    my @rand_bits;
    foreach my $line (@err) {
        if ($line=~/^Got.*/) {
            push @err_got_data, $line;
        }
        elsif ($line=~/^PL_hash_rand_bits=.*/) {
            push @rand_bits, $line;
        }
        elsif ($line=~/HASH_SEED = (0x[a-f0-9]+)/) {
            $seed= $1;
            $line =~ /PERTURB_KEYS = (\d) \((\w+)\)/
                or die "Failed to extract perturb mode: $err";
            $mode_digit = $1;
            $mode_name = $2;

        }
    }
    if (!$seed){
        die "Failed to extract seed: $err";
    }
    my $err_got_data= join("\n",@err_got_data);
    return ($seed, $mode_digit, $mode_name, $out, $err_got_data, \@rand_bits);
}

my @mode_names = (
    'NO',            # 0
    'RANDOM',        # 1
    'DETERMINISTIC', # 2
);

my $repeat = 50; # if this changes adjust the comments below.
my $min_buckets = 100_000;
my $actual_buckets = 1;
$actual_buckets *= 2 while $actual_buckets <= $min_buckets;
my $key_expr = '0..999, "aa".."zz", map { $_ x 30 } "a".."z"'; #1702 keys
my @keys = eval $key_expr
    or die "bad '$key_expr': $@";
my $allowed_fails = 2; # Adjust this up to make the test tolerate
                       # more "errors". Maybe one day we will compute
                       # it from the value of $repeat, and $actual_buckets
                       # and the number of @keys.

plan tests => (4 * $repeat)     # DETERMINISTIC
            + (1 * $repeat)     # NO
            + 1                 # RANDOM mode
            + (8 * @mode_names) # validation per mode
            + @mode_names;      # all modes


# Note the keys(%h) = $n will cause perl to allocate the power of 2 larger
# than $n buckets, so if $n = 100_000, then $actual_buckets will be 131072.

my @perl_args = (
    '-I../lib',
    (is_miniperl() ? () # no Hash::Util here!
                   : '-MHash::Util=hash_traversal_mask,num_buckets'),
    '-e',
    'my %h; keys(%h)=' . $min_buckets . '; ' .
    '@h{' . $key_expr . '}=(); @k=keys %h; ' .
      'print join ":", 0+@k, ' .
      (is_miniperl() ? '' :  # no Hash::Util here!
          'num_buckets(%h),hash_traversal_mask(\\%h), ') .
      'join ",", @k;'
  );

for my $test_mode_digit (0 .. $#mode_names) {
    my $test_mode_name = $mode_names[$test_mode_digit];
    my $descr_mode = "mode = $test_mode_name";

    my $print_keys= [ ($test_mode_name eq "DETERMINISTIC")
                      ? "-Dh" : (), # enable hash diags
                      @perl_args ];

    my $validated_mode= 0;
    my $random_same = 0;
    my $seed_change_same = 0;
    for my $try (1 .. $repeat) {

        my $descr = sprintf "%s, try %2d:", $descr_mode, $try;

        # First let perl choose the seed. We only use the $seed and $err
        # output here. We extract the seed that perl chose, which
        # hardens us against the use of different hash functions with
        # different seed sizes. Also the act of adding the PERL_HASH_SEED
        # to the environment later on will likely change the $out.
        my ( $seed, $digit, $mode )
            = my_runperl( ['-e1'], $test_mode_name );

        # Now we have to run it again.
        my ( $seed1, $digit1, $mode1, $out1, $err_got_data1, $rand_bits1 )
            = my_runperl( $print_keys, $test_mode_name, $seed );

        # And once more, these two should do the same thing for
        # DETERMINISTIC and NO, and be different for RANDOM.
        # We set the mode via the digit not the name here.
        my ( $seed2, $digit2, $mode2, $out2, $err_got_data2, $rand_bits2 )
            = my_runperl( $print_keys, $test_mode_digit, $seed );

        if (!$validated_mode++) {
            is($digit, $test_mode_digit,
                "$descr base run set the mode digit as expected");

            is($mode, $test_mode_name,
                "$descr base run set the mode name as expected");

            is( $seed1, $seed,
                "$descr retry 1 set the seed as expected");

            is( $mode1, $test_mode_name,
                "$descr retry 1 set the mode by name as expected");

            is( $digit2, $test_mode_digit,
                "$descr retry 2 set the mode by digit as expected");

            is( $seed1, $seed2,
                "$descr seeds match between retries");

            is( $digit1, $digit2,
                "$descr mode digits match between retries");

            is( $mode1, $mode2,
                "$descr mode names match between retries");
        }

        {
            # We also test that a 1 bit change to the seed will
            # actually change the output in all modes. It should
            # most of the time.
            my $munged_seed = $seed;
            substr($munged_seed,-1)=~tr/0-9a-f/1-9a-f0/;
            if ( $munged_seed eq $seed ) {
                die "Failed to munge seed '$seed'";
            }

            my ( $new_seed, $new_digit, $new_mode, $new_out )
                = my_runperl( \@perl_args, $test_mode_name, $munged_seed );
            if ($new_seed ne $munged_seed) {
                die "panic: seed change didn't seem to propagate";
            }
            if (
                $new_mode  ne $test_mode_name or
                $new_digit ne $test_mode_digit
            ) {
                die "panic: mode setting not as expected";
            }

            # The result should be different most times, but there
            # is a small chance that we got the same result, so
            # count how many times it happens and then check if it
            # exceeds $allowed_fails later.
            $seed_change_same++ if $out1 eq $new_out;
        }

        if ( $test_mode_name eq 'RANDOM' ) {
            # The result should be different most times, but there is a
            # small chance that we get the same result, so count how
            # many times it happens and then check if it exceeds
            # $allowed_fails later.
            $random_same++ if $out1 eq $out2;
            next;
        }

        # From this point on we are testing DETERMINISTIC and NO
        # modes only.

        is( $out1, $out2,
            "$descr results in the same key order each time"
        );

        next if $test_mode_name eq "NO";

        # From this point on we are testing the DETERMINISTIC
        # mode only.

        SKIP: {
            # skip these tests if we are not running in a DEBUGGING perl.
            skip "$descr not testing rand bits, not a DEBUGGING perl", 3
                if @$rand_bits1 + @$rand_bits2 == 0;

            is ( 0+@$rand_bits1, 0+@$rand_bits2,
                "$descr same count of rand_bits entries each time");

            my $max_i = $#$rand_bits1 > $#$rand_bits2
                      ? $#$rand_bits1 : $#$rand_bits2;

            my $bad_idx;
            for my $i (0 .. $max_i) {
                if (($rand_bits2->[$i] // "") ne
                    ($rand_bits1->[$i] // ""))
                {
                    $bad_idx = $i;
                    last;
                }
            }
            is($bad_idx, undef,
                "$descr bad rand bits data index should be undef");
            if (defined $bad_idx) {
                # we use is() to see the differing data, but this test
                # is expected to fail - the description seems a little
                # odd here, but since it will always fail it makes sense
                # in context.
                is($rand_bits2->[$bad_idx],$rand_bits1->[$bad_idx],
                    "$descr rand bits data is the same at idx $bad_idx");
            } else {
                pass("$descr rand bits data is the same");
            }
        }
    }
    continue {
        # We create a lot of big temp files so clean them up as we go.
        # This is in a continue block so we can do this cleanup after
        # each iteration even if we call next in the middle of the loop.
        unlink_tempfiles();
    }

    # We just finished $repeat tests, now deal with the probabilistic
    # results and ensure that we are under the $allowed_fails threshold

    if ($test_mode_name eq "RANDOM") {
        # There is a small chance we got the same result a few times
        # even when everything is working as expected. So allow a
        # small number number of fails determined by $allowed_fails.
        ok( $random_same <= $allowed_fails,
            "$descr_mode same key order no more than $allowed_fails times")
            or diag(
                "Key order was the same $random_same/$repeat times in",
                "RANDOM mode. This test is probabilistic so if the number",
                "is low and you re-run the tests and it does not fail",
                "again then you can ignore this test fail.");

    }

    # There is a small chance we got the same result a few times even
    # when everything is working as expected. So allow a small number
    # of fails as determined by $allowed_fails.
    ok( $seed_change_same <= $allowed_fails,
        "$descr_mode same key order with different seed no more " .
        "than $allowed_fails times" )
        or diag(
            "Key order was the same $random_same/$repeat times with",
            "a different seed. This test is probabilistic so if the number",
            "is low and you re-run the tests and it does not fail",
            "again then you can ignore this test fail.");
}
