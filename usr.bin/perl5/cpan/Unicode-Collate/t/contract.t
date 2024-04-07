
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

use Unicode::Collate;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

our $kjeEntry = <<'ENTRIES';
0301  ; [.0000.0032.0002.0301] # COMBINING ACUTE ACCENT
0334  ; [.0000.008B.0002.0334] # COMBINING TILDE OVERLAY
043A  ; [.0D31.0020.0002.043A] # CYRILLIC SMALL LETTER KA
041A  ; [.0D31.0020.0008.041A] # CYRILLIC CAPITAL LETTER KA
045C  ; [.0DA1.0020.0002.045C] # CYRILLIC SMALL LETTER KJE
043A 0301 ; [.0DA1.0020.0002.045C] # CYRILLIC SMALL LETTER KJE
040C  ; [.0DA1.0020.0008.040C] # CYRILLIC CAPITAL LETTER KJE
041A 0301 ; [.0DA1.0020.0008.040C] # CYRILLIC CAPITAL LETTER KJE
ENTRIES

our $aaEntry = <<'ENTRIES';
0304  ; [.0000.005A.0002.0304] # COMBINING MACRON (cc = 230)
030A  ; [.0000.0043.0002.030A] # COMBINING RING ABOVE (cc = 230)
0327  ; [.0000.0055.0002.0327] # COMBINING CEDILLA (cc = 202)
031A  ; [.0000.006B.0002.031A] # COMBINING LEFT ANGLE ABOVE (cc = 232)
0061  ; [.0A15.0020.0002.0061] # LATIN SMALL LETTER A
0041  ; [.0A15.0020.0008.0041] # LATIN CAPITAL LETTER A
007A  ; [.0C13.0020.0002.007A] # LATIN SMALL LETTER Z
005A  ; [.0C13.0020.0008.005A] # LATIN CAPITAL LETTER Z
00E5  ; [.0C25.0020.0002.00E5] # LATIN SMALL LETTER A WITH RING ABOVE; QQCM
00C5  ; [.0C25.0020.0008.00C5] # LATIN CAPITAL LETTER A WITH RING ABOVE; QQCM
0061 030A ; [.0C25.0020.0002.0061] # LATIN SMALL LETTER A WITH RING ABOVE
0041 030A ; [.0C25.0020.0008.0041] # LATIN CAPITAL LETTER A WITH RING ABOVE
ENTRIES

#########################

my $kjeNoN = Unicode::Collate->new(
    level => 1,
    table => undef,
    normalization => undef,
    entry => $kjeEntry,
);

ok($kjeNoN->lt("\x{43A}", "\x{43A}\x{301}"));
ok($kjeNoN->gt("\x{45C}", "\x{43A}\x{334}\x{301}"));
ok($kjeNoN->eq("\x{43A}", "\x{43A}\x{334}\x{301}"));
ok($kjeNoN->eq("\x{45C}", "\x{43A}\x{301}\x{334}"));

# 5

our %sortkeys;

$sortkeys{'KAac'} = $kjeNoN->viewSortKey("\x{43A}\x{301}");
$sortkeys{'KAta'} = $kjeNoN->viewSortKey("\x{43A}\x{334}\x{301}");
$sortkeys{'KAat'} = $kjeNoN->viewSortKey("\x{43A}\x{301}\x{334}");

eval { require Unicode::Normalize };
if (!$@) {
    my $kjeNFD = Unicode::Collate->new(
	level => 1,
	table => undef,
	entry => $kjeEntry,
    );

ok($kjeNFD->lt("\x{43A}", "\x{43A}\x{301}"));
ok($kjeNFD->eq("\x{45C}", "\x{43A}\x{334}\x{301}"));
ok($kjeNFD->lt("\x{43A}", "\x{43A}\x{334}\x{301}"));
ok($kjeNFD->eq("\x{45C}", "\x{43A}\x{301}\x{334}"));
# 9

    my $aaNFD = Unicode::Collate->new(
	level => 1,
	table => undef,
	entry => $aaEntry,
    );

ok($aaNFD->lt("Z", "A\x{30A}\x{304}"));
ok($aaNFD->eq("A", "A\x{304}\x{30A}"));
ok($aaNFD->eq(_pack_U(0xE5), "A\x{30A}\x{304}"));
ok($aaNFD->eq("A\x{304}", "A\x{304}\x{30A}"));
ok($aaNFD->lt("Z", "A\x{327}\x{30A}"));
ok($aaNFD->lt("Z", "A\x{30A}\x{327}"));
ok($aaNFD->lt("Z", "A\x{31A}\x{30A}"));
ok($aaNFD->lt("Z", "A\x{30A}\x{31A}"));
# 17

    my $aaPre = Unicode::Collate->new(
	level => 1,
	normalization => "prenormalized",
	table => undef,
	entry => $aaEntry,
    );

ok($aaPre->lt("Z", "A\x{30A}\x{304}"));
ok($aaPre->eq("A", "A\x{304}\x{30A}"));
ok($aaPre->eq(_pack_U(0xE5), "A\x{30A}\x{304}"));
ok($aaPre->eq("A\x{304}", "A\x{304}\x{30A}"));
ok($aaPre->lt("Z", "A\x{327}\x{30A}"));
ok($aaPre->lt("Z", "A\x{30A}\x{327}"));
ok($aaPre->lt("Z", "A\x{31A}\x{30A}"));
ok($aaPre->lt("Z", "A\x{30A}\x{31A}"));
# 25
} else {
    ok(1) for 1..20;
}

# again: loading Unicode::Normalize should not affect $kjeNoN.
ok($kjeNoN->lt("\x{43A}", "\x{43A}\x{301}"));
ok($kjeNoN->gt("\x{45C}", "\x{43A}\x{334}\x{301}"));
ok($kjeNoN->eq("\x{43A}", "\x{43A}\x{334}\x{301}"));
ok($kjeNoN->eq("\x{45C}", "\x{43A}\x{301}\x{334}"));

ok($sortkeys{'KAac'}, $kjeNoN->viewSortKey("\x{43A}\x{301}"));
ok($sortkeys{'KAta'}, $kjeNoN->viewSortKey("\x{43A}\x{334}\x{301}"));
ok($sortkeys{'KAat'}, $kjeNoN->viewSortKey("\x{43A}\x{301}\x{334}"));

# 32

my $aaNoN = Unicode::Collate->new(
    level => 1,
    table => undef,
    entry => $aaEntry,
    normalization => undef,
);

ok($aaNoN->lt("Z", "A\x{30A}\x{304}"));
ok($aaNoN->eq("A", "A\x{304}\x{30A}"));
ok($aaNoN->eq(_pack_U(0xE5), "A\x{30A}\x{304}"));
ok($aaNoN->eq("A\x{304}", "A\x{304}\x{30A}"));
ok($aaNoN->eq("A", "A\x{327}\x{30A}"));
ok($aaNoN->lt("Z", "A\x{30A}\x{327}"));
ok($aaNoN->eq("A", "A\x{31A}\x{30A}"));
ok($aaNoN->lt("Z", "A\x{30A}\x{31A}"));

# 40

# suppress contractions (not affected)

my $kjeSup = Unicode::Collate->new(
    level => 1,
    table => undef,
    normalization => undef,
    entry => $kjeEntry,
    suppress => [0x400..0x45F],
);

ok($kjeSup->lt("\x{43A}", "\x{43A}\x{301}"));
ok($kjeSup->eq("\x{45C}", "\x{43A}\x{301}"));
ok($kjeSup->lt("\x{41A}", "\x{41A}\x{301}"));
ok($kjeSup->eq("\x{40C}", "\x{41A}\x{301}"));

# 44

our $tibetanEntry = <<'ENTRIES';
0000           ; [.0000.0000.0000.0000] # [0000] NULL (in 6429)
0FB2           ; [.205B.0020.0002.0FB2] # TIBETAN SUBJOINED LETTER RA
0FB3           ; [.205E.0020.0002.0FB3] # TIBETAN SUBJOINED LETTER LA
0F71           ; [.206D.0020.0002.0F71] # TIBETAN VOWEL SIGN AA
0F72           ; [.206E.0020.0002.0F72] # TIBETAN VOWEL SIGN I
0F73           ; [.206F.0020.0002.0F73] # TIBETAN VOWEL SIGN II
0F71 0F72      ; [.206F.0020.0002.0F73] # TIBETAN VOWEL SIGN II
0F80           ; [.2070.0020.0002.0F80] # TIBETAN VOWEL SIGN REVERSED I
0F81           ; [.2071.0020.0002.0F81] # TIBETAN VOWEL SIGN REVERSED II
0F71 0F80      ; [.2071.0020.0002.0F81] # TIBETAN VOWEL SIGN REVERSED II
0F74           ; [.2072.0020.0002.0F74] # TIBETAN VOWEL SIGN U
0F75           ; [.2073.0020.0002.0F75] # TIBETAN VOWEL SIGN UU
0F71 0F74      ; [.2073.0020.0002.0F75] # TIBETAN VOWEL SIGN UU
0F76           ; [.2074.0020.0002.0F76] # TIBETAN VOWEL SIGN VOCALIC R
0FB2 0F80      ; [.2074.0020.0002.0F76] # TIBETAN VOWEL SIGN VOCALIC R
0F77           ; [.2075.0020.0002.0F77] # TIBETAN VOWEL SIGN VOCALIC RR
0FB2 0F81      ; [.2075.0020.0002.0F77] # TIBETAN VOWEL SIGN VOCALIC RR
0FB2 0F71 0F80 ; [.2075.0020.0002.0F77] # TIBETAN VOWEL SIGN VOCALIC RR
0F78           ; [.2076.0020.0002.0F78] # TIBETAN VOWEL SIGN VOCALIC L
0FB3 0F80      ; [.2076.0020.0002.0F78] # TIBETAN VOWEL SIGN VOCALIC L
0F79           ; [.2077.0020.0002.0F79] # TIBETAN VOWEL SIGN VOCALIC LL
0FB3 0F81      ; [.2077.0020.0002.0F79] # TIBETAN VOWEL SIGN VOCALIC LL
0FB3 0F71 0F80 ; [.2077.0020.0002.0F79] # TIBETAN VOWEL SIGN VOCALIC LL
ENTRIES

# ccc(0F71) = 129
# ccc(0F80) = 130
# 0F76 = 0FB2 0F80
# 0F78 = 0FB3 0F80
# 0F81 = 0F71 0F80
# 0F77 = <compat> 0FB2 0F81 = 0FB2 0F71 0F80 = 0F76 0F71
# 0F79 = <compat> 0FB3 0F81 = 0FB3 0F71 0F80 = 0F78 0F71

eval { require Unicode::Normalize };
if (!$@) {
    my $tibNFD = Unicode::Collate->new(
	table => undef,
	entry => $tibetanEntry,
	UCA_Version => 24,
    );

    # VOCALIC RR
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{334}\x{F81}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F81}\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F81}\0\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{F76}\x{334}\x{F71}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{F76}\x{F71}\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{F76}\x{F71}\0\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{334}\x{F71}\x{F80}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F71}\x{334}\x{F80}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F71}\x{F80}\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F71}\x{F80}\0\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{334}\x{F80}\x{F71}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F80}\x{334}\x{F71}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F80}\x{F71}\x{334}"));
    ok($tibNFD->eq("\x{F77}\0\x{334}", "\x{FB2}\x{F80}\x{F71}\0\x{334}"));
# 58

    # VOCALIC LL
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{334}\x{F81}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F81}\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F81}\0\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{F78}\x{334}\x{F71}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{F78}\x{F71}\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{F78}\x{F71}\0\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{334}\x{F71}\x{F80}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F71}\x{334}\x{F80}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F71}\x{F80}\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F71}\x{F80}\0\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{334}\x{F80}\x{F71}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F80}\x{334}\x{F71}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F80}\x{F71}\x{334}"));
    ok($tibNFD->eq("\x{F79}\0\x{334}", "\x{FB3}\x{F80}\x{F71}\0\x{334}"));
# 72

    my $a1 = "\x{FB2}\x{334}\x{F81}";
    my $b1 = "\x{F77}\0\x{334}";
    my $a2 = "\x{FB2}\x{334}\x{F81}";
    my $b2 = "\x{FB2}\x{F80}\0\x{334}\x{F71}";

    for my $v (qw/20 22 24 26 28/) {
	my $tib = Unicode::Collate->new(
	    table => undef,
	    entry => $tibetanEntry,
	    UCA_Version => $v,
	);
	my $long = 22 <= $v && $v <= 24;
	ok($tib->cmp($a1, $b1), $long ? 0 : -1);
	ok($tib->cmp($a2, $b2), $long ? 1 : 0);

	$tib->change(long_contraction => 0);
	ok($tib->cmp($a1, $b1), -1);
	ok($tib->cmp($a2, $b2),  0);

	$tib->change(long_contraction => 1);
	ok($tib->cmp($a1, $b1), 0);
	ok($tib->cmp($a2, $b2), 1);
    }
# 102

    # UCA_Version => 22
    ok($tibNFD->cmp($a1, $b1), 0);
    ok($tibNFD->cmp($a2, $b2), 1);

    $tibNFD->change(UCA_Version => 26); # not affect long_contraction
    ok($tibNFD->cmp($a1, $b1), 0);
    ok($tibNFD->cmp($a2, $b2), 1);
# 106

    my $discontNFD = Unicode::Collate->new(
	table => undef,
	UCA_Version => 22,
	entry => <<'ENTRIES',
0000  ; [.0000.0000.0000.0000] # [0000] NULL (in 6429)
0301  ; [.0000.0032.0002.0301] # COMBINING ACUTE ACCENT
0300  ; [.0000.0035.0002.0300] # COMBINING GRAVE ACCENT
0327  ; [.0000.0055.0002.0327] # COMBINING CEDILLA
0334  ; [.0000.008B.0002.0334] # COMBINING TILDE OVERLAY
0041  ; [.0101.0020.0008.0041] # LATIN CAPITAL LETTER A
0041 0327 0301 ; [.0102.0020.0008.0041]
0041 0300 ; [.0103.0020.0008.0041]
ENTRIES
    );

    ok($discontNFD->eq("A\x{334}\x{327}\x{301}", "A\x{327}\x{301}\0\x{334}"));
    ok($discontNFD->eq("A\x{327}\x{300}",        "A\x{300}\0\x{327}"));

    $discontNFD->change(long_contraction => 0);
    ok($discontNFD->lt("A\x{334}\x{327}\x{301}", "A\x{327}\x{301}\0\x{334}"));
    ok($discontNFD->eq("A\x{334}\x{327}\x{301}", "A\0\x{327}\x{301}\x{334}"));
    ok($discontNFD->eq("A\x{327}\x{300}",        "A\x{300}\0\x{327}"));

    $discontNFD->change(level => 1);
    ok($discontNFD->gt("A\x{327}\x{300}", "A\x{327}\0\x{300}"));

    # discontiguous
    ok($discontNFD->lt("A\x{334}\x{327}\x{301}", "A\x{327}\x{301}\0\x{334}"));
    ok($discontNFD->lt("A\x{334}\x{327}\x{301}", "A\x{300}"));
    ok($discontNFD->eq("A\x{334}\x{327}\x{301}", "A"));

    # contiguous
    ok($discontNFD->eq("A\x{327}\x{301}", "A\x{327}\x{301}\0\x{334}"));
    ok($discontNFD->lt("A\x{327}\x{301}", "A\x{300}"));
    ok($discontNFD->gt("A\x{327}\x{301}", "A"));
} else {
    ok(1) for 1..74;
}
# 118
