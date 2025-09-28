
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..126\n"; }
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

my $objSq = Unicode::Collate::Locale->
    new(locale => 'SQ', normalization => undef);

ok($objSq->getlocale, 'sq');

$objSq->change(level => 1);

ok($objSq->lt("c", "c\x{327}"));
ok($objSq->lt("cz","c\x{327}"));
ok($objSq->gt("d", "c\x{327}"));
ok($objSq->lt("d", "dh"));
ok($objSq->lt("dz","dh"));
ok($objSq->gt("e", "dh"));
ok($objSq->lt("e", "e\x{308}"));
ok($objSq->lt("ez","e\x{308}"));
ok($objSq->gt("f", "e\x{308}"));
ok($objSq->lt("g", "gj"));
ok($objSq->lt("gz","gj"));
ok($objSq->gt("h", "gj"));
ok($objSq->lt("l", "ll"));
ok($objSq->lt("lz","ll"));
ok($objSq->gt("m", "ll"));
ok($objSq->lt("n", "nj"));
ok($objSq->lt("nz","nj"));
ok($objSq->gt("o", "nj"));
ok($objSq->lt("r", "rr"));
ok($objSq->lt("rz","rr"));
ok($objSq->gt("s", "rr"));
ok($objSq->lt("s", "sh"));
ok($objSq->lt("sz","sh"));
ok($objSq->gt("t", "sh"));
ok($objSq->lt("t", "th"));
ok($objSq->lt("tz","th"));
ok($objSq->gt("u", "th"));
ok($objSq->lt("x", "xh"));
ok($objSq->lt("xz","xh"));
ok($objSq->gt("y", "xh"));
ok($objSq->lt("z", "zh"));
ok($objSq->lt("zz","zh"));
ok($objSq->lt("zh","\x{1B7}"));

# 35

$objSq->change(level => 2);

ok($objSq->eq("c\x{327}", "C\x{327}"));
ok($objSq->eq("dh", "dH"));
ok($objSq->eq("dH", "Dh"));
ok($objSq->eq("Dh", "DH"));
ok($objSq->eq("e\x{308}", "E\x{308}"));
ok($objSq->eq("gj", "gJ"));
ok($objSq->eq("gJ", "Gj"));
ok($objSq->eq("Gj", "GJ"));
ok($objSq->eq("ll", "lL"));
ok($objSq->eq("lL", "Ll"));
ok($objSq->eq("Ll", "LL"));
ok($objSq->eq("nj", "nJ"));
ok($objSq->eq("nJ", "Nj"));
ok($objSq->eq("Nj", "NJ"));
ok($objSq->eq("rr", "rR"));
ok($objSq->eq("rR", "Rr"));
ok($objSq->eq("Rr", "RR"));
ok($objSq->eq("sh", "sH"));
ok($objSq->eq("sH", "Sh"));
ok($objSq->eq("Sh", "SH"));
ok($objSq->eq("th", "tH"));
ok($objSq->eq("tH", "Th"));
ok($objSq->eq("Th", "TH"));
ok($objSq->eq("xh", "xH"));
ok($objSq->eq("xH", "Xh"));
ok($objSq->eq("Xh", "XH"));
ok($objSq->eq("zh", "zH"));
ok($objSq->eq("zH", "Zh"));
ok($objSq->eq("Zh", "ZH"));

# 64

$objSq->change(level => 3);

ok($objSq->lt("c\x{327}", "C\x{327}"));
ok($objSq->lt("dh", "dH"));
ok($objSq->lt("dH", "Dh"));
ok($objSq->lt("Dh", "DH"));
ok($objSq->lt("e\x{308}", "E\x{308}"));
ok($objSq->lt("gj", "gJ"));
ok($objSq->lt("gJ", "Gj"));
ok($objSq->lt("Gj", "GJ"));
ok($objSq->lt("ll", "lL"));
ok($objSq->lt("lL", "Ll"));
ok($objSq->lt("Ll", "LL"));
ok($objSq->lt("nj", "nJ"));
ok($objSq->lt("nJ", "Nj"));
ok($objSq->lt("Nj", "NJ"));
ok($objSq->lt("rr", "rR"));
ok($objSq->lt("rR", "Rr"));
ok($objSq->lt("Rr", "RR"));
ok($objSq->lt("sh", "sH"));
ok($objSq->lt("sH", "Sh"));
ok($objSq->lt("Sh", "SH"));
ok($objSq->lt("th", "tH"));
ok($objSq->lt("tH", "Th"));
ok($objSq->lt("Th", "TH"));
ok($objSq->lt("xh", "xH"));
ok($objSq->lt("xH", "Xh"));
ok($objSq->lt("Xh", "XH"));
ok($objSq->lt("zh", "zH"));
ok($objSq->lt("zH", "Zh"));
ok($objSq->lt("Zh", "ZH"));

# 93

ok($objSq->eq("c\x{327}", _pack_U(0xE7)));
ok($objSq->eq("C\x{327}", _pack_U(0xC7)));
ok($objSq->eq("e\x{308}", _pack_U(0xEB)));
ok($objSq->eq("E\x{308}", _pack_U(0xCB)));

# 97

$objSq->change(upper_before_lower => 1);

ok($objSq->gt("c\x{327}", "C\x{327}"));
ok($objSq->gt("dh", "dH"));
ok($objSq->gt("dH", "Dh"));
ok($objSq->gt("Dh", "DH"));
ok($objSq->gt("e\x{308}", "E\x{308}"));
ok($objSq->gt("gj", "gJ"));
ok($objSq->gt("gJ", "Gj"));
ok($objSq->gt("Gj", "GJ"));
ok($objSq->gt("ll", "lL"));
ok($objSq->gt("lL", "Ll"));
ok($objSq->gt("Ll", "LL"));
ok($objSq->gt("nj", "nJ"));
ok($objSq->gt("nJ", "Nj"));
ok($objSq->gt("Nj", "NJ"));
ok($objSq->gt("rr", "rR"));
ok($objSq->gt("rR", "Rr"));
ok($objSq->gt("Rr", "RR"));
ok($objSq->gt("sh", "sH"));
ok($objSq->gt("sH", "Sh"));
ok($objSq->gt("Sh", "SH"));
ok($objSq->gt("th", "tH"));
ok($objSq->gt("tH", "Th"));
ok($objSq->gt("Th", "TH"));
ok($objSq->gt("xh", "xH"));
ok($objSq->gt("xH", "Xh"));
ok($objSq->gt("Xh", "XH"));
ok($objSq->gt("zh", "zH"));
ok($objSq->gt("zH", "Zh"));
ok($objSq->gt("Zh", "ZH"));

# 126
