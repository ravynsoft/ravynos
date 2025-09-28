use strict;
use warnings;

# re/fold_grind.t has more complex tests, but doesn't test every fold
# This file also tests the fc() keyword.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_unicode_tables();
    skip_all_if_miniperl("miniperl, no Unicode::Normalize");
    require Config; import Config;
    require './charset_tools.pl';
    require './loc_tools.pl';   # Contains find_utf8_ctype_locale()
}

use feature 'unicode_strings';
use Unicode::UCD qw(all_casefolds);

binmode *STDOUT, ":utf8";

our $TODO;


plan("no_plan");
# Read in the official case folding definitions.
my $casefolds = all_casefolds();
my @folds;
my @CF;
my @simple_folds;
my %reverse_fold;
use Unicode::UCD;
use charnames();

foreach my $decimal_code_point (sort { $a <=> $b } keys %$casefolds) {
    # We only use simple folds in fc(), since the regex engine uses full case
    # folding.

    my $name = charnames::viacode($decimal_code_point);
    my $type = $casefolds->{$decimal_code_point}{'status'};
    my $code = $casefolds->{$decimal_code_point}{'code'};
    my $simple = $casefolds->{$decimal_code_point}{'simple'};
    my $full = $casefolds->{$decimal_code_point}{'full'};

    if ($simple && $simple ne $full) { # If there is a distinction
        push @simple_folds, [ $code, $simple, $type, $name ];
    }

    push @CF, [ $code, $full, $type, $name ];

    # Get the inverse fold for single-char mappings.
    $reverse_fold{pack "W*", hex $simple} = pack "W*", $decimal_code_point if $simple;
}

foreach my $test_ref ( @simple_folds ) {
    use feature 'fc';
    my ($code, $mapping, $type, $name) = @$test_ref;
    my $c = pack("W*", hex $code);
    utf8::upgrade($c);
    my $f = pack("W*", map { hex } split " ", $mapping);

    my $against = join "", "qq{", map("\\x{$_}", split " ", $mapping), "}";
    {
        isnt(fc($c), $f, "$code - $name - $mapping - $type - Full casefolding, fc(\\x{$code}) ne $against");
        isnt("\F$c", $f, "$code - $name - $mapping - $type - Full casefolding, qq{\\F\\x{$code}} ne $against");
    }
}

foreach my $test_ref (@CF) {
    my ($code, $mapping, $type, $name) = @$test_ref;
    my $c = pack("W*", hex $code);
    utf8::upgrade($c);
    my $f = pack("W*", map { hex } split " ", $mapping);
    my $f_length = length $f;
    foreach my $test (
            qq[":$c:" =~ /:$c:/],
            qq[":$c:" =~ /:$c:/i],
            qq[":$c:" =~ /:[_$c]:/], # Place two chars in [] so doesn't get
                                     # optimized to a non-charclass
            qq[":$c:" =~ /:[_$c]:/i],
            qq[":$c:" =~ /:$f:/i],
            qq[":$f:" =~ /:$c:/i],
    ) {
        ok eval $test, "$code - $name - $mapping - $type - $test";
    }

    {
        # fc() tests
        my $against = join "", "qq{", map("\\x{$_}", split " ", $mapping), "}";
        is(CORE::fc($c), $f,
            "$code - $name - $mapping - $type - fc(\\x{$code}) eq $against");
        is("\F$c", $f, "$code - $name - $mapping - $type - qq{\\F\\x{$code}} eq $against");

        # And here we test bytes. For [A-Za-z0-9], the fold is the same as lc under
        # bytes. For everything else, it's the bytes that formed the original string.
        if ( $c =~ /[A-Za-z0-9]/ ) {
            use bytes;
            is(CORE::fc($c), lc($c), "$code - $name - fc and use bytes, ascii");
        } else {
            my $copy = "" . $c;
            utf8::encode($copy);
            is($copy, do { use bytes; CORE::fc($c) }, "$code - $name - fc and use bytes");
        }
    }
    # Certain tests weren't convenient to put in the list above since they are
    # TODO's in multi-character folds.
    if ($f_length == 1) {

        # The qq loses the utf8ness of ":$f:".  These tests are not about
        # finding bugs in utf8ness, so make sure it's utf8.
        my $test = qq[my \$s = ":$f:"; utf8::upgrade(\$s); \$s =~ /:[_$c]:/i];
        ok eval $test, "$code - $name - $mapping - $type - $test";
        $test = qq[":$c:" =~ /:[_$f]:/i];
        ok eval $test, "$code - $name - $mapping - $type - $test";
    }
    else {

        # There are two classes of multi-char folds that need more work.  For
        # example,
        #   ":ß:" =~ /:[_s]{2}:/i
        #   ":ss:" =~ /:[_ß]:/i
        #
        # Some of the old tests for the second case happened to pass somewhat
        # coincidentally.  But none would pass if changed to this.
        #   ":SS:" =~ /:[_ß]:/i
        #
        # As the capital SS doesn't get folded.  When those pass, it means
        # that the code has been changed to take into account folding in the
        # string, and all should pass, capitalized or not (this wouldn't be
        # true for [^complemented character classes], for which the fold case
        # is better, but these aren't used in this .t currently.  So, what is
        # done is to essentially upper-case the string for this class (but use
        # the reverse fold not uc(), as that is more correct)
        my $u;
        for my $i (0 .. $f_length - 1) {
            my $cur_char = substr($f, $i, 1);
            $u .= $reverse_fold{$cur_char} || $cur_char;
        }
        my $test;

        # A multi-char fold should not match just one char;
        # e.g., ":ß:" !~ /:[_s]:/i
        $test = qq[":$c:" !~ /:[_$f]:/i];
        ok eval $test, "$code - $name - $mapping - $type - $test";

        TODO: { # e.g., ":ß:" =~ /:[_s]{2}:/i
            local $TODO = 'Multi-char fold in [character class]';

            $test = qq[":$c:" =~ /:[_$f]{$f_length}:/i];
            ok eval $test, "$code - $name - $mapping - $type - $test";
        }

        # e.g., ":SS:" =~ /:[_ß]:/i now pass, so TODO has been removed, but
        # since they use '$u', they are left out of the main loop
        $test = qq[ my \$s = ":$u:"; utf8::upgrade(\$s); \$s =~ /:[_$c]:/i];
        ok eval $test, "$code - $name - $mapping - $type - $test";

        my $bracketed_f = ($f =~ s/(.)/[$1]/gr);
        $test = qq[":$c:" =~ /:$bracketed_f:/iu];
        ok eval $test, "$code - $name - $mapping - $type - $test";

        my @f_chars = ($f =~ / (.) (.) (.?) /x);
        my $every_other_bracketed_f = "[$f_chars[0]]$f_chars[1]";
        $every_other_bracketed_f .= "[$f_chars[2]]" if $f_chars[2];
        $test = qq[":$c:" =~ /:$every_other_bracketed_f:/iu];
        ok eval $test, "$code - $name - $mapping - $type - $test";

        my $other_every_bracketed_f = "$f_chars[0]";
        $other_every_bracketed_f .= "[$f_chars[1]]";
        $other_every_bracketed_f .= "$f_chars[2]" if $f_chars[2];
        $test = qq[":$c:" =~ /:$other_every_bracketed_f:/iu];
        ok eval $test, "$code - $name - $mapping - $type - $test";
    }
}

{
    use utf8;
    use feature qw(fc);
    # These three come from the ICU project's test suite, more especifically
    # http://icu.sourcearchive.com/documentation/4.4~rc1-1/strcase_8cpp-source.html

    my $s = "A\N{U+00df}\N{U+00b5}\N{U+fb03}\N{U+1040C}\N{U+0130}\N{U+0131}";
    #\N{LATIN CAPITAL LETTER A}\N{LATIN SMALL LETTER SHARP S}\N{MICRO SIGN}\N{LATIN SMALL LIGATURE FFI}\N{DESERET CAPITAL LETTER AY}\N{LATIN CAPITAL LETTER I WITH DOT ABOVE}\N{LATIN SMALL LETTER DOTLESS I}

    my $f = "ass\N{U+03bc}ffi\N{U+10434}i\N{U+0307}\N{U+0131}";
    #\N{LATIN SMALL LETTER A}\N{LATIN SMALL LETTER S}\N{LATIN SMALL LETTER S}\N{GREEK SMALL LETTER MU}\N{LATIN SMALL LETTER F}\N{LATIN SMALL LETTER F}\N{LATIN SMALL LETTER I}\N{DESERET SMALL LETTER AY}\N{LATIN SMALL LETTER I}\N{COMBINING DOT ABOVE}\N{LATIN SMALL LETTER DOTLESS I}

    is(fc($s), $f, "ICU's casefold test passes");
    is("\F$s", $f, "ICU's casefold test passes");

    is( fc("aBİIıϐßﬃ񟿿"), "abi̇iıβssffi񟿿" );
    is( "\FaBİIıϐßﬃ񟿿", "abi̇iıβssffi񟿿" );
#    TODO: {
#        local $::TODO = "turkic special cases";
#        is( fc "aBİIıϐßﬃ񟿿", "abiııβssffi񟿿" );
#    }

    # The next batch come from http://www.devdaily.com/java/jwarehouse/lucene/contrib/icu/src/test/org/apache/lucene/analysis/icu/TestICUFoldingFilter.java.shtml
    # Except the article got most casings wrong. Or maybe Lucene does.

    is( fc("This is a test"), "this is a test" );
    is( fc("Ruß"), "russ"    );
    is( fc("ΜΆΪΟΣ"), "μάϊοσ" );
    is( fc("Μάϊος"), "μάϊοσ" );
    is( fc("𐐖"), "𐐾"       );
    is( fc("r" . uni_to_native("\xe9") . "sum" . uni_to_native("\xe9")),
           "r" . uni_to_native("\xe9") . "sum" . uni_to_native("\xe9") );
    is( fc("re\x{0301}sume\x{0301}"), "re\x{301}sume\x{301}" );
    is( fc("ELİF"), "eli\x{307}f" );
    is( fc("eli\x{307}f"), "eli\x{307}f");

    # This batch comes from
    # http://www.java2s.com/Open-Source/Java-Document/Internationalization-Localization/icu4j/com/ibm/icu/dev/test/lang/UCharacterCaseTest.java.htm
    # Which uses ICU as the backend.

    my @folding_mixed = (
        uni_to_native("\x{61}\x{42}\x{130}\x{49}\x{131}\x{3d0}\x{df}\x{fb03}"),
        "A" . uni_to_native("\x{df}\x{b5}\x{fb03}\x{1040C}\x{130}\x{131}"),
    );

    my @folding_default = (
        uni_to_native("\x{61}\x{62}\x{69}\x{307}\x{69}\x{131}\x{3b2}\x{73}\x{73}\x{66}\x{66}\x{69}"),
        "ass\x{3bc}ffi\x{10434}i\x{307}\x{131}"
    );

    my @folding_exclude_turkic = (
        uni_to_native("\x{61}\x{62}\x{69}\x{131}\x{131}\x{3b2}\x{73}\x{73}\x{66}\x{66}\x{69}"),
                         "ass\x{3bc}ffi\x{10434}i\x{131}",
    );

    is( fc($folding_mixed[1]), $folding_default[1] );

    is( fc($folding_mixed[0]), $folding_default[0] );

}

{
    use utf8;
    # Table stolen from tchrist's mail in
    # http://bugs.python.org/file23051/casing-tests.py
    # and http://98.245.80.27/tcpc/OSCON2011/case-test.python3
    # For reference, it's a longer version of what he posted here:
    # http://stackoverflow.com/questions/6991038/case-insensitive-storage-and-unicode-compatibility

    #Couple of repeats because I'm lazy, not tchrist's fault.

    #This should probably go in t/op/lc.t

    my @test_table = (
# ORIG LC_SIMPLE TC_SIMPLE UC_SIMPLE LC_FULL TC_FULL UC_FULL FC_SIMPLE FC_TURKIC FC_FULL
[ 'þǽr rihtes', 'þǽr rihtes', 'Þǽr Rihtes', 'ÞǼR RIHTES', 'þǽr rihtes', 'Þǽr Rihtes', 'ÞǼR RIHTES', 'þǽr rihtes', 'þǽr rihtes', 'þǽr rihtes',  ],
[ 'duȝeðlice', 'duȝeðlice', 'Duȝeðlice', 'DUȜEÐLICE', 'duȝeðlice', 'Duȝeðlice', 'DUȜEÐLICE', 'duȝeðlice', 'duȝeðlice', 'duȝeðlice',  ],
[ 'Ævar Arnfjörð Bjarmason', 'ævar arnfjörð bjarmason', 'Ævar Arnfjörð Bjarmason', 'ÆVAR ARNFJÖRÐ BJARMASON', 'ævar arnfjörð bjarmason', 'Ævar Arnfjörð Bjarmason', 'ÆVAR ARNFJÖRÐ BJARMASON', 'ævar arnfjörð bjarmason', 'ævar arnfjörð bjarmason', 'ævar arnfjörð bjarmason',  ],
[ 'Кириллица', 'кириллица', 'Кириллица', 'КИРИЛЛИЦА', 'кириллица', 'Кириллица', 'КИРИЛЛИЦА', 'кириллица', 'кириллица', 'кириллица',  ],
[ 'ĳ', 'ĳ', 'Ĳ', 'Ĳ', 'ĳ', 'Ĳ', 'Ĳ', 'ĳ', 'ĳ', 'ĳ',  ],
[ 'Van Dĳke', 'van dĳke', 'Van Dĳke', 'VAN DĲKE', 'van dĳke', 'Van Dĳke', 'VAN DĲKE', 'van dĳke', 'van dĳke', 'van dĳke',  ],
[ 'VAN DĲKE', 'van dĳke', 'Van Dĳke', 'VAN DĲKE', 'van dĳke', 'Van Dĳke', 'VAN DĲKE', 'van dĳke', 'van dĳke', 'van dĳke',  ],
[ 'eﬃcient', 'eﬃcient', 'Eﬃcient', 'EﬃCIENT', 'eﬃcient', 'Eﬃcient', 'EFFICIENT', 'eﬃcient', 'efficient', 'efficient',  ],
[ 'ﬂour', 'ﬂour', 'ﬂour', 'ﬂOUR', 'ﬂour', 'Flour', 'FLOUR', 'ﬂour', 'flour', 'flour',  ],
[ 'ﬂour and water', 'ﬂour and water', 'ﬂour And Water', 'ﬂOUR AND WATER', 'ﬂour and water', 'Flour And Water', 'FLOUR AND WATER', 'ﬂour and water', 'flour and water', 'flour and water',  ],
[ 'ǳur', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǳur', 'ǳur',  ],
[ 'ǲur', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǳur', 'ǳur',  ],
[ 'ǱUR', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǲur', 'ǱUR', 'ǳur', 'ǳur', 'ǳur',  ],
[ 'ǳur mountain', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǳur mountain', 'ǳur mountain',  ],
[ 'ǲur Mountain', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǳur mountain', 'ǳur mountain',  ],
[ 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǲur Mountain', 'ǱUR MOUNTAIN', 'ǳur mountain', 'ǳur mountaın', 'ǳur mountain',  ],
[ 'poſt', 'poſt', 'Poſt', 'POST', 'poſt', 'Poſt', 'POST', 'post', 'post', 'post',  ],
[ 'poﬅ', 'poﬅ', 'Poﬅ', 'POﬅ', 'poﬅ', 'Poﬅ', 'POST', 'poﬅ', 'post', 'post',  ],
[ 'ﬅop', 'ﬅop', 'ﬅop', 'ﬅOP', 'ﬅop', 'Stop', 'STOP', 'ﬅop', 'stop', 'stop',  ],
[ 'tschüß', 'tschüß', 'Tschüß', 'TSCHÜß', 'tschüß', 'Tschüß', 'TSCHÜSS', 'tschüß', 'tschüss', 'tschüss',  ],
[ 'TSCHÜẞ', 'tschüß', 'Tschüß', 'TSCHÜẞ', 'tschüß', 'Tschüß', 'TSCHÜẞ', 'tschüß', 'tschüss', 'tschüss',  ],
[ 'weiß', 'weiß', 'Weiß', 'WEIß', 'weiß', 'Weiß', 'WEISS', 'weiß', 'weiss', 'weiss',  ],
[ 'WEIẞ', 'weiß', 'Weiß', 'WEIẞ', 'weiß', 'Weiß', 'WEIẞ', 'weiß', 'weıss', 'weiss',  ],
[ 'ẞIEW', 'ßiew', 'ẞiew', 'ẞIEW', 'ßiew', 'ẞiew', 'ẞIEW', 'ßiew', 'ssıew', 'ssiew',  ],
[ 'ᾲ', 'ᾲ', 'Ὰͅ', 'ᾺΙ', 'ᾲ', 'Ὰͅ', 'ᾺΙ', 'ὰι', 'ὰι', 'ὰι',  ],
[ 'Ὰι', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'ὰι', 'ὰι',  ],
[ 'ᾺΙ', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'ὰι', 'ὰι',  ],
[ 'ᾲ', 'ᾲ', 'ᾲ', 'ᾲ', 'ᾲ', 'Ὰͅ', 'ᾺΙ', 'ᾲ', 'ὰι', 'ὰι',  ],
[ 'Ὰͅ', 'ᾲ', 'Ὰͅ', 'ᾺΙ', 'ᾲ', 'Ὰͅ', 'ᾺΙ', 'ὰι', 'ὰι', 'ὰι',  ],
[ 'ᾺΙ', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'Ὰι', 'ᾺΙ', 'ὰι', 'ὰι', 'ὰι',  ],
[ 'ᾲ στο διάολο', 'ᾲ στο διάολο', 'ᾲ Στο Διάολο', 'ᾲ ΣΤΟ ΔΙΆΟΛΟ', 'ᾲ στο διάολο', 'Ὰͅ Στο Διάολο', 'ᾺΙ ΣΤΟ ΔΙΆΟΛΟ', 'ᾲ στο διάολο', 'ὰι στο διάολο', 'ὰι στο διάολο',  ],
[ 'ᾲ στο διάολο', 'ᾲ στο διάολο', 'Ὰͅ Στο Διάολο', 'ᾺΙ ΣΤΟ ΔΙΆΟΛΟ', 'ᾲ στο διάολο', 'Ὰͅ Στο Διάολο', 'ᾺΙ ΣΤΟ ΔΙΆΟΛΟ', 'ὰι στο διάολο', 'ὰι στο διάολο', 'ὰι στο διάολο',  ],
[ '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻',  ],
[ '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻',  ],
[ '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐯𐑅𐐨𐑉𐐯𐐻', '𐐔𐐇𐐝𐐀𐐡𐐇𐐓', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻', '𐐼𐐯𐑅𐐨𐑉𐐯𐐻',  ],
[ 'henry ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'henry ⅷ', 'henry ⅷ',  ],
[ 'Henry Ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'henry ⅷ', 'henry ⅷ',  ],
[ 'HENRY Ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'Henry Ⅷ', 'HENRY Ⅷ', 'henry ⅷ', 'henry ⅷ', 'henry ⅷ',  ],
[ 'i work at ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'i work at ⓚ', 'i work at ⓚ',  ],
[ 'I Work At Ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'ı work at ⓚ', 'i work at ⓚ',  ],
[ 'I WORK AT Ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'I Work At Ⓚ', 'I WORK AT Ⓚ', 'i work at ⓚ', 'ı work at ⓚ', 'i work at ⓚ',  ],
[ 'istambul', 'istambul', 'Istambul', 'ISTAMBUL', 'istambul', 'Istambul', 'ISTAMBUL', 'istambul', 'istambul', 'istambul',  ],
[ 'i̇stanbul', 'i̇stanbul', 'İstanbul', 'İSTANBUL', 'i̇stanbul', 'İstanbul', 'İSTANBUL', 'i̇stanbul', 'i̇stanbul', 'i̇stanbul',  ],
[ 'İstanbul', 'i̇stanbul', 'İstanbul', 'İSTANBUL', 'i̇stanbul', 'İstanbul', 'İSTANBUL', 'i̇stanbul', 'ı̇stanbul', 'i̇stanbul',  ],
[ 'İSTANBUL', 'istanbul', 'İstanbul', 'İSTANBUL', 'i̇stanbul', 'İstanbul', 'İSTANBUL', 'İstanbul', 'istanbul', 'i̇stanbul',  ],
[ 'στιγμας', 'στιγμας', 'Στιγμας', 'ΣΤΙΓΜΑΣ', 'στιγμας', 'Στιγμας', 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'στιγμασ', 'στιγμασ',  ],
[ 'στιγμασ', 'στιγμασ', 'Στιγμασ', 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'Στιγμασ', 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'στιγμασ', 'στιγμασ',  ],
[ 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'Στιγμασ', 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'Στιγμασ', 'ΣΤΙΓΜΑΣ', 'στιγμασ', 'στιγμασ', 'στιγμασ',  ],
[ 'ʀᴀʀᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ',  ],
[ 'Ʀᴀʀᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ',  ],
[ 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'Ʀᴀʀᴇ', 'ƦᴀƦᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ', 'ʀᴀʀᴇ',  ],
[ 'Ԧԧ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'ԧԧ', 'ԧԧ',  ],
[ 'ԧԧ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'ԧԧ', 'ԧԧ',  ],
[ 'Ԧԧ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'ԧԧ', 'ԧԧ',  ],
[ 'ԦԦ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'Ԧԧ', 'ԦԦ', 'ԧԧ', 'ԧԧ', 'ԧԧ',  ],
[ "þǽr rihtes", "þǽr rihtes", "Þǽr Rihtes", "ÞǼR RIHTES", "þǽr rihtes", "Þǽr Rihtes", "ÞǼR RIHTES", "þǽr rihtes", "þǽr rihtes", "þǽr rihtes",  ],
[ "duȝeðlice", "duȝeðlice", "Duȝeðlice", "DUȜEÐLICE", "duȝeðlice", "Duȝeðlice", "DUȜEÐLICE", "duȝeðlice", "duȝeðlice", "duȝeðlice",  ],
[ "Van Dĳke", "van dĳke", "Van Dĳke", "VAN DĲKE", "van dĳke", "Van Dĳke", "VAN DĲKE", "van dĳke", "van dĳke", "van dĳke",  ],
[ "ﬁ", "ﬁ", "ﬁ", "ﬁ", "ﬁ", "Fi", "FI", "ﬁ", "fi", "fi",  ],
[ "ﬁlesystem", "ﬁlesystem", "ﬁlesystem", "ﬁLESYSTEM", "ﬁlesystem", "Filesystem", "FILESYSTEM", "ﬁlesystem", "filesystem", "filesystem",  ],
[ "eﬃcient", "eﬃcient", "Eﬃcient", "EﬃCIENT", "eﬃcient", "Eﬃcient", "EFFICIENT", "eﬃcient", "efficient", "efficient",  ],
[ "ﬂour and water", "ﬂour and water", "ﬂour And Water", "ﬂOUR AND WATER", "ﬂour and water", "Flour And Water", "FLOUR AND WATER", "ﬂour and water", "flour and water", "flour and water",  ],
[ "ǳ", "ǳ", "ǲ", "Ǳ", "ǳ", "ǲ", "Ǳ", "ǳ", "ǳ", "ǳ",  ],
[ "ǳur mountain", "ǳur mountain", "ǲur Mountain", "ǱUR MOUNTAIN", "ǳur mountain", "ǲur Mountain", "ǱUR MOUNTAIN", "ǳur mountain", "ǳur mountain", "ǳur mountain",  ],
[ "poſt", "poſt", "Poſt", "POST", "poſt", "Poſt", "POST", "post", "post", "post",  ],
[ "poﬅ", "poﬅ", "Poﬅ", "POﬅ", "poﬅ", "Poﬅ", "POST", "poﬅ", "post", "post",  ],
[ "ﬅop", "ﬅop", "ﬅop", "ﬅOP", "ﬅop", "Stop", "STOP", "ﬅop", "stop", "stop",  ],
[ "tschüß", "tschüß", "Tschüß", "TSCHÜß", "tschüß", "Tschüß", "TSCHÜSS", "tschüß", "tschüss", "tschüss",  ],
[ "TSCHÜẞ", "tschüß", "Tschüß", "TSCHÜẞ", "tschüß", "Tschüß", "TSCHÜẞ", "tschüß", "tschüss", "tschüss",  ],
[ "rußland", "rußland", "Rußland", "RUßLAND", "rußland", "Rußland", "RUSSLAND", "rußland", "russland", "russland",  ],
[ "RUẞLAND", "rußland", "Rußland", "RUẞLAND", "rußland", "Rußland", "RUẞLAND", "rußland", "russland", "russland",  ],
[ "weiß", "weiß", "Weiß", "WEIß", "weiß", "Weiß", "WEISS", "weiß", "weiss", "weiss",  ],
[ "WEIẞ", "weiß", "Weiß", "WEIẞ", "weiß", "Weiß", "WEIẞ", "weiß", "weıss", "weiss",  ],
[ "ẞIEW", "ßiew", "ẞiew", "ẞIEW", "ßiew", "ẞiew", "ẞIEW", "ßiew", "ssıew", "ssiew",  ],
[ "ͅ", "ͅ", "Ι", "Ι", "ͅ", "Ι", "Ι", "ι", "ι", "ι",  ],
[ "ᾲ", "ᾲ", "Ὰͅ", "ᾺΙ", "ᾲ", "Ὰͅ", "ᾺΙ", "ὰι", "ὰι", "ὰι",  ],
[ "Ὰι", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "ὰι", "ὰι",  ],
[ "ᾺΙ", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "ὰι", "ὰι",  ],
[ "ᾲ", "ᾲ", "ᾲ", "ᾲ", "ᾲ", "Ὰͅ", "ᾺΙ", "ᾲ", "ὰι", "ὰι",  ],
[ "Ὰͅ", "ᾲ", "Ὰͅ", "ᾺΙ", "ᾲ", "Ὰͅ", "ᾺΙ", "ὰι", "ὰι", "ὰι",  ],
[ "ᾺΙ", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "Ὰι", "ᾺΙ", "ὰι", "ὰι", "ὰι",  ],
[ "ᾲ στο διάολο", "ᾲ στο διάολο", "ᾲ Στο Διάολο", "ᾲ ΣΤΟ ΔΙΆΟΛΟ", "ᾲ στο διάολο", "Ὰͅ Στο Διάολο", "ᾺΙ ΣΤΟ ΔΙΆΟΛΟ", "ᾲ στο διάολο", "ὰι στο διάολο", "ὰι στο διάολο",  ],
[ "ᾲ στο διάολο", "ᾲ στο διάολο", "Ὰͅ Στο Διάολο", "ᾺΙ ΣΤΟ ΔΙΆΟΛΟ", "ᾲ στο διάολο", "Ὰͅ Στο Διάολο", "ᾺΙ ΣΤΟ ΔΙΆΟΛΟ", "ὰι στο διάολο", "ὰι στο διάολο", "ὰι στο διάολο",  ],
[ "ⅷ", "ⅷ", "Ⅷ", "Ⅷ", "ⅷ", "Ⅷ", "Ⅷ", "ⅷ", "ⅷ", "ⅷ",  ],
[ "henry ⅷ", "henry ⅷ", "Henry Ⅷ", "HENRY Ⅷ", "henry ⅷ", "Henry Ⅷ", "HENRY Ⅷ", "henry ⅷ", "henry ⅷ", "henry ⅷ",  ],
[ "ⓚ", "ⓚ", "Ⓚ", "Ⓚ", "ⓚ", "Ⓚ", "Ⓚ", "ⓚ", "ⓚ", "ⓚ",  ],
[ "i work at ⓚ", "i work at ⓚ", "I Work At Ⓚ", "I WORK AT Ⓚ", "i work at ⓚ", "I Work At Ⓚ", "I WORK AT Ⓚ", "i work at ⓚ", "i work at ⓚ", "i work at ⓚ",  ],
[ "istambul", "istambul", "Istambul", "ISTAMBUL", "istambul", "Istambul", "ISTAMBUL", "istambul", "istambul", "istambul",  ],
[ "i̇stanbul", "i̇stanbul", "İstanbul", "İSTANBUL", "i̇stanbul", "İstanbul", "İSTANBUL", "i̇stanbul", "i̇stanbul", "i̇stanbul",  ],
[ "İstanbul", "i̇stanbul", "İstanbul", "İSTANBUL", "i̇stanbul", "İstanbul", "İSTANBUL", "i̇stanbul", "ı̇stanbul", "i̇stanbul",  ],
[ "İSTANBUL", "istanbul", "İstanbul", "İSTANBUL", "i̇stanbul", "İstanbul", "İSTANBUL", "İstanbul", "istanbul", "i̇stanbul",  ],
[ "στιγμας", "στιγμας", "Στιγμας", "ΣΤΙΓΜΑΣ", "στιγμας", "Στιγμας", "ΣΤΙΓΜΑΣ", "στιγμασ", "στιγμασ", "στιγμασ",  ],
[ "στιγμασ", "στιγμασ", "Στιγμασ", "ΣΤΙΓΜΑΣ", "στιγμασ", "Στιγμασ", "ΣΤΙΓΜΑΣ", "στιγμασ", "στιγμασ", "στιγμασ",  ],
[ "ΣΤΙΓΜΑΣ", "στιγμασ", "Στιγμασ", "ΣΤΙΓΜΑΣ", "στιγμασ", "Στιγμασ", "ΣΤΙΓΜΑΣ", "στιγμασ", "στιγμασ", "στιγμασ",  ],
[ "ʀᴀʀᴇ", "ʀᴀʀᴇ", "Ʀᴀʀᴇ", "ƦᴀƦᴇ", "ʀᴀʀᴇ", "Ʀᴀʀᴇ", "ƦᴀƦᴇ", "ʀᴀʀᴇ", "ʀᴀʀᴇ", "ʀᴀʀᴇ",  ],
[ "𐐼𐐯𐑅𐐨𐑉𐐯𐐻", "𐐼𐐯𐑅𐐨𐑉𐐯𐐻", "𐐔𐐯𐑅𐐨𐑉𐐯𐐻", "𐐔𐐇𐐝𐐀𐐡𐐇𐐓", "𐐼𐐯𐑅𐐨𐑉𐐯𐐻", "𐐔𐐯𐑅𐐨𐑉𐐯𐐻", "𐐔𐐇𐐝𐐀𐐡𐐇𐐓", "𐐼𐐯𐑅𐐨𐑉𐐯𐐻", "𐐼𐐯𐑅𐐨𐑉𐐯𐐻", "𐐼𐐯𐑅𐐨𐑉𐐯𐐻",  ],
[ "Ԧԧ", "ԧԧ", "Ԧԧ", "ԦԦ", "ԧԧ", "Ԧԧ", "ԦԦ", "ԧԧ", "ԧԧ", "ԧԧ",  ],
[ "ﬓﬔﬕﬖﬗ", "ﬓﬔﬕﬖﬗ", "ﬓﬔﬕﬖﬗ", "ﬓﬔﬕﬖﬗ", "ﬓﬔﬕﬖﬗ", "Մնﬔﬕﬖﬗ", "ՄՆՄԵՄԻՎՆՄԽ", "ﬓﬔﬕﬖﬗ", "մնմեմիվնմխ", "մնմեմիվնմխ",  ],
[ "ŉ groot", "ŉ groot", "ŉ Groot", "ŉ GROOT", "ŉ groot", "ʼN Groot", "ʼN GROOT", "ŉ groot", "ʼn groot", "ʼn groot",  ],
[ "ẚ", "ẚ", "ẚ", "ẚ", "ẚ", "Aʾ", "Aʾ", "ẚ", "aʾ", "aʾ",  ],
[ "ﬀ", "ﬀ", "ﬀ", "ﬀ", "ﬀ", "Ff", "FF", "ﬀ", "ff", "ff",  ],
[ "ǰ", "ǰ", "ǰ", "ǰ", "ǰ", "J̌", "J̌", "ǰ", "ǰ", "ǰ",  ],
[ "550 nm or Å", "550 nm or å", "550 Nm Or Å", "550 NM OR Å", "550 nm or å", "550 Nm Or Å", "550 NM OR Å", "550 nm or å", "550 nm or å", "550 nm or å",  ],
);

    use feature qw(fc);

    for (@test_table) {
        my ($simple_lc, $simple_tc, $simple_uc, $simple_fc) = @{$_}[1, 2, 3, 7];
        my ($orig, $lower, $titlecase, $upper, $fc_turkic, $fc_full) = @{$_}[0,4,5,6,8,9];

        if ($orig =~ /(\P{Assigned})/) {   # So can fail gracefully in earlier
                                           # Unicode versions
            fail(sprintf "because U+%04X is unassigned", ord($1));
            next;
        }
        is( fc($orig), $fc_full, "fc('$orig') returns '$fc_full'" );
        is( "\F$orig", $fc_full, '\F works' );
        is( lc($orig), $lower,   "lc('$orig') returns '$lower'" );
        is( "\L$orig", $lower,   '\L works' );
        is( uc($orig), $upper,   "uc('$orig') returns '$upper'" );
        is( "\U$orig", $upper,   '\U works' );
    }
}

{
    use feature qw(fc);
    package Eeyup  { use overload q{""} => sub { main::uni_to_native("\x{df}")   }, fallback => 1 }
    package Uunope { use overload q{""} => sub { "\x{30cb}" }, fallback => 1 }
    package Undef  { use overload q{""} => sub {   undef    }, fallback => 1 }

    my $obj = bless {}, "Eeyup";
    is(fc($obj), "ss", "fc() works on overloaded objects returning latin-1");
    $obj = bless {}, "Eeyup";
    is("\F$obj", "ss", '\F works on overloaded objects returning latin-1');

    $obj = bless {}, "Uunope";
    is(fc($obj), "\x{30cb}", "fc() works on overloaded objects returning UTF-8");
    $obj = bless {}, "Uunope";
    is("\F$obj", "\x{30cb}", '\F works on overloaded objects returning UTF-8');

    $obj = bless {}, "Undef";
    my $warnings;
    {
        no warnings;
        use warnings "uninitialized";
        local $SIG{__WARN__} = sub { $warnings++; like(shift, qr/Use of uninitialized value (?:\$obj )?in fc/) };
        fc(undef);
        fc($obj);
    }
    is( $warnings, 2, "correct number of warnings" );

    my $fetched = 0;
    package Derpy { sub TIESCALAR { bless {}, shift } sub FETCH { $fetched++; main::uni_to_native("\x{df}") } }

    tie my $x, "Derpy";

    is( fc($x), "ss", "fc() works on tied values" );
    is( $fetched, 1, "and only calls the magic once" );

}

{
    use feature qw( fc );
    my $troublesome1 = uni_to_native("\xdf") x 11; #SvLEN should be 12, SvCUR should be 11
                                    #So this should force fc() to grow the string.

    is( fc($troublesome1), "ss" x 11, "fc() grows the string" );

    my $troublesome2 = "abcdef:" . uni_to_native("\x{df}")
                     . ":fjksjs"; #SvLEN should be 16, SvCUR should be 15
    is( fc($troublesome2), "abcdef:ss:fjksjs", "fc() expands \\x{DF} in the middle of a string that needs to grow" );

    my $troublesome3 = ":" . uni_to_native("\x{df}") . ":";
    is( fc($troublesome3), ":ss:", "fc() expands \\x{DF} in the middle of a string" );


    my $troublesome4 = uni_to_native("\x{B5}"); #\N{MICRON SIGN} is latin-1, but its foldcase is in UTF-8

    is( fc($troublesome4), "\x{3BC}", "fc() for a latin-1 \x{B5} returns UTF-8" );
    ok( !utf8::is_utf8($troublesome4), "fc() doesn't upgrade the original string" );


    my $troublesome5 = uni_to_native("\x{C9}") . "abda"
                     . uni_to_native("\x{B5}") . "aaf"
                     . uni_to_native("\x{C8}");  # Up until foldcasing \x{B5}, the string
                                                    # was in Latin-1. This tests that the
                                                    # results don't have illegal UTF-8
                                                    # (i.e. leftover latin-1) in them

    is( fc($troublesome5), uni_to_native("\x{E9}") . "abda\x{3BC}aaf"
                         . uni_to_native("\x{E8}") );
}


SKIP: {
    use feature qw( fc unicode_strings );

    skip "locales not available", 256 unless locales_enabled('LC_ALL');

    setlocale(&POSIX::LC_ALL, "C");

    # This tests both code paths in pp_fc

    for (0..0xff) {
        my $latin1 = chr;
        my $utf8   = $latin1;
        utf8::downgrade($latin1); #No-op, but doesn't hurt
        utf8::upgrade($utf8);
        is(fc($latin1), fc($utf8), "fc() gives the same results for \\x{$_} in Latin-1 and UTF-8 under unicode_strings");
        SKIP: {
            skip 'Locales not available', 2 unless locales_enabled('LC_CTYPE');
            use locale;
            is(fc($latin1), lc($latin1), "use locale; fc(qq{\\x{$_}}), lc(qq{\\x{$_}}) when qq{\\x{$_}} is in latin-1");
            is(fc($utf8), lc($utf8), "use locale; fc(qq{\\x{$_}}), lc(qq{\\x{$_}}) when qq{\\x{$_}} is in latin-1");
        }
        {
            no feature 'unicode_strings';
            is(fc($latin1), lc($latin1), "under nothing, fc() for <256 is the same as lc");
        }
    }
}

my $utf8_locale = find_utf8_ctype_locale();

{
    use feature qw( fc );
    use locale;
    no warnings 'locale';   # Would otherwise warn
    is(fc("\x{1E9E}"), fc("\x{17F}\x{17F}"), 'fc("\x{1E9E}") eq fc("\x{17F}\x{17F}")');
    use warnings 'locale';
    SKIP: {
        skip 'Can\'t find a UTF-8 locale', 1 unless defined $utf8_locale;
        setlocale(&LC_CTYPE, $utf8_locale);
        is(fc("\x{1E9E}"), "ss", 'fc("\x{1E9E}") eq "ss" in a UTF-8 locale)');
    }
}

SKIP: {
    skip 'Can\'t find a UTF-8 locale', 256 unless defined $utf8_locale;

    use feature qw( fc unicode_strings );

    # Get the official fc values outside locale.
    no locale;
    my @unicode_fc;
    for (0..0xff) {
        push @unicode_fc, fc(chr);
    }

    # These should match the UTF-8 locale values
    setlocale(&LC_CTYPE, $utf8_locale);
    use locale;
    for (0..0xff) {
        is(fc(chr), $unicode_fc[$_], "In a UTF-8 locale, fc(chr $_) is the same as official Unicode");
    }
}


my $num_tests = curr_test() - 1;

plan($num_tests);
