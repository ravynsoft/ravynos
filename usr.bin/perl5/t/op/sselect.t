#!./perl

# Four-argument select

my $hires;
BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('.', '../lib');
    $hires = eval 'use Time::HiResx "time"; 1';
}

skip_all("Win32 miniperl has no socket select")
  if $^O eq "MSWin32" && is_miniperl();

plan (23);

my $blank = "";
eval {select undef, $blank, $blank, 0};
is ($@, "", 'select undef  $blank $blank 0');
eval {select $blank, undef, $blank, 0};
is ($@, "", 'select $blank undef  $blank 0');
eval {select $blank, $blank, undef, 0};
is ($@, "", 'select $blank $blank undef  0');

eval {select "", $blank, $blank, 0};
is ($@, "", 'select ""     $blank $blank 0');
eval {select $blank, "", $blank, 0};
is ($@, "", 'select $blank ""     $blank 0');
eval {select $blank, $blank, "", 0};
is ($@, "", 'select $blank $blank ""     0');

# Test with read-only copy-on-write empty string
my($rocow) = keys%{{""=>undef}};
Internals::SvREADONLY($rocow,1);
eval {select $rocow, $blank, $blank, 0};
is ($@, "", 'select $rocow     $blank $blank 0');
eval {select $blank, $rocow, $blank, 0};
is ($@, "", 'select $blank $rocow     $blank 0');
eval {select $blank, $blank, $rocow, 0};
is ($@, "", 'select $blank $blank $rocow     0');

eval {select "a", $blank, $blank, 0};
like ($@, qr/^Modification of a read-only value attempted/,
	    'select "a"    $blank $blank 0');
eval {select $blank, "a", $blank, 0};
like ($@, qr/^Modification of a read-only value attempted/,
	    'select $blank "a"    $blank 0');
eval {select $blank, $blank, "a", 0};
like ($@, qr/^Modification of a read-only value attempted/,
	    'select $blank $blank "a"    0');

my $sleep = 3;
# Actual sleep time on Windows may be rounded down to an integral
# multiple of the system clock tick interval.  Clock tick interval
# is configurable, but usually about 15.625 milliseconds.
# time() however (if we haven;t loaded Time::HiRes), doesn't return
# fractional values, so the observed delay may be 1 second short.
#
# There is also a report that old linux kernels may return 0.5ms early:
# <20110520081714.GC17549@mars.tony.develop-help.com>.
#

my $under = $hires ? 0.1 : 1;

my $t0 = time;
select(undef, undef, undef, $sleep);
my $t1 = time;
my $diff = $t1-$t0;
ok($diff >= $sleep-$under, "select(u,u,u,\$sleep):  at least $sleep seconds have passed");
note("diff=$diff under=$under");

my $empty = "";
vec($empty,0,1) = 0;
$t0 = time;
select($empty, undef, undef, $sleep);
$t1 = time;
$diff = $t1-$t0;
ok($diff >= $sleep-$under, "select(\$e,u,u,\$sleep): at least $sleep seconds have passed");
note("diff=$diff under=$under");

# [perl #120102] CORE::select ignoring timeout var's magic

{
    package RT120102;

    my $count = 0;

    sub TIESCALAR { bless [] }
    sub FETCH { $count++; 0.1 }

    my $sleep;

    tie $sleep, 'RT120102';
    select (undef, undef, undef, $sleep);
    ::is($count, 1, 'RT120102');
}

package _131645{
    sub TIESCALAR { bless [] }
    sub FETCH     { 0        }
    sub STORE     {          }
}
tie $tie, _131645::;
select ($tie, undef, undef, $tie);
ok("no crash from select $numeric_tie, undef, undef, $numeric_tie");

SKIP: {
    skip "Can't load modules under miniperl", 4 if is_miniperl;
    my $SKIP_CR = sub {
        skip shift, 4;
    };

    if ($^O =~ m<win32|vms>i) {
        $SKIP_CR->("Perl's 4-arg select() in $^O only works with sockets.");
    }

    eval { require POSIX } or do {
        $SKIP_CR->("Failed to load POSIX.pm: $@");
    };

    my $mask;

    for (my $f=0; $f<100; $f++) {
        my $fd = POSIX::dup(fileno \*STDOUT);

        if (!defined $fd) {
            $SKIP_CR->("dup(STDOUT): $!");
            last UTF8TEST;
        }

        vec( my $curmask, $fd, 1 ) = 1;

        if ($curmask =~ tr<\x80-\xff><>) {
            note("FD = $fd");
            $mask = $curmask;
            last;
        }
    }


    if (defined $mask) {
        utf8::downgrade($mask);
        my $mask2;

        my $result = select $mask2 = $mask, undef, undef, 0;

        isnt( $result, -1, 'select() read on non-utf8-flagged mask' );

        utf8::upgrade($mask);
        $result = select $mask2 = $mask, undef, undef, 0;

        isnt( $result, -1, 'select() read on utf8-flagged mask' );

        # ----------------------------------------

        utf8::downgrade($mask);
        $result = select undef, $mask2 = $mask, undef, 0;

        isnt( $result, -1, 'select() write on non-utf8-flagged mask' );

        utf8::upgrade($mask);
        $result = select undef, $mask2 = $mask, undef, 0;

        isnt( $result, -1, 'select() write on utf8-flagged mask' );
    }
    else {
        $SKIP_CR->("No suitable file descriptor for UTF-8-flag test found.");
    }
}

{
    my $badmask = "\x{100}";

    eval { select $badmask, undef, undef, 0 };
    ok( $@, 'select() read fails when given a wide character' );

    eval { select undef, $badmask, undef, 0 };
    ok( $@, 'select() write fails when given a wide character' );

    eval { select undef, undef, $badmask, 0 };
    ok( $@, 'select() exception fails when given a wide character' );
}
