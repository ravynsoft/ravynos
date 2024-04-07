
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..616\n"; }
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

my $objCu = Unicode::Collate::Locale->
    new(locale => 'CU', normalization => undef);

ok($objCu->getlocale, 'cu');

# 2

### diacritical marks and combining letters ###

# tailored secondary collation elements
my @sec = (
  "\x{487}",  "\x{485}",  "\x{486}",  "\x{301}",  "\x{300}",  "\x{311}",
  "\x{483}",  "\x{306}",  "\x{308}",  "\x{2DF6}", "\x{2DE0}", "\x{2DE1}",
  "\x{2DE2}", "\x{2DE3}", "\x{2DF7}", "\x{A674}", "\x{2DE4}", "\x{2DE5}",
  "\x{A675}", "\x{A676}", "\x{2DE6}", "\x{2DE7}", "\x{2DE8}", "\x{2DE9}",
  "\x{2DEA}", "\x{A67B}", "\x{2DEB}", "\x{2DEC}", "\x{2DED}", "\x{2DEE}",
  "\x{2DF9}", "\x{A677}", "\x{A69E}", "\x{2DEF}", "\x{2DF0}", "\x{2DF1}",
  "\x{2DF2}", "\x{2DF3}", "\x{33E}",  "\x{A679}", "\x{A67F}", "\x{2DFA}",
  "\x{2DFB}", "\x{2DFE}", "\x{2DFC}", "\x{2DFD}", "\x{2DF4}", "\x{332}",
  "\x{327}",  "\x{328}" # CEDILLA and OGONEK (not tailored)
);

$objCu->change(level => 1);

for (my $i = 1; $i < @sec; $i++) {
    ok($objCu->eq("a$sec[$i-1]", "a$sec[$i]"));
}

# 51

$objCu->change(level => 2);

for (my $i = 1; $i < @sec; $i++) {
    ok($objCu->lt("a$sec[$i-1]", "a$sec[$i]"));
}

ok($objCu->gt("a\x{309A}", "a\x{3099}")); # KANA VOICED and SEMI-VOICED

for (my $i = 1; $i < @sec; $i++) {
    ok($objCu->lt("a\x{309A}b$sec[$i-1]", "a\x{3099}b$sec[$i]"));
}

# 150

$objCu->change(backwards => undef);

for (my $i = 1; $i < @sec; $i++) {
    ok($objCu->lt("a$sec[$i-1]", "a$sec[$i]"));
}

ok($objCu->gt("a\x{309A}", "a\x{3099}")); # KANA VOICED and SEMI-VOICED

for (my $i = 1; $i < @sec; $i++) {
    ok($objCu->gt("a\x{309A}b$sec[$i-1]", "a\x{3099}b$sec[$i]"));
}

# 249

$objCu->change(level => 3);

ok($objCu->eq("", "\x{0487}"));
ok($objCu->eq("", "\x{A67C}"));
ok($objCu->eq("", "\x{A67E}"));

ok($objCu->eq("a\x{487}", "a\x{A67C}"));
ok($objCu->eq("a\x{487}", "a\x{A67E}"));
ok($objCu->eq("a\x{308}", "a\x{30F}"));
ok($objCu->eq("a\x{33E}", "a\x{A678}"));
ok($objCu->eq("a\x{33E}", "a\x{2E2F}"));
ok($objCu->eq("a\x{A67F}","a\x{A67D}"));
ok($objCu->eq("a\x{A67F}","a\x{A67A}"));

ok($objCu->eq("a\x{2DF5}","a\x{2DED}\x{2DEE}"));

# 260

### normal letters ###

$objCu->change(level => 1);

ok($objCu->lt("\x{410}z", "\x{411}"));
ok($objCu->lt("\x{411}z", "\x{412}"));
ok($objCu->lt("\x{412}z", "\x{413}"));
ok($objCu->lt("\x{413}z", "\x{414}"));
ok($objCu->lt("\x{414}z", "\x{415}"));
ok($objCu->lt("\x{415}z", "\x{416}"));
ok($objCu->lt("\x{416}z", "\x{405}")); # Dze
ok($objCu->lt("\x{405}z", "\x{417}"));
ok($objCu->lt("\x{417}z", "\x{418}"));
ok($objCu->lt("\x{418}z", "\x{406}")); # Byel-Ukr I
ok($objCu->lt("\x{406}z", "\x{41a}"));
ok($objCu->lt("\x{41a}z", "\x{41b}"));
ok($objCu->lt("\x{41b}z", "\x{41c}"));
ok($objCu->lt("\x{41c}z", "\x{41d}"));
ok($objCu->lt("\x{41d}z", "\x{40a}")); # Nje
ok($objCu->lt("\x{40a}z", "\x{50a}")); # Komi Nje
ok($objCu->lt("\x{50a}z", "\x{41e}"));
ok($objCu->lt("\x{41e}z", "\x{41f}"));
ok($objCu->lt("\x{41f}z", "\x{420}"));
ok($objCu->lt("\x{420}z", "\x{421}"));
ok($objCu->lt("\x{421}z", "\x{422}"));
ok($objCu->lt("\x{422}z", "\x{40b}")); # Tshe
ok($objCu->lt("\x{40b}z", "\x{423}"));
ok($objCu->lt("\x{423}z", "\x{424}"));
ok($objCu->lt("\x{424}z", "\x{425}"));
ok($objCu->lt("\x{425}z", "\x{426}"));
ok($objCu->lt("\x{426}z", "\x{427}"));
ok($objCu->lt("\x{427}z", "\x{428}"));
ok($objCu->lt("\x{428}z", "\x{429}"));
ok($objCu->lt("\x{429}z", "\x{42a}"));
ok($objCu->lt("\x{42a}z", "\x{42b}"));
ok($objCu->lt("\x{42b}z", "\x{42c}"));
ok($objCu->lt("\x{42c}z", "\x{42d}"));
ok($objCu->lt("\x{42d}z", "\x{462}")); # Yat
ok($objCu->lt("\x{462}z", "\x{42e}"));
ok($objCu->lt("\x{42e}z", "\x{46a}")); # Big Yus
ok($objCu->lt("\x{46a}z", "\x{42f}"));
ok($objCu->lt("\x{42f}z", "\x{466}")); # Little Yus
ok($objCu->lt("\x{466}z", "\x{46e}")); # Ksi
ok($objCu->lt("\x{46e}z", "\x{470}")); # Psi
ok($objCu->lt("\x{470}z", "\x{472}")); # Fita
ok($objCu->lt("\x{472}z", "\x{474}")); # Izhitsa

# 302

$objCu->change(level => 2);

ok($objCu->eq("\x{430}", "\x{410}"));
ok($objCu->eq("\x{431}", "\x{411}"));
ok($objCu->eq("\x{432}", "\x{412}"));
ok($objCu->eq("\x{433}", "\x{413}"));
ok($objCu->eq("\x{434}", "\x{414}"));
ok($objCu->eq("\x{435}", "\x{415}"));
ok($objCu->eq("\x{436}", "\x{416}"));
ok($objCu->eq("\x{455}", "\x{405}")); # Dze
ok($objCu->eq("\x{437}", "\x{417}"));
ok($objCu->eq("\x{438}", "\x{418}"));
ok($objCu->eq("\x{456}", "\x{406}")); # Byel-Ukr I
ok($objCu->eq("\x{43a}", "\x{41a}"));
ok($objCu->eq("\x{43b}", "\x{41b}"));
ok($objCu->eq("\x{43c}", "\x{41c}"));
ok($objCu->eq("\x{43d}", "\x{41d}"));
ok($objCu->eq("\x{45a}", "\x{40a}")); # Nje
ok($objCu->eq("\x{50b}", "\x{50a}")); # Komi Nje
ok($objCu->eq("\x{43e}", "\x{41e}"));
ok($objCu->eq("\x{43f}", "\x{41f}"));
ok($objCu->eq("\x{440}", "\x{420}"));
ok($objCu->eq("\x{441}", "\x{421}"));
ok($objCu->eq("\x{442}", "\x{422}"));
ok($objCu->eq("\x{45b}", "\x{40b}")); # Tshe
ok($objCu->eq("\x{443}", "\x{423}"));
ok($objCu->eq("\x{444}", "\x{424}"));
ok($objCu->eq("\x{445}", "\x{425}"));
ok($objCu->eq("\x{446}", "\x{426}"));
ok($objCu->eq("\x{447}", "\x{427}"));
ok($objCu->eq("\x{448}", "\x{428}"));
ok($objCu->eq("\x{449}", "\x{429}"));
ok($objCu->eq("\x{44a}", "\x{42a}"));
ok($objCu->eq("\x{44b}", "\x{42b}"));
ok($objCu->eq("\x{44c}", "\x{42c}"));
ok($objCu->eq("\x{44d}", "\x{42d}"));
ok($objCu->eq("\x{463}", "\x{462}")); # Yat
ok($objCu->eq("\x{44e}", "\x{42e}"));
ok($objCu->eq("\x{46b}", "\x{46a}")); # Big Yus
ok($objCu->eq("\x{44f}", "\x{42f}"));
ok($objCu->eq("\x{467}", "\x{466}")); # Little Yus
ok($objCu->eq("\x{46f}", "\x{46e}")); # Ksi
ok($objCu->eq("\x{471}", "\x{470}")); # Psi
ok($objCu->eq("\x{473}", "\x{472}")); # Fita
ok($objCu->eq("\x{475}", "\x{474}")); # Izhitsa

# 345

ok($objCu->eq("\x{435}", "\x{454}")); # ie / ukr ie
ok($objCu->eq("\x{435}", "\x{404}")); # ie / UKR IE
ok($objCu->eq("\x{415}", "\x{454}")); # IE / ukr ie
ok($objCu->eq("\x{415}", "\x{404}")); # IE / UKR IE
ok($objCu->eq("\x{454}", "\x{404}")); # ukr ie / UKR IE

ok($objCu->eq("\x{47B}", "\x{47A}")); # round omega / ROUND OMEGA
ok($objCu->eq("\x{47B}", "\x{43E}")); # round omega / o
ok($objCu->eq("\x{47B}", "\x{41E}")); # round omega / O
ok($objCu->eq("\x{47A}", "\x{43E}")); # ROUND OMEGA / o
ok($objCu->eq("\x{47A}", "\x{41E}")); # ROUND OMEGA / O
ok($objCu->eq("\x{43E}", "\x{461}")); # o / omega
ok($objCu->eq("\x{43E}", "\x{460}")); # o / OMEGA
ok($objCu->eq("\x{41E}", "\x{461}")); # O / omega
ok($objCu->eq("\x{41E}", "\x{460}")); # O / OMEGA
ok($objCu->eq("\x{461}", "\x{460}")); # omega / OMEGA
ok($objCu->eq("\x{461}", "\x{A64D}")); # omega / broad omega
ok($objCu->eq("\x{461}", "\x{A64C}")); # omega / BROAD OMEGA
ok($objCu->eq("\x{460}", "\x{A64D}")); # OMEGA / broad omega
ok($objCu->eq("\x{460}", "\x{A64C}")); # OMEGA / BROAD OMEGA
ok($objCu->eq("\x{A64D}","\x{A64C}")); # broad omega / BROAD OMEGA

ok($objCu->eq("\x{479}", "\x{478}"));  # uk / UK
ok($objCu->eq("\x{479}", "\x{A64B}")); # uk / monograph uk
ok($objCu->eq("\x{479}", "\x{A64A}")); # uk / MONOGRAPH UK
ok($objCu->eq("\x{478}", "\x{A64B}")); # UK / monograph uk
ok($objCu->eq("\x{478}", "\x{A64A}")); # UK / MONOGRAPH UK
ok($objCu->eq("\x{A64B}","\x{A64A}")); # monograph uk / MONOGRAPH UK
ok($objCu->eq("\x{A64B}","\x{443}"));  # monograph uk / u
ok($objCu->eq("\x{A64B}","\x{423}"));  # monograph uk / U
ok($objCu->eq("\x{A64A}","\x{443}"));  # MONOGRAPH UK / u
ok($objCu->eq("\x{A64A}","\x{423}"));  # MONOGRAPH UK / U

ok($objCu->eq("\x{A657}","\x{A656}")); # iotified a / IOTIFIED A
ok($objCu->eq("\x{A657}","\x{467}"));  # iotified a / little yus
ok($objCu->eq("\x{A657}","\x{466}"));  # iotified a / LITTLE YUS
ok($objCu->eq("\x{A656}","\x{467}"));  # IOTIFIED A / little yus
ok($objCu->eq("\x{A656}","\x{466}"));  # IOTIFIED A / LITTLE YUS

# 380

$objCu->change(level => 3);

ok($objCu->gt("\x{430}", "\x{410}"));
ok($objCu->gt("\x{431}", "\x{411}"));
ok($objCu->gt("\x{432}", "\x{412}"));
ok($objCu->gt("\x{433}", "\x{413}"));
ok($objCu->gt("\x{434}", "\x{414}"));
ok($objCu->gt("\x{435}", "\x{415}"));
ok($objCu->gt("\x{436}", "\x{416}"));
ok($objCu->gt("\x{455}", "\x{405}")); # Dze
ok($objCu->gt("\x{437}", "\x{417}"));
ok($objCu->gt("\x{438}", "\x{418}"));
ok($objCu->gt("\x{456}", "\x{406}")); # Byel-Ukr I
ok($objCu->gt("\x{43a}", "\x{41a}"));
ok($objCu->gt("\x{43b}", "\x{41b}"));
ok($objCu->gt("\x{43c}", "\x{41c}"));
ok($objCu->gt("\x{43d}", "\x{41d}"));
ok($objCu->gt("\x{45a}", "\x{40a}")); # Nje
ok($objCu->gt("\x{50b}", "\x{50a}")); # Komi Nje
ok($objCu->gt("\x{43e}", "\x{41e}"));
ok($objCu->gt("\x{43f}", "\x{41f}"));
ok($objCu->gt("\x{440}", "\x{420}"));
ok($objCu->gt("\x{441}", "\x{421}"));
ok($objCu->gt("\x{442}", "\x{422}"));
ok($objCu->gt("\x{45b}", "\x{40b}")); # Tshe
ok($objCu->gt("\x{443}", "\x{423}"));
ok($objCu->gt("\x{444}", "\x{424}"));
ok($objCu->gt("\x{445}", "\x{425}"));
ok($objCu->gt("\x{446}", "\x{426}"));
ok($objCu->gt("\x{447}", "\x{427}"));
ok($objCu->gt("\x{448}", "\x{428}"));
ok($objCu->gt("\x{449}", "\x{429}"));
ok($objCu->gt("\x{44a}", "\x{42a}"));
ok($objCu->gt("\x{44b}", "\x{42b}"));
ok($objCu->gt("\x{44c}", "\x{42c}"));
ok($objCu->gt("\x{44d}", "\x{42d}"));
ok($objCu->gt("\x{463}", "\x{462}")); # Yat
ok($objCu->gt("\x{44e}", "\x{42e}"));
ok($objCu->gt("\x{46b}", "\x{46a}")); # Big Yus
ok($objCu->gt("\x{44f}", "\x{42f}"));
ok($objCu->gt("\x{467}", "\x{466}")); # Little Yus
ok($objCu->gt("\x{46f}", "\x{46e}")); # Ksi
ok($objCu->gt("\x{471}", "\x{470}")); # Psi
ok($objCu->gt("\x{473}", "\x{472}")); # Fita
ok($objCu->gt("\x{475}", "\x{474}")); # Izhitsa

# 423

ok($objCu->lt("\x{435}", "\x{454}")); # ie / ukr ie
ok($objCu->lt("\x{435}", "\x{404}")); # ie / UKR IE
ok($objCu->lt("\x{415}", "\x{454}")); # IE / ukr ie
ok($objCu->lt("\x{415}", "\x{404}")); # IE / UKR IE
ok($objCu->gt("\x{454}", "\x{404}")); # ukr ie / UKR IE

ok($objCu->gt("\x{47B}", "\x{47A}")); # round omega / ROUND OMEGA
ok($objCu->lt("\x{47B}", "\x{43E}")); # round omega / o
ok($objCu->lt("\x{47B}", "\x{41E}")); # round omega / O
ok($objCu->lt("\x{47A}", "\x{43E}")); # ROUND OMEGA / o
ok($objCu->lt("\x{47A}", "\x{41E}")); # ROUND OMEGA / O
ok($objCu->lt("\x{43E}", "\x{461}")); # o / omega
ok($objCu->lt("\x{43E}", "\x{460}")); # o / OMEGA
ok($objCu->lt("\x{41E}", "\x{461}")); # O / omega
ok($objCu->lt("\x{41E}", "\x{460}")); # O / OMEGA
ok($objCu->gt("\x{461}", "\x{460}")); # omega / OMEGA
ok($objCu->lt("\x{461}", "\x{A64D}")); # omega / broad omega
ok($objCu->lt("\x{461}", "\x{A64C}")); # omega / BROAD OMEGA
ok($objCu->lt("\x{460}", "\x{A64D}")); # OMEGA / broad omega
ok($objCu->lt("\x{460}", "\x{A64C}")); # OMEGA / BROAD OMEGA
ok($objCu->gt("\x{A64D}","\x{A64C}")); # broad omega / BROAD OMEGA

ok($objCu->gt("\x{479}", "\x{478}"));  # uk / UK
ok($objCu->lt("\x{479}", "\x{A64B}")); # uk / monograph uk
ok($objCu->lt("\x{479}", "\x{A64A}")); # uk / MONOGRAPH UK
ok($objCu->lt("\x{478}", "\x{A64B}")); # UK / monograph uk
ok($objCu->lt("\x{478}", "\x{A64A}")); # UK / MONOGRAPH UK
ok($objCu->gt("\x{A64B}","\x{A64A}")); # monograph uk / MONOGRAPH UK
ok($objCu->lt("\x{A64B}","\x{443}"));  # monograph uk / u
ok($objCu->lt("\x{A64B}","\x{423}"));  # monograph uk / U
ok($objCu->lt("\x{A64A}","\x{443}"));  # MONOGRAPH UK / u
ok($objCu->lt("\x{A64A}","\x{423}"));  # MONOGRAPH UK / U

ok($objCu->gt("\x{A657}","\x{A656}")); # iotified a / IOTIFIED A
ok($objCu->lt("\x{A657}","\x{467}"));  # iotified a / little yus
ok($objCu->lt("\x{A657}","\x{466}"));  # iotified a / LITTLE YUS
ok($objCu->lt("\x{A656}","\x{467}"));  # IOTIFIED A / little yus
ok($objCu->lt("\x{A656}","\x{466}"));  # IOTIFIED A / LITTLE YUS

# 458

$objCu->change(upper_before_lower => 0);

ok($objCu->lt("\x{430}", "\x{410}"));
ok($objCu->lt("\x{431}", "\x{411}"));
ok($objCu->lt("\x{432}", "\x{412}"));
ok($objCu->lt("\x{433}", "\x{413}"));
ok($objCu->lt("\x{434}", "\x{414}"));
ok($objCu->lt("\x{435}", "\x{415}"));
ok($objCu->lt("\x{436}", "\x{416}"));
ok($objCu->lt("\x{455}", "\x{405}")); # Dze
ok($objCu->lt("\x{437}", "\x{417}"));
ok($objCu->lt("\x{438}", "\x{418}"));
ok($objCu->lt("\x{456}", "\x{406}")); # Byel-Ukr I
ok($objCu->lt("\x{43a}", "\x{41a}"));
ok($objCu->lt("\x{43b}", "\x{41b}"));
ok($objCu->lt("\x{43c}", "\x{41c}"));
ok($objCu->lt("\x{43d}", "\x{41d}"));
ok($objCu->lt("\x{45a}", "\x{40a}")); # Nje
ok($objCu->lt("\x{50b}", "\x{50a}")); # Komi Nje
ok($objCu->lt("\x{43e}", "\x{41e}"));
ok($objCu->lt("\x{43f}", "\x{41f}"));
ok($objCu->lt("\x{440}", "\x{420}"));
ok($objCu->lt("\x{441}", "\x{421}"));
ok($objCu->lt("\x{442}", "\x{422}"));
ok($objCu->lt("\x{45b}", "\x{40b}")); # Tshe
ok($objCu->lt("\x{443}", "\x{423}"));
ok($objCu->lt("\x{444}", "\x{424}"));
ok($objCu->lt("\x{445}", "\x{425}"));
ok($objCu->lt("\x{446}", "\x{426}"));
ok($objCu->lt("\x{447}", "\x{427}"));
ok($objCu->lt("\x{448}", "\x{428}"));
ok($objCu->lt("\x{449}", "\x{429}"));
ok($objCu->lt("\x{44a}", "\x{42a}"));
ok($objCu->lt("\x{44b}", "\x{42b}"));
ok($objCu->lt("\x{44c}", "\x{42c}"));
ok($objCu->lt("\x{44d}", "\x{42d}"));
ok($objCu->lt("\x{463}", "\x{462}")); # Yat
ok($objCu->lt("\x{44e}", "\x{42e}"));
ok($objCu->lt("\x{46b}", "\x{46a}")); # Big Yus
ok($objCu->lt("\x{44f}", "\x{42f}"));
ok($objCu->lt("\x{467}", "\x{466}")); # Little Yus
ok($objCu->lt("\x{46f}", "\x{46e}")); # Ksi
ok($objCu->lt("\x{471}", "\x{470}")); # Psi
ok($objCu->lt("\x{473}", "\x{472}")); # Fita
ok($objCu->lt("\x{475}", "\x{474}")); # Izhitsa

# 501

ok($objCu->lt("\x{435}", "\x{454}")); # ie / ukr ie
ok($objCu->lt("\x{435}", "\x{404}")); # ie / UKR IE
ok($objCu->lt("\x{415}", "\x{454}")); # IE / ukr ie
ok($objCu->lt("\x{415}", "\x{404}")); # IE / UKR IE
ok($objCu->lt("\x{454}", "\x{404}")); # ukr ie / UKR IE

ok($objCu->lt("\x{47B}", "\x{47A}")); # round omega / ROUND OMEGA
ok($objCu->lt("\x{47B}", "\x{43E}")); # round omega / o
ok($objCu->lt("\x{47B}", "\x{41E}")); # round omega / O
ok($objCu->lt("\x{47A}", "\x{43E}")); # ROUND OMEGA / o
ok($objCu->lt("\x{47A}", "\x{41E}")); # ROUND OMEGA / O
ok($objCu->lt("\x{43E}", "\x{461}")); # o / omega
ok($objCu->lt("\x{43E}", "\x{460}")); # o / OMEGA
ok($objCu->lt("\x{41E}", "\x{461}")); # O / omega
ok($objCu->lt("\x{41E}", "\x{460}")); # O / OMEGA
ok($objCu->lt("\x{461}", "\x{460}")); # omega / OMEGA
ok($objCu->lt("\x{461}", "\x{A64D}")); # omega / broad omega
ok($objCu->lt("\x{461}", "\x{A64C}")); # omega / BROAD OMEGA
ok($objCu->lt("\x{460}", "\x{A64D}")); # OMEGA / broad omega
ok($objCu->lt("\x{460}", "\x{A64C}")); # OMEGA / BROAD OMEGA
ok($objCu->lt("\x{A64D}","\x{A64C}")); # broad omega / BROAD OMEGA

ok($objCu->lt("\x{479}", "\x{478}"));  # uk / UK
ok($objCu->lt("\x{479}", "\x{A64B}")); # uk / monograph uk
ok($objCu->lt("\x{479}", "\x{A64A}")); # uk / MONOGRAPH UK
ok($objCu->lt("\x{478}", "\x{A64B}")); # UK / monograph uk
ok($objCu->lt("\x{478}", "\x{A64A}")); # UK / MONOGRAPH UK
ok($objCu->lt("\x{A64B}","\x{A64A}")); # monograph uk / MONOGRAPH UK
ok($objCu->lt("\x{A64B}","\x{443}"));  # monograph uk / u
ok($objCu->lt("\x{A64B}","\x{423}"));  # monograph uk / U
ok($objCu->lt("\x{A64A}","\x{443}"));  # MONOGRAPH UK / u
ok($objCu->lt("\x{A64A}","\x{423}"));  # MONOGRAPH UK / U

ok($objCu->lt("\x{A657}","\x{A656}")); # iotified a / IOTIFIED A
ok($objCu->lt("\x{A657}","\x{467}"));  # iotified a / little yus
ok($objCu->lt("\x{A657}","\x{466}"));  # iotified a / LITTLE YUS
ok($objCu->lt("\x{A656}","\x{467}"));  # IOTIFIED A / little yus
ok($objCu->lt("\x{A656}","\x{466}"));  # IOTIFIED A / LITTLE YUS

# 536

# equiv.

ok($objCu->eq("\x{1C81}", "\x{434}"));
ok($objCu->eq("\x{A641}", "\x{437}"));
ok($objCu->eq("\x{A640}", "\x{417}"));
ok($objCu->eq("\x{1C82}", "\x{43E}"));

ok($objCu->eq("\x{479}", "\x{43E}\x{443}"));
ok($objCu->eq("\x{479}","\x{1C82}\x{443}"));
ok($objCu->eq("\x{478}", "\x{41E}\x{443}"));
ok($objCu->eq("\x{478}", "\x{41E}\x{423}"));

# 544

# decomp. equiv.

ok($objCu->eq("\x{457}", "\x{456}\x{308}"));
ok($objCu->eq("\x{407}", "\x{406}\x{308}"));
ok($objCu->eq("\x{439}", "\x{438}\x{306}"));
ok($objCu->eq("\x{419}", "\x{418}\x{306}"));
ok($objCu->eq("\x{4E6}", "\x{41E}\x{308}"));
ok($objCu->eq("\x{4E7}", "\x{43E}\x{308}"));
ok($objCu->eq("\x{45E}", "\x{443}\x{306}"));
ok($objCu->eq("\x{40E}", "\x{423}\x{306}"));
ok($objCu->eq("\x{4F1}", "\x{443}\x{308}"));
ok($objCu->eq("\x{4F0}", "\x{423}\x{308}"));
ok($objCu->eq("\x{4F3}", "\x{443}\x{30B}"));
ok($objCu->eq("\x{4F2}", "\x{423}\x{30B}"));
ok($objCu->eq("\x{4EF}", "\x{443}\x{304}"));
ok($objCu->eq("\x{4EE}", "\x{423}\x{304}"));

ok($objCu->eq("\x{47C}", "\x{A64C}\x{486}\x{311}"));
ok($objCu->eq("\x{47D}", "\x{A64D}\x{486}\x{311}"));
ok($objCu->eq("\x{47E}", "\x{460}\x{442}"));
ok($objCu->eq("\x{47F}", "\x{461}\x{442}"));

# 562

# misc. equiv.

ok($objCu->eq("\x{4D1}", "\x{430}\x{306}"));
ok($objCu->eq("\x{4D0}", "\x{410}\x{306}"));
ok($objCu->eq("\x{4D3}", "\x{430}\x{308}"));
ok($objCu->eq("\x{4D2}", "\x{410}\x{308}"));
ok($objCu->eq("\x{450}", "\x{435}\x{300}"));
ok($objCu->eq("\x{400}", "\x{415}\x{300}"));
ok($objCu->eq("\x{4D7}", "\x{435}\x{306}"));
ok($objCu->eq("\x{4D6}", "\x{415}\x{306}"));
ok($objCu->eq("\x{451}", "\x{435}\x{308}"));
ok($objCu->eq("\x{401}", "\x{415}\x{308}"));
ok($objCu->eq("\x{45D}", "\x{438}\x{300}"));
ok($objCu->eq("\x{40D}", "\x{418}\x{300}"));
ok($objCu->eq("\x{4E5}", "\x{438}\x{308}"));
ok($objCu->eq("\x{4E4}", "\x{418}\x{308}"));
ok($objCu->eq("\x{4ED}", "\x{44D}\x{308}"));
ok($objCu->eq("\x{4EC}", "\x{42D}\x{308}"));
ok($objCu->eq("\x{477}", "\x{475}\x{30F}"));
ok($objCu->eq("\x{476}", "\x{474}\x{30F}"));

# 580

# latin equiv.

ok($objCu->eq("a\x{300}", _pack_U(0xE0)));
ok($objCu->eq("A\x{300}", _pack_U(0xC0)));
ok($objCu->eq("e\x{300}", _pack_U(0xE8)));
ok($objCu->eq("E\x{300}", _pack_U(0xC8)));
ok($objCu->eq("i\x{300}", _pack_U(0xEC)));
ok($objCu->eq("I\x{300}", _pack_U(0xCC)));
ok($objCu->eq("o\x{300}", _pack_U(0xF2)));
ok($objCu->eq("O\x{300}", _pack_U(0xD2)));
ok($objCu->eq("u\x{300}", _pack_U(0xF9)));
ok($objCu->eq("U\x{300}", _pack_U(0xD9)));
ok($objCu->eq("y\x{300}", "\x{1EF3}"));
ok($objCu->eq("Y\x{300}", "\x{1EF2}"));

ok($objCu->eq("a\x{301}", _pack_U(0xE1)));
ok($objCu->eq("A\x{301}", _pack_U(0xC1)));
ok($objCu->eq("e\x{301}", _pack_U(0xE9)));
ok($objCu->eq("E\x{301}", _pack_U(0xC9)));
ok($objCu->eq("i\x{301}", _pack_U(0xED)));
ok($objCu->eq("I\x{301}", _pack_U(0xCD)));
ok($objCu->eq("o\x{301}", _pack_U(0xF3)));
ok($objCu->eq("O\x{301}", _pack_U(0xD3)));
ok($objCu->eq("u\x{301}", _pack_U(0xFA)));
ok($objCu->eq("U\x{301}", _pack_U(0xDA)));
ok($objCu->eq("y\x{301}", _pack_U(0xFD)));
ok($objCu->eq("Y\x{301}", _pack_U(0xDD)));

ok($objCu->eq("a\x{308}", _pack_U(0xE4)));
ok($objCu->eq("A\x{308}", _pack_U(0xC4)));
ok($objCu->eq("e\x{308}", _pack_U(0xEB)));
ok($objCu->eq("E\x{308}", _pack_U(0xCB)));
ok($objCu->eq("i\x{308}", _pack_U(0xEF)));
ok($objCu->eq("I\x{308}", _pack_U(0xCF)));
ok($objCu->eq("o\x{308}", _pack_U(0xF6)));
ok($objCu->eq("O\x{308}", _pack_U(0xD6)));
ok($objCu->eq("u\x{308}", _pack_U(0xFC)));
ok($objCu->eq("U\x{308}", _pack_U(0xDC)));
ok($objCu->eq("y\x{308}", _pack_U(0xFF)));
ok($objCu->eq("Y\x{308}", "\x{178}"));

# 616
