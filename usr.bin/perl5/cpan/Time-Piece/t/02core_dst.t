use Test::More;

# Skip if doing a regular install
# Avoids mystery DST bugs [rt 128240], [GH40]
plan skip_all => "DST tests not required for installation"
  unless ( $ENV{AUTOMATED_TESTING} );

my $is_win32 = ($^O =~ /Win32/);
my $is_qnx = ($^O eq 'qnx');
my $is_vos = ($^O eq 'vos');
my $is_linux = ($^O =~ /linux/);
my $is_bsd = ($^O =~ /bsd/);
my $is_mac = ($^O =~ /darwin/);

use Time::Piece;
use Time::Seconds;

#test using an epoch that can be DST
#because sometimes funny stuff can occur [cpan #93095]
#https://rt.cpan.org/Ticket/Display.html?id=93095#txn-1482590

my $t = gmtime(1373371631); # 2013-07-09T12:07:11

is($t->sec,               11);
is($t->second,            11);
is($t->min,               07);
is($t->minute,            07);
is($t->hour,              12);
is($t->mday,               9);
is($t->day_of_month,       9);
is($t->mon,                7);
is($t->_mon,               6);
is($t->year,            2013);
is($t->_year,            113);
is($t->yy,              '13');

cmp_ok($t->wday,        '==',         3);
cmp_ok($t->_wday,       '==',         2);
cmp_ok($t->day_of_week, '==',         2);
cmp_ok($t->yday,        '==',        189);
cmp_ok($t->day_of_year, '==',        189);

# In GMT there should be no daylight savings ever.
cmp_ok($t->isdst, '==', 0);
cmp_ok($t->epoch, '==',   1373371631);
cmp_ok($t->hms,   'eq',   '12:07:11');
cmp_ok($t->time,  'eq',   '12:07:11');
cmp_ok($t->ymd,   'eq', '2013-07-09');
cmp_ok($t->date,  'eq', '2013-07-09');
cmp_ok($t->mdy,   'eq', '07-09-2013');
cmp_ok($t->dmy,   'eq', '09-07-2013');
cmp_ok($t->cdate, 'eq', 'Tue Jul  9 12:07:11 2013');
cmp_ok("$t",      'eq', 'Tue Jul  9 12:07:11 2013');
cmp_ok($t->datetime, 'eq','2013-07-09T12:07:11');
cmp_ok($t->daylight_savings, '==', 0);


cmp_ok($t->week, '==', 28);

# strftime tests

# %a, %A, %b, %B, %c are locale-dependent

# %C is unportable: sometimes its like asctime(3) or date(1),
# sometimes it's the century (and whether for 2000 the century is
# 20 or 19, is fun, too..as far as I can read SUSv2 it should be 20.)
cmp_ok($t->strftime('%d'), '==', 9);

SKIP: {
  skip "can't strftime %D, %R, %T or %e on Win32", 1 if $is_win32;
  cmp_ok($t->strftime('%D'), 'eq', '07/09/13'); # Yech!
}
SKIP:{
  skip "can't strftime %D, %R, %T or %e on Win32", 1 if $is_win32;
  skip "can't strftime %e on QNX", 1 if $is_qnx;
  cmp_ok($t->strftime('%e'), 'eq', ' 9');       # should test with < 10
}

# %h is locale-dependent
cmp_ok($t->strftime('%H'), 'eq', '12'); # should test with < 10

cmp_ok($t->strftime('%I'), 'eq', '12'); # should test with < 10
cmp_ok($t->strftime('%j'), '==',  190 ); # why ->yday+1 ?
cmp_ok($t->strftime('%M'), 'eq', '07'); # should test with < 10

# %p, %P, and %r are not widely implemented,
# and are possibly unportable (am or AM or a.m., and so on)

SKIP: {
  skip "can't strftime %R on Win32 or QNX", 1 if $is_win32 or $is_qnx;
  cmp_ok($t->strftime('%R'), 'eq', '12:07');    # should test with > 12
}

ok($t->strftime('%S') eq '11'); # should test with < 10

SKIP: {
  skip "can't strftime %T on Win32", 1 if $is_win32;
  cmp_ok($t->strftime('%T'), 'eq', '12:07:11'); # < 12 and > 12
}

# There are bugs in the implementation of %u in many platforms.
# (e.g. Linux seems to think, despite the man page, that %u
# 1-based on Sunday...)

cmp_ok($t->strftime('%U'), 'eq', '27'); # Sun cmp Mon

SKIP: {
    skip "can't strftime %V on Win32 or QNX or VOS", 1 if $is_win32 or $is_qnx or $is_vos;
    # is this test really broken on Mac OS? -- rjbs, 2006-02-08
    cmp_ok($t->strftime('%V'), 'eq', '28'); # Sun cmp Mon
}

cmp_ok($t->strftime('%w'), '==', 2);
cmp_ok($t->strftime('%W'), 'eq', '27'); # Sun cmp Mon

# %x is locale and implementation dependent.

cmp_ok($t->strftime('%y'), '==', 13); # should test with 1999
cmp_ok($t->strftime('%Y'), 'eq', '2013');

ok(not $t->is_leap_year); # should test more with different dates

cmp_ok($t->month_last_day, '==', 31); # test more


SKIP: {
	skip "Extra tests for Linux, BSD only.", 8 unless $is_linux or $is_mac or $is_bsd;

    local $ENV{TZ} = "EST5EDT4,M3.2.0/2,M11.1.0/2";
    Time::Piece::_tzset();
    my $lt = localtime(1373371631); #2013-07-09T12:07:11
    cmp_ok(scalar($lt->tzoffset), 'eq', '-14400');
    cmp_ok($lt->strftime("%Y-%m-%d %H:%M:%S %Z"), 'eq', '2013-07-09 08:07:11 EDT');
    like  ($lt->strftime("%z"), qr/-0400|EDT/); #windows: %Z and %z are the same
    is    ($lt->strftime("%s"), 1373371631, 'Epoch output is the same with EDT');

    $lt = localtime(1357733231); #2013-01-09T12:07:11
    cmp_ok(scalar($lt->tzoffset), 'eq', '-18000');
    cmp_ok($lt->strftime("%Y-%m-%d %H:%M:%S %Z"), 'eq', '2013-01-09 07:07:11 EST');
    like  ($lt->strftime("%z"), qr/-0500|EST/);
    is    ($lt->strftime("%s"), 1357733231, 'Epoch output is the same with EST');
}

done_testing(56);
