
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..87\n"; }
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

my $objDsb = Unicode::Collate::Locale->
    new(locale => 'DSB', normalization => undef);

ok($objDsb->getlocale, 'dsb');

$objDsb->change(level => 1);

my @prim = (
    "cz", "c\x{30C}", "c\x{301}", "d", # 5
    "ez", "e\x{30C}", "f",             # 8
    "hz", "ch", "i",                   # 11
    "kz", "l\x{335}", "l",             # 14
    "nz", "n\x{301}", "o",             # 17
    "rz", "r\x{301}", "s",             # 20
    "sz", "s\x{30C}", "s\x{301}", "t", # 24
    "zz", "z\x{30C}", "z\x{301}", "\x{292}" # EZH
);

for (my $i = 1; $i < @prim; $i++) {
    ok($objDsb->lt($prim[$i-1], $prim[$i]));
}

# 28

$objDsb->change(level => 2);

ok($objDsb->eq("c\x{30C}", "C\x{30C}"));
ok($objDsb->eq("c\x{301}", "C\x{301}"));
ok($objDsb->eq("e\x{30C}", "E\x{30C}"));
ok($objDsb->eq("l\x{335}", "L\x{335}"));
ok($objDsb->eq("n\x{301}", "N\x{301}"));
ok($objDsb->eq("r\x{301}", "R\x{301}"));
ok($objDsb->eq("s\x{30C}", "S\x{30C}"));
ok($objDsb->eq("s\x{301}", "S\x{301}"));
ok($objDsb->eq("z\x{30C}", "Z\x{30C}"));
ok($objDsb->eq("z\x{301}", "Z\x{301}"));

ok($objDsb->eq("ch", "cH"));
ok($objDsb->eq("cH", "Ch"));
ok($objDsb->eq("Ch", "CH"));

# 41

$objDsb->change(level => 3);

ok($objDsb->lt("c\x{30C}", "C\x{30C}"));
ok($objDsb->lt("c\x{301}", "C\x{301}"));
ok($objDsb->lt("e\x{30C}", "E\x{30C}"));
ok($objDsb->lt("l\x{335}", "L\x{335}"));
ok($objDsb->lt("n\x{301}", "N\x{301}"));
ok($objDsb->lt("r\x{301}", "R\x{301}"));
ok($objDsb->lt("s\x{30C}", "S\x{30C}"));
ok($objDsb->lt("s\x{301}", "S\x{301}"));
ok($objDsb->lt("z\x{30C}", "Z\x{30C}"));
ok($objDsb->lt("z\x{301}", "Z\x{301}"));

ok($objDsb->lt("ch", "cH"));
ok($objDsb->lt("cH", "Ch"));
ok($objDsb->lt("Ch", "CH"));

# 54

ok($objDsb->eq("c\x{30C}", "\x{10D}"));
ok($objDsb->eq("C\x{30C}", "\x{10C}"));
ok($objDsb->eq("c\x{301}", "\x{107}"));
ok($objDsb->eq("c\x{341}", "\x{107}"));
ok($objDsb->eq("C\x{301}", "\x{106}"));
ok($objDsb->eq("C\x{341}", "\x{106}"));
ok($objDsb->eq("e\x{30C}", "\x{11B}"));
ok($objDsb->eq("E\x{30C}", "\x{11A}"));
ok($objDsb->eq("l\x{335}", "\x{142}"));
ok($objDsb->eq("L\x{335}", "\x{141}"));
ok($objDsb->eq("n\x{301}", "\x{144}"));
ok($objDsb->eq("n\x{341}", "\x{144}"));
ok($objDsb->eq("N\x{301}", "\x{143}"));
ok($objDsb->eq("N\x{341}", "\x{143}"));
ok($objDsb->eq("r\x{301}", "\x{155}"));
ok($objDsb->eq("r\x{341}", "\x{155}"));
ok($objDsb->eq("R\x{301}", "\x{154}"));
ok($objDsb->eq("R\x{341}", "\x{154}"));
ok($objDsb->eq("s\x{30C}", "\x{161}"));
ok($objDsb->eq("S\x{30C}", "\x{160}"));
ok($objDsb->eq("s\x{301}", "\x{15B}"));
ok($objDsb->eq("s\x{341}", "\x{15B}"));
ok($objDsb->eq("S\x{301}", "\x{15A}"));
ok($objDsb->eq("S\x{341}", "\x{15A}"));
ok($objDsb->eq("z\x{30C}", "\x{17E}"));
ok($objDsb->eq("Z\x{30C}", "\x{17D}"));
ok($objDsb->eq("z\x{301}", "\x{17A}"));
ok($objDsb->eq("z\x{341}", "\x{17A}"));
ok($objDsb->eq("Z\x{301}", "\x{179}"));
ok($objDsb->eq("Z\x{341}", "\x{179}"));

# 84

$objDsb->change(upper_before_lower => 1);

ok($objDsb->gt("ch", "cH"));
ok($objDsb->gt("cH", "Ch"));
ok($objDsb->gt("Ch", "CH"));

# 87
