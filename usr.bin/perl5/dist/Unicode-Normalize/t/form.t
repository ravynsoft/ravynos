
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

#########################

use strict;
use warnings;
BEGIN { $| = 1; print "1..37\n"; }
my $count = 0;
sub ok { Unicode::Normalize::ok(\$count, @_) }

use Unicode::Normalize qw(:all);

ok(1);

sub answer { defined $_[0] ? $_[0] ? "YES" : "NO" : "MAYBE" }

#########################

ok(NFD ("\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{FF76}");
ok(NFC ("\x{304C}\x{FF76}"), "\x{304C}\x{FF76}");
ok(NFKD("\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{30AB}");
ok(NFKC("\x{304C}\x{FF76}"), "\x{304C}\x{30AB}");

ok(answer(checkNFD ("\x{304C}")), "NO");
ok(answer(checkNFC ("\x{304C}")), "YES");
ok(answer(checkNFKD("\x{304C}")), "NO");
ok(answer(checkNFKC("\x{304C}")), "YES");
ok(answer(checkNFD ("\x{FF76}")), "YES");
ok(answer(checkNFC ("\x{FF76}")), "YES");
ok(answer(checkNFKD("\x{FF76}")), "NO");
ok(answer(checkNFKC("\x{FF76}")), "NO");

ok(normalize('D', "\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{FF76}");
ok(normalize('C', "\x{304C}\x{FF76}"), "\x{304C}\x{FF76}");
ok(normalize('KD',"\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{30AB}");
ok(normalize('KC',"\x{304C}\x{FF76}"), "\x{304C}\x{30AB}");

ok(answer(check('D', "\x{304C}")), "NO");
ok(answer(check('C', "\x{304C}")), "YES");
ok(answer(check('KD',"\x{304C}")), "NO");
ok(answer(check('KC',"\x{304C}")), "YES");
ok(answer(check('D' ,"\x{FF76}")), "YES");
ok(answer(check('C' ,"\x{FF76}")), "YES");
ok(answer(check('KD',"\x{FF76}")), "NO");
ok(answer(check('KC',"\x{FF76}")), "NO");

ok(normalize('NFD', "\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{FF76}");
ok(normalize('NFC', "\x{304C}\x{FF76}"), "\x{304C}\x{FF76}");
ok(normalize('NFKD',"\x{304C}\x{FF76}"), "\x{304B}\x{3099}\x{30AB}");
ok(normalize('NFKC',"\x{304C}\x{FF76}"), "\x{304C}\x{30AB}");

ok(answer(check('NFD', "\x{304C}")), "NO");
ok(answer(check('NFC', "\x{304C}")), "YES");
ok(answer(check('NFKD',"\x{304C}")), "NO");
ok(answer(check('NFKC',"\x{304C}")), "YES");
ok(answer(check('NFD' ,"\x{FF76}")), "YES");
ok(answer(check('NFC' ,"\x{FF76}")), "YES");
ok(answer(check('NFKD',"\x{FF76}")), "NO");
ok(answer(check('NFKC',"\x{FF76}")), "NO");

