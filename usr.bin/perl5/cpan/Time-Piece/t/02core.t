use Test::More tests => 100;

my $is_win32 = ($^O =~ /Win32/);
my $is_qnx = ($^O eq 'qnx');
my $is_vos = ($^O eq 'vos');

use Time::Piece;
use Time::Seconds;

my $t = gmtime(951827696); # 2000-02-29T12:34:56

is($t->sec,               56);
is($t->second,            56);
is($t->min,               34);
is($t->minute,            34);
is($t->hour,              12);
is($t->mday,              29);
is($t->day_of_month,      29);
is($t->mon,                2);
is($t->_mon,               1);
is($t->year,            2000);
is($t->_year,            100);
is($t->yy,              '00');

cmp_ok($t->wday,        '==',         3);
cmp_ok($t->_wday,       '==',         2);
cmp_ok($t->day_of_week, '==',         2);
cmp_ok($t->yday,        '==',        59);
cmp_ok($t->day_of_year, '==',        59);

# In GMT there should be no daylight savings ever.
cmp_ok($t->isdst, '==', 0);
cmp_ok($t->epoch, '==', 951827696);
cmp_ok($t->hms,   'eq',   '12:34:56');
cmp_ok($t->time,  'eq',   '12:34:56');
cmp_ok($t->ymd,   'eq', '2000-02-29');
cmp_ok($t->date,  'eq', '2000-02-29');
cmp_ok($t->mdy,   'eq', '02-29-2000');
cmp_ok($t->dmy,   'eq', '29-02-2000');
cmp_ok($t->cdate, 'eq', 'Tue Feb 29 12:34:56 2000');
cmp_ok("$t",      'eq', 'Tue Feb 29 12:34:56 2000');
cmp_ok($t->datetime, 'eq','2000-02-29T12:34:56');
cmp_ok($t->daylight_savings, '==', 0);

# ->tzoffset?
my $is_pseudo_fork = 0;
if (defined &Win32::GetCurrentProcessId
    ? $$ != Win32::GetCurrentProcessId() : $^O eq "MSWin32" && $$ < 0) {
    $is_pseudo_fork = 1;
}
SKIP: {
    skip "can't register TZ changes in a pseudo-fork", 2 if $is_pseudo_fork;
    local $ENV{TZ} = "EST5";
    Time::Piece::_tzset();  # register the environment change
    my $lt = localtime;
    cmp_ok(scalar($lt->tzoffset), 'eq', '-18000');
    cmp_ok($lt->strftime("%Z"), 'eq', 'EST');
}

cmp_ok(($t->julian_day / 2451604.0243 ) - 1, '<', 0.001);
cmp_ok(($t->mjd        /   51603.52426) - 1, '<', 0.001);
cmp_ok($t->week, '==', 9);

# strftime tests

# %a, %A, %b, %B, %c are locale-dependent

# %C is unportable: sometimes its like asctime(3) or date(1),
# sometimes it's the century (and whether for 2000 the century is
# 20 or 19, is fun, too..as far as I can read SUSv2 it should be 20.)
cmp_ok($t->strftime('%d'), '==', 29);

cmp_ok($t->strftime('%D'), 'eq', '02/29/00'); # Yech!
cmp_ok($t->strftime('%e'), 'eq', '29');       # should test with < 10

# %h is locale-dependent
cmp_ok($t->strftime('%H'), 'eq', '12'); # should test with < 10

cmp_ok($t->strftime('%I'), 'eq', '12'); # should test with < 10
cmp_ok($t->strftime('%j'), '==',  60 ); # why ->yday+1 ?
cmp_ok($t->strftime('%M'), 'eq', '34'); # should test with < 10

# %p, %P, and %r are not widely implemented,
# and are possibly unportable (am or AM or a.m., and so on)

cmp_ok($t->strftime('%R'), 'eq', '12:34');    # should test with > 12

ok($t->strftime('%S') eq '56'); # should test with < 10

cmp_ok($t->strftime('%T'), 'eq', '12:34:56'); # < 12 and > 12

# There are bugs in the implementation of %u in many platforms.
# (e.g. Linux seems to think, despite the man page, that %u
# 1-based on Sunday...)

cmp_ok($t->strftime('%U'), 'eq', '09'); # Sun cmp Mon

SKIP: {
    skip "can't strftime %V on QNX or VOS", 1 if $is_qnx or $is_vos;
    # is this test really broken on Mac OS? -- rjbs, 2006-02-08
    cmp_ok($t->strftime('%V'), 'eq', '09'); # Sun cmp Mon
}

cmp_ok($t->strftime('%w'), '==', 2);
cmp_ok($t->strftime('%W'), 'eq', '09'); # Sun cmp Mon

# %x is locale and implementation dependent.

cmp_ok($t->strftime('%y'), '==', 0); # should test with 1999
cmp_ok($t->strftime('%Y'), 'eq', '2000');

# %Z is locale and implementation dependent (s/// to the rescue)
cmp_ok($t->strftime('%z'), 'eq', '+0000');
cmp_ok($t->strftime('%%z%z'), 'eq', '%z+0000');
cmp_ok($t->strftime('%Z'), 'eq', 'UTC');
cmp_ok($t->strftime('%%Z%Z'), 'eq', '%ZUTC');

# (there is NO standard for timezone names)
cmp_ok($t->date(""), 'eq', '20000229');
cmp_ok($t->ymd("") , 'eq', '20000229');
cmp_ok($t->mdy("/"), 'eq', '02/29/2000');
cmp_ok($t->dmy("."), 'eq', '29.02.2000');
cmp_ok($t->date_separator, 'eq', '-');

$t->date_separator("/");
cmp_ok($t->date_separator, 'eq', '/');
cmp_ok(Time::Piece::date_separator(), 'eq', '/');
cmp_ok($t->ymd,            'eq', '2000/02/29');

$t->date_separator("-");
cmp_ok($t->time_separator, 'eq', ':');
cmp_ok($t->hms("."),       'eq', '12.34.56');

$t->time_separator(".");
cmp_ok($t->time_separator, 'eq', '.');
cmp_ok(Time::Piece::time_separator(), 'eq', '.');
cmp_ok($t->hms,            'eq', '12.34.56');

$t->time_separator(":");

my @fidays = qw( sunnuntai maanantai tiistai keskiviikko torstai
                 perjantai lauantai );
my @frdays = qw( Dimanche Lundi Merdi Mercredi Jeudi Vendredi Samedi );

cmp_ok($t->day(@fidays), 'eq', "tiistai");
my @days = $t->day_list();

$t->day_list(@frdays);

cmp_ok($t->day, 'eq', "Merdi");

$t->day_list(@days);

my @nmdays = Time::Piece::day_list();
is_deeply (\@nmdays, \@days);

my @months = $t->mon_list();

my @dumonths = qw(januari februari maart april mei juni
                  juli augustus september oktober november december);

cmp_ok($t->month(@dumonths), 'eq', "februari");

$t->mon_list(@dumonths);

cmp_ok($t->month, 'eq', "februari");

$t->mon_list(@months);

cmp_ok($t->month, 'eq', "Feb");
my @nmmonths = Time::Piece::mon_list();
is_deeply (\@nmmonths, \@months);

cmp_ok(
  $t->datetime(date => '/', T => ' ', time => '-'),
  'eq',
  "2000/02/29 12-34-56"
);

ok($t->is_leap_year); # should test more with different dates

cmp_ok($t->month_last_day, '==', 29); # test more

ok(!Time::Piece::_is_leap_year(1900));

ok(!Time::Piece::_is_leap_year(1901));

ok(Time::Piece::_is_leap_year(1904));

cmp_ok(Time::Piece->strptime("1945", "%Y")->year, '==', 1945, "Year is 1945?");

cmp_ok(Time::Piece->strptime("13:00", "%H:%M")->hour, '==', 13, "Hour is 13?");

# Test week number
# [from Ilya Martynov]
cmp_ok(Time::Piece->strptime("2002/06/10 0", '%Y/%m/%d %H')->week,  '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 1", '%Y/%m/%d %H')->week,  '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 2", '%Y/%m/%d %H')->week,  '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 12", '%Y/%m/%d %H')->week, '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 13", '%Y/%m/%d %H')->week, '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 14", '%Y/%m/%d %H')->week, '==', 24);
cmp_ok(Time::Piece->strptime("2002/06/10 23", '%Y/%m/%d %H')->week, '==', 24);

# Test that strptime populates all relevant fields
cmp_ok(Time::Piece->strptime("2002/07/10", '%Y/%m/%d')->wday,  '==', 4);
cmp_ok(Time::Piece->strptime("2002/12/31", '%Y/%m/%d')->yday,  '==', 364);
cmp_ok(Time::Piece->strptime("2002/07/10", '%Y/%m/%d')->isdst, '==', -1);
cmp_ok(Time::Piece->strptime("2002/07/10", '%Y/%m/%d')->day_of_week, '==', 3);

is(
  Time::Piece->strptime('12212', "%y%j")->ymd(),
  '2012-07-30',
  "day of the year parsing",
);

cmp_ok(
  Time::Piece->strptime("2000/02/29 12:34:56", '%Y/%m/%d %H:%M:%S')->epoch,
  '==',
  951827696
);


my $s = Time::Seconds->new(-691050);
is($s->pretty, 'minus 7 days, 23 hours, 57 minutes, 30 seconds');

$s = Time::Seconds->new(-90061);
is($s->pretty, 'minus 1 day, 1 hour, 1 minute, 1 second');

$s = Time::Seconds->new(10);
is($s->pretty, '10 seconds');
$s = Time::Seconds->new(130);
is($s->pretty, '2 minutes, 10 seconds');
$s = Time::Seconds->new(7330);
is($s->pretty, '2 hours, 2 minutes, 10 seconds', "Format correct");
