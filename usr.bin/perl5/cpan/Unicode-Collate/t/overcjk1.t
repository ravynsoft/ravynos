
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..562\n"; } # 11 + 29 x @Versions
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

# 2..11

my $overCJK = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  entry => <<'ENTRIES',
4E01 ; [.B1FC.0030.0004.4E00] # Ideograph; B1FC = FFFF - 4E03.
ENTRIES
  overrideCJK => sub {
    my $u = 0xFFFF - $_[0]; # reversed
    [$u, 0x20, 0x2, $u];
  },
);

ok($overCJK->gt("B", "A")); # diff. at level 1.
ok($overCJK->lt("a", "A")); # diff. at level 3.
ok($overCJK->lt( "\x{4E03}",  "\x{4E01}")); # diff. at level 2.
ok($overCJK->gt( "\x{4E03}B", "\x{4E01}A"));
ok($overCJK->lt( "\x{4E03}A", "\x{4E01}B"));
ok($overCJK->gt("B\x{4E03}", "A\x{4E01}"));
ok($overCJK->lt("A\x{4E03}", "B\x{4E01}"));
ok($overCJK->lt("A\x{4E03}", "A\x{4E01}"));
ok($overCJK->lt("A\x{4E03}", "a\x{4E01}"));
ok($overCJK->lt("a\x{4E03}", "A\x{4E01}"));

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

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

for my $v (@Versions) {
    $overCJK->change(UCA_Version => $v);
    ok($overCJK->cmp("a\x{3400}", "A\x{4DB5}") == 1);
    ok($overCJK->cmp("a\x{4DB5}", "A\x{4E00}") == 1);
    ok($overCJK->cmp("a\x{4E00}", "A\x{9FA5}") == 1);
    ok($overCJK->cmp("a\x{9FA5}", "A\x{9FA6}") == ($v >= 14 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FA6}", "A\x{9FAF}") == ($v >= 14 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FAF}", "A\x{9FB0}") == ($v >= 14 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FB0}", "A\x{9FBB}") == ($v >= 14 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FBB}", "A\x{9FBC}") == ($v >= 18 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FBC}", "A\x{9FBF}") == ($v >= 18 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FBF}", "A\x{9FC3}") == ($v >= 18 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FC3}", "A\x{9FC4}") == ($v >= 20 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FC4}", "A\x{9FCA}") == ($v >= 20 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FCA}", "A\x{9FCB}") == ($v >= 20 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FCB}", "A\x{9FCC}") == ($v >= 24 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FCC}", "A\x{9FCD}") == ($v >= 32 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FCD}", "A\x{9FCF}") == ($v >= 32 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FCF}", "A\x{9FD5}") == ($v >= 32 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FD5}", "A\x{9FD6}") == ($v >= 36 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FD6}", "A\x{9FDF}") == ($v >= 36 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FDF}", "A\x{9FEA}") == ($v >= 36 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FEA}", "A\x{9FEB}") == ($v >= 38 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FEB}", "A\x{9FEE}") == ($v >= 38 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FEE}", "A\x{9FEF}") == ($v >= 38 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FEF}", "A\x{9FF0}") == ($v >= 43 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FF0}", "A\x{9FFB}") == ($v >= 43 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FFB}", "A\x{9FFC}") == ($v >= 43 ? 1 : -1));
    ok($overCJK->cmp("a\x{9FFC}", "A\x{9FFD}") == -1);
    ok($overCJK->cmp("a\x{9FFD}", "A\x{9FFE}") == -1);
    ok($overCJK->cmp("a\x{9FFE}", "A\x{9FFF}") == -1);
}

