
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

my $objBs = Unicode::Collate::Locale->
    new(locale => 'BS', normalization => undef);

ok($objBs->getlocale, 'bs');

$objBs->change(level => 1);

ok($objBs->lt("c", "c\x{30C}"));
ok($objBs->lt("c\x{30C}", "c\x{301}"));
ok($objBs->gt("d", "c\x{301}"));
ok($objBs->lt("d", "dz\x{30C}"));
ok($objBs->lt("dzz", "dz\x{30C}"));
ok($objBs->lt("dz\x{30C}", "d\x{335}"));
ok($objBs->gt("e", "d\x{335}"));
ok($objBs->lt("l", "lj"));
ok($objBs->lt("lz","lj"));
ok($objBs->gt("m", "lj"));
ok($objBs->lt("n", "nj"));
ok($objBs->lt("nz","nj"));
ok($objBs->gt("o", "nj"));
ok($objBs->lt("s", "s\x{30C}"));
ok($objBs->lt("sz","s\x{30C}"));
ok($objBs->gt("t", "s\x{30C}"));
ok($objBs->lt("z", "z\x{30C}"));
ok($objBs->lt("zz","z\x{30C}"));
ok($objBs->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 21

# not tailored
ok($objBs->lt("dZ\x{30C}","dz\x{30C}"));
ok($objBs->lt("lJ", "lj"));
ok($objBs->lt("nJ", "nj"));

# 24

$objBs->change(level => 2);

ok($objBs->eq("c\x{30C}", "C\x{30C}"));
ok($objBs->eq("c\x{301}", "C\x{301}"));
ok($objBs->eq("dz\x{30C}","Dz\x{30C}"));
ok($objBs->eq("Dz\x{30C}","DZ\x{30C}"));
ok($objBs->eq("d\x{335}", "D\x{335}"));
ok($objBs->eq("lj", "Lj"));
ok($objBs->eq("Lj", "LJ"));
ok($objBs->eq("nj", "Nj"));
ok($objBs->eq("Nj", "NJ"));
ok($objBs->eq("s\x{30C}", "S\x{30C}"));
ok($objBs->eq("z\x{30C}", "Z\x{30C}"));

# 35

ok($objBs->eq("dz\x{30C}", "\x{1C6}"));
ok($objBs->eq("Dz\x{30C}", "\x{1C6}"));
ok($objBs->eq("Dz\x{30C}", "\x{1C5}"));
ok($objBs->eq("DZ\x{30C}", "\x{1C5}"));
ok($objBs->eq("DZ\x{30C}", "\x{1C4}"));

ok($objBs->eq("lj", "\x{1C9}"));
ok($objBs->eq("Lj", "\x{1C9}"));
ok($objBs->eq("Lj", "\x{1C8}"));
ok($objBs->eq("LJ", "\x{1C8}"));
ok($objBs->eq("LJ", "\x{1C7}"));

ok($objBs->eq("nj", "\x{1CC}"));
ok($objBs->eq("Nj", "\x{1CC}"));
ok($objBs->eq("Nj", "\x{1CB}"));
ok($objBs->eq("NJ", "\x{1CB}"));
ok($objBs->eq("NJ", "\x{1CA}"));

# 50

$objBs->change(level => 3);

ok($objBs->lt("c\x{30C}", "C\x{30C}"));
ok($objBs->lt("c\x{301}", "C\x{301}"));
ok($objBs->lt("dz\x{30C}","Dz\x{30C}"));
ok($objBs->lt("Dz\x{30C}","DZ\x{30C}"));
ok($objBs->lt("d\x{335}", "D\x{335}"));
ok($objBs->lt("lj", "Lj"));
ok($objBs->lt("Lj", "LJ"));
ok($objBs->lt("nj", "Nj"));
ok($objBs->lt("Nj", "NJ"));
ok($objBs->lt("s\x{30C}", "S\x{30C}"));
ok($objBs->lt("z\x{30C}", "Z\x{30C}"));

# 61

ok($objBs->lt("dz\x{30C}", "\x{1C6}"));
ok($objBs->gt("Dz\x{30C}", "\x{1C6}"));
ok($objBs->lt("Dz\x{30C}", "\x{1C5}"));
ok($objBs->gt("DZ\x{30C}", "\x{1C5}"));
ok($objBs->lt("DZ\x{30C}", "\x{1C4}"));

ok($objBs->lt("lj", "\x{1C9}"));
ok($objBs->gt("Lj", "\x{1C9}"));
ok($objBs->lt("Lj", "\x{1C8}"));
ok($objBs->gt("LJ", "\x{1C8}"));
ok($objBs->lt("LJ", "\x{1C7}"));

ok($objBs->lt("nj", "\x{1CC}"));
ok($objBs->gt("Nj", "\x{1CC}"));
ok($objBs->lt("Nj", "\x{1CB}"));
ok($objBs->gt("NJ", "\x{1CB}"));
ok($objBs->lt("NJ", "\x{1CA}"));

# 76

ok($objBs->eq("c\x{30C}", "\x{10D}"));
ok($objBs->eq("C\x{30C}", "\x{10C}"));
ok($objBs->eq("c\x{301}", "\x{107}"));
ok($objBs->eq("c\x{341}", "\x{107}"));
ok($objBs->eq("C\x{301}", "\x{106}"));
ok($objBs->eq("C\x{341}", "\x{106}"));
ok($objBs->eq("dz\x{30C}", "d\x{17E}"));
ok($objBs->eq("dZ\x{30C}", "d\x{17D}"));
ok($objBs->eq("Dz\x{30C}", "D\x{17E}"));
ok($objBs->eq("DZ\x{30C}", "D\x{17D}"));
ok($objBs->eq("d\x{335}", "\x{111}"));
ok($objBs->eq("D\x{335}", "\x{110}"));
ok($objBs->eq("s\x{30C}", "\x{161}"));
ok($objBs->eq("S\x{30C}", "\x{160}"));
ok($objBs->eq("z\x{30C}", "\x{17E}"));
ok($objBs->eq("Z\x{30C}", "\x{17D}"));

# 92

$objBs->change(upper_before_lower => 1);

ok($objBs->gt("c\x{30C}", "C\x{30C}"));
ok($objBs->gt("c\x{301}", "C\x{301}"));
ok($objBs->gt("dz\x{30C}","Dz\x{30C}"));
ok($objBs->gt("Dz\x{30C}","DZ\x{30C}"));
ok($objBs->gt("d\x{335}", "D\x{335}"));
ok($objBs->gt("lj", "Lj"));
ok($objBs->gt("Lj", "LJ"));
ok($objBs->gt("nj", "Nj"));
ok($objBs->gt("Nj", "NJ"));
ok($objBs->gt("s\x{30C}", "S\x{30C}"));
ok($objBs->gt("z\x{30C}", "Z\x{30C}"));

# 103

ok($objBs->lt("DZ\x{30C}", "\x{1C4}"));
ok($objBs->gt("Dz\x{30C}", "\x{1C4}"));
ok($objBs->lt("Dz\x{30C}", "\x{1C5}"));
ok($objBs->gt("dz\x{30C}", "\x{1C5}"));
ok($objBs->lt("dz\x{30C}", "\x{1C6}"));

ok($objBs->lt("LJ", "\x{1C7}"));
ok($objBs->gt("Lj", "\x{1C7}"));
ok($objBs->lt("Lj", "\x{1C8}"));
ok($objBs->gt("lj", "\x{1C8}"));
ok($objBs->lt("lj", "\x{1C9}"));

ok($objBs->lt("NJ", "\x{1CA}"));
ok($objBs->gt("Nj", "\x{1CA}"));
ok($objBs->lt("Nj", "\x{1CB}"));
ok($objBs->gt("nj", "\x{1CB}"));
ok($objBs->lt("nj", "\x{1CC}"));

# 118
