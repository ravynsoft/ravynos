
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..73\n"; }
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

my $objPl = Unicode::Collate::Locale->
    new(locale => 'PL', normalization => undef);

ok($objPl->getlocale, 'pl');

$objPl->change(level => 1);

ok($objPl->lt("a", "a\x{328}"));
ok($objPl->lt("az","a\x{328}"));
ok($objPl->gt("b", "a\x{328}"));
ok($objPl->lt("c", "c\x{301}"));
ok($objPl->lt("cz","c\x{301}"));
ok($objPl->gt("d", "c\x{301}"));
ok($objPl->lt("e", "e\x{328}"));
ok($objPl->lt("ez","e\x{328}"));
ok($objPl->gt("f", "e\x{328}"));
ok($objPl->lt("l", "l\x{335}"));
ok($objPl->lt("lz","l\x{335}"));
ok($objPl->gt("m", "l\x{335}"));
ok($objPl->lt("n", "n\x{301}"));
ok($objPl->lt("nz","n\x{301}"));
ok($objPl->gt("o", "n\x{301}"));
ok($objPl->lt("o", "o\x{301}"));
ok($objPl->lt("oz","o\x{301}"));
ok($objPl->gt("p", "o\x{301}"));
ok($objPl->lt("s", "s\x{301}"));
ok($objPl->lt("sz","s\x{301}"));
ok($objPl->gt("t", "s\x{301}"));
ok($objPl->lt("z", "z\x{301}"));
ok($objPl->lt("zz","z\x{301}"));
ok($objPl->lt("z\x{301}", "z\x{307}"));
ok($objPl->lt("z\x{307}", "\x{292}")); # U+0292 EZH

# 27

$objPl->change(level => 2);

ok($objPl->eq("a\x{328}", "A\x{328}"));
ok($objPl->eq("c\x{301}", "C\x{301}"));
ok($objPl->eq("e\x{328}", "E\x{328}"));
ok($objPl->eq("l\x{335}", "L\x{335}"));
ok($objPl->eq("n\x{301}", "N\x{301}"));
ok($objPl->eq("o\x{301}", "O\x{301}"));
ok($objPl->eq("s\x{301}", "S\x{301}"));
ok($objPl->eq("z\x{301}", "Z\x{301}"));
ok($objPl->eq("z\x{307}", "Z\x{307}"));

# 36

$objPl->change(level => 3);

ok($objPl->lt("a\x{328}", "A\x{328}"));
ok($objPl->lt("c\x{301}", "C\x{301}"));
ok($objPl->lt("e\x{328}", "E\x{328}"));
ok($objPl->lt("l\x{335}", "L\x{335}"));
ok($objPl->lt("n\x{301}", "N\x{301}"));
ok($objPl->lt("o\x{301}", "O\x{301}"));
ok($objPl->lt("s\x{301}", "S\x{301}"));
ok($objPl->lt("z\x{301}", "Z\x{301}"));
ok($objPl->lt("z\x{307}", "Z\x{307}"));

# 45

ok($objPl->eq("a\x{328}", "\x{105}"));
ok($objPl->eq("A\x{328}", "\x{104}"));
ok($objPl->eq("c\x{301}", "\x{107}"));
ok($objPl->eq("c\x{341}", "\x{107}"));
ok($objPl->eq("C\x{301}", "\x{106}"));
ok($objPl->eq("C\x{341}", "\x{106}"));
ok($objPl->eq("e\x{328}", "\x{119}"));
ok($objPl->eq("E\x{328}", "\x{118}"));
ok($objPl->eq("l\x{335}", "\x{142}"));
ok($objPl->eq("L\x{335}", "\x{141}"));
ok($objPl->eq("n\x{301}", "\x{144}"));
ok($objPl->eq("n\x{341}", "\x{144}"));
ok($objPl->eq("N\x{301}", "\x{143}"));
ok($objPl->eq("N\x{341}", "\x{143}"));
ok($objPl->eq("o\x{301}", _pack_U(0xF3)));
ok($objPl->eq("o\x{341}", _pack_U(0xF3)));
ok($objPl->eq("O\x{301}", _pack_U(0xD3)));
ok($objPl->eq("O\x{341}", _pack_U(0xD3)));
ok($objPl->eq("s\x{301}", "\x{15B}"));
ok($objPl->eq("s\x{341}", "\x{15B}"));
ok($objPl->eq("S\x{301}", "\x{15A}"));
ok($objPl->eq("S\x{341}", "\x{15A}"));
ok($objPl->eq("z\x{301}", "\x{17A}"));
ok($objPl->eq("z\x{341}", "\x{17A}"));
ok($objPl->eq("Z\x{301}", "\x{179}"));
ok($objPl->eq("Z\x{341}", "\x{179}"));
ok($objPl->eq("z\x{307}", "\x{17C}"));
ok($objPl->eq("Z\x{307}", "\x{17B}"));

# 73
