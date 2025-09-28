
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..90\n"; }
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

my $objFa = Unicode::Collate::Locale->
    new(locale => 'FA', normalization => undef);

ok($objFa->getlocale, 'fa');

$objFa->change(level => 3);

ok($objFa->eq("\x{622}", "\x{627}\x{653}"));
ok($objFa->eq("\x{623}", "\x{627}\x{654}"));
ok($objFa->eq("\x{625}", "\x{627}\x{655}"));
ok($objFa->eq("\x{624}", "\x{648}\x{654}"));
ok($objFa->eq("\x{626}", "\x{64A}\x{654}"));
ok($objFa->eq("\x{6C2}", "\x{6C1}\x{654}"));
ok($objFa->eq("\x{6C0}", "\x{6D5}\x{654}"));
ok($objFa->eq("\x{6D3}", "\x{6D2}\x{654}"));

# 10

ok($objFa->lt("\x{6CC}\x{654}", "\x{649}\x{654}"));
ok($objFa->lt("\x{649}\x{654}", "\x{626}"));

$objFa->change(level => 2);

ok($objFa->eq("\x{6CC}\x{654}", "\x{649}\x{654}"));
ok($objFa->eq("\x{649}\x{654}", "\x{626}"));

# 14

ok($objFa->lt("\x{64E}", "\x{650}"));
ok($objFa->lt("\x{650}", "\x{64F}"));
ok($objFa->lt("\x{64F}", "\x{64B}"));
ok($objFa->lt("\x{64B}", "\x{64D}"));
ok($objFa->lt("\x{64D}", "\x{64C}"));

ok($objFa->lt("\x{627}", "\x{671}"));

ok($objFa->lt("\x{621}", "\x{623}"));
ok($objFa->lt("\x{623}", "\x{672}"));
ok($objFa->lt("\x{672}", "\x{625}"));
ok($objFa->lt("\x{625}", "\x{673}"));
ok($objFa->lt("\x{673}", "\x{624}"));
ok($objFa->lt("\x{624}", "\x{6CC}\x{654}"));

ok($objFa->lt("\x{6A9}", "\x{6AA}"));
ok($objFa->lt("\x{6AA}", "\x{6AB}"));
ok($objFa->lt("\x{6AB}", "\x{643}"));
ok($objFa->lt("\x{643}", "\x{6AC}"));
ok($objFa->lt("\x{6AC}", "\x{6AD}"));
ok($objFa->lt("\x{6AD}", "\x{6AE}"));

ok($objFa->lt("\x{647}", "\x{6D5}"));
ok($objFa->lt("\x{6D5}", "\x{6C1}"));
ok($objFa->lt("\x{6C1}", "\x{629}"));
ok($objFa->lt("\x{629}", "\x{6C3}"));
ok($objFa->lt("\x{6C3}", "\x{6C0}"));
ok($objFa->lt("\x{6C0}", "\x{6BE}"));

ok($objFa->lt("\x{6CC}", "\x{649}"));
ok($objFa->lt("\x{649}", "\x{6D2}"));
ok($objFa->lt("\x{6D2}", "\x{64A}"));
ok($objFa->lt("\x{64A}", "\x{6D0}"));
ok($objFa->lt("\x{6D0}", "\x{6D1}"));
ok($objFa->lt("\x{6D1}", "\x{6CD}"));
ok($objFa->lt("\x{6CD}", "\x{6CE}"));

# 45

$objFa->change(level => 1);

ok($objFa->eq("\x{64E}", "\x{650}"));
ok($objFa->eq("\x{650}", "\x{64F}"));
ok($objFa->eq("\x{64F}", "\x{64B}"));
ok($objFa->eq("\x{64B}", "\x{64D}"));
ok($objFa->eq("\x{64D}", "\x{64C}"));

ok($objFa->eq("\x{627}", "\x{671}"));

ok($objFa->eq("\x{621}", "\x{623}"));
ok($objFa->eq("\x{623}", "\x{672}"));
ok($objFa->eq("\x{672}", "\x{625}"));
ok($objFa->eq("\x{625}", "\x{673}"));
ok($objFa->eq("\x{673}", "\x{624}"));
ok($objFa->eq("\x{624}", "\x{6CC}\x{654}"));

ok($objFa->eq("\x{6A9}", "\x{6AA}"));
ok($objFa->eq("\x{6AA}", "\x{6AB}"));
ok($objFa->eq("\x{6AB}", "\x{643}"));
ok($objFa->eq("\x{643}", "\x{6AC}"));
ok($objFa->eq("\x{6AC}", "\x{6AD}"));
ok($objFa->eq("\x{6AD}", "\x{6AE}"));

ok($objFa->eq("\x{647}", "\x{6D5}"));
ok($objFa->eq("\x{6D5}", "\x{6C1}"));
ok($objFa->eq("\x{6C1}", "\x{629}"));
ok($objFa->eq("\x{629}", "\x{6C3}"));
ok($objFa->eq("\x{6C3}", "\x{6C0}"));
ok($objFa->eq("\x{6C0}", "\x{6BE}"));

ok($objFa->eq("\x{6CC}", "\x{649}"));
ok($objFa->eq("\x{649}", "\x{6D2}"));
ok($objFa->eq("\x{6D2}", "\x{64A}"));
ok($objFa->eq("\x{64A}", "\x{6D0}"));
ok($objFa->eq("\x{6D0}", "\x{6D1}"));
ok($objFa->eq("\x{6D1}", "\x{6CD}"));
ok($objFa->eq("\x{6CD}", "\x{6CE}"));

# 76

for my $h (0, 1) {
    no warnings 'utf8';
    my $t = $h ? _pack_U(0xFFFF) : "";
    $objFa->change(highestFFFF => 1) if $h;

    ok($objFa->lt("\x{8AC}$t", "\x{622}"));
    ok($objFa->lt("\x{622}$t", "\x{627}"));
    ok($objFa->lt("\x{627}$t", "\x{621}"));
    ok($objFa->lt("\x{621}$t", "\x{66E}"));
    ok($objFa->lt("\x{66E}$t", "\x{628}"));

    ok($objFa->lt("\x{6CF}$t", "\x{647}"));
    ok($objFa->lt("\x{647}$t", "\x{778}"));
}
# 90
