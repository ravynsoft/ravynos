
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

my $objSrLatn = Unicode::Collate::Locale->
    new(locale => 'SR-LATN', normalization => undef);

ok($objSrLatn->getlocale, 'sr_Latn');

$objSrLatn->change(level => 1);

ok($objSrLatn->lt("c", "c\x{30C}"));
ok($objSrLatn->lt("c\x{30C}", "c\x{301}"));
ok($objSrLatn->gt("d", "c\x{301}"));
ok($objSrLatn->lt("d", "dz\x{30C}"));
ok($objSrLatn->lt("dzz", "dz\x{30C}"));
ok($objSrLatn->lt("dz\x{30C}", "d\x{335}"));
ok($objSrLatn->gt("e", "d\x{335}"));
ok($objSrLatn->lt("l", "lj"));
ok($objSrLatn->lt("lz","lj"));
ok($objSrLatn->gt("m", "lj"));
ok($objSrLatn->lt("n", "nj"));
ok($objSrLatn->lt("nz","nj"));
ok($objSrLatn->gt("o", "nj"));
ok($objSrLatn->lt("s", "s\x{30C}"));
ok($objSrLatn->lt("sz","s\x{30C}"));
ok($objSrLatn->gt("t", "s\x{30C}"));
ok($objSrLatn->lt("z", "z\x{30C}"));
ok($objSrLatn->lt("zz","z\x{30C}"));
ok($objSrLatn->lt("z\x{30C}", "\x{292}")); # U+0292 EZH

# 21

# not tailored
ok($objSrLatn->lt("dZ\x{30C}","dz\x{30C}"));
ok($objSrLatn->lt("lJ", "lj"));
ok($objSrLatn->lt("nJ", "nj"));

# 24

$objSrLatn->change(level => 2);

ok($objSrLatn->eq("c\x{30C}", "C\x{30C}"));
ok($objSrLatn->eq("c\x{301}", "C\x{301}"));
ok($objSrLatn->eq("dz\x{30C}","Dz\x{30C}"));
ok($objSrLatn->eq("Dz\x{30C}","DZ\x{30C}"));
ok($objSrLatn->eq("d\x{335}", "D\x{335}"));
ok($objSrLatn->eq("lj", "Lj"));
ok($objSrLatn->eq("Lj", "LJ"));
ok($objSrLatn->eq("nj", "Nj"));
ok($objSrLatn->eq("Nj", "NJ"));
ok($objSrLatn->eq("s\x{30C}", "S\x{30C}"));
ok($objSrLatn->eq("z\x{30C}", "Z\x{30C}"));

# 35

ok($objSrLatn->eq("dz\x{30C}", "\x{1C6}"));
ok($objSrLatn->eq("Dz\x{30C}", "\x{1C6}"));
ok($objSrLatn->eq("Dz\x{30C}", "\x{1C5}"));
ok($objSrLatn->eq("DZ\x{30C}", "\x{1C5}"));
ok($objSrLatn->eq("DZ\x{30C}", "\x{1C4}"));

ok($objSrLatn->eq("lj", "\x{1C9}"));
ok($objSrLatn->eq("Lj", "\x{1C9}"));
ok($objSrLatn->eq("Lj", "\x{1C8}"));
ok($objSrLatn->eq("LJ", "\x{1C8}"));
ok($objSrLatn->eq("LJ", "\x{1C7}"));

ok($objSrLatn->eq("nj", "\x{1CC}"));
ok($objSrLatn->eq("Nj", "\x{1CC}"));
ok($objSrLatn->eq("Nj", "\x{1CB}"));
ok($objSrLatn->eq("NJ", "\x{1CB}"));
ok($objSrLatn->eq("NJ", "\x{1CA}"));

# 50

$objSrLatn->change(level => 3);

ok($objSrLatn->lt("c\x{30C}", "C\x{30C}"));
ok($objSrLatn->lt("c\x{301}", "C\x{301}"));
ok($objSrLatn->lt("dz\x{30C}","Dz\x{30C}"));
ok($objSrLatn->lt("Dz\x{30C}","DZ\x{30C}"));
ok($objSrLatn->lt("d\x{335}", "D\x{335}"));
ok($objSrLatn->lt("lj", "Lj"));
ok($objSrLatn->lt("Lj", "LJ"));
ok($objSrLatn->lt("nj", "Nj"));
ok($objSrLatn->lt("Nj", "NJ"));
ok($objSrLatn->lt("s\x{30C}", "S\x{30C}"));
ok($objSrLatn->lt("z\x{30C}", "Z\x{30C}"));

# 61

ok($objSrLatn->lt("dz\x{30C}", "\x{1C6}"));
ok($objSrLatn->gt("Dz\x{30C}", "\x{1C6}"));
ok($objSrLatn->lt("Dz\x{30C}", "\x{1C5}"));
ok($objSrLatn->gt("DZ\x{30C}", "\x{1C5}"));
ok($objSrLatn->lt("DZ\x{30C}", "\x{1C4}"));

ok($objSrLatn->lt("lj", "\x{1C9}"));
ok($objSrLatn->gt("Lj", "\x{1C9}"));
ok($objSrLatn->lt("Lj", "\x{1C8}"));
ok($objSrLatn->gt("LJ", "\x{1C8}"));
ok($objSrLatn->lt("LJ", "\x{1C7}"));

ok($objSrLatn->lt("nj", "\x{1CC}"));
ok($objSrLatn->gt("Nj", "\x{1CC}"));
ok($objSrLatn->lt("Nj", "\x{1CB}"));
ok($objSrLatn->gt("NJ", "\x{1CB}"));
ok($objSrLatn->lt("NJ", "\x{1CA}"));

# 76

ok($objSrLatn->eq("c\x{30C}", "\x{10D}"));
ok($objSrLatn->eq("C\x{30C}", "\x{10C}"));
ok($objSrLatn->eq("c\x{301}", "\x{107}"));
ok($objSrLatn->eq("c\x{341}", "\x{107}"));
ok($objSrLatn->eq("C\x{301}", "\x{106}"));
ok($objSrLatn->eq("C\x{341}", "\x{106}"));
ok($objSrLatn->eq("dz\x{30C}", "d\x{17E}"));
ok($objSrLatn->eq("dZ\x{30C}", "d\x{17D}"));
ok($objSrLatn->eq("Dz\x{30C}", "D\x{17E}"));
ok($objSrLatn->eq("DZ\x{30C}", "D\x{17D}"));
ok($objSrLatn->eq("d\x{335}", "\x{111}"));
ok($objSrLatn->eq("D\x{335}", "\x{110}"));
ok($objSrLatn->eq("s\x{30C}", "\x{161}"));
ok($objSrLatn->eq("S\x{30C}", "\x{160}"));
ok($objSrLatn->eq("z\x{30C}", "\x{17E}"));
ok($objSrLatn->eq("Z\x{30C}", "\x{17D}"));

# 92

$objSrLatn->change(upper_before_lower => 1);

ok($objSrLatn->gt("c\x{30C}", "C\x{30C}"));
ok($objSrLatn->gt("c\x{301}", "C\x{301}"));
ok($objSrLatn->gt("dz\x{30C}","Dz\x{30C}"));
ok($objSrLatn->gt("Dz\x{30C}","DZ\x{30C}"));
ok($objSrLatn->gt("d\x{335}", "D\x{335}"));
ok($objSrLatn->gt("lj", "Lj"));
ok($objSrLatn->gt("Lj", "LJ"));
ok($objSrLatn->gt("nj", "Nj"));
ok($objSrLatn->gt("Nj", "NJ"));
ok($objSrLatn->gt("s\x{30C}", "S\x{30C}"));
ok($objSrLatn->gt("z\x{30C}", "Z\x{30C}"));

# 103

ok($objSrLatn->lt("DZ\x{30C}", "\x{1C4}"));
ok($objSrLatn->gt("Dz\x{30C}", "\x{1C4}"));
ok($objSrLatn->lt("Dz\x{30C}", "\x{1C5}"));
ok($objSrLatn->gt("dz\x{30C}", "\x{1C5}"));
ok($objSrLatn->lt("dz\x{30C}", "\x{1C6}"));

ok($objSrLatn->lt("LJ", "\x{1C7}"));
ok($objSrLatn->gt("Lj", "\x{1C7}"));
ok($objSrLatn->lt("Lj", "\x{1C8}"));
ok($objSrLatn->gt("lj", "\x{1C8}"));
ok($objSrLatn->lt("lj", "\x{1C9}"));

ok($objSrLatn->lt("NJ", "\x{1CA}"));
ok($objSrLatn->gt("Nj", "\x{1CA}"));
ok($objSrLatn->lt("Nj", "\x{1CB}"));
ok($objSrLatn->gt("nj", "\x{1CB}"));
ok($objSrLatn->lt("nj", "\x{1CC}"));

# 118
