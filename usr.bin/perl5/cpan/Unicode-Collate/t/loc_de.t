
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..50\n"; }
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

my $objDe = Unicode::Collate::Locale->
    new(locale => 'DE', normalization => undef);

ok($objDe->getlocale, 'default');

$objDe->change(level => 1);

ok($objDe->lt("a\x{308}", "ae"));
ok($objDe->lt("A\x{308}", "AE"));
ok($objDe->lt("o\x{308}", "oe"));
ok($objDe->lt("O\x{308}", "OE"));
ok($objDe->lt("u\x{308}", "ue"));
ok($objDe->lt("U\x{308}", "UE"));

# 8

ok($objDe->eq("a\x{308}", "a"));
ok($objDe->eq("A\x{308}", "A"));
ok($objDe->eq("o\x{308}", "o"));
ok($objDe->eq("O\x{308}", "O"));
ok($objDe->eq("u\x{308}", "u"));
ok($objDe->eq("U\x{308}", "U"));

# 14

$objDe->change(level => 2);

ok($objDe->gt("a\x{308}", "a"));
ok($objDe->gt("A\x{308}", "A"));
ok($objDe->gt("o\x{308}", "o"));
ok($objDe->gt("O\x{308}", "O"));
ok($objDe->gt("u\x{308}", "u"));
ok($objDe->gt("U\x{308}", "U"));

# 20

ok($objDe->eq("a\x{308}", "A\x{308}"));
ok($objDe->eq("o\x{308}", "O\x{308}"));
ok($objDe->eq("u\x{308}", "U\x{308}"));

ok($objDe->eq($auml, $Auml));
ok($objDe->eq($ouml, $Ouml));
ok($objDe->eq($uuml, $Uuml));

# 26

$objDe->change(level => 3);

ok($objDe->lt("a\x{308}", "A\x{308}"));
ok($objDe->lt("o\x{308}", "O\x{308}"));
ok($objDe->lt("u\x{308}", "U\x{308}"));

ok($objDe->lt($auml, $Auml));
ok($objDe->lt($ouml, $Ouml));
ok($objDe->lt($uuml, $Uuml));

# 32

ok($objDe->eq("a\x{308}", $auml));
ok($objDe->eq("A\x{308}", $Auml));
ok($objDe->eq("o\x{308}", $ouml));
ok($objDe->eq("O\x{308}", $Ouml));
ok($objDe->eq("u\x{308}", $uuml));
ok($objDe->eq("U\x{308}", $Uuml));

# 38

ok($objDe->eq("a\x{308}\x{304}", "\x{1DF}"));
ok($objDe->eq("A\x{308}\x{304}", "\x{1DE}"));
ok($objDe->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objDe->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objDe->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objDe->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objDe->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objDe->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objDe->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objDe->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objDe->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objDe->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 50
