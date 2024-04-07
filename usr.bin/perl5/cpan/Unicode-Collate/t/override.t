
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..65\n"; }
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

##### 2..31

{
    my $all_undef_8 = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideCJK => undef,
	overrideHangul => undef,
	UCA_Version => 8,
    );
    # All in the Unicode code point order.
    # No hangul decomposition.

    ok($all_undef_8->lt("\x{1100}", "\x{3402}"));
    ok($all_undef_8->lt("\x{3402}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4DFF}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4E00}", "\x{AC00}"));
    ok($all_undef_8->gt("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_8->gt("\x{AC00}", "\x{ABFF}"));
    # U+ABFF: not assigned

    # a hangul syllable is decomposed into jamo.
    $all_undef_8->change(overrideHangul => 0);
    ok($all_undef_8->lt("\x{1100}", "\x{3402}"));
    ok($all_undef_8->lt("\x{3402}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4DFF}", "\x{4E00}"));
    ok($all_undef_8->gt("\x{4E00}", "\x{AC00}"));
    ok($all_undef_8->eq("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_8->lt("\x{AC00}", "\x{ABFF}"));

    # CJK defined < Jamo undefined
    $all_undef_8->change(overrideCJK => 0);
    ok($all_undef_8->gt("\x{1100}", "\x{3402}"));
    ok($all_undef_8->lt("\x{3402}", "\x{4E00}"));
    ok($all_undef_8->gt("\x{4DFF}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4E00}", "\x{AC00}"));
    ok($all_undef_8->eq("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_8->lt("\x{AC00}", "\x{ABFF}"));

    # CJK undefined > Jamo undefined
    $all_undef_8->change(overrideCJK => undef);
    ok($all_undef_8->lt("\x{1100}", "\x{3402}"));
    ok($all_undef_8->lt("\x{3402}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4DFF}", "\x{4E00}"));
    ok($all_undef_8->gt("\x{4E00}", "\x{AC00}"));
    ok($all_undef_8->eq("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_8->lt("\x{AC00}", "\x{ABFF}"));

    $all_undef_8->change(overrideHangul => undef);
    ok($all_undef_8->lt("\x{1100}", "\x{3402}"));
    ok($all_undef_8->lt("\x{3402}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4DFF}", "\x{4E00}"));
    ok($all_undef_8->lt("\x{4E00}", "\x{AC00}"));
    ok($all_undef_8->gt("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_8->gt("\x{AC00}", "\x{ABFF}"));
}

##### 32..38

{
    my $all_undef_9 = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideCJK => undef,
	overrideHangul => undef,
	UCA_Version => 9,
    );
    # CJK Ideo. < CJK ext A/B < Others.
    # No hangul decomposition.

    ok($all_undef_9->lt("\x{4E00}", "\x{3402}"));
    ok($all_undef_9->lt("\x{3402}", "\x{20000}"));
    ok($all_undef_9->lt("\x{20000}", "\x{AC00}"));
    ok($all_undef_9->gt("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_9->gt("\x{AC00}", "\x{ABFF}"));
    # U+ABFF: not assigned

    # a hangul syllable is decomposed into jamo.
    $all_undef_9->change(overrideHangul => 0);
    ok($all_undef_9->eq("\x{AC00}", "\x{1100}\x{1161}"));
    ok($all_undef_9->lt("\x{AC00}", "\x{ABFF}"));
}

##### 39..46

{
    my $ignoreHangul = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideHangul => sub {()},
	entry => 'AE00 ; [.0100.0020.0002.AE00]  # Hangul GEUL',
    );
    # All Hangul Syllables except U+AE00 are ignored.

    ok($ignoreHangul->eq("\x{AC00}", ""));
    ok($ignoreHangul->lt("\x{AC00}", "\0"));
    ok($ignoreHangul->lt("\x{AC00}", "\x{AE00}"));
    ok($ignoreHangul->lt("\x{AC00}", "\x{1100}\x{1161}")); # Jamo are not ignored.
    ok($ignoreHangul->eq("Pe\x{AC00}rl", "Perl"));
    ok($ignoreHangul->lt("Pe\x{AE00}rl", "Perl"));
    # 'r' is unassigned.

    $ignoreHangul->change(overrideHangul => 0);
    ok($ignoreHangul->eq("\x{AC00}", "\x{1100}\x{1161}"));

    $ignoreHangul->change(overrideHangul => undef);
    ok($ignoreHangul->gt("\x{AC00}", "\x{1100}\x{1161}"));
}

##### 47..51

{
    my $undefHangul = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideHangul => sub {
	    my $u = shift;
	    return $u == 0xAE00 ? 0x100 : undef;
	}
    );
    # All Hangul Syllables except U+AE00 are undefined.

    ok($undefHangul->lt("\x{AE00}", "r"));
    ok($undefHangul->gt("\x{AC00}", "r"));
    ok($undefHangul->gt("\x{AC00}", "\x{1100}\x{1161}"));
    ok($undefHangul->lt("Pe\x{AE00}rl", "Perl")); # 'r' is unassigned.
    ok($undefHangul->lt("\x{AC00}", "\x{B000}"));
}

##### 52..55

{
    my $undefCJK = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideCJK => sub {
	    my $u = shift;
	    return $u == 0x4E00 ? 0x100 : undef;
	}
    );
    # All CJK Ideographs except U+4E00 are undefined.

    ok($undefCJK->lt("\x{4E00}", "r"));
    ok($undefCJK->lt("\x{5000}", "r")); # still CJK < unassigned
    ok($undefCJK->lt("Pe\x{4E00}rl", "Perl"));
    ok($undefCJK->lt("\x{5000}", "\x{6000}"));
}

##### 56..60

{
    my $cpHangul = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideHangul => sub { shift }
    );

    ok($cpHangul->lt("\x{AC00}", "\x{AC01}"));
    ok($cpHangul->lt("\x{AC01}", "\x{D7A3}"));
    ok($cpHangul->lt("\x{D7A3}", "r"));
    ok($cpHangul->lt("r", "\x{D7A4}"));
    ok($cpHangul->lt("\x{D7A3}", "\x{4E00}"));
}

##### 61..65

{
    my $arrayHangul = Unicode::Collate->new(
	table => undef,
	normalization => undef,
	overrideHangul => sub {
	    my $u = shift;
	    return [$u, 0x20, 0x2, $u];
	}
    );

    ok($arrayHangul->lt("\x{AC00}", "\x{AC01}"));
    ok($arrayHangul->lt("\x{AC01}", "\x{D7A3}"));
    ok($arrayHangul->lt("\x{D7A3}", "r"));
    ok($arrayHangul->lt("r", "\x{D7A4}"));
    ok($arrayHangul->lt("\x{D7A3}", "\x{4E00}"));
}

