
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..32\n"; }
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

{
    # Table is undefined, then no entry is defined.
    my $undef_table = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	level => 1,
    );

    # in the Unicode code point order
    ok($undef_table->lt('', 'A'));
    ok($undef_table->lt('ABC', 'B'));

    # Hangul should be decomposed (even w/o Unicode::Normalize).
    ok($undef_table->lt("Perl", "\x{AC00}"));
    ok($undef_table->eq("\x{AC00}", "\x{1100}\x{1161}"));
    ok($undef_table->eq("\x{AE00}", "\x{1100}\x{1173}\x{11AF}"));
    ok($undef_table->lt("\x{AE00}", "\x{3042}"));

    # U+AC00: Hangul GA
    # U+AE00: Hangul GEUL
    # U+3042: Hiragana A

    # Weight for CJK Ideographs is defined, though.
    ok($undef_table->lt("", "\x{4E00}"));
    ok($undef_table->lt("\x{4E8C}","ABC"));
    ok($undef_table->lt("\x{4E00}","\x{3042}"));
    ok($undef_table->lt("\x{4E00}","\x{4E8C}"));

# 11

    # U+4E00: Ideograph "ONE"
    # U+4E8C: Ideograph "TWO"

    for my $v ('', 8, 9, 11, 14) {
	$undef_table->change(UCA_Version => $v) if $v;
	ok($undef_table->lt("\x{4E00}","\0"));
    }
}

# 16

{
    my $onlyABC = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	entry => << 'ENTRIES',
0061 ; [.0101.0020.0002.0061] # LATIN SMALL LETTER A
0041 ; [.0101.0020.0008.0041] # LATIN CAPITAL LETTER A
0062 ; [.0102.0020.0002.0062] # LATIN SMALL LETTER B
0042 ; [.0102.0020.0008.0042] # LATIN CAPITAL LETTER B
0063 ; [.0103.0020.0002.0063] # LATIN SMALL LETTER C
0043 ; [.0103.0020.0008.0043] # LATIN CAPITAL LETTER C
ENTRIES
    );
    ok(
	join(':', $onlyABC->sort( qw/ ABA BAC cc A Ab cAc aB / ) ),
	join(':',                 qw/ A aB Ab ABA BAC cAc cc / ),
    );
}

# 17

{
    my $few_entries = Unicode::Collate->new(
	entry => <<'ENTRIES',
0050 ; [.0101.0020.0002.0050]  # P
0045 ; [.0102.0020.0002.0045]  # E
0052 ; [.0103.0020.0002.0052]  # R
004C ; [.0104.0020.0002.004C]  # L
1100 ; [.0105.0020.0002.1100]  # Hangul Jamo initial G
1175 ; [.0106.0020.0002.1175]  # Hangul Jamo middle I
5B57 ; [.0107.0020.0002.5B57]  # CJK Ideograph "Letter"
ENTRIES
	table => undef,
	normalization => undef,
    );
    # defined before undefined
    my $sortABC = join '',
	$few_entries->sort(split //, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ");

    ok($sortABC eq "PERL ABCDFGHIJKMNOQSTUVWXYZ");

    ok($few_entries->lt('E', 'D'));
    ok($few_entries->lt("\x{5B57}", "\x{4E00}"));
    ok($few_entries->lt("\x{AE30}", "\x{AC00}"));

    # Hangul must be decomposed.
    ok($few_entries->eq("\x{AC00}", "\x{1100}\x{1161}"));
}

# 22

{
    my $highestNUL = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	level => 1,
	entry => '0000 ; [.FFFE.0020.0005.0000]',
    );

    for my $v ('', 8, 9, 11, 14) {
	$highestNUL->change(UCA_Version => $v) if $v;
	ok($highestNUL->lt("abc\x{4E00}", "abc\0"));
	ok($highestNUL->lt("abc\x{E0000}","abc\0"));
    }
}

# 32
