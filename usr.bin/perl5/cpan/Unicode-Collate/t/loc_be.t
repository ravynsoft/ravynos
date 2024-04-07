
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

my $objBe = Unicode::Collate::Locale->
    new(locale => 'BE', normalization => undef);

ok($objBe->getlocale, 'be');

$objBe->change(level => 1);

ok($objBe->lt("\x{430}z", "\x{431}"));
ok($objBe->lt("\x{431}z", "\x{432}"));
ok($objBe->lt("\x{432}z", "\x{433}"));
ok($objBe->lt("\x{433}z", "\x{434}"));
ok($objBe->lt("\x{434}z", "\x{435}"));
ok($objBe->lt("\x{435}z", "\x{451}"));
ok($objBe->lt("\x{451}z", "\x{436}"));
ok($objBe->lt("\x{436}z", "\x{437}"));
ok($objBe->lt("\x{437}z", "\x{438}"));
ok($objBe->lt("\x{438}z", "\x{456}"));
ok($objBe->lt("\x{456}z", "\x{439}"));
ok($objBe->lt("\x{439}z", "\x{43a}"));
ok($objBe->lt("\x{43a}z", "\x{43b}"));
ok($objBe->lt("\x{43b}z", "\x{43c}"));
ok($objBe->lt("\x{43c}z", "\x{43d}"));
ok($objBe->lt("\x{43d}z", "\x{43e}"));
ok($objBe->lt("\x{43e}z", "\x{43f}"));
ok($objBe->lt("\x{43f}z", "\x{440}"));
ok($objBe->lt("\x{440}z", "\x{441}"));
ok($objBe->lt("\x{441}z", "\x{442}"));
ok($objBe->lt("\x{442}z", "\x{443}"));
ok($objBe->lt("\x{443}z", "\x{45e}"));
ok($objBe->lt("\x{45e}z", "\x{444}"));
ok($objBe->lt("\x{444}z", "\x{445}"));
ok($objBe->lt("\x{445}z", "\x{446}"));
ok($objBe->lt("\x{446}z", "\x{447}"));
ok($objBe->lt("\x{447}z", "\x{448}"));
ok($objBe->lt("\x{448}z", "\x{44b}"));
ok($objBe->lt("\x{44b}z", "\x{44c}"));
ok($objBe->lt("\x{44c}z", "\x{44d}"));
ok($objBe->lt("\x{44d}z", "\x{44e}"));
ok($objBe->lt("\x{44e}z", "\x{44f}"));

ok($objBe->lt("\x{410}z", "\x{411}"));
ok($objBe->lt("\x{411}z", "\x{412}"));
ok($objBe->lt("\x{412}z", "\x{413}"));
ok($objBe->lt("\x{413}z", "\x{414}"));
ok($objBe->lt("\x{414}z", "\x{415}"));
ok($objBe->lt("\x{415}z", "\x{401}"));
ok($objBe->lt("\x{401}z", "\x{416}"));
ok($objBe->lt("\x{416}z", "\x{417}"));
ok($objBe->lt("\x{417}z", "\x{418}"));
ok($objBe->lt("\x{418}z", "\x{406}"));
ok($objBe->lt("\x{406}z", "\x{419}"));
ok($objBe->lt("\x{419}z", "\x{41a}"));
ok($objBe->lt("\x{41a}z", "\x{41b}"));
ok($objBe->lt("\x{41b}z", "\x{41c}"));
ok($objBe->lt("\x{41c}z", "\x{41d}"));
ok($objBe->lt("\x{41d}z", "\x{41e}"));
ok($objBe->lt("\x{41e}z", "\x{41f}"));
ok($objBe->lt("\x{41f}z", "\x{420}"));
ok($objBe->lt("\x{420}z", "\x{421}"));
ok($objBe->lt("\x{421}z", "\x{422}"));
ok($objBe->lt("\x{422}z", "\x{423}"));
ok($objBe->lt("\x{423}z", "\x{40e}"));
ok($objBe->lt("\x{40e}z", "\x{424}"));
ok($objBe->lt("\x{424}z", "\x{425}"));
ok($objBe->lt("\x{425}z", "\x{426}"));
ok($objBe->lt("\x{426}z", "\x{427}"));
ok($objBe->lt("\x{427}z", "\x{428}"));
ok($objBe->lt("\x{428}z", "\x{42b}"));
ok($objBe->lt("\x{42b}z", "\x{42c}"));
ok($objBe->lt("\x{42c}z", "\x{42d}"));
ok($objBe->lt("\x{42d}z", "\x{42e}"));
ok($objBe->lt("\x{42e}z", "\x{42f}"));

# 66

$objBe->change(level => 2);

ok($objBe->eq("\x{430}", "\x{410}"));
ok($objBe->eq("\x{431}", "\x{411}"));
ok($objBe->eq("\x{432}", "\x{412}"));
ok($objBe->eq("\x{433}", "\x{413}"));
ok($objBe->eq("\x{434}", "\x{414}"));
ok($objBe->eq("\x{435}", "\x{415}"));
ok($objBe->eq("\x{451}", "\x{401}"));
ok($objBe->eq("\x{436}", "\x{416}"));
ok($objBe->eq("\x{437}", "\x{417}"));
ok($objBe->eq("\x{438}", "\x{418}"));
ok($objBe->eq("\x{456}", "\x{406}"));
ok($objBe->eq("\x{439}", "\x{419}"));
ok($objBe->eq("\x{43a}", "\x{41a}"));
ok($objBe->eq("\x{43b}", "\x{41b}"));
ok($objBe->eq("\x{43c}", "\x{41c}"));
ok($objBe->eq("\x{43d}", "\x{41d}"));
ok($objBe->eq("\x{43e}", "\x{41e}"));
ok($objBe->eq("\x{43f}", "\x{41f}"));
ok($objBe->eq("\x{440}", "\x{420}"));
ok($objBe->eq("\x{441}", "\x{421}"));
ok($objBe->eq("\x{442}", "\x{422}"));
ok($objBe->eq("\x{443}", "\x{423}"));
ok($objBe->eq("\x{45e}", "\x{40e}"));
ok($objBe->eq("\x{444}", "\x{424}"));
ok($objBe->eq("\x{445}", "\x{425}"));
ok($objBe->eq("\x{446}", "\x{426}"));
ok($objBe->eq("\x{447}", "\x{427}"));
ok($objBe->eq("\x{448}", "\x{428}"));
ok($objBe->eq("\x{44b}", "\x{42b}"));
ok($objBe->eq("\x{44c}", "\x{42c}"));
ok($objBe->eq("\x{44d}", "\x{42d}"));
ok($objBe->eq("\x{44e}", "\x{42e}"));
ok($objBe->eq("\x{44f}", "\x{42f}"));

# 99

$objBe->change(level => 3);

ok($objBe->lt("\x{430}", "\x{410}"));
ok($objBe->lt("\x{431}", "\x{411}"));
ok($objBe->lt("\x{432}", "\x{412}"));
ok($objBe->lt("\x{433}", "\x{413}"));
ok($objBe->lt("\x{434}", "\x{414}"));
ok($objBe->lt("\x{435}", "\x{415}"));
ok($objBe->lt("\x{451}", "\x{401}"));
ok($objBe->lt("\x{436}", "\x{416}"));
ok($objBe->lt("\x{437}", "\x{417}"));
ok($objBe->lt("\x{438}", "\x{418}"));
ok($objBe->lt("\x{456}", "\x{406}"));
ok($objBe->lt("\x{439}", "\x{419}"));
ok($objBe->lt("\x{43a}", "\x{41a}"));
ok($objBe->lt("\x{43b}", "\x{41b}"));
ok($objBe->lt("\x{43c}", "\x{41c}"));
ok($objBe->lt("\x{43d}", "\x{41d}"));
ok($objBe->lt("\x{43e}", "\x{41e}"));
ok($objBe->lt("\x{43f}", "\x{41f}"));
ok($objBe->lt("\x{440}", "\x{420}"));
ok($objBe->lt("\x{441}", "\x{421}"));
ok($objBe->lt("\x{442}", "\x{422}"));
ok($objBe->lt("\x{443}", "\x{423}"));
ok($objBe->lt("\x{45e}", "\x{40e}"));
ok($objBe->lt("\x{444}", "\x{424}"));
ok($objBe->lt("\x{445}", "\x{425}"));
ok($objBe->lt("\x{446}", "\x{426}"));
ok($objBe->lt("\x{447}", "\x{427}"));
ok($objBe->lt("\x{448}", "\x{428}"));
ok($objBe->lt("\x{44b}", "\x{42b}"));
ok($objBe->lt("\x{44c}", "\x{42c}"));
ok($objBe->lt("\x{44d}", "\x{42d}"));
ok($objBe->lt("\x{44e}", "\x{42e}"));
ok($objBe->lt("\x{44f}", "\x{42f}"));

# 132

ok($objBe->eq("\x{451}", "\x{435}\x{308}"));
ok($objBe->eq("\x{401}", "\x{415}\x{308}"));
ok($objBe->eq("\x{439}", "\x{438}\x{306}"));
ok($objBe->eq("\x{419}", "\x{418}\x{306}"));
ok($objBe->eq("\x{45e}", "\x{443}\x{306}"));
ok($objBe->eq("\x{40e}", "\x{423}\x{306}"));

# 138
