
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..1070\n"; } # 6 + 56 x @Versions
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

# 2..6

my $ignoreCJK = Unicode::Collate->new(
  table => undef,
  normalization => undef,
  overrideCJK => sub {()},
  entry => <<'ENTRIES',
5B57 ; [.0107.0020.0002.5B57]  # CJK Ideograph "Letter"
ENTRIES
);

# All CJK Unified Ideographs except U+5B57 are ignored.

ok($ignoreCJK->eq("\x{4E00}", ""));
ok($ignoreCJK->lt("\x{4E00}", "\0"));
ok($ignoreCJK->eq("Pe\x{4E00}rl", "Perl")); # U+4E00 is a CJK.
ok($ignoreCJK->gt("\x{4DFF}", "\x{4E00}")); # U+4DFF is not CJK.
ok($ignoreCJK->lt("Pe\x{5B57}rl", "Perl")); # 'r' is unassigned.

#####

# 4E00..9FA5 are CJK UI.
# 9FA6..9FBB are CJK UI since UCA_Version 14 (Unicode 4.1).
# 9FBC..9FC3 are CJK UI since UCA_Version 18 (Unicode 5.1).
# 9FC4..9FCB are CJK UI since UCA_Version 20 (Unicode 5.2).
# 9FCC       is  CJK UI since UCA_Version 24 (Unicode 6.1).
# 9FCD..9FD5 are CJK UI since UCA_Version 32 (Unicode 8.0).
# 9FD6..9FEA are CJK UI since UCA_Version 36 (Unicode 10.0).
# 9FEB..9FEF are CJK UI since UCA_Version 38 (Unicode 11.0).
# 9FF0..9FFC are CJK UI since UCA_Version 43 (Unicode 13.0).

# 3400..4DB5   are CJK UI Ext.A since UCA_Version 8  (Unicode 3.0).
# 4DB6..4DBF   are CJK UI Ext.A since UCA_Version 43 (Unicode 13.0).
# 20000..2A6D6 are CJK UI Ext.B since UCA_Version 8  (Unicode 3.1).
# 2A6D7..2A6DD are CJK UI Ext.B since UCA_Version 43 (Unicode 13.0).
# 2A700..2B734 are CJK UI Ext.C since UCA_Version 20 (Unicode 5.2).
# 2B740..2B81D are CJK UI Ext.D since UCA_Version 22 (Unicode 6.0).
# 2B820..2CEA1 are CJK UI Ext.E since UCA_Version 32 (Unicode 8.0).
# 2CEB0..2EBE0 are CJK UI Ext.F since UCA_Version 36 (Unicode 10.0).
# 30000..3134A are CJK UI Ext.G since UCA_Version 43 (Unicode 13.0).

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

for my $v (@Versions) {
    $ignoreCJK->change(UCA_Version => $v);

    # UI
    ok($ignoreCJK->cmp("\x{4E00}", "") == 0);
    ok($ignoreCJK->cmp("\x{9FA5}", "") == 0);
    ok($ignoreCJK->cmp("\x{9FA6}", "") == ($v >= 14 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FAF}", "") == ($v >= 14 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FB0}", "") == ($v >= 14 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FBB}", "") == ($v >= 14 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FBC}", "") == ($v >= 18 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FBF}", "") == ($v >= 18 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FC0}", "") == ($v >= 18 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FC3}", "") == ($v >= 18 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FC4}", "") == ($v >= 20 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FCB}", "") == ($v >= 20 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FCC}", "") == ($v >= 24 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FCD}", "") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FCF}", "") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FD0}", "") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FD5}", "") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FD6}", "") == ($v >= 36 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FDF}", "") == ($v >= 36 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FEA}", "") == ($v >= 36 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FEB}", "") == ($v >= 38 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FEF}", "") == ($v >= 38 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FF0}", "") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FFC}", "") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{9FFD}", "") == 1);
    ok($ignoreCJK->cmp("\x{9FFF}", "") == 1);

    # Ext.A
    ok($ignoreCJK->cmp("\x{3400}", "") == 0);
    ok($ignoreCJK->cmp("\x{4DB5}", "") == 0);
    ok($ignoreCJK->cmp("\x{4DB6}", "") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{4DBF}", "") == ($v >= 43 ? 0 : 1));

    # Ext.B
    ok($ignoreCJK->cmp("\x{20000}","") == 0);
    ok($ignoreCJK->cmp("\x{2A6D6}","") == 0);
    ok($ignoreCJK->cmp("\x{2A6D7}","") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2A6DD}","") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2A6DE}","") == 1);
    ok($ignoreCJK->cmp("\x{2A6DF}","") == 1);

    # Ext.C
    ok($ignoreCJK->cmp("\x{2A700}","") == ($v >= 20 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2B734}","") == ($v >= 20 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2B735}","") == 1);
    ok($ignoreCJK->cmp("\x{2B73F}","") == 1);

    # Ext.D
    ok($ignoreCJK->cmp("\x{2B740}","") == ($v >= 22 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2B81D}","") == ($v >= 22 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2B81E}","") == 1);
    ok($ignoreCJK->cmp("\x{2B81F}","") == 1);

    # Ext.E
    ok($ignoreCJK->cmp("\x{2B820}","") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2CEA1}","") == ($v >= 32 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2CEA2}","") == 1);
    ok($ignoreCJK->cmp("\x{2CEAF}","") == 1);

    # Ext.F
    ok($ignoreCJK->cmp("\x{2CEB0}","") == ($v >= 36 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2EBE0}","") == ($v >= 36 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{2EBE1}","") == 1);
    ok($ignoreCJK->cmp("\x{2EBEF}","") == 1);

    # Ext.G
    ok($ignoreCJK->cmp("\x{30000}","") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{3134A}","") == ($v >= 43 ? 0 : 1));
    ok($ignoreCJK->cmp("\x{3134B}","") == 1);
    ok($ignoreCJK->cmp("\x{3134F}","") == 1);
}

