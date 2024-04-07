
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..235\n"; }
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

my $objHu = Unicode::Collate::Locale->
    new(locale => 'HU', normalization => undef);

ok($objHu->getlocale, 'hu');

$objHu->change(level => 1);

ok($objHu->lt("c", "cs"));
ok($objHu->lt("cz","cs"));
ok($objHu->gt("d", "cs"));
ok($objHu->lt("d", "dz"));
ok($objHu->gt("dz","d\x{292}"));
ok($objHu->lt("dz", "dzs"));
ok($objHu->lt("dzz","dzs"));
ok($objHu->gt("e", "dzs"));
ok($objHu->lt("g", "gy"));
ok($objHu->lt("gz","gy"));
ok($objHu->gt("h", "gy"));
ok($objHu->lt("l", "ly"));
ok($objHu->lt("lz","ly"));
ok($objHu->gt("m", "ly"));
ok($objHu->lt("n", "ny"));
ok($objHu->lt("nz","ny"));
ok($objHu->gt("o", "ny"));
ok($objHu->lt("s", "sz"));
ok($objHu->gt("sz","s\x{292}"));
ok($objHu->gt("t", "sz"));
ok($objHu->lt("t", "ty"));
ok($objHu->lt("tz","ty"));
ok($objHu->gt("u", "ty"));
ok($objHu->lt("z", "zs"));
ok($objHu->lt("zz", "zs"));
ok($objHu->lt("zs", "\x{292}"));

# 28

ok($objHu->lt("o", "o\x{308}"));
ok($objHu->gt("p", "o\x{308}"));
ok($objHu->lt("u", "u\x{308}"));
ok($objHu->gt("v", "u\x{308}"));

ok($objHu->eq("o\x{308}", "o\x{30B}"));
ok($objHu->eq("u\x{308}", "u\x{30B}"));

# 34

$objHu->change(level => 2);

ok($objHu->eq("cs", "cS"));
ok($objHu->eq("cS", "Cs"));
ok($objHu->eq("Cs", "CS"));
ok($objHu->eq("dz", "dZ"));
ok($objHu->eq("dZ", "Dz"));
ok($objHu->eq("Dz", "DZ"));
ok($objHu->eq("dzs", "dzS"));
ok($objHu->eq("dzS", "dZs"));
ok($objHu->eq("dZs", "dZS"));
ok($objHu->eq("dZS", "Dzs"));
ok($objHu->eq("Dzs", "DzS"));
ok($objHu->eq("DzS", "DZs"));
ok($objHu->eq("DZs", "DZS"));
ok($objHu->eq("gy", "gY"));
ok($objHu->eq("gY", "Gy"));
ok($objHu->eq("Gy", "GY"));
ok($objHu->eq("ly", "lY"));
ok($objHu->eq("lY", "Ly"));
ok($objHu->eq("Ly", "LY"));
ok($objHu->eq("ny", "nY"));
ok($objHu->eq("nY", "Ny"));
ok($objHu->eq("Ny", "NY"));
ok($objHu->eq("sz", "sZ"));
ok($objHu->eq("sZ", "Sz"));
ok($objHu->eq("Sz", "SZ"));
ok($objHu->eq("ty", "tY"));
ok($objHu->eq("tY", "Ty"));
ok($objHu->eq("Ty", "TY"));
ok($objHu->eq("zs", "zS"));
ok($objHu->eq("zS", "Zs"));
ok($objHu->eq("Zs", "ZS"));

# 65

ok($objHu->lt("o\x{308}", "o\x{30B}"));
ok($objHu->lt("u\x{308}", "u\x{30B}"));

ok($objHu->eq("o\x{308}", "O\x{308}"));
ok($objHu->eq("o\x{30B}", "O\x{30B}"));
ok($objHu->eq("u\x{308}", "U\x{308}"));
ok($objHu->eq("u\x{30B}", "U\x{30B}"));

# 71

$objHu->change(level => 3);

ok($objHu->lt("cs", "cS"));
ok($objHu->lt("cS", "Cs"));
ok($objHu->lt("Cs", "CS"));
ok($objHu->lt("dz", "dZ"));
ok($objHu->lt("dZ", "Dz"));
ok($objHu->lt("Dz", "DZ"));
ok($objHu->lt("dzs", "dzS"));
ok($objHu->lt("dzS", "dZs"));
ok($objHu->lt("dZs", "dZS"));
ok($objHu->lt("dZS", "Dzs"));
ok($objHu->lt("Dzs", "DzS"));
ok($objHu->lt("DzS", "DZs"));
ok($objHu->lt("DZs", "DZS"));
ok($objHu->lt("gy", "gY"));
ok($objHu->lt("gY", "Gy"));
ok($objHu->lt("Gy", "GY"));
ok($objHu->lt("ly", "lY"));
ok($objHu->lt("lY", "Ly"));
ok($objHu->lt("Ly", "LY"));
ok($objHu->lt("ny", "nY"));
ok($objHu->lt("nY", "Ny"));
ok($objHu->lt("Ny", "NY"));
ok($objHu->lt("sz", "sZ"));
ok($objHu->lt("sZ", "Sz"));
ok($objHu->lt("Sz", "SZ"));
ok($objHu->lt("ty", "tY"));
ok($objHu->lt("tY", "Ty"));
ok($objHu->lt("Ty", "TY"));
ok($objHu->lt("zs", "zS"));
ok($objHu->lt("zS", "Zs"));
ok($objHu->lt("Zs", "ZS"));

# 102

ok($objHu->lt("o\x{308}", "O\x{308}"));
ok($objHu->lt("o\x{30B}", "O\x{30B}"));
ok($objHu->lt("u\x{308}", "U\x{308}"));
ok($objHu->lt("u\x{30B}", "U\x{30B}"));

ok($objHu->eq("o\x{308}", _pack_U(0xF6)));
ok($objHu->eq("O\x{308}", _pack_U(0xD6)));
ok($objHu->eq("o\x{30B}", "\x{151}"));
ok($objHu->eq("O\x{30B}", "\x{150}"));
ok($objHu->eq("u\x{308}", _pack_U(0xFC)));
ok($objHu->eq("U\x{308}", _pack_U(0xDC)));
ok($objHu->eq("u\x{30B}", "\x{171}"));
ok($objHu->eq("U\x{30B}", "\x{170}"));

# 114

ok($objHu->eq("o\x{308}\x{304}", "\x{22B}"));
ok($objHu->eq("O\x{308}\x{304}", "\x{22A}"));
ok($objHu->eq("u\x{308}\x{300}", "\x{1DC}"));
ok($objHu->eq("U\x{308}\x{300}", "\x{1DB}"));
ok($objHu->eq("u\x{308}\x{301}", "\x{1D8}"));
ok($objHu->eq("U\x{308}\x{301}", "\x{1D7}"));
ok($objHu->eq("u\x{308}\x{304}", "\x{1D6}"));
ok($objHu->eq("U\x{308}\x{304}", "\x{1D5}"));
ok($objHu->eq("u\x{308}\x{30C}", "\x{1DA}"));
ok($objHu->eq("U\x{308}\x{30C}", "\x{1D9}"));

# 124

ok($objHu->eq("ccs", "cscs"));
ok($objHu->eq("ccS", "cscS"));
ok($objHu->eq("cCs", "csCs"));
ok($objHu->eq("cCS", "csCS"));
ok($objHu->eq("Ccs", "CScs"));
ok($objHu->eq("CcS", "CScS"));
ok($objHu->eq("CCs", "CSCs"));
ok($objHu->eq("CCS", "CSCS"));
ok($objHu->eq("ddz", "dzdz"));
ok($objHu->eq("ddZ", "dzdZ"));
ok($objHu->eq("dDz", "dzDz"));
ok($objHu->eq("dDZ", "dzDZ"));
ok($objHu->eq("Ddz", "DZdz"));
ok($objHu->eq("DdZ", "DZdZ"));
ok($objHu->eq("DDz", "DZDz"));
ok($objHu->eq("DDZ", "DZDZ"));
ok($objHu->eq("ddzs", "dzsdzs"));
ok($objHu->eq("ddzS", "dzsdzS"));
ok($objHu->eq("ddZs", "dzsdZs"));
ok($objHu->eq("ddZS", "dzsdZS"));
ok($objHu->eq("dDzs", "dzsDzs"));
ok($objHu->eq("dDzS", "dzsDzS"));
ok($objHu->eq("dDZs", "dzsDZs"));
ok($objHu->eq("dDZS", "dzsDZS"));
ok($objHu->eq("Ddzs", "DZSdzs"));
ok($objHu->eq("DdzS", "DZSdzS"));
ok($objHu->eq("DdZs", "DZSdZs"));
ok($objHu->eq("DdZS", "DZSdZS"));
ok($objHu->eq("DDzs", "DZSDzs"));
ok($objHu->eq("DDzS", "DZSDzS"));
ok($objHu->eq("DDZs", "DZSDZs"));
ok($objHu->eq("DDZS", "DZSDZS"));
ok($objHu->eq("ggy", "gygy"));
ok($objHu->eq("ggY", "gygY"));
ok($objHu->eq("gGy", "gyGy"));
ok($objHu->eq("gGY", "gyGY"));
ok($objHu->eq("Ggy", "GYgy"));
ok($objHu->eq("GgY", "GYgY"));
ok($objHu->eq("GGy", "GYGy"));
ok($objHu->eq("GGY", "GYGY"));
ok($objHu->eq("lly", "lyly"));
ok($objHu->eq("llY", "lylY"));
ok($objHu->eq("lLy", "lyLy"));
ok($objHu->eq("lLY", "lyLY"));
ok($objHu->eq("Lly", "LYly"));
ok($objHu->eq("LlY", "LYlY"));
ok($objHu->eq("LLy", "LYLy"));
ok($objHu->eq("LLY", "LYLY"));
ok($objHu->eq("nny", "nyny"));
ok($objHu->eq("nnY", "nynY"));
ok($objHu->eq("nNy", "nyNy"));
ok($objHu->eq("nNY", "nyNY"));
ok($objHu->eq("Nny", "NYny"));
ok($objHu->eq("NnY", "NYnY"));
ok($objHu->eq("NNy", "NYNy"));
ok($objHu->eq("NNY", "NYNY"));
ok($objHu->eq("ssz", "szsz"));
ok($objHu->eq("ssZ", "szsZ"));
ok($objHu->eq("sSz", "szSz"));
ok($objHu->eq("sSZ", "szSZ"));
ok($objHu->eq("Ssz", "SZsz"));
ok($objHu->eq("SsZ", "SZsZ"));
ok($objHu->eq("SSz", "SZSz"));
ok($objHu->eq("SSZ", "SZSZ"));
ok($objHu->eq("tty", "tyty"));
ok($objHu->eq("ttY", "tytY"));
ok($objHu->eq("tTy", "tyTy"));
ok($objHu->eq("tTY", "tyTY"));
ok($objHu->eq("Tty", "TYty"));
ok($objHu->eq("TtY", "TYtY"));
ok($objHu->eq("TTy", "TYTy"));
ok($objHu->eq("TTY", "TYTY"));
ok($objHu->eq("zzs", "zszs"));
ok($objHu->eq("zzS", "zszS"));
ok($objHu->eq("zZs", "zsZs"));
ok($objHu->eq("zZS", "zsZS"));
ok($objHu->eq("Zzs", "ZSzs"));
ok($objHu->eq("ZzS", "ZSzS"));
ok($objHu->eq("ZZs", "ZSZs"));
ok($objHu->eq("ZZS", "ZSZS"));

# 204

$objHu->change(upper_before_lower => 1);

ok($objHu->gt("cs", "cS"));
ok($objHu->gt("cS", "Cs"));
ok($objHu->gt("Cs", "CS"));
ok($objHu->gt("dz", "dZ"));
ok($objHu->gt("dZ", "Dz"));
ok($objHu->gt("Dz", "DZ"));
ok($objHu->gt("dzs", "dzS"));
ok($objHu->gt("dzS", "dZs"));
ok($objHu->gt("dZs", "dZS"));
ok($objHu->gt("dZS", "Dzs"));
ok($objHu->gt("Dzs", "DzS"));
ok($objHu->gt("DzS", "DZs"));
ok($objHu->gt("DZs", "DZS"));
ok($objHu->gt("gy", "gY"));
ok($objHu->gt("gY", "Gy"));
ok($objHu->gt("Gy", "GY"));
ok($objHu->gt("ly", "lY"));
ok($objHu->gt("lY", "Ly"));
ok($objHu->gt("Ly", "LY"));
ok($objHu->gt("ny", "nY"));
ok($objHu->gt("nY", "Ny"));
ok($objHu->gt("Ny", "NY"));
ok($objHu->gt("sz", "sZ"));
ok($objHu->gt("sZ", "Sz"));
ok($objHu->gt("Sz", "SZ"));
ok($objHu->gt("ty", "tY"));
ok($objHu->gt("tY", "Ty"));
ok($objHu->gt("Ty", "TY"));
ok($objHu->gt("zs", "zS"));
ok($objHu->gt("zS", "Zs"));
ok($objHu->gt("Zs", "ZS"));

# 235
