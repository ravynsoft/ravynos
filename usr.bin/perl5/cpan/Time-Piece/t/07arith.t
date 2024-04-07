use Test::More tests => 43;

BEGIN { use_ok('Time::Piece'); use_ok('Time::Seconds'); }

ok(1);

my $t = gmtime(951827696); # 2000-02-29T12:34:56

is($t->mon, 2);
is($t->mday, 29);

my $t2 = $t->add_months(1);
is($t2->year, 2000);
is($t2->mon,  3);
is($t2->mday, 29);

my $t3 = $t->add_months(-1);
is($t3->year, 2000);
is($t3->mon,  1);
is($t3->mday, 29);

# this one wraps around to March because of the leap year
my $t4 = $t->add_years(1);
is($t4->year, 2001);
is($t4->mon, 3);
is($t4->mday, 1);

$t = Time::Piece->strptime("01 01 2010","%d %m %Y");
my $t6 = $t->add_months(-12);
is($t6->year, 2009);
is($t6->mon, 1);
is($t6->mday, 1);

my $t7 = $t->add_months(-1);
is($t7->year, 2009);
is($t7->mon, 12);
is($t7->mday, 1);

my $t8 = $t->add_months(-240);
is($t8->year, 1990);
is($t8->mon, 1);
is($t8->mday, 1);

my $t9 = $t->add_months(-13);
is($t9->year, 2008);
is($t9->mon, 12);
is($t9->mday, 1);

eval { $t->add_months(); };
like($@, qr/add_months requires a number of months/);

# Tests for Time::Seconds start here
my $s = $t - $t7;
is($s->minutes, 44640);
is($s->hours,     744);
is($s->days,       31);
is(int($s->weeks),  4);
is(int($s->months), 1);
is(int($s->years),  0);

$s2 = $s->copy;
is($s2->minutes, 44640, 'Copy Time::Seconds object');
$s2 = $s->copy + 60;
is($s2->minutes, 44641, 'Add integer to Time::Seconds object');
$s2 += ONE_HOUR;
is($s2->minutes, 44701, 'Add exported constant to Time::Seconds object');
$s2 += $s2;
is($s2->minutes, 89402, 'Add one Time::Seconds object to another');

$s2 += 300 * ONE_DAY;
is(int($s2->financial_months), 12);
is(int($s2->months),           11);

$s2 = Time::Seconds->new();
is($s2->seconds,  0, 'Empty Time::Seconds constructor is 0s');
my $s3 = Time::Seconds->new(10);
$s2 = $s2 + $s3;
is($s2->seconds, 10, 'Add 2 Time::Seconds objects');
$s2 -= $s3;
is($s2->seconds,  0, 'Subtract one Time::Seconds object from another');

eval { $s2 = $s2 + $t; };
like($@, qr/Can't use non Seconds object in operator overload/);
