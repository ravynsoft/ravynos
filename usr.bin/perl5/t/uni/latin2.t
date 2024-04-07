#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all("encoding.pm is no longer supported by the perl core");
}

plan tests => 94;

no warnings 'deprecated';
use encoding "latin2"; # iso 8859-2

# U+00C1, \xC1, \301, LATIN CAPITAL LETTER A WITH ACUTE
# U+0102, \xC3, \402, LATIN CAPITAL LETTER A WITH BREVE
# U+00E1, \xE1, \303, LATIN SMALL LETTER A WITH ACUTE
# U+0103, \xE3, \403, LATIN SMALL LETTER A WITH BREVE

ok("\xC1"    =~ /\xC1/,     '\xC1 to /\xC1/');
ok("\x{C1}"  =~ /\x{C1}/,   '\x{C1} to /\x{C1}/');
ok("\xC3"    =~ /\xC3/,     '\xC3 to /\xC3/');
ok("\x{102}" =~ /\xC3/,     '\x{102} to /\xC3/');
ok("\xC3"    =~ /\x{C3}/,   '\xC3 to /\x{C3}/');
ok("\x{102}" =~ /\x{C3}/,   '\x{102} to /\x{C3}/');
ok("\xC3"    =~ /\x{102}/,  '\xC3 to /\x{102}/');
ok("\x{102}" =~ /\x{102}/,  '\x{102} to /\x{102}/');

ok("\xC1"    =~ /\xC1/i,    '\xC1 to /\xC1/i');
ok("\xE1"    =~ /\xC1/i,    '\xE1 to /\xC1/i');
ok("\xC1"    =~ /\xE1/i,    '\xC1 to /\xE1/i');
ok("\xE1"    =~ /\xE1/i,    '\xE1 to /\xE1/i');
ok("\x{102}" =~ /\xC3/i,    '\x{102} to /\xC3/i');
ok("\x{103}" =~ /\xC3/i,    '\x{103} to /\xC3/i');
ok("\x{102}" =~ /\xE3/i,    '\x{102} to /\xE3/i');
ok("\x{103}" =~ /\xE3/i,    '\x{103} to /\xE3/i');

ok("\xC1"    =~ /[\xC1]/,     '\xC1 to /[\xC1]/');
ok("\x{C1}"  =~ /[\x{C1}]/,   '\x{C1} to /[\x{C1}]/');
ok("\xC3"    =~ /[\xC3]/,     '\xC3 to /[\xC3]/');
ok("\x{102}" =~ /[\xC3]/,     '\x{102} to /[\xC3]/');
ok("\xC3"    =~ /[\x{C3}]/,   '\xC3 to /[\x{C3}]/');
ok("\x{102}" =~ /[\x{C3}]/,   '\x{102} to /[\x{C3}]/');
ok("\xC3"    =~ /[\x{102}]/,  '\xC3 to /[\x{102}]/');
ok("\x{102}" =~ /[\x{102}]/,  '\x{102} to /[\x{102}]/');

ok("\xC1"    =~ /[\xC1]/i,  '\xC1 to /[\xC1]/i');
ok("\xE1"    =~ /[\xC1]/i,  '\xE1 to /[\xC1]/i');
ok("\xC1"    =~ /[\xE1]/i,  '\xC1 to /[\xE1]/i');
ok("\xE1"    =~ /[\xE1]/i,  '\xE1 to /[\xE1]/i');
ok("\x{102}" =~ /[\xC3]/i,  '\x{102} to /[\xC3]/i');
ok("\x{103}" =~ /[\xC3]/i,  '\x{103} to /[\xC3]/i');
ok("\x{102}" =~ /[\xE3]/i,  '\x{102} to /[\xE3]/i');
ok("\x{103}" =~ /[\xE3]/i,  '\x{103} to /[\xE3]/i');

ok("\xC1"    =~ '\xC1',       '\xC1 to \'\xC1\'');
ok("\xC1"    =~ '\x{C1}',     '\xC1 to \'\x{C1}\'');
ok("\xC3"    =~ '\303',       '\xC3 to \'\303\'');
ok("\xC3"    =~ '\x{102}',    '\xC3 to \'\x{102}\'');
ok("\xC1"    =~ '[\xC1]',     '\xC1 to \'[\xC1]\'');
ok("\xC1"    =~ '[\x{C1}]',   '\xC1 to \'[\x{C1}]\'');
ok("\xC3"    =~ '[\303]',     '\xC3 to \'[\303]\'');
ok("\xC3"    =~ '[\x{102}]',  '\xC3 to \'[\x{102}]\'');

ok("\xC1"    =~ /Á/,     '\xC1 to /<A-acute>/');
ok("\xE1"    !~ /Á/,     '\xE1 to /<A-acute>/');
ok("\xC1"    =~ /Á/i,    '\xC1 to /<A-acute>/i');
ok("\xE1"    =~ /Á/i,    '\xE1 to /<A-acute>/i');
ok("\xC1"    =~ /[Á]/,   '\xC1 to /[<A-acute>]/');
ok("\xE1"    !~ /[Á]/,   '\xE1 to /[<A-acute>]/');
ok("\xC1"    =~ /[Á]/i,  '\xC1 to /[<A-acute>]/i');
ok("\xE1"    =~ /[Á]/i,  '\xE1 to /[<A-acute>]/i');

ok("\xC1\xC1"  =~ /Á\xC1/,    '\xC1\xC1 to /<A-acute>\xC1/');
ok("\xC1\xC1"  =~ /\xC1Á/,    '\xC1\xC1 to /\xC1<A-acute>/');
ok("\xC1\xC1"  =~ /Á\xC1/i,   '\xC1\xC1 to /<A-acute>\xC1/i');
ok("\xC1\xC1"  =~ /\xC1Á/i,   '\xC1\xC1 to /\xC1<A-acute>/i');
ok("\xC1\xE1"  =~ /Á\xC1/i,   '\xC1\xE1 to /<A-acute>\xC1/i');
ok("\xC1\xE1"  =~ /\xC1Á/i,   '\xC1\xE1 to /\xC1<A-acute>/i');
ok("\xE1\xE1"  =~ /Á\xC1/i,   '\xE1\xE1 to /<A-acute>\xC1/i');
ok("\xE1\xE1"  =~ /\xC1Á/i,   '\xE1\xE1 to /\xC1<A-acute>/i');

# \xDF is LATIN SMALL LETTER SHARP S

ok("\xDF" =~ /\xDF/,    '\xDF to /\xDF/');
ok("\xDF" =~ /\xDF/i,   '\xDF to /\xDF/i');
ok("\xDF" =~ /[\xDF]/,  '\xDF to /[\xDF]/');
ok("\xDF" =~ /[\xDF]/i, '\xDF to /[\xDF]/i');
ok("\xDF" =~ /ß/,       '\xDF to /<sharp-s>/');
ok("\xDF" =~ /ß/i,      '\xDF to /<sharp-s>/i');
ok("\xDF" =~ /[ß]/,     '\xDF to /[<sharp-s>]/');
ok("\xDF" =~ /[ß]/i,    '\xDF to /[<sharp-s>]/i');

ok("SS"   =~ /\xDF/i,   'SS to /\xDF/i');
ok("Ss"   =~ /\xDF/i,   'Ss to /\xDF/i');
ok("sS"   =~ /\xDF/i,   'sS to /\xDF/i');
ok("ss"   =~ /\xDF/i,   'ss to /\xDF/i');
ok("SS"   =~ /ß/i,      'SS to /<sharp-s>/i');
ok("Ss"   =~ /ß/i,      'Ss to /<sharp-s>/i');
ok("sS"   =~ /ß/i,      'sS to /<sharp-s>/i');
ok("ss"   =~ /ß/i,      'ss to /<sharp-s>/i');

ok("\xC3" =~ /\303/,     '\xC1 to /\303/');
ok("\303" =~ /\303/,     '\303 to /\303/');
ok("\xC3" =~ /\303/i,    '\xC1 to /\303/i');
ok("\xE3" =~ /\303/i,    '\xC1 to /\303/i');
ok("\xC3" =~ /[\303]/,   '\xC1 to /[\303]/');
ok("\303" =~ /[\303]/,   '\303 to /[\303]/');
ok("\xC3" =~ /[\303]/i,  '\xC1 to /[\303]/i');
ok("\xE3" =~ /[\303]/i,  '\xC1 to /[\303]/i');

ok("\xC3" =~ /\402/,     '\xC1 to /\402/');
ok("\402" =~ /\402/,     '\402 to /\402/');
ok("\xC3" =~ /\402/i,    '\xC1 to /\402/i');
ok("\xE3" =~ /\402/i,    '\xC1 to /\402/i');
ok("\xC3" =~ /[\402]/,   '\xC1 to /[\402]/');
ok("\402" =~ /[\402]/,   '\402 to /[\402]/');
ok("\xC3" =~ /[\402]/i,  '\xC1 to /[\402]/i');
ok("\xE3" =~ /[\402]/i,  '\xC1 to /[\402]/i');

{
    my $re = '(?i:\xC1)';

    ok("\xC1" =~ $re, '\xC1 to (?i:\xC1)');
    ok("\xE1" =~ $re, '\xE1 to (?i:\xC1)');

    utf8::downgrade($re);

    ok("\xC1" =~ $re, '\xC1 to (?i:\xC1) down');
    ok("\xE1" =~ $re, '\xE1 to (?i:\xC1) down');

    utf8::upgrade($re);

    ok("\xC1" =~ $re, '\xC1 to (?i:\xC1) up');
    ok("\xE1" =~ $re, '\xE1 to (?i:\xC1) up');
}

