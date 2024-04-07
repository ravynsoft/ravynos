
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..78\n"; }
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

my $objWae = Unicode::Collate::Locale->
    new(locale => 'WAE', normalization => undef);

ok($objWae->getlocale, 'wae');

$objWae->change(level => 1);

ok($objWae->lt("aa", "a9"));
ok($objWae->gt("aA", "a9"));
ok($objWae->gt("Aa", "a9"));
ok($objWae->gt("AA", "a9"));
ok($objWae->lt("ee", "e9"));
ok($objWae->gt("eE", "e9"));
ok($objWae->gt("Ee", "e9"));
ok($objWae->gt("EE", "e9"));
ok($objWae->lt("ii", "i9"));
ok($objWae->gt("iI", "i9"));
ok($objWae->gt("Ii", "i9"));
ok($objWae->gt("II", "i9"));
ok($objWae->lt("oo", "o9"));
ok($objWae->gt("oO", "o9"));
ok($objWae->gt("Oo", "o9"));
ok($objWae->gt("OO", "o9"));
ok($objWae->lt("uu", "u9"));
ok($objWae->gt("uU", "u9"));
ok($objWae->gt("Uu", "u9"));
ok($objWae->gt("UU", "u9"));

# 22

ok($objWae->lt("ch", "c9"));
ok($objWae->gt("cH", "c9"));
ok($objWae->gt("Ch", "c9"));
ok($objWae->gt("CH", "c9"));

ok($objWae->lt("sch", "s9"));
ok($objWae->gt("scH", "s9"));
ok($objWae->gt("sCh", "s9"));
ok($objWae->gt("sCH", "s9"));
ok($objWae->gt("Sch", "s9"));
ok($objWae->gt("ScH", "s9"));
ok($objWae->gt("SCh", "s9"));
ok($objWae->gt("SCH", "s9"));

# 34

ok($objWae->lt("a\x{308}a\x{308}", "a9"));
ok($objWae->gt("a\x{308}A\x{308}", "a9"));
ok($objWae->gt("A\x{308}a\x{308}", "a9"));
ok($objWae->gt("A\x{308}A\x{308}", "a9"));
ok($objWae->lt("o\x{308}o\x{308}", "o9"));
ok($objWae->gt("o\x{308}O\x{308}", "o9"));
ok($objWae->gt("O\x{308}o\x{308}", "o9"));
ok($objWae->gt("O\x{308}O\x{308}", "o9"));
ok($objWae->lt("u\x{308}u\x{308}", "u9"));
ok($objWae->gt("u\x{308}U\x{308}", "u9"));
ok($objWae->gt("U\x{308}u\x{308}", "u9"));
ok($objWae->gt("U\x{308}U\x{308}", "u9"));

# 46

$objWae->change(level => 3);

ok($objWae->eq("a\x{301}", _pack_U(0xE1)));
ok($objWae->eq("e\x{301}", _pack_U(0xE9)));
ok($objWae->eq("i\x{301}", _pack_U(0xED)));
ok($objWae->eq("o\x{301}", _pack_U(0xF3)));
ok($objWae->eq("u\x{301}", _pack_U(0xFA)));
ok($objWae->eq("a\x{301}", "aa"));
ok($objWae->eq("e\x{301}", "ee"));
ok($objWae->eq("i\x{301}", "ii"));
ok($objWae->eq("o\x{301}", "oo"));
ok($objWae->eq("u\x{301}", "uu"));

# 56

ok($objWae->eq("c\x{30C}", "\x{10D}"));
ok($objWae->eq("s\x{30C}", "\x{161}"));
ok($objWae->eq("c\x{30C}", "ch"));
ok($objWae->eq("s\x{30C}", "sch"));

# 60

my $a1 = _pack_U(0xE4);
my $o1 = _pack_U(0xF6);
my $u1 = _pack_U(0xFC);
my $a2 = "a\x{308}";
my $o2 = "o\x{308}";
my $u2 = "u\x{308}";

ok($objWae->eq($a1, $a2));
ok($objWae->eq($o1, $o2));
ok($objWae->eq($u1, $u2));

ok($objWae->eq("a\x{303}", _pack_U(0xE3)));
ok($objWae->eq("o\x{303}", _pack_U(0xF5)));
ok($objWae->eq("u\x{303}", "\x{169}"));

# 66

ok($objWae->eq("a\x{303}", $a1.$a1));
ok($objWae->eq("a\x{303}", $a1.$a2));
ok($objWae->eq("a\x{303}", $a2.$a1));
ok($objWae->eq("a\x{303}", $a2.$a2));
ok($objWae->eq("o\x{303}", $o1.$o1));
ok($objWae->eq("o\x{303}", $o1.$o2));
ok($objWae->eq("o\x{303}", $o2.$o1));
ok($objWae->eq("o\x{303}", $o2.$o2));
ok($objWae->eq("u\x{303}", $u1.$u1));
ok($objWae->eq("u\x{303}", $u1.$u2));
ok($objWae->eq("u\x{303}", $u2.$u1));
ok($objWae->eq("u\x{303}", $u2.$u2));

# 78
