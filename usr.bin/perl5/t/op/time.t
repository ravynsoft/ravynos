#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 72;

# These tests make sure, among other things, that we don't end up
# burning tons of CPU for dates far in the future.
# watchdog() makes sure that the test script eventually exits if
# the tests are triggering the failing behavior
watchdog(25);

($beguser,$begsys) = times;

$beg = time;

while (($now = time) == $beg) { sleep 1 }

ok($now > $beg && $now - $beg < 10,             'very basic time test');
my $x = "aaaa";
for ($i = 0; $i < 1_000_000; $i++) {
    for my $j (1..1000) { ++$x; }; # burn some user cycles
    ($nowuser, $nowsys) = times;
    $i = 2_000_000 if $nowuser > $beguser && ( $nowsys >= $begsys ||
                                            (!$nowsys && !$begsys));
    last if time - $beg > 20;
}

ok($i >= 2_000_000, 'very basic times test');

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($beg);
($xsec,$foo) = localtime($now);
$localyday = $yday;

isnt($sec, $xsec,      'localtime() list context');
ok $mday,              '  month day';
ok $year,              '  year';

ok(localtime() =~ /^(Sun|Mon|Tue|Wed|Thu|Fri|Sat)[ ]
                    (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)[ ]
                    ([ \d]\d)\ (\d\d):(\d\d):(\d\d)\ (\d{4})$
                  /x,
   'localtime(), scalar context'
  );

SKIP: {
    # This conditional of "No tzset()" is stolen from ext/POSIX/t/time.t
    skip "No tzset()", 1
        if $^O eq "VMS" || $^O eq "cygwin" ||
           $^O eq "MSWin32" ||
           $^O eq "interix";

# check that localtime respects changes to $ENV{TZ}
$ENV{TZ} = "GMT-5";
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($beg);
$ENV{TZ} = "GMT+5";
($sec,$min,$hour2,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($beg);
ok($hour != $hour2,                             'changes to $ENV{TZ} respected');
}


($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime($beg);
($xsec,$foo) = localtime($now);

isnt($sec, $xsec,      'gmtime() list conext');
ok $mday,              '  month day';
ok $year,              '  year';

my $day_diff = $localyday - $yday;
ok( grep({ $day_diff == $_ } (0, 1, -1, 364, 365, -364, -365)),
                     'gmtime() and localtime() agree what day of year');


# This could be stricter.
ok(gmtime() =~ /^(Sun|Mon|Tue|Wed|Thu|Fri|Sat)[ ]
                 (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)[ ]
                 ([ \d]\d)\ (\d\d):(\d\d):(\d\d)\ (\d{4})$
               /x,
   'gmtime(), scalar context'
  );



# Test gmtime over a range of times.
{
    # The range should be limited only by the 53-bit mantissa of an IEEE double (or 
    # whatever kind of double you've got).  Here we just prove that we're comfortably 
    # beyond the range possible with 32-bit time_t.
    my %tests = (
        # time_t         gmtime list                          scalar
        -2**35  => [52, 13, 20, 7, 2, -1019, 5, 65, 0, "Fri Mar  7 20:13:52 881"],
        -2**32  => [44, 31, 17, 24, 10, -67, 0, 327, 0, "Sun Nov 24 17:31:44 1833"],
        -2**31  => [52, 45, 20, 13, 11, 1, 5, 346, 0, "Fri Dec 13 20:45:52 1901"],
        -1      => [59, 59, 23, 31, 11, 69, 3, 364, 0, "Wed Dec 31 23:59:59 1969"],
        0       => [0, 0, 0, 1, 0, 70, 4, 0, 0, "Thu Jan  1 00:00:00 1970"],
        1       => [1, 0, 0, 1, 0, 70, 4, 0, 0, "Thu Jan  1 00:00:01 1970"],
        2**30   => [4, 37, 13, 10, 0, 104, 6, 9, 0, "Sat Jan 10 13:37:04 2004"],
        2**31   => [8, 14, 3, 19, 0, 138, 2, 18, 0, "Tue Jan 19 03:14:08 2038"],
        2**32   => [16, 28, 6, 7, 1, 206, 0, 37, 0, "Sun Feb  7 06:28:16 2106"],
        2**39   => [8, 18, 12, 25, 0, 17491, 2, 24, 0, "Tue Jan 25 12:18:08 19391"],
    );

    for my $time (keys %tests) {
        my @expected  = @{$tests{$time}};
        my $scalar    = pop @expected;

        ok eq_array([gmtime($time)], \@expected),  "gmtime($time) list context";
        is scalar gmtime($time), $scalar,       "  scalar";
    }
}


# Test localtime
{
    # We pick times which fall in the middle of a month, so the month and year should be
    # the same regardless of the time zone.
    my %tests = (
        # time_t           month, year,  scalar
        -8589934592     => [9,    -203,                 qr/Oct \d+ .* 1697$/],
        -1296000        => [11,   69,                   qr/Dec \d+ .* 1969$/],
        1296000         => [0,    70,                   qr/Jan \d+ .* 1970$/],
        5000000000      => [5,    228,                  qr/Jun \d+ .* 2128$/],
        1163500000      => [10,   106,                  qr/Nov \d+ .* 2006$/],
    );

    for my $time (keys %tests) {
        my @expected  = @{$tests{$time}};
        my $scalar    = pop @expected;

        my @time = (localtime($time))[4,5];
        ok( eq_array(\@time, \@expected),  "localtime($time) list context" )
          or diag("@time");
        like scalar localtime($time), $scalar,       "  scalar";
    }
}

# Test floating point args
{
    warning_is(sub {is( (localtime(1296000.23))[5] + 1900, 1970 )},
	       undef, 'Ignore fractional time');
    warning_is(sub {is( (gmtime(1.23))[5] + 1900, 1970 )},
	       undef, 'Ignore fractional time');
}


# Some sanity tests for the far, far future and far, far past
{
    my %time2year = (
        -2**52  => -142711421,
        -2**48  => -8917617,
        -2**46  => -2227927,
         2**46  => 2231866,
         2**48  => 8921556,
         2**52  => 142715360,
    );

    for my $time (sort keys %time2year) {
        my $want = $time2year{$time};

        my $have = (gmtime($time))[5] + 1900;
        is $have, $want, "year check, gmtime($time)";

        $have = (localtime($time))[5] + 1900;
        is $have, $want, "year check, localtime($time)";
    }
}


# Test that Perl warns properly when it can't handle a time.
{
    my $warning;
    local $SIG{__WARN__} = sub { $warning .= join "\n", @_; };

    my $big_time   = 2**60;
    my $small_time = -2**60;

    $warning = '';
    my $date = gmtime($big_time);
    like $warning, qr/^gmtime(.*) too large/;

    $warning = '';
    $date = localtime($big_time);
    like $warning, qr/^localtime(.*) too large/;

    $warning = '';
    $date = gmtime($small_time);
    like $warning, qr/^gmtime(.*) too small/;

    $warning = '';
    $date = localtime($small_time);
    like $warning, qr/^localtime(.*) too small/;
}

SKIP: { #rt #73040
    # these are from the definitions of TIME_LOWER_BOUND AND TIME_UPPER_BOUND
    my $smallest = -67768100567755200.0;
    my $biggest = 67767976233316800.0;

    # offset to a value that will fail
    my $small_time = $smallest - 200;
    my $big_time = $biggest + 200;

    # check they're representable - typically means NV is
    # long double
    if ($small_time + 200 != $smallest
	|| $small_time == $smallest
        || $big_time - 200 != $biggest
	|| $big_time == $biggest) {
	skip "Can't represent test values", 8;
    }
    my $small_time_f = sprintf("%.0f", $small_time);
    my $big_time_f = sprintf("%.0f", $big_time);

    # check the numbers in the warning are correct
    my $warning;
    local $SIG{__WARN__} = sub { $warning .= join "\n", @_; };
    $warning = '';
    my $date = gmtime($big_time);
    like $warning, qr/^gmtime\($big_time_f\) too large/;
    like $warning, qr/^gmtime\($big_time_f\) failed/m;

    $warning = '';
    $date = localtime($big_time);
    like $warning, qr/^localtime\($big_time_f\) too large/;
    like $warning, qr/^localtime\($big_time_f\) failed/m;

    $warning = '';
    $date = gmtime($small_time);
    like $warning, qr/^gmtime\($small_time_f\) too small/;
    like $warning, qr/^gmtime\($small_time_f\) failed/m;

    $warning = '';
    $date = localtime($small_time);
    like $warning, qr/^localtime\($small_time_f\) too small/;
    like $warning, qr/^localtime\($small_time_f\) failed/m;
}

my $is_vax = (pack("d", 1) =~ /^[\x80\x10]\x40/);
my $has_nan = !$is_vax;

SKIP: {
    skip("No NaN", 2) unless $has_nan;
    local $^W;
    is scalar gmtime("NaN"), undef, '[perl #123495] gmtime(NaN)';
    is scalar localtime("NaN"), undef, 'localtime(NaN)';
}
