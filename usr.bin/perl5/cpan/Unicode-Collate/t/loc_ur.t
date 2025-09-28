
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..166\n"; }
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

my $objUr = Unicode::Collate::Locale->
    new(locale => 'UR', normalization => undef);

ok($objUr->getlocale, 'ur');

$objUr->change(level => 3);

ok($objUr->eq("\x{623}", "\x{627}\x{654}"));
ok($objUr->eq("\x{622}", "\x{627}\x{653}"));
ok($objUr->eq("\x{624}", "\x{648}\x{654}"));
ok($objUr->eq("\x{6C2}", "\x{6C1}\x{654}"));
ok($objUr->eq("\x{626}", "\x{64A}\x{654}"));
ok($objUr->eq("\x{6D3}", "\x{6D2}\x{654}"));

# 8

$objUr->change(level => 2);

ok($objUr->lt("\x{627}", "\x{623}"));
ok($objUr->lt("\x{648}", "\x{624}"));
ok($objUr->lt("\x{6C1}", "\x{6C2}"));
ok($objUr->lt("\x{6CC}", "\x{626}"));
ok($objUr->lt("\x{6D2}", "\x{6D3}"));

# 13

ok($objUr->lt("\x{652}", "\x{64E}"));
ok($objUr->lt("\x{64E}", "\x{650}"));
ok($objUr->lt("\x{650}", "\x{64F}"));
ok($objUr->lt("\x{64F}", "\x{670}"));
ok($objUr->lt("\x{670}", "\x{656}"));
ok($objUr->lt("\x{656}", "\x{657}"));
ok($objUr->lt("\x{657}", "\x{64B}"));
ok($objUr->lt("\x{64B}", "\x{64D}"));
ok($objUr->lt("\x{64D}", "\x{64C}"));
ok($objUr->lt("\x{64C}", "\x{654}"));
ok($objUr->lt("\x{654}", "\x{651}"));
ok($objUr->lt("\x{651}", "\x{658}"));
ok($objUr->lt("\x{658}", "\x{653}"));
ok($objUr->lt("\x{653}", "\x{655}"));

# 27

$objUr->change(level => 1);

ok($objUr->eq("\x{627}", "\x{623}"));
ok($objUr->eq("\x{648}", "\x{624}"));
ok($objUr->eq("\x{6C1}", "\x{6C2}"));
ok($objUr->eq("\x{6CC}", "\x{626}"));
ok($objUr->eq("\x{6D2}", "\x{6D3}"));

# 32

ok($objUr->eq("\x{652}", "\x{64E}"));
ok($objUr->eq("\x{64E}", "\x{650}"));
ok($objUr->eq("\x{650}", "\x{64F}"));
ok($objUr->eq("\x{64F}", "\x{670}"));
ok($objUr->eq("\x{670}", "\x{656}"));
ok($objUr->eq("\x{656}", "\x{657}"));
ok($objUr->eq("\x{657}", "\x{64B}"));
ok($objUr->eq("\x{64B}", "\x{64D}"));
ok($objUr->eq("\x{64D}", "\x{64C}"));
ok($objUr->eq("\x{64C}", "\x{654}"));
ok($objUr->eq("\x{654}", "\x{651}"));
ok($objUr->eq("\x{651}", "\x{658}"));
ok($objUr->eq("\x{658}", "\x{653}"));
ok($objUr->eq("\x{653}", "\x{655}"));

# 46

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : "";
    $objUr->change(highestFFFF => 1) if $h;

    ok($objUr->lt("\x{627}$t",        "\x{622}"));
    ok($objUr->lt("\x{622}$t",        "\x{628}"));
    ok($objUr->lt("\x{628}$t",        "\x{628}\x{6BE}"));
    ok($objUr->lt("\x{628}\x{6BE}$t", "\x{67E}"));
    ok($objUr->lt("\x{67E}$t",        "\x{67E}\x{6BE}"));
    ok($objUr->lt("\x{67E}\x{6BE}$t", "\x{62A}"));
    ok($objUr->lt("\x{62A}$t",        "\x{62A}\x{6BE}"));
    ok($objUr->lt("\x{62A}\x{6BE}$t", "\x{679}"));
    ok($objUr->lt("\x{679}$t",        "\x{679}\x{6BE}"));
    ok($objUr->lt("\x{679}\x{6BE}$t", "\x{62B}"));
    ok($objUr->lt("\x{62B}$t",        "\x{62C}"));
    ok($objUr->lt("\x{62C}$t",        "\x{62C}\x{6BE}"));
    ok($objUr->lt("\x{62C}\x{6BE}$t", "\x{686}"));
    ok($objUr->lt("\x{686}$t",        "\x{686}\x{6BE}"));
    ok($objUr->lt("\x{686}\x{6BE}$t", "\x{62D}"));
    ok($objUr->lt("\x{62D}$t",        "\x{62E}"));
    ok($objUr->lt("\x{62E}$t",        "\x{62F}"));
    ok($objUr->lt("\x{62F}$t",        "\x{62F}\x{6BE}"));
    ok($objUr->lt("\x{62F}\x{6BE}$t", "\x{688}"));
    ok($objUr->lt("\x{688}$t",        "\x{688}\x{6BE}"));
    ok($objUr->lt("\x{688}\x{6BE}$t", "\x{630}"));
    ok($objUr->lt("\x{630}$t",        "\x{631}"));
    ok($objUr->lt("\x{631}$t",        "\x{631}\x{6BE}"));
    ok($objUr->lt("\x{631}\x{6BE}$t", "\x{691}"));
    ok($objUr->lt("\x{691}$t",        "\x{691}\x{6BE}"));
    ok($objUr->lt("\x{691}\x{6BE}$t", "\x{632}"));
    ok($objUr->lt("\x{632}$t",        "\x{698}"));
    ok($objUr->lt("\x{698}$t",        "\x{633}"));
    ok($objUr->lt("\x{633}$t",        "\x{634}"));
    ok($objUr->lt("\x{634}$t",        "\x{635}"));
    ok($objUr->lt("\x{635}$t",        "\x{636}"));
    ok($objUr->lt("\x{636}$t",        "\x{637}"));
    ok($objUr->lt("\x{637}$t",        "\x{638}"));
    ok($objUr->lt("\x{638}$t",        "\x{639}"));
    ok($objUr->lt("\x{639}$t",        "\x{63A}"));
    ok($objUr->lt("\x{63A}$t",        "\x{641}"));
    ok($objUr->lt("\x{641}$t",        "\x{642}"));
    ok($objUr->lt("\x{642}$t",        "\x{6A9}"));
    ok($objUr->lt("\x{6A9}$t",        "\x{6A9}\x{6BE}"));
    ok($objUr->lt("\x{6A9}\x{6BE}$t", "\x{6AF}"));
    ok($objUr->lt("\x{6AF}$t",        "\x{6AF}\x{6BE}"));
    ok($objUr->lt("\x{6AF}\x{6BE}$t", "\x{644}"));
    ok($objUr->lt("\x{644}$t",        "\x{644}\x{6BE}"));
    ok($objUr->lt("\x{644}\x{6BE}$t", "\x{645}"));
    ok($objUr->lt("\x{645}$t",        "\x{645}\x{6BE}"));
    ok($objUr->lt("\x{645}\x{6BE}$t", "\x{646}"));
    ok($objUr->lt("\x{646}$t",        "\x{646}\x{6BE}"));
    ok($objUr->lt("\x{646}\x{6BE}$t", "\x{6BA}"));
    ok($objUr->lt("\x{6BA}$t",        "\x{6BA}\x{6BE}"));
    ok($objUr->lt("\x{6BA}\x{6BE}$t", "\x{648}"));
    ok($objUr->lt("\x{648}$t",        "\x{648}\x{6BE}"));
    ok($objUr->lt("\x{648}\x{6BE}$t", "\x{6C1}"));
    ok($objUr->lt("\x{6C1}$t",        "\x{6BE}"));
    ok($objUr->lt("\x{6BE}$t",        "\x{6C3}"));
    ok($objUr->lt("\x{6C3}$t",        "\x{621}"));
    ok($objUr->lt("\x{621}$t",        "\x{6CC}"));
    ok($objUr->lt("\x{6CC}$t",        "\x{6CC}\x{6BE}"));
    ok($objUr->lt("\x{6CC}\x{6BE}$t", "\x{6D2}"));
    ok($objUr->lt("\x{6D2}$t",        "\x{66E}"));
    ok($objUr->lt("\x{66E}$t",        "\x{67B}"));
}

# 166

