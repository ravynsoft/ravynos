#!./perl

use strict;
use warnings;

use Config;
use Test::More 0.96;
use Time::Local qw(
    timegm
    timelocal
    timegm_modern
    timelocal_modern
    timegm_nocheck
    timelocal_nocheck
    timegm_posix
    timelocal_posix
);

my @local_subs = qw(
    timelocal
    timelocal_modern
    timelocal_posix
    timelocal_nocheck
);

my @gm_subs = qw(
    timegm
    timegm_modern
    timegm_posix
    timegm_nocheck
);

# Use 3 days before the start of the epoch because with Borland on
# Win32 it will work for -3600 _if_ your time zone is +01:00 (or
# greater).
my $neg_epoch_ok
    = $^O eq 'VMS' ? 0 : defined( ( localtime(-259200) )[0] ) ? 1 : 0;

my $large_epoch_ok = eval { ( gmtime 2**40 )[5] == 34912 };

subtest( 'valid times',            \&_test_valid_times );
subtest( 'diff between two calls', \&_test_diff_between_two_calls );
subtest(
    'DST transition bug - https://rt.perl.org/Ticket/Display.html?id=19393',
    \&_test_dst_transition_bug,
);
subtest( 'Time::Local::_is_leap_year', \&_test_is_leap_year );
subtest( 'negative epochs',            \&_test_negative_epochs );
subtest( 'large epoch values',         \&_test_large_epoch_values );
subtest( '2-digit years',              \&_test_2_digit_years );
subtest( 'invalid values',             \&_test_invalid_values );

sub _test_valid_times {
    my %tests = (
        'simple times' => [
            [ 1970, 1,  2,  0,  0,  0 ],
            [ 1980, 2,  28, 12, 0,  0 ],
            [ 1980, 2,  29, 12, 0,  0 ],
            [ 1999, 12, 31, 23, 59, 59 ],
            [ 2000, 1,  1,  0,  0,  0 ],
            [ 2010, 10, 12, 14, 13, 12 ],
        ],
        'leap days' => [
            [ 2020, 2, 29, 12, 59, 59 ],
            [ 2030, 7, 4,  17, 7,  6 ],
        ],
        'non-integer seconds' => [
            [ 2010, 10, 12, 14, 13, 12.1 ],
            [ 2010, 10, 12, 14, 13, 59.1 ],
        ],
    );

    # The following test fails on a surprising number of systems
    # so it is commented out. The end of the Epoch for a 32-bit signed
    # implementation of time_t should be Jan 19, 2038  03:14:07 UTC.
    #  [2038,  1, 17, 23, 59, 59],     # last full day in any tz

    # more than 2**31 time_t - requires a 64bit safe localtime/gmtime
    $tests{'greater than 2**31 seconds'} = [ [ 2258, 8, 11, 1, 49, 17 ] ]
        if $] >= 5.012000;

    # use vmsish 'time' makes for oddness around the Unix epoch
    $tests{'simple times'}[0][2]++
        if $^O eq 'VMS';

    $tests{'negative epoch'} = [
        [ 1969, 12, 31, 16, 59, 59 ],
        [ 1950, 4,  12, 9,  30, 31 ],
    ] if $neg_epoch_ok;

    for my $group ( sort keys %tests ) {
        subtest(
            $group,
            sub { _test_group( $tests{$group} ) },
        );
    }
}

sub _test_group {
    my $group = shift;

    for my $vals ( @{$group} ) {
        my ( $year, $mon, $mday, $hour, $min, $sec ) = @{$vals};
        $mon--;

        # 1970 test on VOS fails
        next if $^O eq 'vos' && $year == 1970;

        for my $sub (@local_subs) {
            my $y = $year;
            $y -= 1900 if $sub =~ /posix/;
            my $time = __PACKAGE__->can($sub)
                ->( $sec, $min, $hour, $mday, $mon, $y );

            my @lt = localtime($time);
            is_deeply(
                {
                    second => $lt[0],
                    minute => $lt[1],
                    hour   => $lt[2],
                    day    => $lt[3],
                    month  => $lt[4],
                    year   => $lt[5],
                },
                {
                    second => int($sec),
                    minute => $min,
                    hour   => $hour,
                    day    => $mday,
                    month  => $mon,
                    year   => $year - 1900,
                },
                "$sub( $sec, $min, $hour, $mday, $mon, $y )"
            );
        }

        for my $sub (@gm_subs) {
            my $y = $year;
            $y -= 1900 if $sub =~ /posix/;
            my $time = __PACKAGE__->can($sub)
                ->( $sec, $min, $hour, $mday, $mon, $y );

            my @gt = gmtime($time);
            is_deeply(
                {
                    second => $gt[0],
                    minute => $gt[1],
                    hour   => $gt[2],
                    day    => $gt[3],
                    month  => $gt[4],
                    year   => $gt[5],
                },
                {
                    second => int($sec),
                    minute => $min,
                    hour   => $hour,
                    day    => $mday,
                    month  => $mon,
                    year   => $year - 1900,
                },
                "$sub( $sec, $min, $hour, $mday, $mon, $y )"
            );
        }
    }
}

sub _test_diff_between_two_calls {
    for my $sub (@local_subs) {
        subtest(
            $sub,
            sub {
                my $year = 1990;
                $year -= 1900 if $sub =~ /posix/;
                my $sub_ref = __PACKAGE__->can($sub);
                is(
                          $sub_ref->( 0, 0, 1, 1, 0, $year )
                        - $sub_ref->( 0, 0, 0, 1, 0, $year ),
                    3600,
                    'one hour difference between two calls'
                );

                is(
                          $sub_ref->( 1, 2, 3, 1, 0, $year + 1 )
                        - $sub_ref->( 1, 2, 3, 31, 11, $year ),
                    24 * 3600,
                    'one day difference between two calls across year boundary',
                );
            },
        );
    }

    for my $sub (@gm_subs) {
        subtest(
            $sub,
            sub {
                my $year = 1980;
                $year -= 1900 if $sub =~ /posix/;
                my $sub_ref = __PACKAGE__->can($sub);

                # Diff beween Jan 1, 1980 and Mar 1, 1980 = (31 + 29 = 60 days)
                is(
                          $sub_ref->( 0, 0, 0, 1, 2, 80 )
                        - $sub_ref->( 0, 0, 0, 1, 0, 80 ),
                    60 * 24 * 3600,
                    '60 day difference between two calls',
                );
            },
        );
    }
}

sub _test_dst_transition_bug {
    for my $sub (@local_subs) {
        subtest(
            $sub,
            sub {
                my $year = 2002;
                $year -= 2002 if $sub =~ /posix/;
                my $sub_ref = __PACKAGE__->can($sub);

                # At a DST transition, the clock skips forward, eg from
                # 01:59:59 to 03:00:00. In this case, 02:00:00 is an
                # invalid time, and should be treated like 03:00:00 rather
                # than 01:00:00 - negative zone offsets used to do the
                # latter.
                my $hour
                    = ( localtime( $sub_ref->( 0, 0, 2, 7, 3, 102 ) ) )[2];

                # testers in US/Pacific should get 3,
                # other testers should get 2
                ok( $hour == 2 || $hour == 3, 'hour should be 2 or 3' );
            },
        );
    }
}

sub _test_is_leap_year {
    my @years = (
        [ 1900 => 0 ],
        [ 1947 => 0 ],
        [ 1996 => 1 ],
        [ 2000 => 1 ],
        [ 2100 => 0 ],
    );

    for my $p (@years) {
        my ( $year, $is_leap_year ) = @$p;

        my $string = $is_leap_year ? 'is' : 'is not';
        ## no critic (Subroutines::ProtectPrivateSubs)
        is(
            Time::Local::_is_leap_year($year), $is_leap_year,
            "$year $string a leap year"
        );
    }
}

sub _test_negative_epochs {
    plan skip_all => 'this platform does not support negative epochs.'
        unless $neg_epoch_ok;

    for my $sub (@gm_subs) {
        subtest(
            $sub,
            sub {
                my $year_mod = $sub =~ /posix/ ? -1900 : 0;
                my $sub_ref  = __PACKAGE__->can($sub);

                unless ( $sub =~ /nocheck/ ) {
                    local $@ = undef;
                    eval { $sub_ref->( 0, 0, 0, 29, 1, 1900 + $year_mod ); };
                    like(
                        $@, qr/Day '29' out of range 1\.\.28/,
                        'does not accept leap day in 1900'
                    );

                    local $@ = undef;
                    eval { $sub_ref->( 0, 0, 0, 29, 1, 200 + $year_mod ) };
                    like(
                        $@, qr/Day '29' out of range 1\.\.28/,
                        'does not accept leap day in 2100 (year passed as 200)'
                    );
                }

                local $@ = undef;
                eval { $sub_ref->( 0, 0, 0, 29, 1, 0 + $year_mod ) };
                is(
                    $@, q{},
                    'no error with leap day of 2000 (year passed as 0)'
                );

                local $@ = undef;
                eval { $sub_ref->( 0, 0, 0, 29, 1, 1904 + $year_mod ) };
                is( $@, q{}, 'no error with leap day of 1904' );

                local $@ = undef;
                eval { $sub_ref->( 0, 0, 0, 29, 1, 4 + $year_mod ) };
                is(
                    $@, q{},
                    'no error with leap day of 2004 (year passed as 4)'
                );

                local $@ = undef;
                eval { $sub_ref->( 0, 0, 0, 29, 1, 96 + $year_mod ) };
                is(
                    $@, q{},
                    'no error with leap day of 1996 (year passed as 96)'
                );
            },
        );
    }
}

sub _test_large_epoch_values {
    plan skip_all => 'These tests require support for large epoch values'
        unless $large_epoch_ok;

    for my $sub (@gm_subs) {
        subtest(
            $sub,
            sub {
                my $year_mod = $sub =~ /posix/ ? -1900 : 0;
                my $sub_ref  = __PACKAGE__->can($sub);

                is(
                    $sub_ref->( 8, 14, 3, 19, 0, 2038 + $year_mod ),
                    2**31,
                    'can call with 2**31 epoch seconds',
                );
                is(
                    $sub_ref->( 16, 28, 6, 7, 1, 2106 + $year_mod ),
                    2**32,
                    'can call with 2**32 epoch seconds (on a 64-bit system)',
                );
                is(
                    $sub_ref->( 16, 36, 0, 20, 1, 36812 + $year_mod ),
                    2**40,
                    'can call with 2**40 epoch seconds (on a 64-bit system)',
                );
            },
        );
    }
}

sub _test_2_digit_years {
    my $current_year = ( localtime() )[5];
    my $pre_break    = ( $current_year + 49 ) - 100;
    my $break        = ( $current_year + 50 ) - 100;
    my $post_break   = ( $current_year + 51 ) - 100;

    subtest(
        'legacy year munging',
        sub {
            plan skip_all => 'Requires support for an large epoch values'
                unless $large_epoch_ok;

            is(
                (
                    ( localtime( timelocal( 0, 0, 0, 1, 1, $pre_break ) ) )[5]
                ),
                $pre_break + 100,
                "year $pre_break is treated as next century",
            );
            is(
                ( ( localtime( timelocal( 0, 0, 0, 1, 1, $break ) ) )[5] ),
                $break + 100,
                "year $break is treated as next century",
            );
            is(
                (
                    ( localtime( timelocal( 0, 0, 0, 1, 1, $post_break ) ) )
                    [5]
                ),
                $post_break,
                "year $post_break is treated as current century",
            );
        }
    );

    subtest(
        'modern',
        sub {
            plan skip_all =>
                'Requires negative epoch support and large epoch support'
                unless $neg_epoch_ok && $large_epoch_ok;

            is(
                (
                    (
                        localtime(
                            timelocal_modern( 0, 0, 0, 1, 1, $pre_break )
                        )
                    )[5]
                ) + 1900,
                $pre_break,
                "year $pre_break is treated as year $pre_break",
            );
            is(
                (
                    (
                        localtime(
                            timelocal_modern( 0, 0, 0, 1, 1, $break )
                        )
                    )[5]
                ) + 1900,
                $break,
                "year $break is treated as year $break",
            );
            is(
                (
                    (
                        localtime(
                            timelocal_modern( 0, 0, 0, 1, 1, $post_break )
                        )
                    )[5]
                ) + 1900,
                $post_break,
                "year $post_break is treated as year $post_break",
            );
        },
    );
}

sub _test_invalid_values {
    my %bad = (
        'month > bounds'  => [ 1995, 13, 1,  1,  1,  1 ],
        'day > bounds'    => [ 1995, 2,  30, 1,  1,  1 ],
        'hour > bounds'   => [ 1995, 2,  10, 25, 1,  1 ],
        'minute > bounds' => [ 1995, 2,  10, 1,  60, 1 ],
        'second > bounds' => [ 1995, 2,  10, 1,  1,  60 ],
        'month < bounds'  => [ 1995, -1, 1,  1,  1,  1 ],
        'day < bounds'    => [ 1995, 2,  -1, 1,  1,  1 ],
        'hour < bounds'   => [ 1995, 2,  10, -1, 1,  1 ],
        'minute < bounds' => [ 1995, 2,  10, 1,  -1, 1 ],
        'second < bounds' => [ 1995, 2,  10, 1,  1,  -1 ],
    );

    for my $sub ( grep { !/nocheck/ } @local_subs, @gm_subs ) {
        subtest(
            $sub,
            sub {
                for my $key ( sort keys %bad ) {
                    my ( $year, $mon, $mday, $hour, $min, $sec )
                        = @{ $bad{$key} };
                    $mon--;

                    local $@ = undef;
                    eval {
                        __PACKAGE__->can($sub)
                            ->( $sec, $min, $hour, $mday, $mon, $year );
                    };

                    like(
                        $@, qr/.*out of range.*/,
                        "$key - @{ $bad{$key} }"
                    );
                }
            },
        );
    }

    for my $sub ( grep {/nocheck/} @local_subs, @gm_subs ) {
        subtest(
            $sub,
            sub {
                for my $key ( sort keys %bad ) {
                    local $@ = q{};
                    eval { __PACKAGE__->can($sub)->( @{ $bad{$key} } ); };
                    is(
                        $@, q{},
                        "$key - @{ $bad{$key} } - no exception with checks disabled"
                    );
                }
            },
        );
    }
}

done_testing();
