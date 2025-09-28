BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use warnings;

$|=1;

# The Script_Extension property has only recently become reasonably stable, so
# later Unicode releases may change things.   Some of these tests were
# designed to provide more code covereage in regexec.c, so changes in it or
# later Standards could cause them to not test what they originally were aimed
# to do.

# Since there's so few tests currently, we can afford to try each syntax on
# all of them
foreach my $type ('script_run', 'sr', 'atomic_script_run', 'asr') {
    my $script_run;
    eval '$script_run = qr/ ^ (*$type: .* ) $ /x;';

    unlike("\N{CYRILLIC SMALL LETTER ER}\N{CYRILLIC SMALL LETTER A}\N{CYRILLIC SMALL LETTER U}}\N{CYRILLIC SMALL LETTER ER}\N{CYRILLIC SMALL LETTER A}l", $script_run, "Cyrillic 'paypal' with a Latin 'l' is not a script run");
    unlike("A\N{GREEK CAPITAL LETTER GAMMA}", $script_run, "Latin followed by Greek isn't a script run");

    like("\N{CYRILLIC THOUSANDS SIGN}\N{COMBINING CYRILLIC TITLO}", $script_run, "Cyrillic followed by Permic-Arabic is Arabic");
    like("\N{OLD PERMIC LETTER AN}\N{COMBINING CYRILLIC TITLO}", $script_run, "Permic followed by Permic-Arabic is Permic");
    unlike("\N{GLAGOLITIC CAPITAL LETTER AZU}\N{COMBINING CYRILLIC TITLO}", $script_run, "Glagolithic followed by Permic-Arabic isn't a script run");

    like("\N{CYRILLIC THOUSANDS SIGN}\N{COMBINING CYRILLIC PALATALIZATION}", $script_run, "Cyrillic followed by Glagolithic-Arabic is Arabic");
    like("\N{GLAGOLITIC CAPITAL LETTER AZU}\N{COMBINING CYRILLIC PALATALIZATION}", $script_run, "Glagolithic followed by Glagolithic-Arabic is Glagolithic");
    unlike("\N{OLD PERMIC LETTER AN}\N{COMBINING CYRILLIC PALATALIZATION}", $script_run, "Permic followed by Glagolithic-Arabic isn't a script run");

    like("\N{ARABIC-INDIC DIGIT ZERO}\N{ARABIC-INDIC DIGIT ONE}\N{ARABIC-INDIC DIGIT TWO}\N{ARABIC-INDIC DIGIT THREE}\N{ARABIC COMMA}\N{ARABIC-INDIC DIGIT FOUR}\N{THAANA LETTER HAA}", $script_run, "Arabic-Thaana chars followed by Thaana is Thaana");
    unlike("\N{ARABIC-INDIC DIGIT ZERO}\N{ARABIC-INDIC DIGIT ONE}A", $script_run, "Arabic-Thaana chars followed by Latin isn't a script run");
    like("\N{ARABIC-INDIC DIGIT ZERO}\N{ARABIC-INDIC DIGIT ONE}\N{ARABIC-INDIC DIGIT TWO}\N{ARABIC-INDIC DIGIT THREE}\N{ARABIC COMMA}\N{ARABIC-INDIC DIGIT FOUR}\N{ARABIC NUMBER SIGN}", $script_run, "Arabic-Thaana chars followed by Arabic is Arabic");
    unlike("\N{ARABIC-INDIC DIGIT ZERO}\N{ARABIC-INDIC DIGIT ONE}\N{ARABIC-INDIC DIGIT TWO}\N{ARABIC-INDIC DIGIT THREE}\N{EXTENDED ARABIC-INDIC DIGIT NINE}", $script_run, "Arabic-Thaana digits followed by an Arabic digit from a different sequence isn't a script run");
    like("\N{ARABIC-INDIC DIGIT ZERO}\N{ARABIC-INDIC DIGIT ONE}\N{ARABIC-INDIC DIGIT TWO}\N{ARABIC-INDIC DIGIT THREE}\N{THAANA LETTER HAA}", $script_run, "Arabic-Thaana digits followed by a Thaana leter is a script run");

    # The next tests are at a hard-coded boundary in regexec.c at the time of this
    # writing (U+02B9/02BA).
    like("abc\N{MODIFIER LETTER SMALL Y}", $script_run, "All Latin is a script run");
    like("abc\N{MODIFIER LETTER PRIME}", $script_run, "Latin then Common is a script run");
    like(":a", $script_run, "Common then Latin is a script run");
    like("-\N{SINHALA LETTER RAYANNA}", $script_run, "Common then Sinhala (which has its own 0) is a script run");

    like("\N{HEBREW LETTER ALEF}\N{HEBREW LETTER TAV}\N{MODIFIER LETTER PRIME}", $script_run, "Hebrew then Common is a script run");
    unlike("\N{HEBREW LETTER ALEF}\N{HEBREW LETTER TAV}\N{MODIFIER LETTER SMALL Y}", $script_run, "Hebrew then Latin isn't a script run");
    like("9876543210\N{DESERET SMALL LETTER WU}", $script_run, "0-9 are the digits for Deseret");
    like("\N{DESERET SMALL LETTER WU}9876543210", $script_run, "Also when they aren't in the initial position");
    like("\N{DESERET SMALL LETTER WU}\N{FULLWIDTH DIGIT FIVE}", $script_run, "Fullwidth digits may be digits for Deseret");
    like("\N{FULLWIDTH DIGIT SIX}\N{DESERET SMALL LETTER LONG I}", $script_run, "... likewise if the digits come first");

    like("1234567890\N{ARABIC LETTER ALEF}", $script_run, "[0-9] work for Arabic");
    unlike("1234567890\N{ARABIC LETTER ALEF}\N{ARABIC-INDIC DIGIT FOUR}\N{ARABIC-INDIC DIGIT FIVE}", $script_run, "... but not in combination with real ARABIC digits");
    unlike("\N{ARABIC LETTER ALEF}\N{ARABIC-INDIC DIGIT SIX}\N{ARABIC-INDIC DIGIT SEVEN}1", $script_run, "... nor when the ARABIC digits come before them");

    # This exercises the case where the script zero but not the script is
    # ambiguous until a non-ambiguous digit is found.
    like("\N{ARABIC LETTER ALEF}\N{EXTENDED ARABIC-INDIC DIGIT EIGHT}", $script_run, "ARABIC with a Shia digit is a script run");

    like("\N{U+03A2}", $script_run, "A single unassigned code point is a script run");
    unlike("\N{U+03A2}\N{U+03A2}", $script_run, "But not more than one");
    unlike("A\N{U+03A2}", $script_run, "... and not in combination with an assigned one");
    unlike("\N{U+03A2}A", $script_run, "... in either order");
    unlike("\N{U+03A2}0", $script_run, "... nor with a digit following");

    like("A\N{COMBINING GRAVE ACCENT}", $script_run, "An inherited script matches others");
    like("\N{COMBINING GRAVE ACCENT}A", $script_run, "... even if first in the sequence");

    like("\N{COMBINING TILDE}\N{COMBINING GRAVE ACCENT}", $script_run, "A script containing only inherited characters matches");

    like("\N{DEVANAGARI DOUBLE DANDA}\N{DEVANAGARI DANDA}\N{DEVANAGARI STRESS SIGN UDATTA}\N{DEVANAGARI STRESS SIGN ANUDATTA}\N{NORTH INDIC FRACTION ONE QUARTER}\N{NORTH INDIC QUANTITY MARK}", $script_run, "A bunch of narrowing down of multiple possible scripts");

    unlike("\N{JAVANESE PANGRANGKEP}\N{GEORGIAN PARAGRAPH SEPARATOR}", $script_run, "Two code points each in multiple scripts that don't intersect aren't a script run");
    like("\N{DEVANAGARI SIGN CANDRABINDU VIRAMA}\N{VEDIC TONE YAJURVEDIC KATHAKA INDEPENDENT SVARITA}", $script_run, "Two code points each in multiple scripts that 't intersect singly are a script run");

    like("", $script_run, "An empty string is a script run");

    use utf8;

    # From UTS 39
    like("写真だけの結婚式", $script_run, "Mixed Hiragana and Han");

    unlike "\N{THAI DIGIT FIVE}1", $script_run, "Thai digit followed by '1'";
    unlike "1\N{THAI DIGIT FIVE}", $script_run, "'1' followed by Thai digit ";
    unlike "\N{BENGALI DIGIT ZERO}\N{CHAKMA DIGIT SEVEN}", $script_run,
           "Two digits in same extended script but from different sets of 10";
}

    # Until fixed, this was skipping the '['
    unlike("abc]c", qr/^ (*sr:a(*sr:[bc]*)c) $/x,
           "Doesn't skip parts of exact matches");

    like("abc", qr/(*asr:a[bc]*c)/, "Outer asr works on a run");
    unlike("abc", qr/(*asr:a(*asr:[bc]*)c)/,
           "Nested asr works to exclude some things");

    like("\x{0980}12\x{0993}", qr/^(*sr:.{4})/,
         "Script with own zero works with ASCII digits"); # perl #133547
    like("\x{3041}12\x{3041}", qr/^(*sr:.{4})/,
         "Script without own zero works with ASCII digits");

    like("A\x{ff10}\x{ff19}B", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Latin"); # perl #133547
    like("A\x{ff10}BC", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Latin"); # perl #133547
    like("A\x{1d7ce}\x{1d7cf}B", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Latin"); # perl #133547
    like("A\x{1d7ce}BC", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Latin"); # perl #133547
    like("\x{1d7ce}\x{1d7cf}AB", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Latin"); # perl #133547
    like("α\x{1d7ce}βγ", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Greek"); # perl #133547
    like("\x{1d7ce}αβγ", qr/^(*sr:.{4})/,
         "Non-ASCII Common digits work with Greek"); # perl #133547

    fresh_perl_is('print scalar "0" =~ m!(((*sr:()|)0)(*sr:)0|)!;',
                  1, {}, '[perl #133997]');

done_testing();
