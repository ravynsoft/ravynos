#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all("encoding.pm is no longer supported by the perl core");
}

plan tests => 72;

no warnings 'deprecated';
use encoding "greek"; # iso 8859-7

# U+0391, \xC1, \301, GREEK CAPITAL LETTER ALPHA
# U+03B1, \xE1, \341, GREEK SMALL LETTER ALPHA

ok("\xC1"    =~ /\xC1/,     '\xC1 to /\xC1/');
ok("\x{391}" =~ /\xC1/,     '\x{391} to /\xC1/');
ok("\xC1"    =~ /\x{C1}/,   '\xC1 to /\x{C1}/');
ok("\x{391}" =~ /\x{C1}/,   '\x{391} to /\x{C1}/');
ok("\xC1"    =~ /\301/,     '\xC1 to /\301/');
ok("\x{391}" =~ /\301/,     '\x{391} to /\301/');
ok("\xC1"    =~ /\x{391}/,  '\xC1 to /\x{391}/');
ok("\x{391}" =~ /\x{391}/,  '\x{391} to /\x{391}/');

ok("\xC1"    =~ /\xC1/i,    '\xC1 to /\xC1/i');
ok("\xE1"    =~ /\xC1/i,    '\xE1 to /\xC1/i');
ok("\xC1"    =~ /\xE1/i,    '\xC1 to /\xE1/i');
ok("\xE1"    =~ /\xE1/i,    '\xE1 to /\xE1/i');
ok("\xC1"    =~ /\x{391}/i, '\xC1 to /\x{391}/i');
ok("\xE1"    =~ /\x{391}/i, '\xE1 to /\x{391}/i');
ok("\xC1"    =~ /\x{3B1}/i, '\xC1 to /\x{3B1}/i');
ok("\xE1"    =~ /\x{3B1}/i, '\xE1 to /\x{3B1}/i');

ok("\xC1"    =~ /[\xC1]/,     '\xC1 to /[\xC1]/');
ok("\x{391}" =~ /[\xC1]/,     '\x{391} to /[\xC1]/');
ok("\xC1"    =~ /[\x{C1}]/,   '\xC1 to /[\x{C1}]/');
ok("\x{391}" =~ /[\x{C1}]/,   '\x{391} to /[\x{C1}]/');
ok("\xC1"    =~ /[\301]/,     '\xC1 to /[\301]/');
ok("\x{391}" =~ /[\301]/,     '\x{391} to /[\301]/');
ok("\xC1"    =~ /[\x{391}]/,  '\xC1 to /[\x{391}]/');
ok("\x{391}" =~ /[\x{391}]/,  '\x{391} to /[\x{391}]/');

ok("\xC1"    =~ /[\xC1]/i,    '\xC1 to /[\xC1]/i');
ok("\xE1"    =~ /[\xC1]/i,    '\xE1 to /[\xC1]/i');
ok("\xC1"    =~ /[\xE1]/i,    '\xC1 to /[\xE1]/i');
ok("\xE1"    =~ /[\xE1]/i,    '\xE1 to /[\xE1]/i');
ok("\xC1"    =~ /[\x{391}]/i, '\xC1 to /[\x{391}]/i');
ok("\xE1"    =~ /[\x{391}]/i, '\xE1 to /[\x{391}]/i');
ok("\xC1"    =~ /[\x{3B1}]/i, '\xC1 to /[\x{3B1}]/i');
ok("\xE1"    =~ /[\x{3B1}]/i, '\xE1 to /[\x{3B1}]/i');

ok("\xC1"    =~ '\xC1',       '\xC1 to \'\xC1\'');
ok("\xC1"    =~ '\x{C1}',     '\xC1 to \'\x{C1}\'');
ok("\xC1"    =~ '\301',       '\xC1 to \'\301\'');
ok("\xC1"    =~ '\x{391}',    '\xC1 to \'\x{391}\'');
ok("\xC1"    =~ '[\xC1]',     '\xC1 to \'[\xC1]\'');
ok("\xC1"    =~ '[\x{C1}]',   '\xC1 to \'[\x{C1}]\'');
ok("\xC1"    =~ '[\301]',     '\xC1 to \'[\301]\'');
ok("\xC1"    =~ '[\x{391}]',  '\xC1 to \'[\x{391}]\'');

ok("\xC1"    =~ /Á/,     '\xC1 to /<ALPHA>/');
ok("\xE1"    !~ /Á/,     '\xE1 to /<ALPHA>/');
ok("\xC1"    =~ /Á/i,    '\xC1 to /<ALPHA>/i');
ok("\xE1"    =~ /Á/i,    '\xE1 to /<ALPHA>/i');
ok("\xC1"    =~ /[Á]/,   '\xC1 to /[<ALPHA>]/');
ok("\xE1"    !~ /[Á]/,   '\xE1 to /[<ALPHA>]/');
ok("\xC1"    =~ /[Á]/i,  '\xC1 to /[<ALPHA>]/i');
ok("\xE1"    =~ /[Á]/i,  '\xE1 to /[<ALPHA>]/i');

ok("\xC1\xC1"  =~ /Á\xC1/,    '\xC1\xC1 to /<ALPHA>\xC1/');
ok("\xC1\xC1"  =~ /\xC1Á/,    '\xC1\xC1 to /\xC1<ALPHA>/');
ok("\xC1\xC1"  =~ /Á\xC1/i,   '\xC1\xC1 to /<ALPHA>\xC1/i');
ok("\xC1\xC1"  =~ /\xC1Á/i,   '\xC1\xC1 to /\xC1<ALPHA>/i');
ok("\xC1\xE1"  =~ /Á\xC1/i,   '\xC1\xE1 to /<ALPHA>\xC1/i');
ok("\xC1\xE1"  =~ /\xC1Á/i,   '\xC1\xE1 to /\xC1<ALPHA>/i');
ok("\xE1\xE1"  =~ /Á\xC1/i,   '\xE1\xE1 to /<ALPHA>\xC1/i');
ok("\xE1\xE1"  =~ /\xC1Á/i,   '\xE1\xE1 to /\xC1<ALPHA>/i');

# U+038A, \xBA, GREEK CAPITAL LETTER IOTA WITH TONOS
# U+03AF, \xDF, GREEK SMALL LETTER IOTA WITH TONOS

ok("\x{38A}"  =~ /\xBA/,      '\x{38A} to /\xBA/');
ok("\x{38A}"  !~ /\xDF/,      '\x{38A} to /\xDF/');
ok("\x{38A}"  =~ /\xBA/i,     '\x{38A} to /\xBA/i');
ok("\x{38A}"  =~ /\xDF/i,     '\x{38A} to /\xDF/i');
ok("\x{38A}"  =~ /[\xBA]/,    '\x{38A} to /[\xBA]/');
ok("\x{38A}"  !~ /[\xDF]/,    '\x{38A} to /[\xDF]/');
ok("\x{38A}"  =~ /[\xBA]/i,   '\x{38A} to /[\xBA]/i');
ok("\x{38A}"  =~ /[\xDF]/i,   '\x{38A} to /[\xDF]/i');

# \xDF is not LATIN SMALL LETTER SHARP S

ok("SS"   !~ /\xDF/i,   'SS to /\xDF/i');
ok("Ss"   !~ /\xDF/i,   'Ss to /\xDF/i');
ok("sS"   !~ /\xDF/i,   'sS to /\xDF/i');
ok("ss"   !~ /\xDF/i,   'ss to /\xDF/i');
ok("SS"   !~ /ß/i,      'SS to /<iota-tonos>/i');
ok("Ss"   !~ /ß/i,      'Ss to /<iota-tonos>/i');
ok("sS"   !~ /ß/i,      'sS to /<iota-tonos>/i');
ok("ss"   !~ /ß/i,      'ss to /<iota-tonos>/i');

