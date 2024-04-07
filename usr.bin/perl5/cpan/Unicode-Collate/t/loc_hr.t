
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..118\n"; }
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

my $objHr = Unicode::Collate::Locale->
    new(locale => 'HR', normalization => undef);

ok($objHr->getlocale, 'hr');

$objHr->change(level => 1);

ok($objHr->lt("c", "c\x{30C}"));
ok($objHr->lt("c\x{30C}", "c\x{301}"));
ok($objHr->gt("d", "c\x{301}"));
ok($objHr->lt("d", "dz\x{30C}"));
ok($objHr->lt("dzz", "dz\x{30C}"));
ok($objHr->lt("dz\x{30C}", "d\x{335}"));
ok($objHr->gt("e", "d\x{335}"));
ok($objHr->lt("l", "lj"));
ok($objHr->lt("lz","lj"));
ok($objHr->gt("m", "lj"));
ok($objHr->lt("n", "nj"));
ok($objHr->lt("nz","nj"));
ok($objHr->gt("o", "nj"));
ok($objHr->lt("s", "s\x{30C}"));
ok($objHr->lt("sz","s\x{30C}"));
ok($objHr->gt("t", "s\x{30C}"));
ok($objHr->lt("z", "z\x{30C}"));
ok($objHr->lt("zz","z\x{30C}"));
ok($objHr->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 21

# not tailored
ok($objHr->lt("dZ\x{30C}","dz\x{30C}"));
ok($objHr->lt("lJ", "lj"));
ok($objHr->lt("nJ", "nj"));

# 24

$objHr->change(level => 2);

ok($objHr->eq("c\x{30C}", "C\x{30C}"));
ok($objHr->eq("c\x{301}", "C\x{301}"));
ok($objHr->eq("dz\x{30C}","Dz\x{30C}"));
ok($objHr->eq("Dz\x{30C}","DZ\x{30C}"));
ok($objHr->eq("d\x{335}", "D\x{335}"));
ok($objHr->eq("lj", "Lj"));
ok($objHr->eq("Lj", "LJ"));
ok($objHr->eq("nj", "Nj"));
ok($objHr->eq("Nj", "NJ"));
ok($objHr->eq("s\x{30C}", "S\x{30C}"));
ok($objHr->eq("z\x{30C}", "Z\x{30C}"));

# 35

ok($objHr->eq("dz\x{30C}", "\x{1C6}"));
ok($objHr->eq("Dz\x{30C}", "\x{1C6}"));
ok($objHr->eq("Dz\x{30C}", "\x{1C5}"));
ok($objHr->eq("DZ\x{30C}", "\x{1C5}"));
ok($objHr->eq("DZ\x{30C}", "\x{1C4}"));

ok($objHr->eq("lj", "\x{1C9}"));
ok($objHr->eq("Lj", "\x{1C9}"));
ok($objHr->eq("Lj", "\x{1C8}"));
ok($objHr->eq("LJ", "\x{1C8}"));
ok($objHr->eq("LJ", "\x{1C7}"));

ok($objHr->eq("nj", "\x{1CC}"));
ok($objHr->eq("Nj", "\x{1CC}"));
ok($objHr->eq("Nj", "\x{1CB}"));
ok($objHr->eq("NJ", "\x{1CB}"));
ok($objHr->eq("NJ", "\x{1CA}"));

# 50

$objHr->change(level => 3);

ok($objHr->lt("c\x{30C}", "C\x{30C}"));
ok($objHr->lt("c\x{301}", "C\x{301}"));
ok($objHr->lt("dz\x{30C}","Dz\x{30C}"));
ok($objHr->lt("Dz\x{30C}","DZ\x{30C}"));
ok($objHr->lt("d\x{335}", "D\x{335}"));
ok($objHr->lt("lj", "Lj"));
ok($objHr->lt("Lj", "LJ"));
ok($objHr->lt("nj", "Nj"));
ok($objHr->lt("Nj", "NJ"));
ok($objHr->lt("s\x{30C}", "S\x{30C}"));
ok($objHr->lt("z\x{30C}", "Z\x{30C}"));

# 61

ok($objHr->lt("dz\x{30C}", "\x{1C6}"));
ok($objHr->gt("Dz\x{30C}", "\x{1C6}"));
ok($objHr->lt("Dz\x{30C}", "\x{1C5}"));
ok($objHr->gt("DZ\x{30C}", "\x{1C5}"));
ok($objHr->lt("DZ\x{30C}", "\x{1C4}"));

ok($objHr->lt("lj", "\x{1C9}"));
ok($objHr->gt("Lj", "\x{1C9}"));
ok($objHr->lt("Lj", "\x{1C8}"));
ok($objHr->gt("LJ", "\x{1C8}"));
ok($objHr->lt("LJ", "\x{1C7}"));

ok($objHr->lt("nj", "\x{1CC}"));
ok($objHr->gt("Nj", "\x{1CC}"));
ok($objHr->lt("Nj", "\x{1CB}"));
ok($objHr->gt("NJ", "\x{1CB}"));
ok($objHr->lt("NJ", "\x{1CA}"));

# 76

ok($objHr->eq("c\x{30C}", "\x{10D}"));
ok($objHr->eq("C\x{30C}", "\x{10C}"));
ok($objHr->eq("c\x{301}", "\x{107}"));
ok($objHr->eq("c\x{341}", "\x{107}"));
ok($objHr->eq("C\x{301}", "\x{106}"));
ok($objHr->eq("C\x{341}", "\x{106}"));
ok($objHr->eq("dz\x{30C}", "d\x{17E}"));
ok($objHr->eq("dZ\x{30C}", "d\x{17D}"));
ok($objHr->eq("Dz\x{30C}", "D\x{17E}"));
ok($objHr->eq("DZ\x{30C}", "D\x{17D}"));
ok($objHr->eq("d\x{335}", "\x{111}"));
ok($objHr->eq("D\x{335}", "\x{110}"));
ok($objHr->eq("s\x{30C}", "\x{161}"));
ok($objHr->eq("S\x{30C}", "\x{160}"));
ok($objHr->eq("z\x{30C}", "\x{17E}"));
ok($objHr->eq("Z\x{30C}", "\x{17D}"));

# 92

$objHr->change(upper_before_lower => 1);

ok($objHr->gt("c\x{30C}", "C\x{30C}"));
ok($objHr->gt("c\x{301}", "C\x{301}"));
ok($objHr->gt("dz\x{30C}","Dz\x{30C}"));
ok($objHr->gt("Dz\x{30C}","DZ\x{30C}"));
ok($objHr->gt("d\x{335}", "D\x{335}"));
ok($objHr->gt("lj", "Lj"));
ok($objHr->gt("Lj", "LJ"));
ok($objHr->gt("nj", "Nj"));
ok($objHr->gt("Nj", "NJ"));
ok($objHr->gt("s\x{30C}", "S\x{30C}"));
ok($objHr->gt("z\x{30C}", "Z\x{30C}"));

# 103

ok($objHr->lt("DZ\x{30C}", "\x{1C4}"));
ok($objHr->gt("Dz\x{30C}", "\x{1C4}"));
ok($objHr->lt("Dz\x{30C}", "\x{1C5}"));
ok($objHr->gt("dz\x{30C}", "\x{1C5}"));
ok($objHr->lt("dz\x{30C}", "\x{1C6}"));

ok($objHr->lt("LJ", "\x{1C7}"));
ok($objHr->gt("Lj", "\x{1C7}"));
ok($objHr->lt("Lj", "\x{1C8}"));
ok($objHr->gt("lj", "\x{1C8}"));
ok($objHr->lt("lj", "\x{1C9}"));

ok($objHr->lt("NJ", "\x{1CA}"));
ok($objHr->gt("Nj", "\x{1CA}"));
ok($objHr->lt("Nj", "\x{1CB}"));
ok($objHr->gt("nj", "\x{1CB}"));
ok($objHr->lt("nj", "\x{1CC}"));

# 118
