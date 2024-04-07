
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..46\n"; }
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
my $eses = _pack_U(0xDF);
my $Eses = _pack_U(0x1E9E);

my $objDeAtPhone = Unicode::Collate::Locale->
    new(locale => 'DE_AT_phonebook', normalization => undef);

ok($objDeAtPhone->getlocale, 'de_AT_phonebook');

$objDeAtPhone->change(level => 1);

ok($objDeAtPhone->gt($auml, "az"));
ok($objDeAtPhone->lt($auml, "b"));
ok($objDeAtPhone->gt($ouml, "oz"));
ok($objDeAtPhone->lt($ouml, "p"));
ok($objDeAtPhone->gt($uuml, "uz"));
ok($objDeAtPhone->lt($uuml, "v"));
ok($objDeAtPhone->gt($eses, "ssz"));
ok($objDeAtPhone->lt($eses, "st"));

# 10

$objDeAtPhone->change(level => 2);

ok($objDeAtPhone->eq("a\x{308}", "A\x{308}"));
ok($objDeAtPhone->eq("o\x{308}", "O\x{308}"));
ok($objDeAtPhone->eq("u\x{308}", "U\x{308}"));

ok($objDeAtPhone->eq($auml, $Auml));
ok($objDeAtPhone->eq($ouml, $Ouml));
ok($objDeAtPhone->eq($uuml, $Uuml));
ok($objDeAtPhone->eq($eses, $Eses));

# 17

$objDeAtPhone->change(level => 3);

ok($objDeAtPhone->lt("a\x{308}", "A\x{308}"));
ok($objDeAtPhone->lt("o\x{308}", "O\x{308}"));
ok($objDeAtPhone->lt("u\x{308}", "U\x{308}"));

ok($objDeAtPhone->lt($auml, $Auml));
ok($objDeAtPhone->lt($ouml, $Ouml));
ok($objDeAtPhone->lt($uuml, $Uuml));
ok($objDeAtPhone->lt($eses, $Eses));

# 24

ok($objDeAtPhone->eq("a\x{308}", $auml));
ok($objDeAtPhone->eq("A\x{308}", $Auml));
ok($objDeAtPhone->eq("o\x{308}", $ouml));
ok($objDeAtPhone->eq("O\x{308}", $Ouml));
ok($objDeAtPhone->eq("u\x{308}", $uuml));
ok($objDeAtPhone->eq("U\x{308}", $Uuml));

# 30

ok($objDeAtPhone->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objDeAtPhone->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objDeAtPhone->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objDeAtPhone->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objDeAtPhone->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objDeAtPhone->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objDeAtPhone->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objDeAtPhone->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objDeAtPhone->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objDeAtPhone->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objDeAtPhone->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objDeAtPhone->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 42

{
  my $objDeLatnAtPhone = Unicode::Collate::Locale->
    new(locale => 'DE_Latn_AT_phonebook', normalization => undef);
  ok($objDeLatnAtPhone->getlocale, 'de_AT_phonebook');
  $objDeLatnAtPhone->change(level => 1);
  ok($objDeLatnAtPhone->gt($auml, 'az'));
}

# 44

{
  my $objDeAt = Unicode::Collate::Locale->
    new(locale => 'DE_AT', normalization => undef);
  ok($objDeAt->getlocale, 'default');
  $objDeAt->change(level => 1);
  ok($objDeAt->eq($auml, 'a'));
}

# 46
