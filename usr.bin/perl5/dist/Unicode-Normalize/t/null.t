
BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir('t') if -d 't';
        @INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

#########################

use strict;
use warnings;

use Unicode::Normalize qw(:all);
print "1..24\n";

print "ok 1\n";

# if $_ is not NULL-terminated, test may fail.

$_ = compose('abc');
print /c$/ ? "ok" : "not ok", " 2\n";

$_ = decompose('abc');
print /c$/ ? "ok" : "not ok", " 3\n";

$_ = reorder('abc');
print /c$/ ? "ok" : "not ok", " 4\n";

$_ = NFD('abc');
print /c$/ ? "ok" : "not ok", " 5\n";

$_ = NFC('abc');
print /c$/ ? "ok" : "not ok", " 6\n";

$_ = NFKD('abc');
print /c$/ ? "ok" : "not ok", " 7\n";

$_ = NFKC('abc');
print /c$/ ? "ok" : "not ok", " 8\n";

$_ = FCC('abc');
print /c$/ ? "ok" : "not ok", " 9\n";

$_ = decompose("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 10\n";

$_ = decompose("\x{304B}\x{3099}abc");
print /c$/ ? "ok" : "not ok", " 11\n";

$_ = reorder("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 12\n";

$_ = reorder("\x{304B}\x{3099}abc");
print /c$/ ? "ok" : "not ok", " 13\n";

$_ = compose("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 14\n";

$_ = compose("\x{304B}\x{3099}abc");
print /c$/ ? "ok" : "not ok", " 15\n";

$_ = NFD("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 16\n";

$_ = NFC("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 17\n";

$_ = NFKD("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 18\n";

$_ = NFKC("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 19\n";

$_ = FCC("\x{304C}abc");
print /c$/ ? "ok" : "not ok", " 20\n";

$_ = getCanon(0x100);
print s/.$// ? "ok" : "not ok", " 21\n";

$_ = getCompat(0x100);
print s/.$// ? "ok" : "not ok", " 22\n";

$_ = getCanon(0xAC00);
print s/.$// ? "ok" : "not ok", " 23\n";

$_ = getCompat(0xAC00);
print s/.$// ? "ok" : "not ok", " 24\n";

