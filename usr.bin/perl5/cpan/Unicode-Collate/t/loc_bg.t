
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..122\n"; }
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

my $objBg = Unicode::Collate::Locale->
    new(locale => 'BG', normalization => undef);

ok($objBg->getlocale, 'default'); # no tailoring since 1.17

$objBg->change(level => 1);

ok($objBg->lt("\x{430}z", "\x{431}"));
ok($objBg->lt("\x{431}z", "\x{432}"));
ok($objBg->lt("\x{432}z", "\x{433}"));
ok($objBg->lt("\x{433}z", "\x{434}"));
ok($objBg->lt("\x{434}z", "\x{435}"));
ok($objBg->lt("\x{435}z", "\x{436}"));
ok($objBg->lt("\x{436}z", "\x{437}"));
ok($objBg->lt("\x{437}z", "\x{438}"));
ok($objBg->lt("\x{438}z", "\x{439}"));
ok($objBg->lt("\x{439}z", "\x{43a}"));
ok($objBg->lt("\x{43a}z", "\x{43b}"));
ok($objBg->lt("\x{43b}z", "\x{43c}"));
ok($objBg->lt("\x{43c}z", "\x{43d}"));
ok($objBg->lt("\x{43d}z", "\x{43e}"));
ok($objBg->lt("\x{43e}z", "\x{43f}"));
ok($objBg->lt("\x{43f}z", "\x{440}"));
ok($objBg->lt("\x{440}z", "\x{441}"));
ok($objBg->lt("\x{441}z", "\x{442}"));
ok($objBg->lt("\x{442}z", "\x{443}"));
ok($objBg->lt("\x{443}z", "\x{444}"));
ok($objBg->lt("\x{444}z", "\x{445}"));
ok($objBg->lt("\x{445}z", "\x{446}"));
ok($objBg->lt("\x{446}z", "\x{447}"));
ok($objBg->lt("\x{447}z", "\x{448}"));
ok($objBg->lt("\x{448}z", "\x{449}"));
ok($objBg->lt("\x{449}z", "\x{44a}"));
ok($objBg->lt("\x{44a}z", "\x{44c}"));
ok($objBg->lt("\x{44c}z", "\x{44e}"));
ok($objBg->lt("\x{44e}z", "\x{44f}"));

ok($objBg->lt("\x{410}z", "\x{411}"));
ok($objBg->lt("\x{411}z", "\x{412}"));
ok($objBg->lt("\x{412}z", "\x{413}"));
ok($objBg->lt("\x{413}z", "\x{414}"));
ok($objBg->lt("\x{414}z", "\x{415}"));
ok($objBg->lt("\x{415}z", "\x{416}"));
ok($objBg->lt("\x{416}z", "\x{417}"));
ok($objBg->lt("\x{417}z", "\x{418}"));
ok($objBg->lt("\x{418}z", "\x{419}"));
ok($objBg->lt("\x{419}z", "\x{41a}"));
ok($objBg->lt("\x{41a}z", "\x{41b}"));
ok($objBg->lt("\x{41b}z", "\x{41c}"));
ok($objBg->lt("\x{41c}z", "\x{41d}"));
ok($objBg->lt("\x{41d}z", "\x{41e}"));
ok($objBg->lt("\x{41e}z", "\x{41f}"));
ok($objBg->lt("\x{41f}z", "\x{420}"));
ok($objBg->lt("\x{420}z", "\x{421}"));
ok($objBg->lt("\x{421}z", "\x{422}"));
ok($objBg->lt("\x{422}z", "\x{423}"));
ok($objBg->lt("\x{423}z", "\x{424}"));
ok($objBg->lt("\x{424}z", "\x{425}"));
ok($objBg->lt("\x{425}z", "\x{426}"));
ok($objBg->lt("\x{426}z", "\x{427}"));
ok($objBg->lt("\x{427}z", "\x{428}"));
ok($objBg->lt("\x{428}z", "\x{429}"));
ok($objBg->lt("\x{429}z", "\x{42a}"));
ok($objBg->lt("\x{42a}z", "\x{42c}"));
ok($objBg->lt("\x{42c}z", "\x{42e}"));
ok($objBg->lt("\x{42e}z", "\x{42f}"));

# 60

$objBg->change(level => 2);

ok($objBg->eq("\x{430}", "\x{410}"));
ok($objBg->eq("\x{431}", "\x{411}"));
ok($objBg->eq("\x{432}", "\x{412}"));
ok($objBg->eq("\x{433}", "\x{413}"));
ok($objBg->eq("\x{434}", "\x{414}"));
ok($objBg->eq("\x{435}", "\x{415}"));
ok($objBg->eq("\x{436}", "\x{416}"));
ok($objBg->eq("\x{437}", "\x{417}"));
ok($objBg->eq("\x{438}", "\x{418}"));
ok($objBg->eq("\x{439}", "\x{419}"));
ok($objBg->eq("\x{43a}", "\x{41a}"));
ok($objBg->eq("\x{43b}", "\x{41b}"));
ok($objBg->eq("\x{43c}", "\x{41c}"));
ok($objBg->eq("\x{43d}", "\x{41d}"));
ok($objBg->eq("\x{43e}", "\x{41e}"));
ok($objBg->eq("\x{43f}", "\x{41f}"));
ok($objBg->eq("\x{440}", "\x{420}"));
ok($objBg->eq("\x{441}", "\x{421}"));
ok($objBg->eq("\x{442}", "\x{422}"));
ok($objBg->eq("\x{443}", "\x{423}"));
ok($objBg->eq("\x{444}", "\x{424}"));
ok($objBg->eq("\x{445}", "\x{425}"));
ok($objBg->eq("\x{446}", "\x{426}"));
ok($objBg->eq("\x{447}", "\x{427}"));
ok($objBg->eq("\x{448}", "\x{428}"));
ok($objBg->eq("\x{449}", "\x{429}"));
ok($objBg->eq("\x{44a}", "\x{42a}"));
ok($objBg->eq("\x{44c}", "\x{42c}"));
ok($objBg->eq("\x{44e}", "\x{42e}"));
ok($objBg->eq("\x{44f}", "\x{42f}"));

# 90

$objBg->change(level => 3);

ok($objBg->lt("\x{430}", "\x{410}"));
ok($objBg->lt("\x{431}", "\x{411}"));
ok($objBg->lt("\x{432}", "\x{412}"));
ok($objBg->lt("\x{433}", "\x{413}"));
ok($objBg->lt("\x{434}", "\x{414}"));
ok($objBg->lt("\x{435}", "\x{415}"));
ok($objBg->lt("\x{436}", "\x{416}"));
ok($objBg->lt("\x{437}", "\x{417}"));
ok($objBg->lt("\x{438}", "\x{418}"));
ok($objBg->lt("\x{439}", "\x{419}"));
ok($objBg->lt("\x{43a}", "\x{41a}"));
ok($objBg->lt("\x{43b}", "\x{41b}"));
ok($objBg->lt("\x{43c}", "\x{41c}"));
ok($objBg->lt("\x{43d}", "\x{41d}"));
ok($objBg->lt("\x{43e}", "\x{41e}"));
ok($objBg->lt("\x{43f}", "\x{41f}"));
ok($objBg->lt("\x{440}", "\x{420}"));
ok($objBg->lt("\x{441}", "\x{421}"));
ok($objBg->lt("\x{442}", "\x{422}"));
ok($objBg->lt("\x{443}", "\x{423}"));
ok($objBg->lt("\x{444}", "\x{424}"));
ok($objBg->lt("\x{445}", "\x{425}"));
ok($objBg->lt("\x{446}", "\x{426}"));
ok($objBg->lt("\x{447}", "\x{427}"));
ok($objBg->lt("\x{448}", "\x{428}"));
ok($objBg->lt("\x{449}", "\x{429}"));
ok($objBg->lt("\x{44a}", "\x{42a}"));
ok($objBg->lt("\x{44c}", "\x{42c}"));
ok($objBg->lt("\x{44e}", "\x{42e}"));
ok($objBg->lt("\x{44f}", "\x{42f}"));

# 120

ok($objBg->eq("\x{439}", "\x{438}\x{306}"));
ok($objBg->eq("\x{419}", "\x{418}\x{306}"));

# 122
