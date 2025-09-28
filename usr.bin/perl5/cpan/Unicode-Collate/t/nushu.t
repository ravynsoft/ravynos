
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..233\n"; } # 5 + 12 x @Versions
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
    UCA_Version => 36,
);

ok($Collator->viewSortKey("\x{1B170}"),
   '[FB01 8000 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{1B171}"),
   '[FB01 8001 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{1B200}"),
   '[FB01 8090 | 0020 | 0002 | FFFF |]');
ok($Collator->viewSortKey("\x{1B2FB}"),
   '[FB01 818B | 0020 | 0002 | FFFF |]');

# Nushu < CJK UI (4E00) < Unassigned.

# 1B170..1B2FB are Nushu Characters since UCA_Version 36 (Unicode 10.0).

for my $v (@Versions) {
    $Collator->change(UCA_Version => $v);

    ok($Collator->cmp("\x{1B16F}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{1B170}", "\x{4E00}") == ($v >= 36 ? -1 : 1));
    ok($Collator->cmp("\x{1B171}", "\x{4E00}") == ($v >= 36 ? -1 : 1));
    ok($Collator->cmp("\x{1B1FF}", "\x{4E00}") == ($v >= 36 ? -1 : 1));
    ok($Collator->cmp("\x{1B200}", "\x{4E00}") == ($v >= 36 ? -1 : 1));
    ok($Collator->cmp("\x{1B2FB}", "\x{4E00}") == ($v >= 36 ? -1 : 1));
    ok($Collator->cmp("\x{1B2FC}", "\x{4E00}") == 1);
    ok($Collator->cmp("\x{1B2FF}", "\x{4E00}") == 1);

    ok($Collator->lt("\x{1B170}", "\x{1B171}"));
    ok($Collator->lt("\x{1B171}", "\x{1B1FF}"));
    ok($Collator->lt("\x{1B1FF}", "\x{1B200}"));
    ok($Collator->lt("\x{1B200}", "\x{1B2FB}"));
}
