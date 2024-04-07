
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..52\n"; }
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

my $auml = _pack_U(0xE4);
my $Auml = _pack_U(0xC4);
my $ouml = _pack_U(0xF6);
my $Ouml = _pack_U(0xD6);
my $uuml = _pack_U(0xFC);
my $Uuml = _pack_U(0xDC);

my $objDePhone = Unicode::Collate::Locale->
    new(locale => 'DE-PHONE', normalization => undef);

ok($objDePhone->getlocale, 'de__phonebook');

$objDePhone->change(level => 1);

ok($objDePhone->eq("a\x{308}", "ae"));
ok($objDePhone->eq("A\x{308}", "AE"));
ok($objDePhone->eq("o\x{308}", "oe"));
ok($objDePhone->eq("O\x{308}", "OE"));
ok($objDePhone->eq("u\x{308}", "ue"));
ok($objDePhone->eq("U\x{308}", "UE"));

# 8

$objDePhone->change(level => 2);

ok($objDePhone->gt("a\x{308}", "ae"));
ok($objDePhone->gt("A\x{308}", "AE"));
ok($objDePhone->gt("o\x{308}", "oe"));
ok($objDePhone->gt("O\x{308}", "OE"));
ok($objDePhone->gt("u\x{308}", "ue"));
ok($objDePhone->gt("U\x{308}", "UE"));

# 14

ok($objDePhone->eq("a\x{308}", "A\x{308}"));
ok($objDePhone->eq("o\x{308}", "O\x{308}"));
ok($objDePhone->eq("u\x{308}", "U\x{308}"));

ok($objDePhone->eq($auml, $Auml));
ok($objDePhone->eq($ouml, $Ouml));
ok($objDePhone->eq($uuml, $Uuml));

# 20

$objDePhone->change(level => 3);

ok($objDePhone->lt("a\x{308}", "A\x{308}"));
ok($objDePhone->lt("o\x{308}", "O\x{308}"));
ok($objDePhone->lt("u\x{308}", "U\x{308}"));

ok($objDePhone->lt($auml, $Auml));
ok($objDePhone->lt($ouml, $Ouml));
ok($objDePhone->lt($uuml, $Uuml));

# 26

ok($objDePhone->eq("a\x{308}", $auml));
ok($objDePhone->eq("A\x{308}", $Auml));
ok($objDePhone->eq("o\x{308}", $ouml));
ok($objDePhone->eq("O\x{308}", $Ouml));
ok($objDePhone->eq("u\x{308}", $uuml));
ok($objDePhone->eq("U\x{308}", $Uuml));

# 32

ok($objDePhone->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objDePhone->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objDePhone->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objDePhone->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objDePhone->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objDePhone->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objDePhone->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objDePhone->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objDePhone->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objDePhone->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objDePhone->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objDePhone->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 44

{
  my $objDePhoneBook = Unicode::Collate::Locale->
    new(locale => 'de__phonebook', normalization => undef);
  ok($objDePhoneBook->getlocale, 'de__phonebook');
  $objDePhoneBook->change(level => 1);
  ok($objDePhoneBook->eq("a\x{308}", "ae"));
}

# 46

{
  my $objDePhonebk = Unicode::Collate::Locale->
    new(locale => 'de-phonebk', normalization => undef);
  ok($objDePhonebk->getlocale, 'de__phonebook');
  $objDePhonebk->change(level => 1);
  ok($objDePhonebk->eq("a\x{308}", "ae"));
}

# 48

{
  my $objDeDePhone = Unicode::Collate::Locale->
    new(locale => 'de_DE_phone', normalization => undef);
  ok($objDeDePhone->getlocale, 'de__phonebook');
  $objDeDePhone->change(level => 1);
  ok($objDeDePhone->eq("a\x{308}", "ae"));
}

# 50

{
  my $objDeChPhonebk = Unicode::Collate::Locale->
    new(locale => 'de_CH_phonebk', normalization => undef);
  ok($objDeChPhonebk->getlocale, 'de__phonebook');
  $objDeChPhonebk->change(level => 1);
  ok($objDeChPhonebk->eq("a\x{308}", "ae"));
}

# 52
