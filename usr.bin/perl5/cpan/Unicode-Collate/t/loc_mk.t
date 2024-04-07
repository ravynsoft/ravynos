
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..148\n"; }
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

my $objMk = Unicode::Collate::Locale->
    new(locale => 'MK', normalization => undef);

ok($objMk->getlocale, 'mk');

$objMk->change(level => 1);

ok($objMk->lt("\x{430}z", "\x{431}"));
ok($objMk->lt("\x{431}z", "\x{432}"));
ok($objMk->lt("\x{432}z", "\x{433}"));
ok($objMk->lt("\x{433}z", "\x{434}"));
ok($objMk->lt("\x{434}z", "\x{503}"));
ok($objMk->lt("\x{503}z", "\x{453}"));
ok($objMk->lt("\x{453}z", "\x{435}"));
ok($objMk->lt("\x{435}z", "\x{436}"));
ok($objMk->lt("\x{436}z", "\x{437}"));
ok($objMk->lt("\x{437}z", "\x{455}"));
ok($objMk->lt("\x{455}z", "\x{438}"));
ok($objMk->lt("\x{438}z", "\x{458}"));
ok($objMk->lt("\x{458}z", "\x{43a}"));
ok($objMk->lt("\x{43a}z", "\x{43b}"));
ok($objMk->lt("\x{43b}z", "\x{459}"));
ok($objMk->lt("\x{459}z", "\x{43c}"));
ok($objMk->lt("\x{43c}z", "\x{43d}"));
ok($objMk->lt("\x{43d}z", "\x{45a}"));
ok($objMk->lt("\x{45a}z", "\x{43e}"));
ok($objMk->lt("\x{43e}z", "\x{43f}"));
ok($objMk->lt("\x{43f}z", "\x{440}"));
ok($objMk->lt("\x{440}z", "\x{441}"));
ok($objMk->lt("\x{441}z", "\x{442}"));
ok($objMk->lt("\x{442}z", "\x{45b}"));
ok($objMk->lt("\x{45b}z", "\x{45c}"));
ok($objMk->lt("\x{45c}z", "\x{443}"));
ok($objMk->lt("\x{443}z", "\x{444}"));
ok($objMk->lt("\x{444}z", "\x{445}"));
ok($objMk->lt("\x{445}z", "\x{446}"));
ok($objMk->lt("\x{446}z", "\x{447}"));
ok($objMk->lt("\x{447}z", "\x{45f}"));
ok($objMk->lt("\x{45f}z", "\x{448}"));

ok($objMk->lt("\x{410}z", "\x{411}"));
ok($objMk->lt("\x{411}z", "\x{412}"));
ok($objMk->lt("\x{412}z", "\x{413}"));
ok($objMk->lt("\x{413}z", "\x{414}"));
ok($objMk->lt("\x{414}z", "\x{502}"));
ok($objMk->lt("\x{502}z", "\x{403}"));
ok($objMk->lt("\x{403}z", "\x{415}"));
ok($objMk->lt("\x{415}z", "\x{416}"));
ok($objMk->lt("\x{416}z", "\x{417}"));
ok($objMk->lt("\x{417}z", "\x{405}"));
ok($objMk->lt("\x{405}z", "\x{418}"));
ok($objMk->lt("\x{418}z", "\x{408}"));
ok($objMk->lt("\x{408}z", "\x{41a}"));
ok($objMk->lt("\x{41a}z", "\x{41b}"));
ok($objMk->lt("\x{41b}z", "\x{409}"));
ok($objMk->lt("\x{409}z", "\x{41c}"));
ok($objMk->lt("\x{41c}z", "\x{41d}"));
ok($objMk->lt("\x{41d}z", "\x{40a}"));
ok($objMk->lt("\x{40a}z", "\x{41e}"));
ok($objMk->lt("\x{41e}z", "\x{41f}"));
ok($objMk->lt("\x{41f}z", "\x{420}"));
ok($objMk->lt("\x{420}z", "\x{421}"));
ok($objMk->lt("\x{421}z", "\x{422}"));
ok($objMk->lt("\x{422}z", "\x{40b}"));
ok($objMk->lt("\x{40b}z", "\x{40c}"));
ok($objMk->lt("\x{40c}z", "\x{423}"));
ok($objMk->lt("\x{423}z", "\x{424}"));
ok($objMk->lt("\x{424}z", "\x{425}"));
ok($objMk->lt("\x{425}z", "\x{426}"));
ok($objMk->lt("\x{426}z", "\x{427}"));
ok($objMk->lt("\x{427}z", "\x{40f}"));
ok($objMk->lt("\x{40f}z", "\x{428}"));

# 66

ok($objMk->eq("\x{438}", "\x{439}"));
ok($objMk->eq("\x{418}", "\x{419}"));

# 68

$objMk->change(level => 2);

ok($objMk->lt("\x{438}", "\x{439}"));
ok($objMk->lt("\x{418}", "\x{419}"));

# 70

ok($objMk->eq("\x{430}", "\x{410}"));
ok($objMk->eq("\x{431}", "\x{411}"));
ok($objMk->eq("\x{432}", "\x{412}"));
ok($objMk->eq("\x{433}", "\x{413}"));
ok($objMk->eq("\x{434}", "\x{414}"));
ok($objMk->eq("\x{503}", "\x{502}"));
ok($objMk->eq("\x{453}", "\x{403}"));
ok($objMk->eq("\x{435}", "\x{415}"));
ok($objMk->eq("\x{436}", "\x{416}"));
ok($objMk->eq("\x{437}", "\x{417}"));
ok($objMk->eq("\x{455}", "\x{405}"));
ok($objMk->eq("\x{438}", "\x{418}"));
ok($objMk->eq("\x{439}", "\x{419}"));
ok($objMk->eq("\x{458}", "\x{408}"));
ok($objMk->eq("\x{43a}", "\x{41a}"));
ok($objMk->eq("\x{43b}", "\x{41b}"));
ok($objMk->eq("\x{459}", "\x{409}"));
ok($objMk->eq("\x{43c}", "\x{41c}"));
ok($objMk->eq("\x{43d}", "\x{41d}"));
ok($objMk->eq("\x{45a}", "\x{40a}"));
ok($objMk->eq("\x{43e}", "\x{41e}"));
ok($objMk->eq("\x{43f}", "\x{41f}"));
ok($objMk->eq("\x{440}", "\x{420}"));
ok($objMk->eq("\x{441}", "\x{421}"));
ok($objMk->eq("\x{442}", "\x{422}"));
ok($objMk->eq("\x{45b}", "\x{40b}"));
ok($objMk->eq("\x{45c}", "\x{40c}"));
ok($objMk->eq("\x{443}", "\x{423}"));
ok($objMk->eq("\x{444}", "\x{424}"));
ok($objMk->eq("\x{445}", "\x{425}"));
ok($objMk->eq("\x{446}", "\x{426}"));
ok($objMk->eq("\x{447}", "\x{427}"));
ok($objMk->eq("\x{45f}", "\x{40f}"));
ok($objMk->eq("\x{448}", "\x{428}"));

# 104

$objMk->change(level => 3);

ok($objMk->lt("\x{430}", "\x{410}"));
ok($objMk->lt("\x{431}", "\x{411}"));
ok($objMk->lt("\x{432}", "\x{412}"));
ok($objMk->lt("\x{433}", "\x{413}"));
ok($objMk->lt("\x{434}", "\x{414}"));
ok($objMk->lt("\x{503}", "\x{502}"));
ok($objMk->lt("\x{453}", "\x{403}"));
ok($objMk->lt("\x{435}", "\x{415}"));
ok($objMk->lt("\x{436}", "\x{416}"));
ok($objMk->lt("\x{437}", "\x{417}"));
ok($objMk->lt("\x{455}", "\x{405}"));
ok($objMk->lt("\x{438}", "\x{418}"));
ok($objMk->lt("\x{439}", "\x{419}"));
ok($objMk->lt("\x{458}", "\x{408}"));
ok($objMk->lt("\x{43a}", "\x{41a}"));
ok($objMk->lt("\x{43b}", "\x{41b}"));
ok($objMk->lt("\x{459}", "\x{409}"));
ok($objMk->lt("\x{43c}", "\x{41c}"));
ok($objMk->lt("\x{43d}", "\x{41d}"));
ok($objMk->lt("\x{45a}", "\x{40a}"));
ok($objMk->lt("\x{43e}", "\x{41e}"));
ok($objMk->lt("\x{43f}", "\x{41f}"));
ok($objMk->lt("\x{440}", "\x{420}"));
ok($objMk->lt("\x{441}", "\x{421}"));
ok($objMk->lt("\x{442}", "\x{422}"));
ok($objMk->lt("\x{45b}", "\x{40b}"));
ok($objMk->lt("\x{45c}", "\x{40c}"));
ok($objMk->lt("\x{443}", "\x{423}"));
ok($objMk->lt("\x{444}", "\x{424}"));
ok($objMk->lt("\x{445}", "\x{425}"));
ok($objMk->lt("\x{446}", "\x{426}"));
ok($objMk->lt("\x{447}", "\x{427}"));
ok($objMk->lt("\x{45f}", "\x{40f}"));
ok($objMk->lt("\x{448}", "\x{428}"));

# 138

ok($objMk->eq("\x{453}", "\x{433}\x{301}"));
ok($objMk->eq("\x{453}", "\x{433}\x{341}"));
ok($objMk->eq("\x{403}", "\x{413}\x{301}"));
ok($objMk->eq("\x{403}", "\x{413}\x{341}"));
ok($objMk->eq("\x{439}", "\x{438}\x{306}"));
ok($objMk->eq("\x{419}", "\x{418}\x{306}"));
ok($objMk->eq("\x{45c}", "\x{43a}\x{301}"));
ok($objMk->eq("\x{45c}", "\x{43a}\x{341}"));
ok($objMk->eq("\x{40c}", "\x{41a}\x{301}"));
ok($objMk->eq("\x{40c}", "\x{41a}\x{341}"));

# 148
