
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..689\n"; } # 5 + 36 x @Versions
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

my @Versions = ( 8,  9, 11, 14, 16, 18, 20, 22, 24, 26,
		28, 30, 32, 34, 36, 38, 40, 41, 43);

my $Collator = Unicode::Collate->new(
    table => 'keys.txt',
    normalization => undef,
    UCA_Version => 34,
);

ok($Collator->viewSortKey("\x{17000}"),
   '[FB00 8000 | 0020 | 0002 | FFFF FFFF |]');
ok($Collator->viewSortKey("\x{17001}"),
   '[FB00 8001 | 0020 | 0002 | FFFF FFFF |]');
ok($Collator->viewSortKey("\x{18800}"),
   '[FB00 9800 | 0020 | 0002 | FFFF FFFF |]');
ok($Collator->viewSortKey("\x{18AF2}"),
   '[FB00 9AF2 | 0020 | 0002 | FFFF FFFF |]');

# Tangut < CJK UI (4E00) < Unassigned.

# 17000..187EC are Tangut Ideographs since UCA_Version 34 (Unicode 9.0).
# 187ED..187F1 are Tangut Ideographs since UCA_Version 38 (Unicode 11.0).
# 187F2..187F7 are Tangut Ideographs since UCA_Version 40 (Unicode 12.0).
# 18800..18AF2 are Tangut Components since UCA_Version 34 (Unicode 9.0).
# 18AF3..18AFF are Tangut Components since UCA_Version 43 (Unicode 13.0)
# 18D00..18D08 are Tangut Ideographs since UCA_Version 43 (Unicode 13.0)

for my $v (@Versions) {
    $Collator->change(UCA_Version => $v);

    ok($Collator->cmp("\x{16FFF}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{17000}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{17001}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{17FFF}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{18000}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{187EB}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{187EC}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{187ED}", "\x{4E00}") == ($v >= 38 ? -1 : 1));
    ok($Collator->cmp("\x{187EF}", "\x{4E00}") == ($v >= 38 ? -1 : 1));
    ok($Collator->cmp("\x{187F1}", "\x{4E00}") == ($v >= 38 ? -1 : 1));
    ok($Collator->cmp("\x{187F2}", "\x{4E00}") == ($v >= 40 ? -1 : 1));
    ok($Collator->cmp("\x{187F4}", "\x{4E00}") == ($v >= 40 ? -1 : 1));
    ok($Collator->cmp("\x{187F7}", "\x{4E00}") == ($v >= 40 ? -1 : 1));
    ok($Collator->cmp("\x{187F8}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{187FE}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{187FF}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{18800}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{18801}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{18AF1}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{18AF2}", "\x{4E00}") == ($v >= 34 ? -1 : 1));
    ok($Collator->cmp("\x{18AF3}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18AFF}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18D00}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18D08}", "\x{4E00}") == ($v >= 43 ? -1 : 1));
    ok($Collator->cmp("\x{18D09}", "\x{4E00}") == 1);

    ok($Collator->lt("\x{17000}", "\x{17001}"));
    ok($Collator->lt("\x{17001}", "\x{17002}"));
    ok($Collator->lt("\x{17002}", "\x{17FFF}"));
    ok($Collator->lt("\x{17FFF}", "\x{18000}"));
    ok($Collator->lt("\x{18000}", "\x{187EB}"));
    ok($Collator->lt("\x{187EB}", "\x{187EC}"));
    ok($Collator->lt("\x{187ED}", "\x{187EE}"));
    ok($Collator->lt("\x{187F0}", "\x{187F1}"));

    ok($Collator->lt("\x{18800}", "\x{18801}"));
    ok($Collator->lt("\x{18801}", "\x{18AF1}"));
    ok($Collator->lt("\x{18AF1}", "\x{18AF2}"));
}
