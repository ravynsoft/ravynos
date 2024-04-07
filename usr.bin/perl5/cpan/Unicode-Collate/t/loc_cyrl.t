
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..208\n"; }
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

my $objNoSuppress = Unicode::Collate::Locale->
    new(locale => 'NoSuppress', normalization => undef);

ok($objNoSuppress->getlocale, 'default');

$objNoSuppress->change(level => 1);

ok($objNoSuppress->gt("\x{439}", "\x{438}")); # not suppressed
ok($objNoSuppress->gt("\x{419}", "\x{418}")); # not suppressed

# 4

ok($objNoSuppress->eq("\x{4D1}", "\x{430}"));
ok($objNoSuppress->eq("\x{4D0}", "\x{410}"));
ok($objNoSuppress->eq("\x{4D3}", "\x{430}"));
ok($objNoSuppress->eq("\x{4D2}", "\x{410}"));
ok($objNoSuppress->eq("\x{4DB}", "\x{4D9}"));
ok($objNoSuppress->eq("\x{4DA}", "\x{4D8}"));
ok($objNoSuppress->eq("\x{453}", "\x{433}"));
ok($objNoSuppress->eq("\x{403}", "\x{413}"));
ok($objNoSuppress->eq("\x{450}", "\x{435}"));
ok($objNoSuppress->eq("\x{400}", "\x{415}"));
ok($objNoSuppress->eq("\x{4D7}", "\x{435}"));
ok($objNoSuppress->eq("\x{4D6}", "\x{415}"));
ok($objNoSuppress->eq("\x{451}", "\x{435}"));
ok($objNoSuppress->eq("\x{401}", "\x{415}"));
ok($objNoSuppress->eq("\x{4C2}", "\x{436}"));
ok($objNoSuppress->eq("\x{4C1}", "\x{416}"));
ok($objNoSuppress->eq("\x{4DD}", "\x{436}"));
ok($objNoSuppress->eq("\x{4DC}", "\x{416}"));
ok($objNoSuppress->eq("\x{4DF}", "\x{437}"));
ok($objNoSuppress->eq("\x{4DE}", "\x{417}"));
ok($objNoSuppress->eq("\x{45D}", "\x{438}"));
ok($objNoSuppress->eq("\x{40D}", "\x{418}"));
ok($objNoSuppress->eq("\x{4E5}", "\x{438}"));
ok($objNoSuppress->eq("\x{4E4}", "\x{418}"));
ok($objNoSuppress->eq("\x{4E3}", "\x{438}"));
ok($objNoSuppress->eq("\x{4E2}", "\x{418}"));
ok($objNoSuppress->eq("\x{457}", "\x{456}"));
ok($objNoSuppress->eq("\x{407}", "\x{406}"));
ok($objNoSuppress->eq("\x{45C}", "\x{43A}"));
ok($objNoSuppress->eq("\x{40C}", "\x{41A}"));
ok($objNoSuppress->eq("\x{4E7}", "\x{43E}"));
ok($objNoSuppress->eq("\x{4E6}", "\x{41E}"));
ok($objNoSuppress->eq("\x{4EB}", "\x{4E9}"));
ok($objNoSuppress->eq("\x{4EA}", "\x{4E8}"));
ok($objNoSuppress->eq("\x{45E}", "\x{443}"));
ok($objNoSuppress->eq("\x{40E}", "\x{423}"));
ok($objNoSuppress->eq("\x{4F1}", "\x{443}"));
ok($objNoSuppress->eq("\x{4F0}", "\x{423}"));
ok($objNoSuppress->eq("\x{4F3}", "\x{443}"));
ok($objNoSuppress->eq("\x{4F2}", "\x{423}"));
ok($objNoSuppress->eq("\x{4EF}", "\x{443}"));
ok($objNoSuppress->eq("\x{4EE}", "\x{423}"));
ok($objNoSuppress->eq("\x{4F5}", "\x{447}"));
ok($objNoSuppress->eq("\x{4F4}", "\x{427}"));
ok($objNoSuppress->eq("\x{4F9}", "\x{44B}"));
ok($objNoSuppress->eq("\x{4F8}", "\x{42B}"));
ok($objNoSuppress->eq("\x{4ED}", "\x{44D}"));
ok($objNoSuppress->eq("\x{4EC}", "\x{42D}"));
ok($objNoSuppress->eq("\x{477}", "\x{475}"));
ok($objNoSuppress->eq("\x{476}", "\x{474}"));

# 54

$objNoSuppress->change(level => 2);

ok($objNoSuppress->gt("\x{4D1}", "\x{430}"));
ok($objNoSuppress->gt("\x{4D0}", "\x{410}"));
ok($objNoSuppress->gt("\x{4D3}", "\x{430}"));
ok($objNoSuppress->gt("\x{4D2}", "\x{410}"));
ok($objNoSuppress->gt("\x{4DB}", "\x{4D9}"));
ok($objNoSuppress->gt("\x{4DA}", "\x{4D8}"));
ok($objNoSuppress->gt("\x{453}", "\x{433}"));
ok($objNoSuppress->gt("\x{403}", "\x{413}"));
ok($objNoSuppress->gt("\x{450}", "\x{435}"));
ok($objNoSuppress->gt("\x{400}", "\x{415}"));
ok($objNoSuppress->gt("\x{4D7}", "\x{435}"));
ok($objNoSuppress->gt("\x{4D6}", "\x{415}"));
ok($objNoSuppress->gt("\x{451}", "\x{435}"));
ok($objNoSuppress->gt("\x{401}", "\x{415}"));
ok($objNoSuppress->gt("\x{4C2}", "\x{436}"));
ok($objNoSuppress->gt("\x{4C1}", "\x{416}"));
ok($objNoSuppress->gt("\x{4DD}", "\x{436}"));
ok($objNoSuppress->gt("\x{4DC}", "\x{416}"));
ok($objNoSuppress->gt("\x{4DF}", "\x{437}"));
ok($objNoSuppress->gt("\x{4DE}", "\x{417}"));
ok($objNoSuppress->gt("\x{45D}", "\x{438}"));
ok($objNoSuppress->gt("\x{40D}", "\x{418}"));
ok($objNoSuppress->gt("\x{4E5}", "\x{438}"));
ok($objNoSuppress->gt("\x{4E4}", "\x{418}"));
ok($objNoSuppress->gt("\x{4E3}", "\x{438}"));
ok($objNoSuppress->gt("\x{4E2}", "\x{418}"));
ok($objNoSuppress->gt("\x{457}", "\x{456}"));
ok($objNoSuppress->gt("\x{407}", "\x{406}"));
ok($objNoSuppress->gt("\x{45C}", "\x{43A}"));
ok($objNoSuppress->gt("\x{40C}", "\x{41A}"));
ok($objNoSuppress->gt("\x{4E7}", "\x{43E}"));
ok($objNoSuppress->gt("\x{4E6}", "\x{41E}"));
ok($objNoSuppress->gt("\x{4EB}", "\x{4E9}"));
ok($objNoSuppress->gt("\x{4EA}", "\x{4E8}"));
ok($objNoSuppress->gt("\x{45E}", "\x{443}"));
ok($objNoSuppress->gt("\x{40E}", "\x{423}"));
ok($objNoSuppress->gt("\x{4F1}", "\x{443}"));
ok($objNoSuppress->gt("\x{4F0}", "\x{423}"));
ok($objNoSuppress->gt("\x{4F3}", "\x{443}"));
ok($objNoSuppress->gt("\x{4F2}", "\x{423}"));
ok($objNoSuppress->gt("\x{4EF}", "\x{443}"));
ok($objNoSuppress->gt("\x{4EE}", "\x{423}"));
ok($objNoSuppress->gt("\x{4F5}", "\x{447}"));
ok($objNoSuppress->gt("\x{4F4}", "\x{427}"));
ok($objNoSuppress->gt("\x{4F9}", "\x{44B}"));
ok($objNoSuppress->gt("\x{4F8}", "\x{42B}"));
ok($objNoSuppress->gt("\x{4ED}", "\x{44D}"));
ok($objNoSuppress->gt("\x{4EC}", "\x{42D}"));
ok($objNoSuppress->gt("\x{477}", "\x{475}"));
ok($objNoSuppress->gt("\x{476}", "\x{474}"));

# 104

$objNoSuppress->change(level => 3);

ok($objNoSuppress->eq("\x{439}", "\x{438}\x{306}")); # not suppressed
ok($objNoSuppress->eq("\x{419}", "\x{418}\x{306}")); # not suppressed

# 106

ok($objNoSuppress->gt("\x{439}", "\x{438}\0\x{306}")); # not suppressed
ok($objNoSuppress->gt("\x{419}", "\x{418}\0\x{306}")); # not suppressed

# 108

for my $i ("", "\0") {
  ok($objNoSuppress->eq("\x{4D1}", "\x{430}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4D0}", "\x{410}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4D3}", "\x{430}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4D2}", "\x{410}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4DB}", "\x{4D9}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4DA}", "\x{4D8}$i\x{308}"));
  ok($objNoSuppress->eq("\x{453}", "\x{433}$i\x{301}"));
  ok($objNoSuppress->eq("\x{403}", "\x{413}$i\x{301}"));
  ok($objNoSuppress->eq("\x{450}", "\x{435}$i\x{300}"));
  ok($objNoSuppress->eq("\x{400}", "\x{415}$i\x{300}"));
  ok($objNoSuppress->eq("\x{4D7}", "\x{435}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4D6}", "\x{415}$i\x{306}"));
  ok($objNoSuppress->eq("\x{451}", "\x{435}$i\x{308}"));
  ok($objNoSuppress->eq("\x{401}", "\x{415}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4C2}", "\x{436}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4C1}", "\x{416}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4DD}", "\x{436}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4DC}", "\x{416}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4DF}", "\x{437}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4DE}", "\x{417}$i\x{308}"));
  ok($objNoSuppress->eq("\x{45D}", "\x{438}$i\x{300}"));
  ok($objNoSuppress->eq("\x{40D}", "\x{418}$i\x{300}"));
  ok($objNoSuppress->eq("\x{4E5}", "\x{438}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4E4}", "\x{418}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4E3}", "\x{438}$i\x{304}"));
  ok($objNoSuppress->eq("\x{4E2}", "\x{418}$i\x{304}"));
  ok($objNoSuppress->eq("\x{457}", "\x{456}$i\x{308}"));
  ok($objNoSuppress->eq("\x{407}", "\x{406}$i\x{308}"));
  ok($objNoSuppress->eq("\x{45C}", "\x{43A}$i\x{301}"));
  ok($objNoSuppress->eq("\x{40C}", "\x{41A}$i\x{301}"));
  ok($objNoSuppress->eq("\x{4E7}", "\x{43E}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4E6}", "\x{41E}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4EB}", "\x{4E9}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4EA}", "\x{4E8}$i\x{308}"));
  ok($objNoSuppress->eq("\x{45E}", "\x{443}$i\x{306}"));
  ok($objNoSuppress->eq("\x{40E}", "\x{423}$i\x{306}"));
  ok($objNoSuppress->eq("\x{4F1}", "\x{443}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4F0}", "\x{423}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4F3}", "\x{443}$i\x{30B}"));
  ok($objNoSuppress->eq("\x{4F2}", "\x{423}$i\x{30B}"));
  ok($objNoSuppress->eq("\x{4EF}", "\x{443}$i\x{304}"));
  ok($objNoSuppress->eq("\x{4EE}", "\x{423}$i\x{304}"));
  ok($objNoSuppress->eq("\x{4F5}", "\x{447}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4F4}", "\x{427}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4F9}", "\x{44B}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4F8}", "\x{42B}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4ED}", "\x{44D}$i\x{308}"));
  ok($objNoSuppress->eq("\x{4EC}", "\x{42D}$i\x{308}"));
  ok($objNoSuppress->eq("\x{477}", "\x{475}$i\x{30F}"));
  ok($objNoSuppress->eq("\x{476}", "\x{474}$i\x{30F}"));
}

# 208
