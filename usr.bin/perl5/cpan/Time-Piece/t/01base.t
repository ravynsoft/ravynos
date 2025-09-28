use Test::More tests => 15;

BEGIN { use_ok('Time::Piece'); }

my $t = gmtime(315532800); # 00:00:00 1/1/1980

isa_ok($t, 'Time::Piece', 'specific gmtime');

cmp_ok($t->year, '==', 1980, 'correct year');

cmp_ok($t->hour, '==',    0, 'correct hour');

cmp_ok($t->mon,  '==',    1, 'correct mon');

my $g = gmtime;
isa_ok($g, 'Time::Piece', 'current gmtime');

my $l = localtime;
isa_ok($l, 'Time::Piece', 'current localtime');

#without export
$g = Time::Piece::gmtime;
isa_ok($g, 'Time::Piece', 'fully qualified gmtime');

$l = Time::Piece::localtime;
isa_ok($l, 'Time::Piece', 'full qualified localtime');

#via new
$l = Time::Piece->new(315532800);
isa_ok($l, 'Time::Piece', 'custom localtime via new');

#via new again
$l = $l->new();
isa_ok($l, 'Time::Piece', 'custom localtime via new again');

#via clone
my $l_clone = Time::Piece->new($l);
isa_ok($l, 'Time::Piece', 'custom localtime via clone');
cmp_ok("$l_clone", 'eq', "$l", 'Clones match');

#via clone with gmtime
my $g_clone = Time::Piece->new($g);
isa_ok($g, 'Time::Piece', 'custom gmtime via clone');
cmp_ok("$g_clone", 'eq', "$g", 'Clones match');
