
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..58\n"; }
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

# a standard collator (3.1.1)
my $Collator = Unicode::Collate->new(
  level => 1,
  table => 'keys.txt',
  normalization => undef,

  entry => <<'ENTRIES',
326E  ; [.1831.0020.0006.326E][.188D.0020.0006.326E] # c.h.s. GA
326F  ; [.1833.0020.0006.326F][.188D.0020.0006.326F] # c.h.s. NA
3270  ; [.1834.0020.0006.3270][.188D.0020.0006.3270] # c.h.s. DA
3271  ; [.1836.0020.0006.3271][.188D.0020.0006.3271] # c.h.s. RA
3272  ; [.1837.0020.0006.3272][.188D.0020.0006.3272] # c.h.s. MA
3273  ; [.1838.0020.0006.3273][.188D.0020.0006.3273] # c.h.s. BA
3274  ; [.183A.0020.0006.3274][.188D.0020.0006.3274] # c.h.s. SA
3275  ; [.183C.0020.0006.3275][.188D.0020.0006.3275] # c.h.s. A
3276  ; [.183D.0020.0006.3276][.188D.0020.0006.3276] # c.h.s. JA
3277  ; [.183F.0020.0006.3277][.188D.0020.0006.3277] # c.h.s. CA
3278  ; [.1840.0020.0006.3278][.188D.0020.0006.3278] # c.h.s. KA
3279  ; [.1841.0020.0006.3279][.188D.0020.0006.3279] # c.h.s. TA
327A  ; [.1842.0020.0006.327A][.188D.0020.0006.327A] # c.h.s. PA
327B  ; [.1843.0020.0006.327B][.188D.0020.0006.327B] # c.h.s. HA
ENTRIES
);

my $hangul = Unicode::Collate->new(
  level => 1,
  table => 'keys.txt',
  normalization => undef,
  hangul_terminator => 16,

  entry => <<'ENTRIES',
326E  ; [.1831.0020.0006.326E][.188D.0020.0006.326E] # c.h.s. GA
326F  ; [.1833.0020.0006.326F][.188D.0020.0006.326F] # c.h.s. NA
3270  ; [.1834.0020.0006.3270][.188D.0020.0006.3270] # c.h.s. DA
3271  ; [.1836.0020.0006.3271][.188D.0020.0006.3271] # c.h.s. RA
3272  ; [.1837.0020.0006.3272][.188D.0020.0006.3272] # c.h.s. MA
3273  ; [.1838.0020.0006.3273][.188D.0020.0006.3273] # c.h.s. BA
3274  ; [.183A.0020.0006.3274][.188D.0020.0006.3274] # c.h.s. SA
3275  ; [.183C.0020.0006.3275][.188D.0020.0006.3275] # c.h.s. A
3276  ; [.183D.0020.0006.3276][.188D.0020.0006.3276] # c.h.s. JA
3277  ; [.183F.0020.0006.3277][.188D.0020.0006.3277] # c.h.s. CA
3278  ; [.1840.0020.0006.3278][.188D.0020.0006.3278] # c.h.s. KA
3279  ; [.1841.0020.0006.3279][.188D.0020.0006.3279] # c.h.s. TA
327A  ; [.1842.0020.0006.327A][.188D.0020.0006.327A] # c.h.s. PA
327B  ; [.1843.0020.0006.327B][.188D.0020.0006.327B] # c.h.s. HA
ENTRIES
);

ok(ref $hangul, "Unicode::Collate");

#########################

# LVX vs LVV: /GAA/ vs /GA/.latinA
ok($Collator->gt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}A"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}A"));

# LVX vs LVV: /GAA/ vs /GA/.hiraganaA
ok($Collator->lt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}\x{3042}"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}\x{3042}"));

# LVX vs LVV: /GAA/ vs /GA/.hanja
ok($Collator->lt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}\x{4E00}"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{1161}", "\x{1100}\x{1161}\x{4E00}"));

# LVL vs LVT: /GA/./G/ vs /GAG/
ok($Collator->lt("\x{1100}\x{1161}\x{1100}", "\x{1100}\x{1161}\x{11A8}"));
ok($hangul  ->lt("\x{1100}\x{1161}\x{1100}", "\x{1100}\x{1161}\x{11A8}"));

# LVT vs LVX: /GAG/ vs /GA/.latinA
ok($Collator->gt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}A"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}A"));

# LVT vs LVX: /GAG/ vs /GA/.hiraganaA
ok($Collator->lt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{3042}"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{3042}"));

# LVT vs LVX: /GAG/ vs /GA/.hanja
ok($Collator->lt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{4E00}"));
ok($hangul  ->gt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{4E00}"));

# LV vs Syl(LV): /GA/ vs /[GA]/
ok($Collator->eq("\x{1100}\x{1161}", "\x{AC00}"));
ok($hangul  ->eq("\x{1100}\x{1161}", "\x{AC00}"));

# LVT vs Syl(LV)T: /GAG/ vs /[GA]G/
ok($Collator->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC00}\x{11A8}"));
ok($hangul  ->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC00}\x{11A8}"));

# LVT vs Syl(LVT): /GAG/ vs /[GAG]/
ok($Collator->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));
ok($hangul  ->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));

# LVTT vs Syl(LVTT): /GAGG/ vs /[GAGG]/
ok($Collator->eq("\x{1100}\x{1161}\x{11A9}", "\x{AC02}"));
ok($hangul  ->eq("\x{1100}\x{1161}\x{11A9}", "\x{AC02}"));

# Syl(LVT) vs : /GAG/ vs /[GAG]/
ok($Collator->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));
ok($hangul  ->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));

#########################

my $hangcirc = Unicode::Collate->new(
  level => 1,
  table => 'keys.txt',
  normalization => undef,
  hangul_terminator => 16,

  entry => <<'ENTRIES',
326E  ; [.1831.0020.0006.326E][.188D.0020.0006.326E][.10.0.0.0] # c.h.s. GA
326F  ; [.1833.0020.0006.326F][.188D.0020.0006.326F][.10.0.0.0] # c.h.s. NA
3270  ; [.1834.0020.0006.3270][.188D.0020.0006.3270][.10.0.0.0] # c.h.s. DA
3271  ; [.1836.0020.0006.3271][.188D.0020.0006.3271][.10.0.0.0] # c.h.s. RA
3272  ; [.1837.0020.0006.3272][.188D.0020.0006.3272][.10.0.0.0] # c.h.s. MA
3273  ; [.1838.0020.0006.3273][.188D.0020.0006.3273][.10.0.0.0] # c.h.s. BA
3274  ; [.183A.0020.0006.3274][.188D.0020.0006.3274][.10.0.0.0] # c.h.s. SA
3275  ; [.183C.0020.0006.3275][.188D.0020.0006.3275][.10.0.0.0] # c.h.s. A
3276  ; [.183D.0020.0006.3276][.188D.0020.0006.3276][.10.0.0.0] # c.h.s. JA
3277  ; [.183F.0020.0006.3277][.188D.0020.0006.3277][.10.0.0.0] # c.h.s. CA
3278  ; [.1840.0020.0006.3278][.188D.0020.0006.3278][.10.0.0.0] # c.h.s. KA
3279  ; [.1841.0020.0006.3279][.188D.0020.0006.3279][.10.0.0.0] # c.h.s. TA
327A  ; [.1842.0020.0006.327A][.188D.0020.0006.327A][.10.0.0.0] # c.h.s. PA
327B  ; [.1843.0020.0006.327B][.188D.0020.0006.327B][.10.0.0.0] # c.h.s. HA
ENTRIES
);

# LV vs Circled Syl(LV): /GA/ vs /(GA)/
ok($Collator->eq("\x{1100}\x{1161}", "\x{326E}"));
ok($hangul  ->gt("\x{1100}\x{1161}", "\x{326E}"));
ok($hangcirc->eq("\x{1100}\x{1161}", "\x{326E}"));

# LV vs Circled Syl(LV): followed by latin A
ok($Collator->eq("\x{1100}\x{1161}A", "\x{326E}A"));
ok($hangul  ->lt("\x{1100}\x{1161}A", "\x{326E}A"));
ok($hangcirc->eq("\x{1100}\x{1161}A", "\x{326E}A"));

# LV vs Circled Syl(LV): followed by hiragana A
ok($Collator->eq("\x{1100}\x{1161}\x{3042}", "\x{326E}\x{3042}"));
ok($hangul  ->lt("\x{1100}\x{1161}\x{3042}", "\x{326E}\x{3042}"));
ok($hangcirc->eq("\x{1100}\x{1161}\x{3042}", "\x{326E}\x{3042}"));

# LVT vs LVX: /GAG/ vs /GA/.hanja
ok($Collator->eq("\x{1100}\x{1161}\x{4E00}", "\x{326E}\x{4E00}"));
ok($hangul  ->lt("\x{1100}\x{1161}\x{4E00}", "\x{326E}\x{4E00}"));
ok($hangcirc->eq("\x{1100}\x{1161}\x{4E00}", "\x{326E}\x{4E00}"));

#########################

# checks contraction in LVT:
# weights of these contractions may be non-sense.

my $hangcont = Unicode::Collate->new(
  level => 1,
  table => 'keys.txt',
  normalization => undef,
  hangul_terminator => 16,

  entry => <<'ENTRIES',
1100 1161 ; [.1831.0020.0002.1100][.188D.0020.0002.1161] # KIYEOK+A
1161 11A8 ; [.188D.0020.0002.1161][.18CF.0020.0002.11A8] # A+KIYEOK
ENTRIES
);

# cont<LV> vs Syl(LV): /<GA>/ vs /[GA]/
ok($Collator->eq("\x{1100}\x{1161}", "\x{AC00}"));
ok($hangcont->eq("\x{1100}\x{1161}", "\x{AC00}"));

# cont<LV>.T vs Syl(LV).T: /<GA>G/ vs /[GA]G/
ok($Collator->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC00}\x{11A8}"));
ok($hangcont->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC00}\x{11A8}"));

# cont<LV>.T vs Syl(LVT): /<GA>G/ vs /[GAG]/
ok($Collator->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));
ok($hangcont->eq("\x{1100}\x{1161}\x{11A8}", "\x{AC01}"));

# L.cont<VT> vs Syl(LV).T: /D<AG>/ vs /[DA]G/
ok($Collator->eq("\x{1103}\x{1161}\x{11A8}", "\x{B2E4}\x{11A8}"));
ok($hangcont->eq("\x{1103}\x{1161}\x{11A8}", "\x{B2E4}\x{11A8}"));

# L.cont<VT> vs Syl(LVT): /D<AG>/ vs /[DAG]/
ok($Collator->eq("\x{1103}\x{1161}\x{11A8}", "\x{B2E5}"));
ok($hangcont->eq("\x{1103}\x{1161}\x{11A8}", "\x{B2E5}"));

#####

$Collator->change(hangul_terminator => 16);

ok($Collator->gt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{4E00}"));
ok($Collator->gt("\x{1100}\x{1161}", "\x{326E}"));
ok($Collator->lt("\x{1100}\x{1161}A", "\x{326E}A"));
ok($Collator->lt("\x{1100}\x{1161}\x{3042}", "\x{326E}\x{3042}"));
ok($Collator->lt("\x{1100}\x{1161}\x{4E00}", "\x{326E}\x{4E00}"));

$Collator->change(hangul_terminator => 0);

ok($Collator->lt("\x{1100}\x{1161}\x{11A8}", "\x{1100}\x{1161}\x{4E00}"));
ok($Collator->eq("\x{1100}\x{1161}", "\x{326E}"));
ok($Collator->eq("\x{1100}\x{1161}A", "\x{326E}A"));
ok($Collator->eq("\x{1100}\x{1161}\x{3042}", "\x{326E}\x{3042}"));
ok($Collator->eq("\x{1100}\x{1161}\x{4E00}", "\x{326E}\x{4E00}"));

