
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..138\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $objUgCyrl = Unicode::Collate::Locale->
    new(locale => 'UG-CYRL', normalization => undef);

ok($objUgCyrl->getlocale, 'ug_Cyrl');

$objUgCyrl->change(level => 1);

ok($objUgCyrl->lt("\x{430}z", "\x{431}"));
ok($objUgCyrl->lt("\x{431}z", "\x{432}"));
ok($objUgCyrl->lt("\x{432}z", "\x{433}"));
ok($objUgCyrl->lt("\x{433}z", "\x{493}"));
ok($objUgCyrl->lt("\x{493}z", "\x{434}"));
ok($objUgCyrl->lt("\x{434}z", "\x{435}"));
ok($objUgCyrl->lt("\x{435}z", "\x{4d9}"));
ok($objUgCyrl->lt("\x{4d9}z", "\x{436}"));
ok($objUgCyrl->lt("\x{436}z", "\x{497}"));
ok($objUgCyrl->lt("\x{497}z", "\x{437}"));
ok($objUgCyrl->lt("\x{437}z", "\x{438}"));
ok($objUgCyrl->lt("\x{438}z", "\x{439}"));
ok($objUgCyrl->lt("\x{439}z", "\x{43a}"));
ok($objUgCyrl->lt("\x{43a}z", "\x{49b}"));
ok($objUgCyrl->lt("\x{49b}z", "\x{43b}"));
ok($objUgCyrl->lt("\x{43b}z", "\x{43c}"));
ok($objUgCyrl->lt("\x{43c}z", "\x{43d}"));
ok($objUgCyrl->lt("\x{43d}z", "\x{4a3}"));
ok($objUgCyrl->lt("\x{4a3}z", "\x{43e}"));
ok($objUgCyrl->lt("\x{43e}z", "\x{4e9}"));
ok($objUgCyrl->lt("\x{4e9}z", "\x{43f}"));
ok($objUgCyrl->lt("\x{43f}z", "\x{440}"));
ok($objUgCyrl->lt("\x{440}z", "\x{441}"));
ok($objUgCyrl->lt("\x{441}z", "\x{442}"));
ok($objUgCyrl->lt("\x{442}z", "\x{443}"));
ok($objUgCyrl->lt("\x{443}z", "\x{4af}"));
ok($objUgCyrl->lt("\x{4af}z", "\x{444}"));
ok($objUgCyrl->lt("\x{444}z", "\x{445}"));
ok($objUgCyrl->lt("\x{445}z", "\x{4bb}"));
ok($objUgCyrl->lt("\x{4bb}z", "\x{447}"));
ok($objUgCyrl->lt("\x{447}z", "\x{448}"));
ok($objUgCyrl->lt("\x{448}z", "\x{44e}"));
ok($objUgCyrl->lt("\x{44e}z", "\x{44f}"));

ok($objUgCyrl->lt("\x{410}z", "\x{411}"));
ok($objUgCyrl->lt("\x{411}z", "\x{412}"));
ok($objUgCyrl->lt("\x{412}z", "\x{413}"));
ok($objUgCyrl->lt("\x{413}z", "\x{492}"));
ok($objUgCyrl->lt("\x{492}z", "\x{414}"));
ok($objUgCyrl->lt("\x{414}z", "\x{415}"));
ok($objUgCyrl->lt("\x{415}z", "\x{4d8}"));
ok($objUgCyrl->lt("\x{4d8}z", "\x{416}"));
ok($objUgCyrl->lt("\x{416}z", "\x{496}"));
ok($objUgCyrl->lt("\x{496}z", "\x{417}"));
ok($objUgCyrl->lt("\x{417}z", "\x{418}"));
ok($objUgCyrl->lt("\x{418}z", "\x{419}"));
ok($objUgCyrl->lt("\x{419}z", "\x{41a}"));
ok($objUgCyrl->lt("\x{41a}z", "\x{49a}"));
ok($objUgCyrl->lt("\x{49a}z", "\x{41b}"));
ok($objUgCyrl->lt("\x{41b}z", "\x{41c}"));
ok($objUgCyrl->lt("\x{41c}z", "\x{41d}"));
ok($objUgCyrl->lt("\x{41d}z", "\x{4a2}"));
ok($objUgCyrl->lt("\x{4a2}z", "\x{41e}"));
ok($objUgCyrl->lt("\x{41e}z", "\x{4e8}"));
ok($objUgCyrl->lt("\x{4e8}z", "\x{41f}"));
ok($objUgCyrl->lt("\x{41f}z", "\x{420}"));
ok($objUgCyrl->lt("\x{420}z", "\x{421}"));
ok($objUgCyrl->lt("\x{421}z", "\x{422}"));
ok($objUgCyrl->lt("\x{422}z", "\x{423}"));
ok($objUgCyrl->lt("\x{423}z", "\x{4ae}"));
ok($objUgCyrl->lt("\x{4ae}z", "\x{424}"));
ok($objUgCyrl->lt("\x{424}z", "\x{425}"));
ok($objUgCyrl->lt("\x{425}z", "\x{4ba}"));
ok($objUgCyrl->lt("\x{4ba}z", "\x{427}"));
ok($objUgCyrl->lt("\x{427}z", "\x{428}"));
ok($objUgCyrl->lt("\x{428}z", "\x{42e}"));
ok($objUgCyrl->lt("\x{42e}z", "\x{42f}"));

# 68

$objUgCyrl->change(level => 2);

ok($objUgCyrl->eq("\x{430}", "\x{410}"));
ok($objUgCyrl->eq("\x{431}", "\x{411}"));
ok($objUgCyrl->eq("\x{432}", "\x{412}"));
ok($objUgCyrl->eq("\x{433}", "\x{413}"));
ok($objUgCyrl->eq("\x{493}", "\x{492}"));
ok($objUgCyrl->eq("\x{434}", "\x{414}"));
ok($objUgCyrl->eq("\x{435}", "\x{415}"));
ok($objUgCyrl->eq("\x{4d9}", "\x{4d8}"));
ok($objUgCyrl->eq("\x{436}", "\x{416}"));
ok($objUgCyrl->eq("\x{497}", "\x{496}"));
ok($objUgCyrl->eq("\x{437}", "\x{417}"));
ok($objUgCyrl->eq("\x{438}", "\x{418}"));
ok($objUgCyrl->eq("\x{439}", "\x{419}"));
ok($objUgCyrl->eq("\x{43a}", "\x{41a}"));
ok($objUgCyrl->eq("\x{49b}", "\x{49a}"));
ok($objUgCyrl->eq("\x{43b}", "\x{41b}"));
ok($objUgCyrl->eq("\x{43c}", "\x{41c}"));
ok($objUgCyrl->eq("\x{43d}", "\x{41d}"));
ok($objUgCyrl->eq("\x{4a3}", "\x{4a2}"));
ok($objUgCyrl->eq("\x{43e}", "\x{41e}"));
ok($objUgCyrl->eq("\x{4e9}", "\x{4e8}"));
ok($objUgCyrl->eq("\x{43f}", "\x{41f}"));
ok($objUgCyrl->eq("\x{440}", "\x{420}"));
ok($objUgCyrl->eq("\x{441}", "\x{421}"));
ok($objUgCyrl->eq("\x{442}", "\x{422}"));
ok($objUgCyrl->eq("\x{443}", "\x{423}"));
ok($objUgCyrl->eq("\x{4af}", "\x{4ae}"));
ok($objUgCyrl->eq("\x{444}", "\x{424}"));
ok($objUgCyrl->eq("\x{445}", "\x{425}"));
ok($objUgCyrl->eq("\x{4bb}", "\x{4ba}"));
ok($objUgCyrl->eq("\x{447}", "\x{427}"));
ok($objUgCyrl->eq("\x{448}", "\x{428}"));
ok($objUgCyrl->eq("\x{44e}", "\x{42e}"));
ok($objUgCyrl->eq("\x{44f}", "\x{42f}"));

# 102

$objUgCyrl->change(level => 3);

ok($objUgCyrl->lt("\x{430}", "\x{410}"));
ok($objUgCyrl->lt("\x{431}", "\x{411}"));
ok($objUgCyrl->lt("\x{432}", "\x{412}"));
ok($objUgCyrl->lt("\x{433}", "\x{413}"));
ok($objUgCyrl->lt("\x{493}", "\x{492}"));
ok($objUgCyrl->lt("\x{434}", "\x{414}"));
ok($objUgCyrl->lt("\x{435}", "\x{415}"));
ok($objUgCyrl->lt("\x{4d9}", "\x{4d8}"));
ok($objUgCyrl->lt("\x{436}", "\x{416}"));
ok($objUgCyrl->lt("\x{497}", "\x{496}"));
ok($objUgCyrl->lt("\x{437}", "\x{417}"));
ok($objUgCyrl->lt("\x{438}", "\x{418}"));
ok($objUgCyrl->lt("\x{439}", "\x{419}"));
ok($objUgCyrl->lt("\x{43a}", "\x{41a}"));
ok($objUgCyrl->lt("\x{49b}", "\x{49a}"));
ok($objUgCyrl->lt("\x{43b}", "\x{41b}"));
ok($objUgCyrl->lt("\x{43c}", "\x{41c}"));
ok($objUgCyrl->lt("\x{43d}", "\x{41d}"));
ok($objUgCyrl->lt("\x{4a3}", "\x{4a2}"));
ok($objUgCyrl->lt("\x{43e}", "\x{41e}"));
ok($objUgCyrl->lt("\x{4e9}", "\x{4e8}"));
ok($objUgCyrl->lt("\x{43f}", "\x{41f}"));
ok($objUgCyrl->lt("\x{440}", "\x{420}"));
ok($objUgCyrl->lt("\x{441}", "\x{421}"));
ok($objUgCyrl->lt("\x{442}", "\x{422}"));
ok($objUgCyrl->lt("\x{443}", "\x{423}"));
ok($objUgCyrl->lt("\x{4af}", "\x{4ae}"));
ok($objUgCyrl->lt("\x{444}", "\x{424}"));
ok($objUgCyrl->lt("\x{445}", "\x{425}"));
ok($objUgCyrl->lt("\x{4bb}", "\x{4ba}"));
ok($objUgCyrl->lt("\x{447}", "\x{427}"));
ok($objUgCyrl->lt("\x{448}", "\x{428}"));
ok($objUgCyrl->lt("\x{44e}", "\x{42e}"));
ok($objUgCyrl->lt("\x{44f}", "\x{42f}"));

# 136

ok($objUgCyrl->eq("\x{439}", "\x{438}\x{306}"));
ok($objUgCyrl->eq("\x{419}", "\x{418}\x{306}"));

# 138
