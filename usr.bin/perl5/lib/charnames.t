#!./perl
use strict;

# Test charnames.pm.  If $ENV{PERL_RUN_SLOW_TESTS} is unset or 0, a random
# selection of names is tested, a higher percentage of regular names is tested
# than algorithmically-determined names.

my $run_slow_tests = $ENV{PERL_RUN_SLOW_TESTS} || 0;

my $RUN_SLOW_TESTS_EVERY_CODE_POINT = 100;

# If $ENV{PERL_RUN_SLOW_TESTS} is at least 1 and less than the number above,
# all code points with names are tested, including wildcard search names.  If
# it is at least that number, all 1,114,112 Unicode code points are tested.

# Because \N{} is compile time, any warnings will get generated before
# execution, so have to have an array, and arrange things so no warning
# is generated twice to verify that in fact a warning did happen
my @WARN;

BEGIN {
    unless(grep /blib/, @INC) {
	chdir 't' if -d 't';
	@INC = '../lib';
    }
    $SIG{__WARN__} = sub { push @WARN, @_ };
}

our $local_tests = 'no_plan';

# ---- For the alias extensions
require "../t/lib/common.pl";

is("Here\N{EXCLAMATION MARK}?", "Here!?", "Basic sanity, autoload of :full upon \\N");
is("\N{latin: Q}", "Q", "autoload of :short upon \\N");

{
    use bytes;			# TEST -utf8 can switch utf8 on

    my $res = eval <<'EOE';
use charnames ":full";
"Here: \N{CYRILLIC SMALL LETTER BE}!";
1
EOE

    like($@, qr/above 0xFF/, "Verify get warning for \\N{above ff} under 'use bytes' with :full");
    ok(! defined $res, "... and result is undefined");

    $res = eval <<'EOE';
use charnames 'cyrillic';
"Here: \N{Be}!";
1
EOE
    like($@, qr/CYRILLIC CAPITAL LETTER BE.*above 0xFF/, "Verify get warning under 'use bytes' with explicit script");
    ok(! defined $res, "... and result is undefined");

    $res = eval <<'EOE';
use charnames ':full', ":alias" => { BOM => "LATIN SMALL LETTER B" };
"\N{BOM}";
EOE
    is ($@, "", "Verify that there is no warning for \\N{below 256} under 'use bytes'");
    is ($res, 'b', "Verify that can redefine a standard alias");
}

{

    use charnames ":alias" => { mychar1 => "0xE8000",
                                mychar2 => 983040,  # U+F0000
                                mychar3 => "U+100000",
                                myctrl => utf8::unicode_to_native(0x80),
                                mylarge => "U+111000",
                              };
    is ("\N{PILE OF POO}", chr(0x1F4A9), "Verify :alias alone implies :full");
    is ("\N{mychar1}", chr(0xE8000), "Verify that can define hex alias");
    is (charnames::viacode(0xE8000), "mychar1", "And that can get the alias back");
    is ("\N{mychar2}", chr(0xF0000), "Verify that can define decimal alias");
    is (charnames::viacode(0xF0000), "mychar2", "And that can get the alias back");
    is ("\N{mychar3}", chr(0x100000), "Verify that can define U+... alias");
    is (charnames::viacode(0x100000), "mychar3", "And that can get the alias back");
    is ("\N{mylarge}", chr(0x111000), "Verify that can define alias beyond Unicode");
    is (charnames::viacode(0x111000), "mylarge", "And that can get the alias back");
    is (charnames::viacode(utf8::unicode_to_native(0x80)), "myctrl", "Verify that can name a nameless control");

}

my $encoded_be = byte_utf8a_to_utf8n("\320\261");
my $encoded_alpha = byte_utf8a_to_utf8n("\316\261");
my $encoded_bet = byte_utf8a_to_utf8n("\327\221");
my $encoded_deseng = byte_utf8a_to_utf8n("\360\220\221\215");

sub to_bytes {
    unpack"U0a*", shift;
}

sub get_loose_name ($) { # Modify name to stress the loose tests.

    # First, all lower case,
    my $loose_name = lc shift;

    # Then squeeze out all the blanks not adjacent to hyphens, but make the
    # spaces that are adjacent to hypens into two, to make sure the code isn't
    # looking for just one when looking for non-medial hyphens.
    $loose_name =~ s/ (?<! - ) \ + (?! - )//gx;
    $loose_name =~ s/ /  /g;

    # Similarly, double the hyphens
    $loose_name =~ s/-/--/g;

    # And convert ABC into "A B-C" to add medial hyphens and spaces.  Probably
    # better to do this randomly, but  think this is sufficient.
    $loose_name =~ s/ ([^-\s]) ([^-\s]) ([^-\s]) /$1 $2-$3/gx;

    return $loose_name
}

sub test_vianame ($$$) {
    CORE::state $wildcard_count = 0;

    # Run the vianame tests on a code point, both loose and full

    my $all_pass = 1;

    # $i is the code point in decimal; $hex in hexadecimal; $name is
    # character name to test
    my ($i, $hex, $name) = @_;

    # Get a copy of the name modified to stress the loose tests.
    my $loose_name = get_loose_name($name);

    my $right_anchor;

    # Switch loose and full in vianame vs string_vianame half the time
    if (rand() < .5) {
        use charnames ":full";
        $all_pass &= is(charnames::vianame($name), $i,
                        "Verify vianame(\"$name\") is 0x$hex");
        use charnames ":loose";
        $all_pass &= is(charnames::string_vianame($loose_name), chr($i),
                    "Verify string_vianame(\"$loose_name\") is chr(0x$hex)");
        $right_anchor = '\\Z';
    }
    else {
        use charnames ":loose";
        $all_pass &= is(charnames::vianame($loose_name), $i,
                        "Verify vianame(\"$loose_name\") is 0x$hex");
        use charnames ":full";
        $all_pass &= is(charnames::string_vianame($name), chr($i),
                        "Verify string_vianame(\"$name\") is chr(0x$hex)");
        $right_anchor = '\\z';
    }

    my $left_anchor = (rand() < .5) ? '^' : '\\A';

    # \p{name=} is always loose matching
    $all_pass &= like(chr($i), qr/^\p{name=$loose_name}$/,
                      "Verify /\\p{name=$loose_name}/ matches chr(0x$hex)");

    $wildcard_count++;

    # XXX temporary to see if the failure we are occasionally seeing is
    # confined to this code point.  GH #17671
    next if $i == 0;

    # Because wildcard name matching is so real-time intensive, do it less
    # frequently than the others
    if ($wildcard_count >= 10) {
        $wildcard_count = 0;

        # A few control characters have anomalous names containing
        # parentheses, which need to be escaped.
        my $name_ref = \$name;
        my $mod_name;
        if ($i <= 0x85) {   # NEL in ASCII; affected controls are lower than
                            # this in EBCDIC
            $mod_name = $name =~ s/([()])/\\$1/gr;
            $name_ref = \$mod_name;
        }

        # We anchor the name, randomly with the possible anchors.
        my $assembled = $left_anchor. $$name_ref . $right_anchor;

        # \p{name=/.../} is always full matching
        $all_pass &= like(chr($i), qr!^\p{name=/$assembled/}!,
                          "Verify /\\p{name=/$assembled/} matches chr(0x$hex)");
    }

    return $all_pass;
}

{
  use charnames ':full';

  is(to_bytes("\N{CYRILLIC SMALL LETTER BE}"), $encoded_be,
              'Verify \N{CYRILLIC SMALL LETTER BE} is the correct UTF8');

  use charnames qw(cyrillic greek :short);

  is(to_bytes("\N{be},\N{alpha},\N{hebrew:bet}"),
                                    "$encoded_be,$encoded_alpha,$encoded_bet",
              'Verify using scripts gives the correct UTF8');
}

{
  my $caught_error;
  local $SIG{__WARN__} = sub { $caught_error = shift; };
  eval q{
    use charnames qw(runic greek);
    is($caught_error, undef, "no letter name clashes between runic and greek");
  };
}

{
  my $caught_error;
  local $SIG{__WARN__} = sub { $caught_error = shift; };
  eval q{
    use charnames qw(hebrew arabic :full);
    like(
      $caught_error,
      qr/charnames: some short character names may clash in \[ARABIC, HEBREW\], for example ALEF/,
      "warned about potential character name clashes when asking for 'hebrew' and 'arabic'"
    );
    ok("\N{alef}"  eq "\N{HEBREW LETTER ALEF}",  '\N{alef} gives HEBREW LETTER ALEF because we asked for Hebrew first');
    ok("\N{bet}"   eq "\N{HEBREW LETTER BET}",   '\N{bet} gives HEBREW LETTER BET');
    ok("\N{sheen}" eq "\N{ARABIC LETTER SHEEN}", 'and \N{sheen} gives ARABIC LETTER SHEEN');
  };
  eval q{
    use charnames qw(arabic hebrew :full);
    like(
      $caught_error,
      qr/charnames: some short character names may clash in \[ARABIC, HEBREW\], for example ALEF/,
      "warned about potential character name clashes when asking for 'arabic' and 'hebrew'"
    );
    ok("\N{alef}"  eq "\N{ARABIC LETTER ALEF}",  '\N{alef} gives ARABIC LETTER ALEF because we asked for Arabic first');
    ok("\N{bet}"   eq "\N{HEBREW LETTER BET}",   '\N{bet} gives HEBREW LETTER BET');
    ok("\N{sheen}" eq "\N{ARABIC LETTER SHEEN}", 'and \N{sheen} gives ARABIC LETTER SHEEN');
  };
}

{
    use charnames ':full';
    is("\x{263a}", "\N{WHITE SMILING FACE}", 'Verify "\x{263a}" eq "\N{WHITE SMILING FACE}"');
    cmp_ok(length("\x{263a}"), '==', 1, 'Verify length of \x{263a} is 1');
    cmp_ok(length("\N{WHITE SMILING FACE}"), '==', 1, '... as is the length of \N{WHITE SMILING FACE}');
    is(sprintf("%vx", "\x{263a}"), "263a", 'Verify sprintf("%vx", "\x{263a}") eq "263a"');
    is(sprintf("%vx", "\N{WHITE SMILING FACE}"), "263a", 'Verify sprintf("%vx", "\N{WHITE SMILING FACE}") eq "263a"');
    is(sprintf("%vx", "\xFF\N{WHITE SMILING FACE}"), "ff.263a", 'Verify sprintf("%vx" eq "\xFF\N{WHITE SMILING FACE}"), "ff.263a"');
    is(sprintf("%vx", "\x{ff}\N{WHITE SMILING FACE}"), "ff.263a", 'Verify sprintf("%vx", "\x{ff}\N{WHITE SMILING FACE}") eq "ff.263a"');
}

{
    use charnames qw(:full);
    use utf8;

    my $x = "\x{221b}";
    my $named = "\N{CUBE ROOT}";

    cmp_ok(ord($x), '==', ord($named), 'Verify ord("\x{221b}") == ord("\N{CUBE ROOT}"');
}

{
    use charnames qw(:full);
    use utf8;
    is("\x{100}\N{CENT SIGN}", "\x{100}"."\N{CENT SIGN}", 'Verify "\x{100}\N{CENT SIGN}" eq "\x{100}"."\N{CENT SIGN}"');
}

{
    use charnames ':full';

    is(to_bytes("\N{DESERET SMALL LETTER ENG}"), $encoded_deseng,
                'Verify bytes of "\N{DESERET SMALL LETTER ENG}" are correct');
}

{
    # 20001114.001 (#4690)

    no utf8; # naked Latin-1

    use charnames ':full';
    my $text = "\N{LATIN CAPITAL LETTER A WITH DIAERESIS}";
    is($text, chr utf8::unicode_to_native(0xc4), 'Verify \N{} returns correct string under "no utf8"');

    # I'm not sure that this tests anything different from the above.
    cmp_ok(ord($text), '==', utf8::unicode_to_native(0xc4), '... and ords are ok');
}

{
    is(charnames::viacode(0x1234), "ETHIOPIC SYLLABLE SEE",
                          'Verify viacode(0x1234) eq "ETHIOPIC SYLLABLE SEE"');

    # No name
    ok(! defined charnames::viacode(0xFFFF), 'Verify \x{FFFF} has no name');
}

{
    cmp_ok(charnames::vianame("GOTHIC LETTER AHSA"), "==", 0x10330, "Verify vianame \\N{name} returns an ord");
    is(charnames::vianame("U+10330"), "\x{10330}", "Verify vianame \\N{U+hex} returns a chr");
    use warnings;
    my $warning_count = @WARN;
    ok (! defined charnames::vianame("NONE SUCH"), "Verify vianame returns undef for an undefined name");
    cmp_ok($warning_count, '==', scalar @WARN, "Verify vianame doesn't warn on unknown names");
    ok (! defined charnames::string_vianame("MORE NONE SUCH"), "Verify string_vianame returns undef for an undefined name");
    cmp_ok($warning_count, '==', scalar @WARN, "Verify string_vianame doesn't warn on unknown names");
    ok (! defined charnames::vianame(""), "Verify vianame returns undef for an empty value");
    cmp_ok($warning_count, '==', scalar @WARN, "... and no warning is generated");
    ok (! defined charnames::string_vianame(""), "Verify string_vianame returns undef for an empty value");
    cmp_ok($warning_count, '==', scalar @WARN, "... and no warning is generated");

    eval "qr/\\p{name=MORE NONE SUCH}/";
    like($@, qr/Can't find Unicode property definition "name=MORE NONE SUCH"/,
            '\p{name=} returns an appropriate error message on an undefined name');

    use bytes;
    is(charnames::vianame("GOTHIC LETTER AHSA"), 0x10330, "Verify vianame \\N{name} is unaffected by 'use bytes'");
    is(charnames::vianame("U+FF"), chr(utf8::unicode_to_native(0xFF)), "Verify vianame \\N{U+FF} is unaffected by 'use bytes'");
    cmp_ok($warning_count, '==', scalar @WARN, "Verify vianame doesn't warn on legal inputs under 'use bytes'");
    ok(! defined charnames::vianame("U+100"), "Verify vianame \\N{U+100} is undef under 'use bytes'");
    ok($warning_count == scalar @WARN - 1 && $WARN[-1] =~ /above 0xFF/, "Verify vianame gives appropriate warning for previous test");

    $warning_count = @WARN;
    ok(! defined charnames::string_vianame("GOTHIC LETTER AHSA"), "Verify string_vianame(\"GOTHIC LETTER AHSA\") is undefined under 'use bytes'");
    ok($warning_count == scalar @WARN - 1 && $WARN[-1] =~ /above 0xFF/, "Verify string_vianame gives appropriate warning for previous test");
    $warning_count = @WARN;
    eval "qr/\\p{name=GOTHIC LETTER AHSA}/";
    is($@, "", '\p{name=...} is unaffect by "use bytes"');
    is(charnames::string_vianame("U+FF"), chr(utf8::unicode_to_native(0xFF)), "Verify string_vianame(\"U+FF\") is chr(0xFF) under 'use bytes'");
    cmp_ok($warning_count, '==', scalar @WARN, "Verify string_vianame doesn't warn on legal inputs under 'use bytes'");
    is(charnames::string_vianame("LATIN SMALL LETTER Y WITH DIAERESIS"), chr(utf8::unicode_to_native(0xFF)), "Verify string_vianame(\"LATIN SMALL LETTER Y WITH DIAERESIS\") is chr(native 0xFF) under 'use bytes'");
    cmp_ok($warning_count, '==', scalar @WARN, "Verify string_vianame doesn't warn on legal inputs under 'use bytes'");
    ok(! defined charnames::string_vianame("U+100"), "Verify string_vianame \\N{U+100} is undef under 'use bytes'");
    ok($warning_count == scalar @WARN - 1 && $WARN[-1] =~ /above 0xFF/, "Verify string_vianame gives appropriate warning for previous test");
    $warning_count = @WARN;
    ok(! defined charnames::string_vianame("LATIN SMALL LETTER L WITH TILDE"), "Verify string_vianame(\"LATIN SMALL LETTER L WITH TILDE\") is undef under 'use bytes'");
    ok($warning_count == scalar @WARN - 1 && $WARN[-1] =~ /String.*above 0xFF/, "Verify string_vianame gives appropriate warning for previous test");

}

{
    # check that caching at least hasn't broken anything

    is(charnames::viacode(0x1234), "ETHIOPIC SYLLABLE SEE",
        'Verify caching');

    is(sprintf("%04X", charnames::vianame("GOTHIC LETTER AHSA")), "10330",
        'More caching');

}

# That these return the correct values is tested below when reading
# NamedSequences.txt
is("\N{TAMIL CONSONANT K}", charnames::string_vianame("TAMIL CONSONANT K"), "Verify \\N{TAMIL CONSONANT K} eq charnames::vianame(\"TAMIL CONSONANT K\")");

is("\N{CHARACTER TABULATION}", "\t", 'Verify "\N{CHARACTER TABULATION}" eq "\t"');

is("\N{ESCAPE}", "\e", 'Verify "\N{ESCAPE}" eq "\e"');
is("\N{NULL}", "\c@", 'Verify "\N{NULL}" eq "\c@"');
is("\N{LINE FEED (LF)}", "\n", 'Verify "\N{LINE FEED (LF)}" eq "\n"');
is("\N{LINE FEED}", "\n", 'Verify "\N{LINE FEED}" eq "\n"');
is("\N{LF}", "\n", 'Verify "\N{LF}" eq "\n"');

my $nel = chr utf8::unicode_to_native(0x85);
$nel = qr/^$nel$/;

like("\N{NEXT LINE (NEL)}", $nel, 'Verify "\N{NEXT LINE (NEL)}" is correct');
like("\N{NEXT LINE}", $nel, 'Verify "\N{NEXT LINE)" is correct');
like("\N{NEL}", $nel, 'Verify "\N{NEL}" is correct');
is("\N{BYTE ORDER MARK}", chr(0xFEFF), 'Verify "\N{BYTE ORDER MARK}" is correct');
is("\N{BOM}", chr(0xFEFF), 'Verify "\N{BOM}" is correct');

{
    use warnings 'deprecated';

    is("\N{HORIZONTAL TABULATION}", "\t", 'Verify "\N{HORIZONTAL TABULATION}" eq "\t"');

    my $ok = ! grep { /"HORIZONTAL TABULATION" is deprecated.*"CHARACTER TABULATION"/ } @WARN;
    ok($ok, '... and doesnt give deprecated warning');

    if ($^V lt v5.17.0) {
        is("\N{BELL}", "\a", 'Verify "\N{BELL}" eq "\a"');
        my $ok = grep { /"BELL" is deprecated.*"ALERT"/ } @WARN;
        ok($ok, '... and that gives correct deprecated warning');
    }

    no warnings 'deprecated';

    is("\N{VERTICAL TABULATION}", "\013", 'Verify "\N{VERTICAL TABULATION}" eq "\013"');

    my $nok = grep { /"VERTICAL TABULATION" is deprecated/ } @WARN;
    ok(! $nok,
    '... and doesnt give deprecated warning under no warnings "deprecated"');
}

is(charnames::viacode(0xFEFF), "ZERO WIDTH NO-BREAK SPACE",
   'Verify viacode(0xFEFF) is correct');

# These test that the changes to these in 6.1 are recognized.  (The double
# test of using viacode and vianame is less than optimal as two errors could
# cancel each other out, but later each is tested individually, and this
# sidesteps and EBCDIC issues.
is(charnames::viacode(charnames::vianame("CR")), "CARRIAGE RETURN",
            'Verify viacode(vianame("CR")) is "CARRIAGE RETURN"');
is(charnames::viacode(charnames::vianame("LF")), "LINE FEED",
            'Verify viacode(vianame("LF")) is "LINE FEED"');
is(charnames::viacode(charnames::vianame("FF")), "FORM FEED",
            'Verify viacode(vianame("FF")) is "FORM FEED"');
is(charnames::viacode(charnames::vianame("NEL")), "NEXT LINE",
            'Verify viacode(vianame("NEL")) is "NEXT LINE"');

{
    use warnings;
    cmp_ok(ord("\N{BOM}"), '==', 0xFEFF, 'Verify \N{BOM} is correct');
}

cmp_ok(ord("\N{ZWNJ}"), '==', 0x200C, 'Verify \N{ZWNJ} is correct');

cmp_ok(ord("\N{ZWJ}"), '==', 0x200D, 'Verify \N{ZWJ} is correct');

is("\N{U+263A}", "\N{WHITE SMILING FACE}", 'Verify "\N{U+263A}" eq "\N{WHITE SMILING FACE}"');

{
    cmp_ok( 0x3093, '==', charnames::vianame("HIRAGANA LETTER N"),
            'Verify vianame("HIRAGANA LETTER N") is correct');
    cmp_ok(0x0397, '==', charnames::vianame("GREEK CAPITAL LETTER ETA"),
           'Verify vianame("GREEK CAPITAL LETTER ETA") is correct');
}

ok(! defined charnames::viacode(0x110000),
   'Verify viacode(above unicode) is undefined');
ok((grep { /\Qyou asked for U+110000/ } @WARN), '... and gives warning');

is(charnames::viacode(0), "NULL", 'Verify charnames::viacode(0) eq "NULL"');
my $three_quarters = sprintf("%2X", utf8::unicode_to_native(0xBE));
is(charnames::viacode("$three_quarters"), "VULGAR FRACTION THREE QUARTERS", 'Verify charnames::viacode(native "BE") eq "VULGAR FRACTION THREE QUARTERS"');
is(charnames::viacode("U+00000000000FEED"), "ARABIC LETTER WAW ISOLATED FORM", 'Verify charnames::viacode("U+00000000000FEED") eq "ARABIC LETTER WAW ISOLATED FORM"');

test_vianame(0x116C, "116C", "HANGUL JUNGSEONG OE");
test_vianame(0x1180, "1180", "HANGUL JUNGSEONG O-E");
like(chr(0x59C3), qr/\p{name=\/\ACJK UNIFIED IDEOGRAPH-59C3\z\/}/,
         'Verify name wildcards delimitters can be escaped');
like(chr(0xD800), qr!\p{name=/\A\z/}!,
                                'Verify works on matching an empty name');

{
    no warnings 'deprecated';
    is("\N{LINE FEED}", "\N{LINE FEED (LF)}", 'Verify "\N{LINE FEED}" eq "\N{LINE FEED (LF)}"', 'Verify \N{LINE FEED} eq \N{LINE FEED (LF)}');
    is("\N{FORM FEED}", "\N{FORM FEED (FF)}", 'Verify "\N{FORM FEED}" eq "\N{FORM FEED (FF)}"');
    is("\N{CARRIAGE RETURN}", "\N{CARRIAGE RETURN (CR)}", 'Verify "\N{CARRIAGE RETURN}" eq "\N{CARRIAGE RETURN (CR)}"');
    is("\N{NEXT LINE}", "\N{NEXT LINE (NEL)}", 'Verify "\N{NEXT LINE}" eq "\N{NEXT LINE (NEL)}"');
    is("\N{NUL}", "\N{NULL}", 'Verify "\N{NUL}" eq "\N{NULL}"');
    is("\N{SOH}", "\N{START OF HEADING}", 'Verify "\N{SOH}" eq "\N{START OF HEADING}"');
    is("\N{STX}", "\N{START OF TEXT}", 'Verify "\N{STX}" eq "\N{START OF TEXT}"');
    is("\N{ETX}", "\N{END OF TEXT}", 'Verify "\N{ETX}" eq "\N{END OF TEXT}"');
    is("\N{EOT}", "\N{END OF TRANSMISSION}", 'Verify "\N{EOT}" eq "\N{END OF TRANSMISSION}"');
    is("\N{ENQ}", "\N{ENQUIRY}", 'Verify "\N{ENQ}" eq "\N{ENQUIRY}"');
    is("\N{ACK}", "\N{ACKNOWLEDGE}", 'Verify "\N{ACK}" eq "\N{ACKNOWLEDGE}"');
    is("\N{BEL}", "\N{BELL}", 'Verify "\N{BEL}" eq "\N{BELL}"') if $^V lt v5.17.0;
    is("\N{BS}", "\N{BACKSPACE}", 'Verify "\N{BS}" eq "\N{BACKSPACE}"');
    is("\N{HT}", "\N{HORIZONTAL TABULATION}", 'Verify "\N{HT}" eq "\N{HORIZONTAL TABULATION}"');
    is("\N{LF}", "\N{LINE FEED (LF)}", 'Verify "\N{LF}" eq "\N{LINE FEED (LF)}"');
    is("\N{VT}", "\N{VERTICAL TABULATION}", 'Verify "\N{VT}" eq "\N{VERTICAL TABULATION}"');
    is("\N{FF}", "\N{FORM FEED (FF)}", 'Verify "\N{FF}" eq "\N{FORM FEED (FF)}"');
    is("\N{CR}", "\N{CARRIAGE RETURN (CR)}", 'Verify "\N{CR}" eq "\N{CARRIAGE RETURN (CR)}"');
    is("\N{SO}", "\N{SHIFT OUT}", 'Verify "\N{SO}" eq "\N{SHIFT OUT}"');
    is("\N{SI}", "\N{SHIFT IN}", 'Verify "\N{SI}" eq "\N{SHIFT IN}"');
    is("\N{DLE}", "\N{DATA LINK ESCAPE}", 'Verify "\N{DLE}" eq "\N{DATA LINK ESCAPE}"');
    is("\N{DC1}", "\N{DEVICE CONTROL ONE}", 'Verify "\N{DC1}" eq "\N{DEVICE CONTROL ONE}"');
    is("\N{DC2}", "\N{DEVICE CONTROL TWO}", 'Verify "\N{DC2}" eq "\N{DEVICE CONTROL TWO}"');
    is("\N{DC3}", "\N{DEVICE CONTROL THREE}", 'Verify "\N{DC3}" eq "\N{DEVICE CONTROL THREE}"');
    is("\N{DC4}", "\N{DEVICE CONTROL FOUR}", 'Verify "\N{DC4}" eq "\N{DEVICE CONTROL FOUR}"');
    is("\N{NAK}", "\N{NEGATIVE ACKNOWLEDGE}", 'Verify "\N{NAK}" eq "\N{NEGATIVE ACKNOWLEDGE}"');
    is("\N{SYN}", "\N{SYNCHRONOUS IDLE}", 'Verify "\N{SYN}" eq "\N{SYNCHRONOUS IDLE}"');
    is("\N{ETB}", "\N{END OF TRANSMISSION BLOCK}", 'Verify "\N{ETB}" eq "\N{END OF TRANSMISSION BLOCK}"');
    is("\N{CAN}", "\N{CANCEL}", 'Verify "\N{CAN}" eq "\N{CANCEL}"');
    is("\N{EOM}", "\N{END OF MEDIUM}", 'Verify "\N{EOM}" eq "\N{END OF MEDIUM}"');
    is("\N{SUB}", "\N{SUBSTITUTE}", 'Verify "\N{SUB}" eq "\N{SUBSTITUTE}"');
    is("\N{ESC}", "\N{ESCAPE}", 'Verify "\N{ESC}" eq "\N{ESCAPE}"');
    is("\N{FS}", "\N{FILE SEPARATOR}", 'Verify "\N{FS}" eq "\N{FILE SEPARATOR}"');
    is("\N{GS}", "\N{GROUP SEPARATOR}", 'Verify "\N{GS}" eq "\N{GROUP SEPARATOR}"');
    is("\N{RS}", "\N{RECORD SEPARATOR}", 'Verify "\N{RS}" eq "\N{RECORD SEPARATOR}"');
    is("\N{US}", "\N{UNIT SEPARATOR}", 'Verify "\N{US}" eq "\N{UNIT SEPARATOR}"');
    is("\N{DEL}", "\N{DELETE}", 'Verify "\N{DEL}" eq "\N{DELETE}"');
    is("\N{BPH}", "\N{BREAK PERMITTED HERE}", 'Verify "\N{BPH}" eq "\N{BREAK PERMITTED HERE}"');
    is("\N{NBH}", "\N{NO BREAK HERE}", 'Verify "\N{NBH}" eq "\N{NO BREAK HERE}"');
    is("\N{NEL}", "\N{NEXT LINE (NEL)}", 'Verify "\N{NEL}" eq "\N{NEXT LINE (NEL)}"');
    is("\N{SSA}", "\N{START OF SELECTED AREA}", 'Verify "\N{SSA}" eq "\N{START OF SELECTED AREA}"');
    is("\N{ESA}", "\N{END OF SELECTED AREA}", 'Verify "\N{ESA}" eq "\N{END OF SELECTED AREA}"');
    is("\N{HTS}", "\N{CHARACTER TABULATION SET}", 'Verify "\N{HTS}" eq "\N{CHARACTER TABULATION SET}"');
    is("\N{HTJ}", "\N{CHARACTER TABULATION WITH JUSTIFICATION}", 'Verify "\N{HTJ}" eq "\N{CHARACTER TABULATION WITH JUSTIFICATION}"');
    is("\N{VTS}", "\N{LINE TABULATION SET}", 'Verify "\N{VTS}" eq "\N{LINE TABULATION SET}"');
    is("\N{PLD}", "\N{PARTIAL LINE FORWARD}", 'Verify "\N{PLD}" eq "\N{PARTIAL LINE FORWARD}"');
    is("\N{PLU}", "\N{PARTIAL LINE BACKWARD}", 'Verify "\N{PLU}" eq "\N{PARTIAL LINE BACKWARD}"');
    is("\N{RI}", "\N{REVERSE LINE FEED}", 'Verify "\N{RI}" eq "\N{REVERSE LINE FEED}"');
    is("\N{SS2}", "\N{SINGLE SHIFT TWO}", 'Verify "\N{SS2}" eq "\N{SINGLE SHIFT TWO}"');
    is("\N{SS3}", "\N{SINGLE SHIFT THREE}", 'Verify "\N{SS3}" eq "\N{SINGLE SHIFT THREE}"');
    is("\N{DCS}", "\N{DEVICE CONTROL STRING}", 'Verify "\N{DCS}" eq "\N{DEVICE CONTROL STRING}"');
    is("\N{PU1}", "\N{PRIVATE USE ONE}", 'Verify "\N{PU1}" eq "\N{PRIVATE USE ONE}"');
    is("\N{PU2}", "\N{PRIVATE USE TWO}", 'Verify "\N{PU2}" eq "\N{PRIVATE USE TWO}"');
    is("\N{STS}", "\N{SET TRANSMIT STATE}", 'Verify "\N{STS}" eq "\N{SET TRANSMIT STATE}"');
    is("\N{CCH}", "\N{CANCEL CHARACTER}", 'Verify "\N{CCH}" eq "\N{CANCEL CHARACTER}"');
    is("\N{MW}", "\N{MESSAGE WAITING}", 'Verify "\N{MW}" eq "\N{MESSAGE WAITING}"');
    is("\N{SPA}", "\N{START OF GUARDED AREA}", 'Verify "\N{SPA}" eq "\N{START OF GUARDED AREA}"');
    is("\N{EPA}", "\N{END OF GUARDED AREA}", 'Verify "\N{EPA}" eq "\N{END OF GUARDED AREA}"');
    is("\N{SOS}", "\N{START OF STRING}", 'Verify "\N{SOS}" eq "\N{START OF STRING}"');
    is("\N{SCI}", "\N{SINGLE CHARACTER INTRODUCER}", 'Verify "\N{SCI}" eq "\N{SINGLE CHARACTER INTRODUCER}"');
    is("\N{CSI}", "\N{CONTROL SEQUENCE INTRODUCER}", 'Verify "\N{CSI}" eq "\N{CONTROL SEQUENCE INTRODUCER}"');
    is("\N{ST}", "\N{STRING TERMINATOR}", 'Verify "\N{ST}" eq "\N{STRING TERMINATOR}"');
    is("\N{OSC}", "\N{OPERATING SYSTEM COMMAND}", 'Verify "\N{OSC}" eq "\N{OPERATING SYSTEM COMMAND}"');
    is("\N{PM}", "\N{PRIVACY MESSAGE}", 'Verify "\N{PM}" eq "\N{PRIVACY MESSAGE}"');
    is("\N{APC}", "\N{APPLICATION PROGRAM COMMAND}", 'Verify "\N{APC}" eq "\N{APPLICATION PROGRAM COMMAND}"');
    is("\N{PADDING CHARACTER}", "\N{PAD}", 'Verify "\N{PADDING CHARACTER}" eq "\N{PAD}"');
    is("\N{HIGH OCTET PRESET}","\N{HOP}", 'Verify "\N{HIGH OCTET PRESET}" eq "\N{HOP}"');
    is("\N{INDEX}", "\N{IND}", 'Verify "\N{INDEX}" eq "\N{IND}"');
    is("\N{SINGLE GRAPHIC CHARACTER INTRODUCER}", "\N{SGC}", 'Verify "\N{SINGLE GRAPHIC CHARACTER INTRODUCER}" eq "\N{SGC}"');
    is("\N{BOM}", "\N{BYTE ORDER MARK}", 'Verify "\N{BOM}" eq "\N{BYTE ORDER MARK}"');
    is("\N{CGJ}", "\N{COMBINING GRAPHEME JOINER}", 'Verify "\N{CGJ}" eq "\N{COMBINING GRAPHEME JOINER}"');
    is("\N{FVS1}", "\N{MONGOLIAN FREE VARIATION SELECTOR ONE}", 'Verify "\N{FVS1}" eq "\N{MONGOLIAN FREE VARIATION SELECTOR ONE}"');
    is("\N{FVS2}", "\N{MONGOLIAN FREE VARIATION SELECTOR TWO}", 'Verify "\N{FVS2}" eq "\N{MONGOLIAN FREE VARIATION SELECTOR TWO}"');
    is("\N{FVS3}", "\N{MONGOLIAN FREE VARIATION SELECTOR THREE}", 'Verify "\N{FVS3}" eq "\N{MONGOLIAN FREE VARIATION SELECTOR THREE}"');
    is("\N{LRE}", "\N{LEFT-TO-RIGHT EMBEDDING}", 'Verify "\N{LRE}" eq "\N{LEFT-TO-RIGHT EMBEDDING}"');
    is("\N{LRM}", "\N{LEFT-TO-RIGHT MARK}", 'Verify "\N{LRM}" eq "\N{LEFT-TO-RIGHT MARK}"');
    is("\N{LRO}", "\N{LEFT-TO-RIGHT OVERRIDE}", 'Verify "\N{LRO}" eq "\N{LEFT-TO-RIGHT OVERRIDE}"');
    is("\N{MMSP}", "\N{MEDIUM MATHEMATICAL SPACE}", 'Verify "\N{MMSP}" eq "\N{MEDIUM MATHEMATICAL SPACE}"');
    is("\N{MVS}", "\N{MONGOLIAN VOWEL SEPARATOR}", 'Verify "\N{MVS}" eq "\N{MONGOLIAN VOWEL SEPARATOR}"');
    is("\N{NBSP}", "\N{NO-BREAK SPACE}", 'Verify "\N{NBSP}" eq "\N{NO-BREAK SPACE}"');
    is("\N{NNBSP}", "\N{NARROW NO-BREAK SPACE}", 'Verify "\N{NNBSP}" eq "\N{NARROW NO-BREAK SPACE}"');
    is("\N{PDF}", "\N{POP DIRECTIONAL FORMATTING}", 'Verify "\N{PDF}" eq "\N{POP DIRECTIONAL FORMATTING}"');
    is("\N{RLE}", "\N{RIGHT-TO-LEFT EMBEDDING}", 'Verify "\N{RLE}" eq "\N{RIGHT-TO-LEFT EMBEDDING}"');
    is("\N{RLM}", "\N{RIGHT-TO-LEFT MARK}", 'Verify "\N{RLM}" eq "\N{RIGHT-TO-LEFT MARK}"');
    is("\N{RLO}", "\N{RIGHT-TO-LEFT OVERRIDE}", 'Verify "\N{RLO}" eq "\N{RIGHT-TO-LEFT OVERRIDE}"');
    is("\N{SHY}", "\N{SOFT HYPHEN}", 'Verify "\N{SHY}" eq "\N{SOFT HYPHEN}"');
    is("\N{WJ}", "\N{WORD JOINER}", 'Verify "\N{WJ}" eq "\N{WORD JOINER}"');
    is("\N{ZWJ}", "\N{ZERO WIDTH JOINER}", 'Verify "\N{ZWJ}" eq "\N{ZERO WIDTH JOINER}"');
    is("\N{ZWNJ}", "\N{ZERO WIDTH NON-JOINER}", 'Verify "\N{ZWNJ}" eq "\N{ZERO WIDTH NON-JOINER}"');
    is("\N{ZWSP}", "\N{ZERO WIDTH SPACE}", 'Verify "\N{ZWSP}" eq "\N{ZERO WIDTH SPACE}"');
    is("\N{HORIZONTAL TABULATION}", "\N{CHARACTER TABULATION}", 'Verify "\N{HORIZONTAL TABULATION}" eq "\N{CHARACTER TABULATION}"');
    is("\N{VERTICAL TABULATION}", "\N{LINE TABULATION}", 'Verify "\N{VERTICAL TABULATION}" eq "\N{LINE TABULATION}"');
    is("\N{FILE SEPARATOR}", "\N{INFORMATION SEPARATOR FOUR}", 'Verify "\N{FILE SEPARATOR}" eq "\N{INFORMATION SEPARATOR FOUR}"');
    is("\N{GROUP SEPARATOR}", "\N{INFORMATION SEPARATOR THREE}", 'Verify "\N{GROUP SEPARATOR}" eq "\N{INFORMATION SEPARATOR THREE}"');
    is("\N{RECORD SEPARATOR}", "\N{INFORMATION SEPARATOR TWO}", 'Verify "\N{RECORD SEPARATOR}" eq "\N{INFORMATION SEPARATOR TWO}"');
    is("\N{UNIT SEPARATOR}", "\N{INFORMATION SEPARATOR ONE}", 'Verify "\N{UNIT SEPARATOR}" eq "\N{INFORMATION SEPARATOR ONE}"');
    is("\N{HORIZONTAL TABULATION SET}", "\N{CHARACTER TABULATION SET}", 'Verify "\N{HORIZONTAL TABULATION SET}" eq "\N{CHARACTER TABULATION SET}"');
    is("\N{HORIZONTAL TABULATION WITH JUSTIFICATION}", "\N{CHARACTER TABULATION WITH JUSTIFICATION}", 'Verify "\N{HORIZONTAL TABULATION WITH JUSTIFICATION}" eq "\N{CHARACTER TABULATION WITH JUSTIFICATION}"');
    is("\N{PARTIAL LINE DOWN}", "\N{PARTIAL LINE FORWARD}", 'Verify "\N{PARTIAL LINE DOWN}" eq "\N{PARTIAL LINE FORWARD}"');
    is("\N{PARTIAL LINE UP}", "\N{PARTIAL LINE BACKWARD}", 'Verify "\N{PARTIAL LINE UP}" eq "\N{PARTIAL LINE BACKWARD}"');
    is("\N{VERTICAL TABULATION SET}", "\N{LINE TABULATION SET}", 'Verify "\N{VERTICAL TABULATION SET}" eq "\N{LINE TABULATION SET}"');
    is("\N{REVERSE INDEX}", "\N{REVERSE LINE FEED}", 'Verify "\N{REVERSE INDEX}" eq "\N{REVERSE LINE FEED}"');
    is("\N{SINGLE-SHIFT 2}", "\N{SINGLE SHIFT TWO}", 'Verify "\N{SINGLE-SHIFT 2}" eq "\N{SINGLE SHIFT TWO}"');
    is("\N{SINGLE-SHIFT-2}", "\N{SINGLE-SHIFT 2}", 'Verify "\N{SINGLE-SHIFT-2}" eq "\N{SINGLE SHIFT 2}"');
    is("\N{SINGLE-SHIFT 3}", "\N{SINGLE SHIFT THREE}", 'Verify "\N{SINGLE-SHIFT 3}" eq "\N{SINGLE SHIFT THREE}"');
    is("\N{SINGLE-SHIFT-3}", "\N{SINGLE-SHIFT 3}", 'Verify "\N{SINGLE-SHIFT-3}" eq "\N{SINGLE SHIFT 3}"');
    is("\N{PRIVATE USE 1}", "\N{PRIVATE USE ONE}", 'Verify "\N{PRIVATE USE 1}" eq "\N{PRIVATE USE ONE}"');
    is("\N{PRIVATE USE-1}", "\N{PRIVATE USE 1}", 'Verify "\N{PRIVATE USE-1}" eq "\N{PRIVATE USE 1}"');
    is("\N{PRIVATE USE 2}", "\N{PRIVATE USE TWO}", 'Verify "\N{PRIVATE USE 2}" eq "\N{PRIVATE USE TWO}"');
    is("\N{PRIVATE USE-2}", "\N{PRIVATE USE 2}", 'Verify "\N{PRIVATE USE-2}" eq "\N{PRIVATE USE 2}"');
    is("\N{START OF PROTECTED AREA}", "\N{START OF GUARDED AREA}", 'Verify "\N{START OF PROTECTED AREA}" eq "\N{START OF GUARDED AREA}"');
    is("\N{END OF PROTECTED AREA}", "\N{END OF GUARDED AREA}", 'Verify "\N{END OF PROTECTED AREA}" eq "\N{END OF GUARDED AREA}"');
    is("\N{VS1}", "\N{VARIATION SELECTOR-1}", 'Verify "\N{VS1}" eq "\N{VARIATION SELECTOR-1}"');
    is("\N{VS2}", "\N{VARIATION SELECTOR-2}", 'Verify "\N{VS2}" eq "\N{VARIATION SELECTOR-2}"');
    is("\N{VS3}", "\N{VARIATION SELECTOR-3}", 'Verify "\N{VS3}" eq "\N{VARIATION SELECTOR-3}"');
    is("\N{VS4}", "\N{VARIATION SELECTOR-4}", 'Verify "\N{VS4}" eq "\N{VARIATION SELECTOR-4}"');
    is("\N{VS5}", "\N{VARIATION SELECTOR-5}", 'Verify "\N{VS5}" eq "\N{VARIATION SELECTOR-5}"');
    is("\N{VS6}", "\N{VARIATION SELECTOR-6}", 'Verify "\N{VS6}" eq "\N{VARIATION SELECTOR-6}"');
    is("\N{VS7}", "\N{VARIATION SELECTOR-7}", 'Verify "\N{VS7}" eq "\N{VARIATION SELECTOR-7}"');
    is("\N{VS8}", "\N{VARIATION SELECTOR-8}", 'Verify "\N{VS8}" eq "\N{VARIATION SELECTOR-8}"');
    is("\N{VS9}", "\N{VARIATION SELECTOR-9}", 'Verify "\N{VS9}" eq "\N{VARIATION SELECTOR-9}"');
    is("\N{VS10}", "\N{VARIATION SELECTOR-10}", 'Verify "\N{VS10}" eq "\N{VARIATION SELECTOR-10}"');
    is("\N{VS11}", "\N{VARIATION SELECTOR-11}", 'Verify "\N{VS11}" eq "\N{VARIATION SELECTOR-11}"');
    is("\N{VS12}", "\N{VARIATION SELECTOR-12}", 'Verify "\N{VS12}" eq "\N{VARIATION SELECTOR-12}"');
    is("\N{VS13}", "\N{VARIATION SELECTOR-13}", 'Verify "\N{VS13}" eq "\N{VARIATION SELECTOR-13}"');
    is("\N{VS14}", "\N{VARIATION SELECTOR-14}", 'Verify "\N{VS14}" eq "\N{VARIATION SELECTOR-14}"');
    is("\N{VS15}", "\N{VARIATION SELECTOR-15}", 'Verify "\N{VS15}" eq "\N{VARIATION SELECTOR-15}"');
    is("\N{VS16}", "\N{VARIATION SELECTOR-16}", 'Verify "\N{VS16}" eq "\N{VARIATION SELECTOR-16}"');
    is("\N{VS17}", "\N{VARIATION SELECTOR-17}", 'Verify "\N{VS17}" eq "\N{VARIATION SELECTOR-17}"');
    is("\N{VS18}", "\N{VARIATION SELECTOR-18}", 'Verify "\N{VS18}" eq "\N{VARIATION SELECTOR-18}"');
    is("\N{VS19}", "\N{VARIATION SELECTOR-19}", 'Verify "\N{VS19}" eq "\N{VARIATION SELECTOR-19}"');
    is("\N{VS20}", "\N{VARIATION SELECTOR-20}", 'Verify "\N{VS20}" eq "\N{VARIATION SELECTOR-20}"');
    is("\N{VS21}", "\N{VARIATION SELECTOR-21}", 'Verify "\N{VS21}" eq "\N{VARIATION SELECTOR-21}"');
    is("\N{VS22}", "\N{VARIATION SELECTOR-22}", 'Verify "\N{VS22}" eq "\N{VARIATION SELECTOR-22}"');
    is("\N{VS23}", "\N{VARIATION SELECTOR-23}", 'Verify "\N{VS23}" eq "\N{VARIATION SELECTOR-23}"');
    is("\N{VS24}", "\N{VARIATION SELECTOR-24}", 'Verify "\N{VS24}" eq "\N{VARIATION SELECTOR-24}"');
    is("\N{VS25}", "\N{VARIATION SELECTOR-25}", 'Verify "\N{VS25}" eq "\N{VARIATION SELECTOR-25}"');
    is("\N{VS26}", "\N{VARIATION SELECTOR-26}", 'Verify "\N{VS26}" eq "\N{VARIATION SELECTOR-26}"');
    is("\N{VS27}", "\N{VARIATION SELECTOR-27}", 'Verify "\N{VS27}" eq "\N{VARIATION SELECTOR-27}"');
    is("\N{VS28}", "\N{VARIATION SELECTOR-28}", 'Verify "\N{VS28}" eq "\N{VARIATION SELECTOR-28}"');
    is("\N{VS29}", "\N{VARIATION SELECTOR-29}", 'Verify "\N{VS29}" eq "\N{VARIATION SELECTOR-29}"');
    is("\N{VS30}", "\N{VARIATION SELECTOR-30}", 'Verify "\N{VS30}" eq "\N{VARIATION SELECTOR-30}"');
    is("\N{VS31}", "\N{VARIATION SELECTOR-31}", 'Verify "\N{VS31}" eq "\N{VARIATION SELECTOR-31}"');
    is("\N{VS32}", "\N{VARIATION SELECTOR-32}", 'Verify "\N{VS32}" eq "\N{VARIATION SELECTOR-32}"');
    is("\N{VS33}", "\N{VARIATION SELECTOR-33}", 'Verify "\N{VS33}" eq "\N{VARIATION SELECTOR-33}"');
    is("\N{VS34}", "\N{VARIATION SELECTOR-34}", 'Verify "\N{VS34}" eq "\N{VARIATION SELECTOR-34}"');
    is("\N{VS35}", "\N{VARIATION SELECTOR-35}", 'Verify "\N{VS35}" eq "\N{VARIATION SELECTOR-35}"');
    is("\N{VS36}", "\N{VARIATION SELECTOR-36}", 'Verify "\N{VS36}" eq "\N{VARIATION SELECTOR-36}"');
    is("\N{VS37}", "\N{VARIATION SELECTOR-37}", 'Verify "\N{VS37}" eq "\N{VARIATION SELECTOR-37}"');
    is("\N{VS38}", "\N{VARIATION SELECTOR-38}", 'Verify "\N{VS38}" eq "\N{VARIATION SELECTOR-38}"');
    is("\N{VS39}", "\N{VARIATION SELECTOR-39}", 'Verify "\N{VS39}" eq "\N{VARIATION SELECTOR-39}"');
    is("\N{VS40}", "\N{VARIATION SELECTOR-40}", 'Verify "\N{VS40}" eq "\N{VARIATION SELECTOR-40}"');
    is("\N{VS41}", "\N{VARIATION SELECTOR-41}", 'Verify "\N{VS41}" eq "\N{VARIATION SELECTOR-41}"');
    is("\N{VS42}", "\N{VARIATION SELECTOR-42}", 'Verify "\N{VS42}" eq "\N{VARIATION SELECTOR-42}"');
    is("\N{VS43}", "\N{VARIATION SELECTOR-43}", 'Verify "\N{VS43}" eq "\N{VARIATION SELECTOR-43}"');
    is("\N{VS44}", "\N{VARIATION SELECTOR-44}", 'Verify "\N{VS44}" eq "\N{VARIATION SELECTOR-44}"');
    is("\N{VS45}", "\N{VARIATION SELECTOR-45}", 'Verify "\N{VS45}" eq "\N{VARIATION SELECTOR-45}"');
    is("\N{VS46}", "\N{VARIATION SELECTOR-46}", 'Verify "\N{VS46}" eq "\N{VARIATION SELECTOR-46}"');
    is("\N{VS47}", "\N{VARIATION SELECTOR-47}", 'Verify "\N{VS47}" eq "\N{VARIATION SELECTOR-47}"');
    is("\N{VS48}", "\N{VARIATION SELECTOR-48}", 'Verify "\N{VS48}" eq "\N{VARIATION SELECTOR-48}"');
    is("\N{VS49}", "\N{VARIATION SELECTOR-49}", 'Verify "\N{VS49}" eq "\N{VARIATION SELECTOR-49}"');
    is("\N{VS50}", "\N{VARIATION SELECTOR-50}", 'Verify "\N{VS50}" eq "\N{VARIATION SELECTOR-50}"');
    is("\N{VS51}", "\N{VARIATION SELECTOR-51}", 'Verify "\N{VS51}" eq "\N{VARIATION SELECTOR-51}"');
    is("\N{VS52}", "\N{VARIATION SELECTOR-52}", 'Verify "\N{VS52}" eq "\N{VARIATION SELECTOR-52}"');
    is("\N{VS53}", "\N{VARIATION SELECTOR-53}", 'Verify "\N{VS53}" eq "\N{VARIATION SELECTOR-53}"');
    is("\N{VS54}", "\N{VARIATION SELECTOR-54}", 'Verify "\N{VS54}" eq "\N{VARIATION SELECTOR-54}"');
    is("\N{VS55}", "\N{VARIATION SELECTOR-55}", 'Verify "\N{VS55}" eq "\N{VARIATION SELECTOR-55}"');
    is("\N{VS56}", "\N{VARIATION SELECTOR-56}", 'Verify "\N{VS56}" eq "\N{VARIATION SELECTOR-56}"');
    is("\N{VS57}", "\N{VARIATION SELECTOR-57}", 'Verify "\N{VS57}" eq "\N{VARIATION SELECTOR-57}"');
    is("\N{VS58}", "\N{VARIATION SELECTOR-58}", 'Verify "\N{VS58}" eq "\N{VARIATION SELECTOR-58}"');
    is("\N{VS59}", "\N{VARIATION SELECTOR-59}", 'Verify "\N{VS59}" eq "\N{VARIATION SELECTOR-59}"');
    is("\N{VS60}", "\N{VARIATION SELECTOR-60}", 'Verify "\N{VS60}" eq "\N{VARIATION SELECTOR-60}"');
    is("\N{VS61}", "\N{VARIATION SELECTOR-61}", 'Verify "\N{VS61}" eq "\N{VARIATION SELECTOR-61}"');
    is("\N{VS62}", "\N{VARIATION SELECTOR-62}", 'Verify "\N{VS62}" eq "\N{VARIATION SELECTOR-62}"');
    is("\N{VS63}", "\N{VARIATION SELECTOR-63}", 'Verify "\N{VS63}" eq "\N{VARIATION SELECTOR-63}"');
    is("\N{VS64}", "\N{VARIATION SELECTOR-64}", 'Verify "\N{VS64}" eq "\N{VARIATION SELECTOR-64}"');
    is("\N{VS65}", "\N{VARIATION SELECTOR-65}", 'Verify "\N{VS65}" eq "\N{VARIATION SELECTOR-65}"');
    is("\N{VS66}", "\N{VARIATION SELECTOR-66}", 'Verify "\N{VS66}" eq "\N{VARIATION SELECTOR-66}"');
    is("\N{VS67}", "\N{VARIATION SELECTOR-67}", 'Verify "\N{VS67}" eq "\N{VARIATION SELECTOR-67}"');
    is("\N{VS68}", "\N{VARIATION SELECTOR-68}", 'Verify "\N{VS68}" eq "\N{VARIATION SELECTOR-68}"');
    is("\N{VS69}", "\N{VARIATION SELECTOR-69}", 'Verify "\N{VS69}" eq "\N{VARIATION SELECTOR-69}"');
    is("\N{VS70}", "\N{VARIATION SELECTOR-70}", 'Verify "\N{VS70}" eq "\N{VARIATION SELECTOR-70}"');
    is("\N{VS71}", "\N{VARIATION SELECTOR-71}", 'Verify "\N{VS71}" eq "\N{VARIATION SELECTOR-71}"');
    is("\N{VS72}", "\N{VARIATION SELECTOR-72}", 'Verify "\N{VS72}" eq "\N{VARIATION SELECTOR-72}"');
    is("\N{VS73}", "\N{VARIATION SELECTOR-73}", 'Verify "\N{VS73}" eq "\N{VARIATION SELECTOR-73}"');
    is("\N{VS74}", "\N{VARIATION SELECTOR-74}", 'Verify "\N{VS74}" eq "\N{VARIATION SELECTOR-74}"');
    is("\N{VS75}", "\N{VARIATION SELECTOR-75}", 'Verify "\N{VS75}" eq "\N{VARIATION SELECTOR-75}"');
    is("\N{VS76}", "\N{VARIATION SELECTOR-76}", 'Verify "\N{VS76}" eq "\N{VARIATION SELECTOR-76}"');
    is("\N{VS77}", "\N{VARIATION SELECTOR-77}", 'Verify "\N{VS77}" eq "\N{VARIATION SELECTOR-77}"');
    is("\N{VS78}", "\N{VARIATION SELECTOR-78}", 'Verify "\N{VS78}" eq "\N{VARIATION SELECTOR-78}"');
    is("\N{VS79}", "\N{VARIATION SELECTOR-79}", 'Verify "\N{VS79}" eq "\N{VARIATION SELECTOR-79}"');
    is("\N{VS80}", "\N{VARIATION SELECTOR-80}", 'Verify "\N{VS80}" eq "\N{VARIATION SELECTOR-80}"');
    is("\N{VS81}", "\N{VARIATION SELECTOR-81}", 'Verify "\N{VS81}" eq "\N{VARIATION SELECTOR-81}"');
    is("\N{VS82}", "\N{VARIATION SELECTOR-82}", 'Verify "\N{VS82}" eq "\N{VARIATION SELECTOR-82}"');
    is("\N{VS83}", "\N{VARIATION SELECTOR-83}", 'Verify "\N{VS83}" eq "\N{VARIATION SELECTOR-83}"');
    is("\N{VS84}", "\N{VARIATION SELECTOR-84}", 'Verify "\N{VS84}" eq "\N{VARIATION SELECTOR-84}"');
    is("\N{VS85}", "\N{VARIATION SELECTOR-85}", 'Verify "\N{VS85}" eq "\N{VARIATION SELECTOR-85}"');
    is("\N{VS86}", "\N{VARIATION SELECTOR-86}", 'Verify "\N{VS86}" eq "\N{VARIATION SELECTOR-86}"');
    is("\N{VS87}", "\N{VARIATION SELECTOR-87}", 'Verify "\N{VS87}" eq "\N{VARIATION SELECTOR-87}"');
    is("\N{VS88}", "\N{VARIATION SELECTOR-88}", 'Verify "\N{VS88}" eq "\N{VARIATION SELECTOR-88}"');
    is("\N{VS89}", "\N{VARIATION SELECTOR-89}", 'Verify "\N{VS89}" eq "\N{VARIATION SELECTOR-89}"');
    is("\N{VS90}", "\N{VARIATION SELECTOR-90}", 'Verify "\N{VS90}" eq "\N{VARIATION SELECTOR-90}"');
    is("\N{VS91}", "\N{VARIATION SELECTOR-91}", 'Verify "\N{VS91}" eq "\N{VARIATION SELECTOR-91}"');
    is("\N{VS92}", "\N{VARIATION SELECTOR-92}", 'Verify "\N{VS92}" eq "\N{VARIATION SELECTOR-92}"');
    is("\N{VS93}", "\N{VARIATION SELECTOR-93}", 'Verify "\N{VS93}" eq "\N{VARIATION SELECTOR-93}"');
    is("\N{VS94}", "\N{VARIATION SELECTOR-94}", 'Verify "\N{VS94}" eq "\N{VARIATION SELECTOR-94}"');
    is("\N{VS95}", "\N{VARIATION SELECTOR-95}", 'Verify "\N{VS95}" eq "\N{VARIATION SELECTOR-95}"');
    is("\N{VS96}", "\N{VARIATION SELECTOR-96}", 'Verify "\N{VS96}" eq "\N{VARIATION SELECTOR-96}"');
    is("\N{VS97}", "\N{VARIATION SELECTOR-97}", 'Verify "\N{VS97}" eq "\N{VARIATION SELECTOR-97}"');
    is("\N{VS98}", "\N{VARIATION SELECTOR-98}", 'Verify "\N{VS98}" eq "\N{VARIATION SELECTOR-98}"');
    is("\N{VS99}", "\N{VARIATION SELECTOR-99}", 'Verify "\N{VS99}" eq "\N{VARIATION SELECTOR-99}"');
    is("\N{VS100}", "\N{VARIATION SELECTOR-100}", 'Verify "\N{VS100}" eq "\N{VARIATION SELECTOR-100}"');
    is("\N{VS101}", "\N{VARIATION SELECTOR-101}", 'Verify "\N{VS101}" eq "\N{VARIATION SELECTOR-101}"');
    is("\N{VS102}", "\N{VARIATION SELECTOR-102}", 'Verify "\N{VS102}" eq "\N{VARIATION SELECTOR-102}"');
    is("\N{VS103}", "\N{VARIATION SELECTOR-103}", 'Verify "\N{VS103}" eq "\N{VARIATION SELECTOR-103}"');
    is("\N{VS104}", "\N{VARIATION SELECTOR-104}", 'Verify "\N{VS104}" eq "\N{VARIATION SELECTOR-104}"');
    is("\N{VS105}", "\N{VARIATION SELECTOR-105}", 'Verify "\N{VS105}" eq "\N{VARIATION SELECTOR-105}"');
    is("\N{VS106}", "\N{VARIATION SELECTOR-106}", 'Verify "\N{VS106}" eq "\N{VARIATION SELECTOR-106}"');
    is("\N{VS107}", "\N{VARIATION SELECTOR-107}", 'Verify "\N{VS107}" eq "\N{VARIATION SELECTOR-107}"');
    is("\N{VS108}", "\N{VARIATION SELECTOR-108}", 'Verify "\N{VS108}" eq "\N{VARIATION SELECTOR-108}"');
    is("\N{VS109}", "\N{VARIATION SELECTOR-109}", 'Verify "\N{VS109}" eq "\N{VARIATION SELECTOR-109}"');
    is("\N{VS110}", "\N{VARIATION SELECTOR-110}", 'Verify "\N{VS110}" eq "\N{VARIATION SELECTOR-110}"');
    is("\N{VS111}", "\N{VARIATION SELECTOR-111}", 'Verify "\N{VS111}" eq "\N{VARIATION SELECTOR-111}"');
    is("\N{VS112}", "\N{VARIATION SELECTOR-112}", 'Verify "\N{VS112}" eq "\N{VARIATION SELECTOR-112}"');
    is("\N{VS113}", "\N{VARIATION SELECTOR-113}", 'Verify "\N{VS113}" eq "\N{VARIATION SELECTOR-113}"');
    is("\N{VS114}", "\N{VARIATION SELECTOR-114}", 'Verify "\N{VS114}" eq "\N{VARIATION SELECTOR-114}"');
    is("\N{VS115}", "\N{VARIATION SELECTOR-115}", 'Verify "\N{VS115}" eq "\N{VARIATION SELECTOR-115}"');
    is("\N{VS116}", "\N{VARIATION SELECTOR-116}", 'Verify "\N{VS116}" eq "\N{VARIATION SELECTOR-116}"');
    is("\N{VS117}", "\N{VARIATION SELECTOR-117}", 'Verify "\N{VS117}" eq "\N{VARIATION SELECTOR-117}"');
    is("\N{VS118}", "\N{VARIATION SELECTOR-118}", 'Verify "\N{VS118}" eq "\N{VARIATION SELECTOR-118}"');
    is("\N{VS119}", "\N{VARIATION SELECTOR-119}", 'Verify "\N{VS119}" eq "\N{VARIATION SELECTOR-119}"');
    is("\N{VS120}", "\N{VARIATION SELECTOR-120}", 'Verify "\N{VS120}" eq "\N{VARIATION SELECTOR-120}"');
    is("\N{VS121}", "\N{VARIATION SELECTOR-121}", 'Verify "\N{VS121}" eq "\N{VARIATION SELECTOR-121}"');
    is("\N{VS122}", "\N{VARIATION SELECTOR-122}", 'Verify "\N{VS122}" eq "\N{VARIATION SELECTOR-122}"');
    is("\N{VS123}", "\N{VARIATION SELECTOR-123}", 'Verify "\N{VS123}" eq "\N{VARIATION SELECTOR-123}"');
    is("\N{VS124}", "\N{VARIATION SELECTOR-124}", 'Verify "\N{VS124}" eq "\N{VARIATION SELECTOR-124}"');
    is("\N{VS125}", "\N{VARIATION SELECTOR-125}", 'Verify "\N{VS125}" eq "\N{VARIATION SELECTOR-125}"');
    is("\N{VS126}", "\N{VARIATION SELECTOR-126}", 'Verify "\N{VS126}" eq "\N{VARIATION SELECTOR-126}"');
    is("\N{VS127}", "\N{VARIATION SELECTOR-127}", 'Verify "\N{VS127}" eq "\N{VARIATION SELECTOR-127}"');
    is("\N{VS128}", "\N{VARIATION SELECTOR-128}", 'Verify "\N{VS128}" eq "\N{VARIATION SELECTOR-128}"');
    is("\N{VS129}", "\N{VARIATION SELECTOR-129}", 'Verify "\N{VS129}" eq "\N{VARIATION SELECTOR-129}"');
    is("\N{VS130}", "\N{VARIATION SELECTOR-130}", 'Verify "\N{VS130}" eq "\N{VARIATION SELECTOR-130}"');
    is("\N{VS131}", "\N{VARIATION SELECTOR-131}", 'Verify "\N{VS131}" eq "\N{VARIATION SELECTOR-131}"');
    is("\N{VS132}", "\N{VARIATION SELECTOR-132}", 'Verify "\N{VS132}" eq "\N{VARIATION SELECTOR-132}"');
    is("\N{VS133}", "\N{VARIATION SELECTOR-133}", 'Verify "\N{VS133}" eq "\N{VARIATION SELECTOR-133}"');
    is("\N{VS134}", "\N{VARIATION SELECTOR-134}", 'Verify "\N{VS134}" eq "\N{VARIATION SELECTOR-134}"');
    is("\N{VS135}", "\N{VARIATION SELECTOR-135}", 'Verify "\N{VS135}" eq "\N{VARIATION SELECTOR-135}"');
    is("\N{VS136}", "\N{VARIATION SELECTOR-136}", 'Verify "\N{VS136}" eq "\N{VARIATION SELECTOR-136}"');
    is("\N{VS137}", "\N{VARIATION SELECTOR-137}", 'Verify "\N{VS137}" eq "\N{VARIATION SELECTOR-137}"');
    is("\N{VS138}", "\N{VARIATION SELECTOR-138}", 'Verify "\N{VS138}" eq "\N{VARIATION SELECTOR-138}"');
    is("\N{VS139}", "\N{VARIATION SELECTOR-139}", 'Verify "\N{VS139}" eq "\N{VARIATION SELECTOR-139}"');
    is("\N{VS140}", "\N{VARIATION SELECTOR-140}", 'Verify "\N{VS140}" eq "\N{VARIATION SELECTOR-140}"');
    is("\N{VS141}", "\N{VARIATION SELECTOR-141}", 'Verify "\N{VS141}" eq "\N{VARIATION SELECTOR-141}"');
    is("\N{VS142}", "\N{VARIATION SELECTOR-142}", 'Verify "\N{VS142}" eq "\N{VARIATION SELECTOR-142}"');
    is("\N{VS143}", "\N{VARIATION SELECTOR-143}", 'Verify "\N{VS143}" eq "\N{VARIATION SELECTOR-143}"');
    is("\N{VS144}", "\N{VARIATION SELECTOR-144}", 'Verify "\N{VS144}" eq "\N{VARIATION SELECTOR-144}"');
    is("\N{VS145}", "\N{VARIATION SELECTOR-145}", 'Verify "\N{VS145}" eq "\N{VARIATION SELECTOR-145}"');
    is("\N{VS146}", "\N{VARIATION SELECTOR-146}", 'Verify "\N{VS146}" eq "\N{VARIATION SELECTOR-146}"');
    is("\N{VS147}", "\N{VARIATION SELECTOR-147}", 'Verify "\N{VS147}" eq "\N{VARIATION SELECTOR-147}"');
    is("\N{VS148}", "\N{VARIATION SELECTOR-148}", 'Verify "\N{VS148}" eq "\N{VARIATION SELECTOR-148}"');
    is("\N{VS149}", "\N{VARIATION SELECTOR-149}", 'Verify "\N{VS149}" eq "\N{VARIATION SELECTOR-149}"');
    is("\N{VS150}", "\N{VARIATION SELECTOR-150}", 'Verify "\N{VS150}" eq "\N{VARIATION SELECTOR-150}"');
    is("\N{VS151}", "\N{VARIATION SELECTOR-151}", 'Verify "\N{VS151}" eq "\N{VARIATION SELECTOR-151}"');
    is("\N{VS152}", "\N{VARIATION SELECTOR-152}", 'Verify "\N{VS152}" eq "\N{VARIATION SELECTOR-152}"');
    is("\N{VS153}", "\N{VARIATION SELECTOR-153}", 'Verify "\N{VS153}" eq "\N{VARIATION SELECTOR-153}"');
    is("\N{VS154}", "\N{VARIATION SELECTOR-154}", 'Verify "\N{VS154}" eq "\N{VARIATION SELECTOR-154}"');
    is("\N{VS155}", "\N{VARIATION SELECTOR-155}", 'Verify "\N{VS155}" eq "\N{VARIATION SELECTOR-155}"');
    is("\N{VS156}", "\N{VARIATION SELECTOR-156}", 'Verify "\N{VS156}" eq "\N{VARIATION SELECTOR-156}"');
    is("\N{VS157}", "\N{VARIATION SELECTOR-157}", 'Verify "\N{VS157}" eq "\N{VARIATION SELECTOR-157}"');
    is("\N{VS158}", "\N{VARIATION SELECTOR-158}", 'Verify "\N{VS158}" eq "\N{VARIATION SELECTOR-158}"');
    is("\N{VS159}", "\N{VARIATION SELECTOR-159}", 'Verify "\N{VS159}" eq "\N{VARIATION SELECTOR-159}"');
    is("\N{VS160}", "\N{VARIATION SELECTOR-160}", 'Verify "\N{VS160}" eq "\N{VARIATION SELECTOR-160}"');
    is("\N{VS161}", "\N{VARIATION SELECTOR-161}", 'Verify "\N{VS161}" eq "\N{VARIATION SELECTOR-161}"');
    is("\N{VS162}", "\N{VARIATION SELECTOR-162}", 'Verify "\N{VS162}" eq "\N{VARIATION SELECTOR-162}"');
    is("\N{VS163}", "\N{VARIATION SELECTOR-163}", 'Verify "\N{VS163}" eq "\N{VARIATION SELECTOR-163}"');
    is("\N{VS164}", "\N{VARIATION SELECTOR-164}", 'Verify "\N{VS164}" eq "\N{VARIATION SELECTOR-164}"');
    is("\N{VS165}", "\N{VARIATION SELECTOR-165}", 'Verify "\N{VS165}" eq "\N{VARIATION SELECTOR-165}"');
    is("\N{VS166}", "\N{VARIATION SELECTOR-166}", 'Verify "\N{VS166}" eq "\N{VARIATION SELECTOR-166}"');
    is("\N{VS167}", "\N{VARIATION SELECTOR-167}", 'Verify "\N{VS167}" eq "\N{VARIATION SELECTOR-167}"');
    is("\N{VS168}", "\N{VARIATION SELECTOR-168}", 'Verify "\N{VS168}" eq "\N{VARIATION SELECTOR-168}"');
    is("\N{VS169}", "\N{VARIATION SELECTOR-169}", 'Verify "\N{VS169}" eq "\N{VARIATION SELECTOR-169}"');
    is("\N{VS170}", "\N{VARIATION SELECTOR-170}", 'Verify "\N{VS170}" eq "\N{VARIATION SELECTOR-170}"');
    is("\N{VS171}", "\N{VARIATION SELECTOR-171}", 'Verify "\N{VS171}" eq "\N{VARIATION SELECTOR-171}"');
    is("\N{VS172}", "\N{VARIATION SELECTOR-172}", 'Verify "\N{VS172}" eq "\N{VARIATION SELECTOR-172}"');
    is("\N{VS173}", "\N{VARIATION SELECTOR-173}", 'Verify "\N{VS173}" eq "\N{VARIATION SELECTOR-173}"');
    is("\N{VS174}", "\N{VARIATION SELECTOR-174}", 'Verify "\N{VS174}" eq "\N{VARIATION SELECTOR-174}"');
    is("\N{VS175}", "\N{VARIATION SELECTOR-175}", 'Verify "\N{VS175}" eq "\N{VARIATION SELECTOR-175}"');
    is("\N{VS176}", "\N{VARIATION SELECTOR-176}", 'Verify "\N{VS176}" eq "\N{VARIATION SELECTOR-176}"');
    is("\N{VS177}", "\N{VARIATION SELECTOR-177}", 'Verify "\N{VS177}" eq "\N{VARIATION SELECTOR-177}"');
    is("\N{VS178}", "\N{VARIATION SELECTOR-178}", 'Verify "\N{VS178}" eq "\N{VARIATION SELECTOR-178}"');
    is("\N{VS179}", "\N{VARIATION SELECTOR-179}", 'Verify "\N{VS179}" eq "\N{VARIATION SELECTOR-179}"');
    is("\N{VS180}", "\N{VARIATION SELECTOR-180}", 'Verify "\N{VS180}" eq "\N{VARIATION SELECTOR-180}"');
    is("\N{VS181}", "\N{VARIATION SELECTOR-181}", 'Verify "\N{VS181}" eq "\N{VARIATION SELECTOR-181}"');
    is("\N{VS182}", "\N{VARIATION SELECTOR-182}", 'Verify "\N{VS182}" eq "\N{VARIATION SELECTOR-182}"');
    is("\N{VS183}", "\N{VARIATION SELECTOR-183}", 'Verify "\N{VS183}" eq "\N{VARIATION SELECTOR-183}"');
    is("\N{VS184}", "\N{VARIATION SELECTOR-184}", 'Verify "\N{VS184}" eq "\N{VARIATION SELECTOR-184}"');
    is("\N{VS185}", "\N{VARIATION SELECTOR-185}", 'Verify "\N{VS185}" eq "\N{VARIATION SELECTOR-185}"');
    is("\N{VS186}", "\N{VARIATION SELECTOR-186}", 'Verify "\N{VS186}" eq "\N{VARIATION SELECTOR-186}"');
    is("\N{VS187}", "\N{VARIATION SELECTOR-187}", 'Verify "\N{VS187}" eq "\N{VARIATION SELECTOR-187}"');
    is("\N{VS188}", "\N{VARIATION SELECTOR-188}", 'Verify "\N{VS188}" eq "\N{VARIATION SELECTOR-188}"');
    is("\N{VS189}", "\N{VARIATION SELECTOR-189}", 'Verify "\N{VS189}" eq "\N{VARIATION SELECTOR-189}"');
    is("\N{VS190}", "\N{VARIATION SELECTOR-190}", 'Verify "\N{VS190}" eq "\N{VARIATION SELECTOR-190}"');
    is("\N{VS191}", "\N{VARIATION SELECTOR-191}", 'Verify "\N{VS191}" eq "\N{VARIATION SELECTOR-191}"');
    is("\N{VS192}", "\N{VARIATION SELECTOR-192}", 'Verify "\N{VS192}" eq "\N{VARIATION SELECTOR-192}"');
    is("\N{VS193}", "\N{VARIATION SELECTOR-193}", 'Verify "\N{VS193}" eq "\N{VARIATION SELECTOR-193}"');
    is("\N{VS194}", "\N{VARIATION SELECTOR-194}", 'Verify "\N{VS194}" eq "\N{VARIATION SELECTOR-194}"');
    is("\N{VS195}", "\N{VARIATION SELECTOR-195}", 'Verify "\N{VS195}" eq "\N{VARIATION SELECTOR-195}"');
    is("\N{VS196}", "\N{VARIATION SELECTOR-196}", 'Verify "\N{VS196}" eq "\N{VARIATION SELECTOR-196}"');
    is("\N{VS197}", "\N{VARIATION SELECTOR-197}", 'Verify "\N{VS197}" eq "\N{VARIATION SELECTOR-197}"');
    is("\N{VS198}", "\N{VARIATION SELECTOR-198}", 'Verify "\N{VS198}" eq "\N{VARIATION SELECTOR-198}"');
    is("\N{VS199}", "\N{VARIATION SELECTOR-199}", 'Verify "\N{VS199}" eq "\N{VARIATION SELECTOR-199}"');
    is("\N{VS200}", "\N{VARIATION SELECTOR-200}", 'Verify "\N{VS200}" eq "\N{VARIATION SELECTOR-200}"');
    is("\N{VS201}", "\N{VARIATION SELECTOR-201}", 'Verify "\N{VS201}" eq "\N{VARIATION SELECTOR-201}"');
    is("\N{VS202}", "\N{VARIATION SELECTOR-202}", 'Verify "\N{VS202}" eq "\N{VARIATION SELECTOR-202}"');
    is("\N{VS203}", "\N{VARIATION SELECTOR-203}", 'Verify "\N{VS203}" eq "\N{VARIATION SELECTOR-203}"');
    is("\N{VS204}", "\N{VARIATION SELECTOR-204}", 'Verify "\N{VS204}" eq "\N{VARIATION SELECTOR-204}"');
    is("\N{VS205}", "\N{VARIATION SELECTOR-205}", 'Verify "\N{VS205}" eq "\N{VARIATION SELECTOR-205}"');
    is("\N{VS206}", "\N{VARIATION SELECTOR-206}", 'Verify "\N{VS206}" eq "\N{VARIATION SELECTOR-206}"');
    is("\N{VS207}", "\N{VARIATION SELECTOR-207}", 'Verify "\N{VS207}" eq "\N{VARIATION SELECTOR-207}"');
    is("\N{VS208}", "\N{VARIATION SELECTOR-208}", 'Verify "\N{VS208}" eq "\N{VARIATION SELECTOR-208}"');
    is("\N{VS209}", "\N{VARIATION SELECTOR-209}", 'Verify "\N{VS209}" eq "\N{VARIATION SELECTOR-209}"');
    is("\N{VS210}", "\N{VARIATION SELECTOR-210}", 'Verify "\N{VS210}" eq "\N{VARIATION SELECTOR-210}"');
    is("\N{VS211}", "\N{VARIATION SELECTOR-211}", 'Verify "\N{VS211}" eq "\N{VARIATION SELECTOR-211}"');
    is("\N{VS212}", "\N{VARIATION SELECTOR-212}", 'Verify "\N{VS212}" eq "\N{VARIATION SELECTOR-212}"');
    is("\N{VS213}", "\N{VARIATION SELECTOR-213}", 'Verify "\N{VS213}" eq "\N{VARIATION SELECTOR-213}"');
    is("\N{VS214}", "\N{VARIATION SELECTOR-214}", 'Verify "\N{VS214}" eq "\N{VARIATION SELECTOR-214}"');
    is("\N{VS215}", "\N{VARIATION SELECTOR-215}", 'Verify "\N{VS215}" eq "\N{VARIATION SELECTOR-215}"');
    is("\N{VS216}", "\N{VARIATION SELECTOR-216}", 'Verify "\N{VS216}" eq "\N{VARIATION SELECTOR-216}"');
    is("\N{VS217}", "\N{VARIATION SELECTOR-217}", 'Verify "\N{VS217}" eq "\N{VARIATION SELECTOR-217}"');
    is("\N{VS218}", "\N{VARIATION SELECTOR-218}", 'Verify "\N{VS218}" eq "\N{VARIATION SELECTOR-218}"');
    is("\N{VS219}", "\N{VARIATION SELECTOR-219}", 'Verify "\N{VS219}" eq "\N{VARIATION SELECTOR-219}"');
    is("\N{VS220}", "\N{VARIATION SELECTOR-220}", 'Verify "\N{VS220}" eq "\N{VARIATION SELECTOR-220}"');
    is("\N{VS221}", "\N{VARIATION SELECTOR-221}", 'Verify "\N{VS221}" eq "\N{VARIATION SELECTOR-221}"');
    is("\N{VS222}", "\N{VARIATION SELECTOR-222}", 'Verify "\N{VS222}" eq "\N{VARIATION SELECTOR-222}"');
    is("\N{VS223}", "\N{VARIATION SELECTOR-223}", 'Verify "\N{VS223}" eq "\N{VARIATION SELECTOR-223}"');
    is("\N{VS224}", "\N{VARIATION SELECTOR-224}", 'Verify "\N{VS224}" eq "\N{VARIATION SELECTOR-224}"');
    is("\N{VS225}", "\N{VARIATION SELECTOR-225}", 'Verify "\N{VS225}" eq "\N{VARIATION SELECTOR-225}"');
    is("\N{VS226}", "\N{VARIATION SELECTOR-226}", 'Verify "\N{VS226}" eq "\N{VARIATION SELECTOR-226}"');
    is("\N{VS227}", "\N{VARIATION SELECTOR-227}", 'Verify "\N{VS227}" eq "\N{VARIATION SELECTOR-227}"');
    is("\N{VS228}", "\N{VARIATION SELECTOR-228}", 'Verify "\N{VS228}" eq "\N{VARIATION SELECTOR-228}"');
    is("\N{VS229}", "\N{VARIATION SELECTOR-229}", 'Verify "\N{VS229}" eq "\N{VARIATION SELECTOR-229}"');
    is("\N{VS230}", "\N{VARIATION SELECTOR-230}", 'Verify "\N{VS230}" eq "\N{VARIATION SELECTOR-230}"');
    is("\N{VS231}", "\N{VARIATION SELECTOR-231}", 'Verify "\N{VS231}" eq "\N{VARIATION SELECTOR-231}"');
    is("\N{VS232}", "\N{VARIATION SELECTOR-232}", 'Verify "\N{VS232}" eq "\N{VARIATION SELECTOR-232}"');
    is("\N{VS233}", "\N{VARIATION SELECTOR-233}", 'Verify "\N{VS233}" eq "\N{VARIATION SELECTOR-233}"');
    is("\N{VS234}", "\N{VARIATION SELECTOR-234}", 'Verify "\N{VS234}" eq "\N{VARIATION SELECTOR-234}"');
    is("\N{VS235}", "\N{VARIATION SELECTOR-235}", 'Verify "\N{VS235}" eq "\N{VARIATION SELECTOR-235}"');
    is("\N{VS236}", "\N{VARIATION SELECTOR-236}", 'Verify "\N{VS236}" eq "\N{VARIATION SELECTOR-236}"');
    is("\N{VS237}", "\N{VARIATION SELECTOR-237}", 'Verify "\N{VS237}" eq "\N{VARIATION SELECTOR-237}"');
    is("\N{VS238}", "\N{VARIATION SELECTOR-238}", 'Verify "\N{VS238}" eq "\N{VARIATION SELECTOR-238}"');
    is("\N{VS239}", "\N{VARIATION SELECTOR-239}", 'Verify "\N{VS239}" eq "\N{VARIATION SELECTOR-239}"');
    is("\N{VS240}", "\N{VARIATION SELECTOR-240}", 'Verify "\N{VS240}" eq "\N{VARIATION SELECTOR-240}"');
    is("\N{VS241}", "\N{VARIATION SELECTOR-241}", 'Verify "\N{VS241}" eq "\N{VARIATION SELECTOR-241}"');
    is("\N{VS242}", "\N{VARIATION SELECTOR-242}", 'Verify "\N{VS242}" eq "\N{VARIATION SELECTOR-242}"');
    is("\N{VS243}", "\N{VARIATION SELECTOR-243}", 'Verify "\N{VS243}" eq "\N{VARIATION SELECTOR-243}"');
    is("\N{VS244}", "\N{VARIATION SELECTOR-244}", 'Verify "\N{VS244}" eq "\N{VARIATION SELECTOR-244}"');
    is("\N{VS245}", "\N{VARIATION SELECTOR-245}", 'Verify "\N{VS245}" eq "\N{VARIATION SELECTOR-245}"');
    is("\N{VS246}", "\N{VARIATION SELECTOR-246}", 'Verify "\N{VS246}" eq "\N{VARIATION SELECTOR-246}"');
    is("\N{VS247}", "\N{VARIATION SELECTOR-247}", 'Verify "\N{VS247}" eq "\N{VARIATION SELECTOR-247}"');
    is("\N{VS248}", "\N{VARIATION SELECTOR-248}", 'Verify "\N{VS248}" eq "\N{VARIATION SELECTOR-248}"');
    is("\N{VS249}", "\N{VARIATION SELECTOR-249}", 'Verify "\N{VS249}" eq "\N{VARIATION SELECTOR-249}"');
    is("\N{VS250}", "\N{VARIATION SELECTOR-250}", 'Verify "\N{VS250}" eq "\N{VARIATION SELECTOR-250}"');
    is("\N{VS251}", "\N{VARIATION SELECTOR-251}", 'Verify "\N{VS251}" eq "\N{VARIATION SELECTOR-251}"');
    is("\N{VS252}", "\N{VARIATION SELECTOR-252}", 'Verify "\N{VS252}" eq "\N{VARIATION SELECTOR-252}"');
    is("\N{VS253}", "\N{VARIATION SELECTOR-253}", 'Verify "\N{VS253}" eq "\N{VARIATION SELECTOR-253}"');
    is("\N{VS254}", "\N{VARIATION SELECTOR-254}", 'Verify "\N{VS254}" eq "\N{VARIATION SELECTOR-254}"');
    is("\N{VS255}", "\N{VARIATION SELECTOR-255}", 'Verify "\N{VS255}" eq "\N{VARIATION SELECTOR-255}"');
    is("\N{VS256}", "\N{VARIATION SELECTOR-256}", 'Verify "\N{VS256}" eq "\N{VARIATION SELECTOR-256}"');

    # Test a few of the above with :loose
    use charnames ":loose";
    is("\N{n-e xt l-i ne}", "\N{n-e xt l-i ne (-n-e l-)}", 'Verify "\N{n-e xt l-i ne}" eq "\N{n-e xt l-i ne (-n-e l-)}"');
    is("\N{n e-l}", "\N{n e-xt l i-ne ( n e-l )}", 'Verify "\N{n e-l}" eq "\N{n e-xt l i-ne ( n e-l )}"');
    is("\N{p-a dd-i ng c-h ar-a ct-e r}", "\N{p-a d}", 'Verify "\N{p-a dd-i ng c-h ar-a ct-e r}" eq "\N{p-a d}"');
    is("\N{s i-ng l-e-s h-i f-t 3}", "\N{s i-ng l-e s h-i f-t t h-r e-e}", 'Verify "\N{s i-ng l-e-s h-i f-t 3}" eq "\N{s i-ng l-e s h-i f-t t h-r e-e}"');
    is("\N{vs256}", "\N{v-a ri-a ti-o n s-e le-c t o-r-256}", 'Verify "\N{vs256}" eq "\N{v-a ri-a ti-o n s-e le-c t o-r-256}"');
}

# [perl #30409] charnames.pm clobbers default variable
$_ = 'foobar';
eval "use charnames ':full';";
is($_, 'foobar', 'Verify charnames.pm doesnt clobbers $_');

# Unicode slowdown noted by Phil Pennock, traced to a bug fix in index
# SADAHIRO Tomoyuki's suggestion is to ensure that the UTF-8ness of both
# arguments are identical before calling index.
# To do this can take advantage of the fact that unicore/Name.pl is 7 bit
# (or at least should be). So assert that that is true here.  EBCDIC
# may be a problem (khw).

my $names = do "unicore/Name.pl";
ok(defined $names, "Verify can read 'unicore/Name.pl'");
my $non_ascii = native_to_uni($names) =~ tr/\0-\177//c;
ok(! $non_ascii, "Verify all official names are ASCII-only");

# Verify that charnames propagate to eval("")
my $evaltry = eval q[ "Eval: \N{LEFT-POINTING DOUBLE ANGLE QUOTATION MARK}" ];
if ($@) {
    fail('charnames failed to propagate to eval("")');
    fail('next test also fails to make the same number of tests');
} else {
    pass('charnames propagated to eval("")');
    is($evaltry, "Eval: \N{LEFT-POINTING DOUBLE ANGLE QUOTATION MARK}",
       "... and got correct answer");
}

# Verify that db includes the normative NameAliases.txt names
is("\N{U+1D0C5}", "\N{BYZANTINE MUSICAL SYMBOL FTHORA SKLIRON CHROMA VASIS}", 'Verify "\N{U+1D0C5}" eq "\N{BYZANTINE MUSICAL SYMBOL FTHORA SKLIRON CHROMA VASIS}"');

# [perl #73174] use of \N{FOO} used to reset %^H

{
    use charnames ":full";
    my $res;
    BEGIN { $^H{73174} = "foo" }
    BEGIN { $res = ($^H{73174} // "") }
    # forces loading of utf8.pm, which used to reset %^H
    $res .= '-1' if ":" =~ /\N{COLON}/i;
    BEGIN { $res .= '-' . ($^H{73174} // "") }
    $res .= '-' . ($^H{73174} // "");
    $res .= '-2' if ":" =~ /\N{COLON}/;
    $res .= '-3' if ":" =~ /\N{COLON}/i;
    is($res, "foo-foo-1--2-3", "Verify %^H doesn't get reset by \\N{...}");
}

{   use charnames qw(.*);
    ok (! defined charnames::vianame("a"), "Verify that metachars in script names get quoted");
}

{
    # Test scoping.  Outer block sets up some things; inner blocks
    # override them, and then see if get restored.

    use charnames ":full",
                  ":alias" => {
                            mychar1 => "LATIN SMALL LETTER E",
                            mychar2 => "LATIN CAPITAL LETTER A",
                            myprivate1 => 0xE8000,  # Private use area
                            myprivate2 => 0x100000,  # Private use area
                    },
                  ":short",
                  qw( katakana ),
                ;

    my $hiragana_be = "\N{HIRAGANA LETTER BE}";

    is("\N{mychar1}", "e", "Outer block: verify that \\N{mychar1} works");
    is(charnames::vianame("mychar1"), ord("e"), "Outer block: verify that vianame(mychar1) works");
    is(charnames::string_vianame("mychar1"), "e", "Outer block: verify that string_vianame(mychar1) works");
    eval "qr/\\p{name=mychar1}/";
    like($@, qr/Can't find Unicode property definition "name=mychar1"/,
            '\p{name=} returns an appropriate error message on an alias');
    is("\N{mychar2}", "A", "Outer block: verify that \\N{mychar2} works");
    is(charnames::vianame("mychar2"), ord("A"), "Outer block: verify that vianame(mychar2) works");
    is(charnames::string_vianame("mychar2"), "A", "Outer block: verify that string_vianame(mychar2) works");
    eval "qr/\\p{name=mychar2}/";
    like($@, qr/Can't find Unicode property definition "name=mychar2"/,
            '\p{name=} returns an appropriate error message on an alias');
    is("\N{myprivate1}", "\x{E8000}", "Outer block: verify that \\N{myprivate1} works");
    cmp_ok(charnames::vianame("myprivate1"), "==", 0xE8000, "Outer block: verify that vianame(myprivate1) works");
    is(charnames::string_vianame("myprivate1"), chr(0xE8000), "Outer block: verify that string_vianame(myprivate1) works");
    eval "qr/\\p{name=myprivate1}/";
    like($@, qr/Can't find Unicode property definition "name=myprivate1"/,
            '\p{name=} returns an appropriate error message on an alias');
    is(charnames::viacode(0xE8000), "myprivate1", "Outer block: verify that myprivate1 viacode works");
    is("\N{myprivate2}", "\x{100000}", "Outer block: verify that \\N{myprivate2} works");
    cmp_ok(charnames::vianame("myprivate2"), "==", 0x100000, "Outer block: verify that vianame(myprivate2) works");
    is(charnames::string_vianame("myprivate2"), chr(0x100000), "Outer block: verify that string_vianame(myprivate2) works");
    eval "qr/\\p{name=myprivate2}/";
    like($@, qr/Can't find Unicode property definition "name=myprivate2"/,
            '\p{name=} returns an appropriate error message on an alias');
    is(charnames::viacode(0x100000), "myprivate2", "Outer block: verify that myprivate2 viacode works");
    is("\N{BE}", "\N{KATAKANA LETTER BE}", "Outer block: verify that \\N uses the correct script ");
    cmp_ok(charnames::vianame("BE"), "==", ord("\N{KATAKANA LETTER BE}"), "Outer block: verify that vianame uses the correct script");
    cmp_ok(charnames::string_vianame("BE"), "==", "\N{KATAKANA LETTER BE}", "Outer block: verify that string_vianame uses the correct script");
    is("\N{Hiragana: BE}", $hiragana_be, "Outer block: verify that :short works with \\N");
    cmp_ok(charnames::vianame("Hiragana: BE"), "==", ord($hiragana_be), "Outer block: verify that :short works with vianame");
    cmp_ok(charnames::string_vianame("Hiragana: BE"), "==", $hiragana_be, "Outer block: verify that :short works with string_vianame");
    eval "qr/\\p{name=Hiragana: BE}/";
    like($@, qr/Can't find Unicode property definition "name=Hiragana: BE"/,
            '\p{name=} returns an appropriate error message on :short attempt');

    {
        use charnames ":full",
                      ":alias" => {
                                    mychar1 => "LATIN SMALL LETTER F",
                                    myprivate1 => 0xE8001,  # Private use area
                                },

                      # BE is in both hiragana and katakana; see if
                      # different default script delivers different
                      # letter.
                      qw( hiragana ),
            ;
        is("\N{mychar1}", "f", "Inner block: verify that \\N{mychar1} is redefined");
        is(charnames::vianame("mychar1"), ord("f"), "Inner block: verify that vianame(mychar1) is redefined");
        is(charnames::string_vianame("mychar1"), "f", "Inner block: verify that string_vianame(mychar1) is redefined");
        eval '"\N{mychar2}"';
        like($@, qr/Unknown charname 'mychar2'/, "Inner block: verify that \\N{mychar2} outer definition didn't leak");
        ok( ! defined charnames::vianame("mychar2"), "Inner block: verify that vianame(mychar2) outer definition didn't leak");
        ok( ! defined charnames::string_vianame("mychar2"), "Inner block: verify that string_vianame(mychar2) outer definition didn't leak");
        is("\N{myprivate1}", "\x{E8001}", "Inner block: verify that \\N{myprivate1} is redefined ");
        cmp_ok(charnames::vianame("myprivate1"), "==", 0xE8001, "Inner block: verify that vianame(myprivate1) is redefined");
        is(charnames::string_vianame("myprivate1"), chr(0xE8001), "Inner block: verify that string_vianame(myprivate1) is redefined");
        is(charnames::viacode(0xE8001), "myprivate1", "Inner block: verify that myprivate1 viacode is redefined");
        ok(! defined charnames::viacode(0xE8000), "Inner block: verify that outer myprivate1 viacode didn't leak");
        eval '"\N{myprivate2}"';
        like($@, qr/Unknown charname 'myprivate2'/, "Inner block: verify that \\N{myprivate2} outer definition didn't leak");
        ok(! defined charnames::vianame("myprivate2"), "Inner block: verify that vianame(myprivate2) outer definition didn't leak");
        ok(! defined charnames::string_vianame("myprivate2"), "Inner block: verify that string_vianame(myprivate2) outer definition didn't leak");
        ok(! defined charnames::viacode(0x100000), "Inner block: verify that myprivate2 viacode outer definition didn't leak");
        is("\N{BE}", $hiragana_be, "Inner block: verify that \\N uses the correct script");
        cmp_ok(charnames::vianame("BE"), "==", ord($hiragana_be), "Inner block: verify that vianame uses the correct script");
        cmp_ok(charnames::string_vianame("BE"), "==", $hiragana_be, "Inner block: verify that string_vianame uses the correct script");
        eval '"\N{Hiragana: BE}"';
        like($@, qr/Unknown charname 'Hiragana: BE'/, "Inner block without :short: \\N with short doesn't work");
        ok(! defined charnames::vianame("Hiragana: BE"), "Inner block without :short: verify that vianame with short doesn't work");
        ok(! defined charnames::string_vianame("Hiragana: BE"), "Inner block without :short: verify that string_vianame with short doesn't work");

        {   # An inner block where only :short definitions are valid.
            use charnames ":short";
            eval '"\N{mychar1}"';
            like($@, qr/Unknown charname 'mychar1'/, "Inner inner block: verify that mychar1 outer definition didn't leak with \\N");
            ok( ! defined charnames::vianame("mychar1"), "Inner inner block: verify that mychar1 outer definition didn't leak with vianame");
            ok( ! defined charnames::string_vianame("mychar1"), "Inner inner block: verify that mychar1 outer definition didn't leak with string_vianame");
            eval '"\N{mychar2}"';
            like($@, qr/Unknown charname 'mychar2'/, "Inner inner block: verify that mychar2 outer definition didn't leak with \\N");
            ok( ! defined charnames::vianame("mychar2"), "Inner inner block: verify that mychar2 outer definition didn't leak with vianame");
            ok( ! defined charnames::string_vianame("mychar2"), "Inner inner block: verify that mychar2 outer definition didn't leak with string_vianame");
            eval '"\N{myprivate1}"';
            like($@, qr/Unknown charname 'myprivate1'/, "Inner inner block: verify that myprivate1 outer definition didn't leak with \\N");
            ok(! defined charnames::vianame("myprivate1"), "Inner inner block: verify that myprivate1 outer definition didn't leak with vianame");
            ok(! defined charnames::string_vianame("myprivate1"), "Inner inner block: verify that myprivate1 outer definition didn't leak with string_vianame");
            eval '"\N{myprivate2}"';
            like($@, qr/Unknown charname 'myprivate2'/, "Inner inner block: verify that myprivate2 outer definition didn't leak with \\N");
            ok(! defined charnames::vianame("myprivate2"), "Inner inner block: verify that myprivate2 outer definition didn't leak with vianame");
            ok(! defined charnames::string_vianame("myprivate2"), "Inner inner block: verify that myprivate2 outer definition didn't leak with string_vianame");
            ok(! defined charnames::viacode(0xE8000), "Inner inner block: verify that mychar1 outer outer definition didn't leak with viacode");
            ok(! defined charnames::viacode(0xE8001), "Inner inner block: verify that mychar1 outer definition didn't leak with viacode");
            ok(! defined charnames::viacode(0x100000), "Inner inner block: verify that mychar2 outer definition didn't leak with viacode");
            eval '"\N{BE}"';
            like($@, qr/Unknown charname 'BE'/, "Inner inner block without script: verify that outer :script didn't leak with \\N");
            ok(! defined charnames::vianame("BE"), "Inner inner block without script: verify that outer :script didn't leak with vianames");
            ok(! defined charnames::string_vianame("BE"), "Inner inner block without script: verify that outer :script didn't leak with string_vianames");
            eval '"\N{HIRAGANA LETTER BE}"';
            like($@, qr/Unknown charname 'HIRAGANA LETTER BE'/, "Inner inner block without :full: verify that outer :full didn't leak with \\N");
            is("\N{Hiragana: BE}", $hiragana_be, "Inner inner block with :short: verify that \\N works with :short");
            cmp_ok(charnames::vianame("Hiragana: BE"), "==", ord($hiragana_be), "Inner inner block with :short: verify that vianame works with :short");
            cmp_ok(charnames::string_vianame("Hiragana: BE"), "==", $hiragana_be, "Inner inner block with :short: verify that string_vianame works with :short");
        }

        # Back to previous block.  All previous tests should work again.
        is("\N{mychar1}", "f", "Inner block: verify that \\N{mychar1} is redefined");
        is(charnames::vianame("mychar1"), ord("f"), "Inner block: verify that vianame(mychar1) is redefined");
        is(charnames::string_vianame("mychar1"), "f", "Inner block: verify that string_vianame(mychar1) is redefined");
        eval '"\N{mychar2}"';
        like($@, qr/Unknown charname 'mychar2'/, "Inner block: verify that \\N{mychar2} outer definition didn't leak");
        ok( ! defined charnames::vianame("mychar2"), "Inner block: verify that vianame(mychar2) outer definition didn't leak");
        ok( ! defined charnames::string_vianame("mychar2"), "Inner block: verify that string_vianame(mychar2) outer definition didn't leak");
        is("\N{myprivate1}", "\x{E8001}", "Inner block: verify that \\N{myprivate1} is redefined ");
        cmp_ok(charnames::vianame("myprivate1"), "==", 0xE8001, "Inner block: verify that vianame(myprivate1) is redefined");
        is(charnames::string_vianame("myprivate1"), chr(0xE8001), "Inner block: verify that string_vianame(myprivate1) is redefined");
        is(charnames::viacode(0xE8001), "myprivate1", "Inner block: verify that myprivate1 viacode is redefined");
        ok(! defined charnames::viacode(0xE8000), "Inner block: verify that outer myprivate1 viacode didn't leak");
        eval '"\N{myprivate2}"';
        like($@, qr/Unknown charname 'myprivate2'/, "Inner block: verify that \\N{myprivate2} outer definition didn't leak");
        ok(! defined charnames::vianame("myprivate2"), "Inner block: verify that vianame(myprivate2) outer definition didn't leak");
        ok(! defined charnames::string_vianame("myprivate2"), "Inner block: verify that string_vianame(myprivate2) outer definition didn't leak");
        ok(! defined charnames::viacode(0x100000), "Inner block: verify that myprivate2 viacode outer definition didn't leak");
        is("\N{BE}", $hiragana_be, "Inner block: verify that \\N uses the correct script");
        cmp_ok(charnames::vianame("BE"), "==", ord($hiragana_be), "Inner block: verify that vianame uses the correct script");
        cmp_ok(charnames::string_vianame("BE"), "==", $hiragana_be, "Inner block: verify that string_vianame uses the correct script");
        eval '"\N{Hiragana: BE}"';
        like($@, qr/Unknown charname 'Hiragana: BE'/, "Inner block without :short: \\N with short doesn't work");
        ok(! defined charnames::vianame("Hiragana: BE"), "Inner block without :short: verify that vianame with short doesn't work");
        ok(! defined charnames::string_vianame("Hiragana: BE"), "Inner block without :short: verify that string_vianame with short doesn't work");
    }

    # Back to previous block.  All tests from that block should work again.
    is("\N{mychar1}", "e", "Outer block: verify that \\N{mychar1} works");
    is(charnames::vianame("mychar1"), ord("e"), "Outer block: verify that vianame(mychar1) works");
    is(charnames::string_vianame("mychar1"), "e", "Outer block: verify that string_vianame(mychar1) works");
    is("\N{mychar2}", "A", "Outer block: verify that \\N{mychar2} works");
    is(charnames::vianame("mychar2"), ord("A"), "Outer block: verify that vianame(mychar2) works");
    is(charnames::string_vianame("mychar2"), "A", "Outer block: verify that string_vianame(mychar2) works");
    is("\N{myprivate1}", "\x{E8000}", "Outer block: verify that \\N{myprivate1} works");
    cmp_ok(charnames::vianame("myprivate1"), "==", 0xE8000, "Outer block: verify that vianame(myprivate1) works");
    is(charnames::string_vianame("myprivate1"), chr(0xE8000), "Outer block: verify that string_vianame(myprivate1) works");
    is(charnames::viacode(0xE8000), "myprivate1", "Outer block: verify that myprivate1 viacode works");
    is("\N{myprivate2}", "\x{100000}", "Outer block: verify that \\N{myprivate2} works");
    cmp_ok(charnames::vianame("myprivate2"), "==", 0x100000, "Outer block: verify that vianame(myprivate2) works");
    is(charnames::string_vianame("myprivate2"), chr(0x100000), "Outer block: verify that string_vianame(myprivate2) works");
    is(charnames::viacode(0x100000), "myprivate2", "Outer block: verify that myprivate2 viacode works");
    is("\N{BE}", "\N{KATAKANA LETTER BE}", "Outer block: verify that \\N uses the correct script ");
    cmp_ok(charnames::vianame("BE"), "==", ord("\N{KATAKANA LETTER BE}"), "Outer block: verify that vianame uses the correct script");
    cmp_ok(charnames::string_vianame("BE"), "==", "\N{KATAKANA LETTER BE}", "Outer block: verify that string_vianame uses the correct script");
    is("\N{Hiragana: BE}", $hiragana_be, "Outer block: verify that :short works with \\N");
    cmp_ok(charnames::vianame("Hiragana: BE"), "==", ord($hiragana_be), "Outer block: verify that :short works with vianame");
    cmp_ok(charnames::string_vianame("Hiragana: BE"), "==", $hiragana_be, "Outer block: verify that :short works with string_vianame");
    {
        use charnames qw(:loose new_tai_lue des_eret);
        is("\N{latincapitallettera}", "A", "Verify that loose matching works");
        cmp_ok("\N{high-qa}", "==", chr(0x1980), "Verify that loose script list matching works");
        is(charnames::string_vianame("O-i"), chr(0x10426), "Verify that loose script list matching works with string_vianame");
        is(charnames::vianame("o i"), 0x1044E, "Verify that loose script list matching works with vianame");
    }
    eval '"\N{latincapitallettera}"';
    like($@, qr/Unknown charname 'latincapitallettera'/, "Verify that loose matching caching doesn't leak outside of scope");
    {
        use charnames qw(:loose :short);
        cmp_ok("\N{co pt-ic:she-i}", "==", chr(0x3E3), "Verify that loose :short matching works");
        is(charnames::string_vianame("co pt_ic: She i"), chr(0x3E2), "Verify that loose :short matching works with string_vianame");
        is(charnames::vianame("  Arm-en-ian: x e h_"), 0x56D, "Verify that loose :short matching works with vianame");
    }
}

{
    # Go through the whole Unicode db.  It takes quite a while to test
    # all 1 million code points, so this tests a randomly selected
    # subset.  For now, don't test with \N{}, to avoid filling the internal
    # cache at compile time; use vianame

    # For randomized tests below.
    my $seed;
    if (defined $ENV{PERL_TEST_CHARNAMES_SEED}) {
        $seed = srand($ENV{PERL_TEST_CHARNAMES_SEED});
        if ($seed != $ENV{PERL_TEST_CHARNAMES_SEED}) {
            die "srand returned '$seed' instead of '$ENV{PERL_TEST_CHARNAMES_SEED}'";
        };
    }
    else {
        $seed = srand;
    }

    # We will look at the data grouped in "blocks" of the following
    # size.
    my $block_size_bits = 8;   # above 16 is not sensible
    my $block_size = 2**$block_size_bits;

    # There are the regular names, like "SPACE", plus the ones
    # that are algorithmically determinable, such as "CKJ UNIFIED
    # IDEOGRAPH-hhhh" where the hhhh is the actual hex code point number
    # of the character.  The percentage of each type to test is
    # fuzzily independently settable.  This breaks down when the block size is
    # 1 or is large enough that both types of names occur in the same block
    my $percentage_of_regular_names = ($run_slow_tests) ? 100 : 10;
    my $percentage_of_algorithmic_names = (100 / $block_size); # 1 test/block

    # If wants everything tested, do so by changing the block size to 1 so
    # every character is in its own block, otherwise there is a risk that the
    # randomness will cause something to be tested more than once at the
    # expense of testing something else not at all.
    if ($percentage_of_regular_names >= 100
        || $percentage_of_algorithmic_names >= 100)
    {
        $block_size_bits = 0;
        $block_size = 2**$block_size_bits;
    }

    # Changing the block size doesn't change anything with regards to
    # testing the regular names (except if you set it to 1 so that each code
    # point is in its own block), but will affect the algorithmic names.
    # If you make the size too big so that blocks include both regular
    # names and algorithmic, the whole block will be sampled at the sum
    # of the two rates.  If you make it too small, then more algorithmic
    # names will be tested than you probably intended.

    my @names;  # The names of every code point.

    # We look at one block past the Unicode maximum, to verify there are
    # no names in it.
    my $block_count = 1 + 0x110000 / $block_size;

    my @regular_names_count = (0) x $block_count ;
    my @algorithmic_names_count = (0) x $block_count;

    # Read the DB, and fill in @names with the character names.
    open my $fh, "<", "../../lib/unicore/UnicodeData.txt" or
        die "Can't open ../../lib/unicore/UnicodeData.txt: $!";
    while (<$fh>) {
        chomp;
        my ($code, $name, $category, undef, undef, undef, undef, undef, undef, undef, $u1name) = split ";";
        my $decimal = utf8::unicode_to_native(hex $code);
        $code = sprintf("%04X", $decimal) unless $::IS_ASCII;

        $decimal = hex $code;

        # The Unicode version 1 name is used instead of any that are
        # marked <control>.
        $name = $u1name if $name eq "<control>";

        # In earlier Perls, we reject this code point's name (BELL)
        $name = "" if $^V lt v5.17.0 && $decimal == 0x1F514;

        # ALERT overrides BELL
        $name = 'ALERT' if $decimal == utf8::unicode_to_native(7);

        # Some don't have names, leave those array elements undefined
        next unless $name;

        # If the name isn't of this special form, it is a regular one.
        if ($name !~ /First>$/) {
            my $block = $decimal >> $block_size_bits;
            $names[$decimal] = $name;
            $regular_names_count[$block]++;
        }
        else {

            # The next line after a <First> is the <Last>, which is the
            # ending point of the range.
            $_ = <$fh>;
            /^(.*?);/;
            my $end_decimal = hex $1;

            # Only the ones whose category is a letter currently have names,
            # and of those the Hangul Syllables are dealt with below
            if ( $category eq 'Lo' && $name !~ /^Hangul/i) {

                # The CJK ones all get translated to a particular form; we
                # just capitalize any others in the hopes that Unicode will
                # use the correct term in any future ones it might add.
                if ($name =~ /^<CJK/) {
                    $name = "CJK UNIFIED IDEOGRAPH";
                }
                else {
                    $name =~ s/<//;
                    $name =~ s/,.*//;
                    $name = uc($name);
                }

                # They all have the code point as part of the name, which we
                # can construct
                for my $i ($decimal .. $end_decimal) {
                    $names[$i] = sprintf "$name-%04X", $i;
                    my $block = $i >> $block_size_bits;
                    $algorithmic_names_count[$block]++;
                }
            }
        }
    }
    close $fh;

    use Unicode::UCD;
    if (pack("C*", split /\./, Unicode::UCD::UnicodeVersion()) gt v1.1.5) {
        # The Hangul syllable names aren't in the file above; their names
        # are algorithmically determinable, but to avoid perpetuating any
        # programming errors, this file contains the complete list, gathered
        # from the web.
        while (<DATA>) {
            chomp;
            next unless $_;     # Guard against empty lines getting inserted.
            my ($code, $name) = split ";";
            my $decimal = hex $code;
            $names[$decimal] = $name;
            my $block = $decimal >> $block_size_bits;
            $algorithmic_names_count[$block] = 1;
        }
    }

    my @name_aliases;
    use Unicode::UCD;
    if (ord('A') == 65
        && pack( "C*", split /\./, Unicode::UCD::UnicodeVersion()) ge v6.1.0)
    {
        open my $fh, "<", "../../lib/unicore/NameAliases.txt"
            or die "Can't open ../../lib/unicore/NameAliases.txt: $!";
        @name_aliases = <$fh>
    }
    else {

        # If this Unicode version doesn't have the full .txt file, or are on
        # an EBCDIC platform where they need to be translated, get the data
        # from prop_invmap() (which should do the translation) and convert it
        # to the file's format
        use Unicode::UCD 'prop_invmap';
        my ($invlist_ref, $invmap_ref, undef, $default)
                                                = prop_invmap('Name_Alias');
        for my $i (0 .. @$invlist_ref - 1) {

            # Convert the aliases for code points that have just one alias to
            # single element arrays for uniform handling below.
            if (! ref $invmap_ref->[$i]) {

                # But we test only the real aliases, not the ones which are
                # just really placeholders.
                next if $invmap_ref->[$i] eq $default;

                $invmap_ref->[$i] = [ $invmap_ref->[$i] ];
            }


            # Change each alias for the code point to the form that the file
            # has
            foreach my $j ($invlist_ref->[$i] .. $invlist_ref->[$i+1] - 1) {
                foreach my $value (@{$invmap_ref->[$i]}) {
                    $value =~ s/: /;/;
                    push @name_aliases, sprintf("%04X;%s\n", $j, $value);
                }
            }
        }
    }

    for (@name_aliases) {
        chomp;
        s/^\s*#.*//;
        next unless $_;
        my ($hex, $name, $type) = split ";";
        my $i = CORE::hex $hex;

        # Make sure that both aliases (the one in UnicodeData, and the one we
        # just read) return the same code point.
        test_vianame($i, $hex, $name);
        test_vianame($i, $hex, $names[$i]) if $names[$i] ne "";

        # Set up so that a test below of this code point will use the alias
        # instead of the less-correct original.  We can't test here that
        # viacode is correct, because the alias file may contain multiple
        # aliases for the same code point, and viacode should return only the
        # final one.  So don't do it here; instead rely on the loop below to
        # pick up the test.
        $names[$i] = $name if $type eq 'correction';
    }
    close $fh;

    # Now, have all the names populated.  Do the tests

    my $all_pass = 1;   # Assume everything will pass.

    my $block = 0;  # Start at the beginning.
    while ($block < $block_count) {

        # Calculate how many tests to run on this block, based on the
        # how many names of each type are in it, and what percentage to
        # test of each type.
        my $test_count = 0;
        if ($algorithmic_names_count[$block]) {
            $test_count += int($regular_names_count[$block] * $percentage_of_algorithmic_names / 100  + .5);
            $test_count = 1 unless $test_count; # Make sure at least one
        }
        if ($regular_names_count[$block]) {
            $test_count += int($regular_names_count[$block] * $percentage_of_regular_names / 100  + .5);
            $test_count = 1 unless $test_count;
        }

        # For very small block sizes, we could come up with more tests
        # than characters in it
        $test_count = $block_size if $test_count > $block_size;

        # To avoid testing all the gazillions of code points that have
        # no names, and are almost certainly going to succeed, we
        # coalesce all such adjacent blocks into one, and have just one
        # test for that super-sized block
        my $end_block = $block;
        if ($test_count == 0) {
            $test_count = 1;
            if ($run_slow_tests < $RUN_SLOW_TESTS_EVERY_CODE_POINT) {
                $end_block++;

                # Keep coalescing until find a block that has something in
                # it.  But don't cross plane boundaries (the 16 bits below),
                # so there is at least one test for every plane.
                while ($end_block < $block_count
                       && $end_block >> (16 - $block_size_bits)
                                        == $block >> (16 - $block_size_bits)
                       && ! $algorithmic_names_count[$end_block]
                       && ! $regular_names_count[$end_block])
                {
                    $end_block++;
                }
                $end_block--;   # Back-off to a block that has no defined names
            }
        }

        # Calculated how many tests.  Do them
        for (1 .. $test_count) {

            # Randomly choose a code point in the block
            my $i = $block * $block_size + int(rand(($end_block - $block + 1) * $block_size));
            my $hex = sprintf("%04X", $i);
            if (! $names[$i]) {

                # These four code points now have names, from NameAlias, but
                # aren't listed as having names in UnicodeData.txt, so viacode
                # returns their alias names, not undef
                next if $i == utf8::unicode_to_native(0x80)
                              || $i == utf8::unicode_to_native(0x81)
                              || $i == utf8::unicode_to_native(0x84)
                              || $i == utf8::unicode_to_native(0x99);

                # If there is no name for this code point, all we can
                # test is that.
                $all_pass &= ok(! defined charnames::viacode($i), "Verify viacode(0x$hex) is undefined");
            } else {

                # Otherwise, test that the name and code point map
                # correctly.
                $all_pass &= test_vianame($i, $hex, $names[$i]);

                # These four code points have a different Unicode1 name than
                # regular name, and viacode has already specifically tested
                # for the regular name
                if ($i != utf8::unicode_to_native(0x0a)
                    && $i != utf8::unicode_to_native(0x0c)
                    && $i != utf8::unicode_to_native(0x0d)
                    && $i != utf8::unicode_to_native(0x85))
                {
                    $all_pass &= is(charnames::viacode($i), $names[$i], "Verify viacode(0x$hex) is \"$names[$i]\"");
                }

                # And make sure that a non-algorithmically named code
                # point doesn't also map to one that is.
                if ($names[$i] !~ /$hex$/) {
                    if (rand() < .5) {
                        $all_pass &= ok(! defined charnames::vianame("CJK UNIFIED IDEOGRAPH-$hex"), "Verify vianame(\"CJK UNIFIED IDEOGRAPH-$hex\") is undefined");
                    } else {
                        $all_pass &= ok(! defined charnames::string_vianame("CJK UNIFIED IDEOGRAPH-$hex"), "Verify string_vianame(\"CJK UNIFIED IDEOGRAPH-$hex\") is undefined");
                        eval "qr/\\p{name=CJK UNIFIED IDEOGRAPH-$hex}/";
                        $all_pass &= like($@, qr/Can't find Unicode property definition "name=CJK UNIFIED IDEOGRAPH-$hex\"/,
                                                "Verify string_vianame(\"CJK UNIFIED IDEOGRAPH-$hex\") is undefined");
                    }
                }
            }
        }

        # Skip to the next untested block.
        $block = $end_block + 1;
    }

    if (open my $fh, "<", "../../lib/unicore/NamedSequences.txt") {
        while (<$fh>) {
            chomp;
            s/^\s*#.*//;
            next unless $_;
            my ($name, $codes) = split ";";
            $codes =~ s{ \b 00 ( [0-9A-F]{2} ) \b }
                       { sprintf("%04X", utf8::unicode_to_native(hex $1)) }gxe
                                                            if ord "A" != 65;
            my $utf8 = pack("W*", map { hex } split " ", $codes);
            is(charnames::string_vianame($name), $utf8, "Verify string_vianame(\"$name\") is the proper utf8");
            my $loose_name = get_loose_name($name);
            use charnames ":loose";
            is(charnames::string_vianame($loose_name), $utf8, "Verify string_vianame(\"$loose_name\") is the proper utf8");

            like($utf8, qr/^\p{name=$name}$/, "Verify /\\p{name=$name}\$/ is the proper utf8");
            like($utf8, qr/^\p{name=$loose_name}$/, "Verify /\\p{name=$loose_name}\$/ is the proper utf8");
            like($utf8, qr!^\p{name=/\A$name\z/}!, "Verify /\\p{name=/$\A$name\z/} is the proper utf8");
            #diag("$name, $utf8");
        }
        close $fh;
    }
    else {
        use Unicode::UCD;
        die "Can't open ../../lib/unicore/NamedSequences.txt: $!"
        if pack("C*", split /\./, Unicode::UCD::UnicodeVersion()) ge v4.1.0;
    }


    unless ($all_pass) {
        diag(<<END
Not all tests succeeded.  Because testing every single Unicode code
point would take too long, $0
tests a random subset every run.  In order to reproduce this failure exactly,
the same seed must be used.  Save this seed!!: $seed

Setting the environment variable PERL_TEST_CHARNAMES_SEED with
$seed
when running this test will cause it to run exactly as it did here.
END
        );
    }
}

plan(curr_test() - 1);

# Thanks to http://www.inames.net/lang/out/out_p1s3_hangul.html for
# listing all the Hangul syllable names.
__DATA__
AC00;HANGUL SYLLABLE GA
AC01;HANGUL SYLLABLE GAG
AC02;HANGUL SYLLABLE GAGG
AC03;HANGUL SYLLABLE GAGS
AC04;HANGUL SYLLABLE GAN
AC05;HANGUL SYLLABLE GANJ
AC06;HANGUL SYLLABLE GANH
AC07;HANGUL SYLLABLE GAD
AC08;HANGUL SYLLABLE GAL
AC09;HANGUL SYLLABLE GALG
AC0A;HANGUL SYLLABLE GALM
AC0B;HANGUL SYLLABLE GALB
AC0C;HANGUL SYLLABLE GALS
AC0D;HANGUL SYLLABLE GALT
AC0E;HANGUL SYLLABLE GALP
AC0F;HANGUL SYLLABLE GALH
AC10;HANGUL SYLLABLE GAM
AC11;HANGUL SYLLABLE GAB
AC12;HANGUL SYLLABLE GABS
AC13;HANGUL SYLLABLE GAS
AC14;HANGUL SYLLABLE GASS
AC15;HANGUL SYLLABLE GANG
AC16;HANGUL SYLLABLE GAJ
AC17;HANGUL SYLLABLE GAC
AC18;HANGUL SYLLABLE GAK
AC19;HANGUL SYLLABLE GAT
AC1A;HANGUL SYLLABLE GAP
AC1B;HANGUL SYLLABLE GAH
AC1C;HANGUL SYLLABLE GAE
AC1D;HANGUL SYLLABLE GAEG
AC1E;HANGUL SYLLABLE GAEGG
AC1F;HANGUL SYLLABLE GAEGS
AC20;HANGUL SYLLABLE GAEN
AC21;HANGUL SYLLABLE GAENJ
AC22;HANGUL SYLLABLE GAENH
AC23;HANGUL SYLLABLE GAED
AC24;HANGUL SYLLABLE GAEL
AC25;HANGUL SYLLABLE GAELG
AC26;HANGUL SYLLABLE GAELM
AC27;HANGUL SYLLABLE GAELB
AC28;HANGUL SYLLABLE GAELS
AC29;HANGUL SYLLABLE GAELT
AC2A;HANGUL SYLLABLE GAELP
AC2B;HANGUL SYLLABLE GAELH
AC2C;HANGUL SYLLABLE GAEM
AC2D;HANGUL SYLLABLE GAEB
AC2E;HANGUL SYLLABLE GAEBS
AC2F;HANGUL SYLLABLE GAES
AC30;HANGUL SYLLABLE GAESS
AC31;HANGUL SYLLABLE GAENG
AC32;HANGUL SYLLABLE GAEJ
AC33;HANGUL SYLLABLE GAEC
AC34;HANGUL SYLLABLE GAEK
AC35;HANGUL SYLLABLE GAET
AC36;HANGUL SYLLABLE GAEP
AC37;HANGUL SYLLABLE GAEH
AC38;HANGUL SYLLABLE GYA
AC39;HANGUL SYLLABLE GYAG
AC3A;HANGUL SYLLABLE GYAGG
AC3B;HANGUL SYLLABLE GYAGS
AC3C;HANGUL SYLLABLE GYAN
AC3D;HANGUL SYLLABLE GYANJ
AC3E;HANGUL SYLLABLE GYANH
AC3F;HANGUL SYLLABLE GYAD
AC40;HANGUL SYLLABLE GYAL
AC41;HANGUL SYLLABLE GYALG
AC42;HANGUL SYLLABLE GYALM
AC43;HANGUL SYLLABLE GYALB
AC44;HANGUL SYLLABLE GYALS
AC45;HANGUL SYLLABLE GYALT
AC46;HANGUL SYLLABLE GYALP
AC47;HANGUL SYLLABLE GYALH
AC48;HANGUL SYLLABLE GYAM
AC49;HANGUL SYLLABLE GYAB
AC4A;HANGUL SYLLABLE GYABS
AC4B;HANGUL SYLLABLE GYAS
AC4C;HANGUL SYLLABLE GYASS
AC4D;HANGUL SYLLABLE GYANG
AC4E;HANGUL SYLLABLE GYAJ
AC4F;HANGUL SYLLABLE GYAC
AC50;HANGUL SYLLABLE GYAK
AC51;HANGUL SYLLABLE GYAT
AC52;HANGUL SYLLABLE GYAP
AC53;HANGUL SYLLABLE GYAH
AC54;HANGUL SYLLABLE GYAE
AC55;HANGUL SYLLABLE GYAEG
AC56;HANGUL SYLLABLE GYAEGG
AC57;HANGUL SYLLABLE GYAEGS
AC58;HANGUL SYLLABLE GYAEN
AC59;HANGUL SYLLABLE GYAENJ
AC5A;HANGUL SYLLABLE GYAENH
AC5B;HANGUL SYLLABLE GYAED
AC5C;HANGUL SYLLABLE GYAEL
AC5D;HANGUL SYLLABLE GYAELG
AC5E;HANGUL SYLLABLE GYAELM
AC5F;HANGUL SYLLABLE GYAELB
AC60;HANGUL SYLLABLE GYAELS
AC61;HANGUL SYLLABLE GYAELT
AC62;HANGUL SYLLABLE GYAELP
AC63;HANGUL SYLLABLE GYAELH
AC64;HANGUL SYLLABLE GYAEM
AC65;HANGUL SYLLABLE GYAEB
AC66;HANGUL SYLLABLE GYAEBS
AC67;HANGUL SYLLABLE GYAES
AC68;HANGUL SYLLABLE GYAESS
AC69;HANGUL SYLLABLE GYAENG
AC6A;HANGUL SYLLABLE GYAEJ
AC6B;HANGUL SYLLABLE GYAEC
AC6C;HANGUL SYLLABLE GYAEK
AC6D;HANGUL SYLLABLE GYAET
AC6E;HANGUL SYLLABLE GYAEP
AC6F;HANGUL SYLLABLE GYAEH
AC70;HANGUL SYLLABLE GEO
AC71;HANGUL SYLLABLE GEOG
AC72;HANGUL SYLLABLE GEOGG
AC73;HANGUL SYLLABLE GEOGS
AC74;HANGUL SYLLABLE GEON
AC75;HANGUL SYLLABLE GEONJ
AC76;HANGUL SYLLABLE GEONH
AC77;HANGUL SYLLABLE GEOD
AC78;HANGUL SYLLABLE GEOL
AC79;HANGUL SYLLABLE GEOLG
AC7A;HANGUL SYLLABLE GEOLM
AC7B;HANGUL SYLLABLE GEOLB
AC7C;HANGUL SYLLABLE GEOLS
AC7D;HANGUL SYLLABLE GEOLT
AC7E;HANGUL SYLLABLE GEOLP
AC7F;HANGUL SYLLABLE GEOLH
AC80;HANGUL SYLLABLE GEOM
AC81;HANGUL SYLLABLE GEOB
AC82;HANGUL SYLLABLE GEOBS
AC83;HANGUL SYLLABLE GEOS
AC84;HANGUL SYLLABLE GEOSS
AC85;HANGUL SYLLABLE GEONG
AC86;HANGUL SYLLABLE GEOJ
AC87;HANGUL SYLLABLE GEOC
AC88;HANGUL SYLLABLE GEOK
AC89;HANGUL SYLLABLE GEOT
AC8A;HANGUL SYLLABLE GEOP
AC8B;HANGUL SYLLABLE GEOH
AC8C;HANGUL SYLLABLE GE
AC8D;HANGUL SYLLABLE GEG
AC8E;HANGUL SYLLABLE GEGG
AC8F;HANGUL SYLLABLE GEGS
AC90;HANGUL SYLLABLE GEN
AC91;HANGUL SYLLABLE GENJ
AC92;HANGUL SYLLABLE GENH
AC93;HANGUL SYLLABLE GED
AC94;HANGUL SYLLABLE GEL
AC95;HANGUL SYLLABLE GELG
AC96;HANGUL SYLLABLE GELM
AC97;HANGUL SYLLABLE GELB
AC98;HANGUL SYLLABLE GELS
AC99;HANGUL SYLLABLE GELT
AC9A;HANGUL SYLLABLE GELP
AC9B;HANGUL SYLLABLE GELH
AC9C;HANGUL SYLLABLE GEM
AC9D;HANGUL SYLLABLE GEB
AC9E;HANGUL SYLLABLE GEBS
AC9F;HANGUL SYLLABLE GES
ACA0;HANGUL SYLLABLE GESS
ACA1;HANGUL SYLLABLE GENG
ACA2;HANGUL SYLLABLE GEJ
ACA3;HANGUL SYLLABLE GEC
ACA4;HANGUL SYLLABLE GEK
ACA5;HANGUL SYLLABLE GET
ACA6;HANGUL SYLLABLE GEP
ACA7;HANGUL SYLLABLE GEH
ACA8;HANGUL SYLLABLE GYEO
ACA9;HANGUL SYLLABLE GYEOG
ACAA;HANGUL SYLLABLE GYEOGG
ACAB;HANGUL SYLLABLE GYEOGS
ACAC;HANGUL SYLLABLE GYEON
ACAD;HANGUL SYLLABLE GYEONJ
ACAE;HANGUL SYLLABLE GYEONH
ACAF;HANGUL SYLLABLE GYEOD
ACB0;HANGUL SYLLABLE GYEOL
ACB1;HANGUL SYLLABLE GYEOLG
ACB2;HANGUL SYLLABLE GYEOLM
ACB3;HANGUL SYLLABLE GYEOLB
ACB4;HANGUL SYLLABLE GYEOLS
ACB5;HANGUL SYLLABLE GYEOLT
ACB6;HANGUL SYLLABLE GYEOLP
ACB7;HANGUL SYLLABLE GYEOLH
ACB8;HANGUL SYLLABLE GYEOM
ACB9;HANGUL SYLLABLE GYEOB
ACBA;HANGUL SYLLABLE GYEOBS
ACBB;HANGUL SYLLABLE GYEOS
ACBC;HANGUL SYLLABLE GYEOSS
ACBD;HANGUL SYLLABLE GYEONG
ACBE;HANGUL SYLLABLE GYEOJ
ACBF;HANGUL SYLLABLE GYEOC
ACC0;HANGUL SYLLABLE GYEOK
ACC1;HANGUL SYLLABLE GYEOT
ACC2;HANGUL SYLLABLE GYEOP
ACC3;HANGUL SYLLABLE GYEOH
ACC4;HANGUL SYLLABLE GYE
ACC5;HANGUL SYLLABLE GYEG
ACC6;HANGUL SYLLABLE GYEGG
ACC7;HANGUL SYLLABLE GYEGS
ACC8;HANGUL SYLLABLE GYEN
ACC9;HANGUL SYLLABLE GYENJ
ACCA;HANGUL SYLLABLE GYENH
ACCB;HANGUL SYLLABLE GYED
ACCC;HANGUL SYLLABLE GYEL
ACCD;HANGUL SYLLABLE GYELG
ACCE;HANGUL SYLLABLE GYELM
ACCF;HANGUL SYLLABLE GYELB
ACD0;HANGUL SYLLABLE GYELS
ACD1;HANGUL SYLLABLE GYELT
ACD2;HANGUL SYLLABLE GYELP
ACD3;HANGUL SYLLABLE GYELH
ACD4;HANGUL SYLLABLE GYEM
ACD5;HANGUL SYLLABLE GYEB
ACD6;HANGUL SYLLABLE GYEBS
ACD7;HANGUL SYLLABLE GYES
ACD8;HANGUL SYLLABLE GYESS
ACD9;HANGUL SYLLABLE GYENG
ACDA;HANGUL SYLLABLE GYEJ
ACDB;HANGUL SYLLABLE GYEC
ACDC;HANGUL SYLLABLE GYEK
ACDD;HANGUL SYLLABLE GYET
ACDE;HANGUL SYLLABLE GYEP
ACDF;HANGUL SYLLABLE GYEH
ACE0;HANGUL SYLLABLE GO
ACE1;HANGUL SYLLABLE GOG
ACE2;HANGUL SYLLABLE GOGG
ACE3;HANGUL SYLLABLE GOGS
ACE4;HANGUL SYLLABLE GON
ACE5;HANGUL SYLLABLE GONJ
ACE6;HANGUL SYLLABLE GONH
ACE7;HANGUL SYLLABLE GOD
ACE8;HANGUL SYLLABLE GOL
ACE9;HANGUL SYLLABLE GOLG
ACEA;HANGUL SYLLABLE GOLM
ACEB;HANGUL SYLLABLE GOLB
ACEC;HANGUL SYLLABLE GOLS
ACED;HANGUL SYLLABLE GOLT
ACEE;HANGUL SYLLABLE GOLP
ACEF;HANGUL SYLLABLE GOLH
ACF0;HANGUL SYLLABLE GOM
ACF1;HANGUL SYLLABLE GOB
ACF2;HANGUL SYLLABLE GOBS
ACF3;HANGUL SYLLABLE GOS
ACF4;HANGUL SYLLABLE GOSS
ACF5;HANGUL SYLLABLE GONG
ACF6;HANGUL SYLLABLE GOJ
ACF7;HANGUL SYLLABLE GOC
ACF8;HANGUL SYLLABLE GOK
ACF9;HANGUL SYLLABLE GOT
ACFA;HANGUL SYLLABLE GOP
ACFB;HANGUL SYLLABLE GOH
ACFC;HANGUL SYLLABLE GWA
ACFD;HANGUL SYLLABLE GWAG
ACFE;HANGUL SYLLABLE GWAGG
ACFF;HANGUL SYLLABLE GWAGS
AD00;HANGUL SYLLABLE GWAN
AD01;HANGUL SYLLABLE GWANJ
AD02;HANGUL SYLLABLE GWANH
AD03;HANGUL SYLLABLE GWAD
AD04;HANGUL SYLLABLE GWAL
AD05;HANGUL SYLLABLE GWALG
AD06;HANGUL SYLLABLE GWALM
AD07;HANGUL SYLLABLE GWALB
AD08;HANGUL SYLLABLE GWALS
AD09;HANGUL SYLLABLE GWALT
AD0A;HANGUL SYLLABLE GWALP
AD0B;HANGUL SYLLABLE GWALH
AD0C;HANGUL SYLLABLE GWAM
AD0D;HANGUL SYLLABLE GWAB
AD0E;HANGUL SYLLABLE GWABS
AD0F;HANGUL SYLLABLE GWAS
AD10;HANGUL SYLLABLE GWASS
AD11;HANGUL SYLLABLE GWANG
AD12;HANGUL SYLLABLE GWAJ
AD13;HANGUL SYLLABLE GWAC
AD14;HANGUL SYLLABLE GWAK
AD15;HANGUL SYLLABLE GWAT
AD16;HANGUL SYLLABLE GWAP
AD17;HANGUL SYLLABLE GWAH
AD18;HANGUL SYLLABLE GWAE
AD19;HANGUL SYLLABLE GWAEG
AD1A;HANGUL SYLLABLE GWAEGG
AD1B;HANGUL SYLLABLE GWAEGS
AD1C;HANGUL SYLLABLE GWAEN
AD1D;HANGUL SYLLABLE GWAENJ
AD1E;HANGUL SYLLABLE GWAENH
AD1F;HANGUL SYLLABLE GWAED
AD20;HANGUL SYLLABLE GWAEL
AD21;HANGUL SYLLABLE GWAELG
AD22;HANGUL SYLLABLE GWAELM
AD23;HANGUL SYLLABLE GWAELB
AD24;HANGUL SYLLABLE GWAELS
AD25;HANGUL SYLLABLE GWAELT
AD26;HANGUL SYLLABLE GWAELP
AD27;HANGUL SYLLABLE GWAELH
AD28;HANGUL SYLLABLE GWAEM
AD29;HANGUL SYLLABLE GWAEB
AD2A;HANGUL SYLLABLE GWAEBS
AD2B;HANGUL SYLLABLE GWAES
AD2C;HANGUL SYLLABLE GWAESS
AD2D;HANGUL SYLLABLE GWAENG
AD2E;HANGUL SYLLABLE GWAEJ
AD2F;HANGUL SYLLABLE GWAEC
AD30;HANGUL SYLLABLE GWAEK
AD31;HANGUL SYLLABLE GWAET
AD32;HANGUL SYLLABLE GWAEP
AD33;HANGUL SYLLABLE GWAEH
AD34;HANGUL SYLLABLE GOE
AD35;HANGUL SYLLABLE GOEG
AD36;HANGUL SYLLABLE GOEGG
AD37;HANGUL SYLLABLE GOEGS
AD38;HANGUL SYLLABLE GOEN
AD39;HANGUL SYLLABLE GOENJ
AD3A;HANGUL SYLLABLE GOENH
AD3B;HANGUL SYLLABLE GOED
AD3C;HANGUL SYLLABLE GOEL
AD3D;HANGUL SYLLABLE GOELG
AD3E;HANGUL SYLLABLE GOELM
AD3F;HANGUL SYLLABLE GOELB
AD40;HANGUL SYLLABLE GOELS
AD41;HANGUL SYLLABLE GOELT
AD42;HANGUL SYLLABLE GOELP
AD43;HANGUL SYLLABLE GOELH
AD44;HANGUL SYLLABLE GOEM
AD45;HANGUL SYLLABLE GOEB
AD46;HANGUL SYLLABLE GOEBS
AD47;HANGUL SYLLABLE GOES
AD48;HANGUL SYLLABLE GOESS
AD49;HANGUL SYLLABLE GOENG
AD4A;HANGUL SYLLABLE GOEJ
AD4B;HANGUL SYLLABLE GOEC
AD4C;HANGUL SYLLABLE GOEK
AD4D;HANGUL SYLLABLE GOET
AD4E;HANGUL SYLLABLE GOEP
AD4F;HANGUL SYLLABLE GOEH
AD50;HANGUL SYLLABLE GYO
AD51;HANGUL SYLLABLE GYOG
AD52;HANGUL SYLLABLE GYOGG
AD53;HANGUL SYLLABLE GYOGS
AD54;HANGUL SYLLABLE GYON
AD55;HANGUL SYLLABLE GYONJ
AD56;HANGUL SYLLABLE GYONH
AD57;HANGUL SYLLABLE GYOD
AD58;HANGUL SYLLABLE GYOL
AD59;HANGUL SYLLABLE GYOLG
AD5A;HANGUL SYLLABLE GYOLM
AD5B;HANGUL SYLLABLE GYOLB
AD5C;HANGUL SYLLABLE GYOLS
AD5D;HANGUL SYLLABLE GYOLT
AD5E;HANGUL SYLLABLE GYOLP
AD5F;HANGUL SYLLABLE GYOLH
AD60;HANGUL SYLLABLE GYOM
AD61;HANGUL SYLLABLE GYOB
AD62;HANGUL SYLLABLE GYOBS
AD63;HANGUL SYLLABLE GYOS
AD64;HANGUL SYLLABLE GYOSS
AD65;HANGUL SYLLABLE GYONG
AD66;HANGUL SYLLABLE GYOJ
AD67;HANGUL SYLLABLE GYOC
AD68;HANGUL SYLLABLE GYOK
AD69;HANGUL SYLLABLE GYOT
AD6A;HANGUL SYLLABLE GYOP
AD6B;HANGUL SYLLABLE GYOH
AD6C;HANGUL SYLLABLE GU
AD6D;HANGUL SYLLABLE GUG
AD6E;HANGUL SYLLABLE GUGG
AD6F;HANGUL SYLLABLE GUGS
AD70;HANGUL SYLLABLE GUN
AD71;HANGUL SYLLABLE GUNJ
AD72;HANGUL SYLLABLE GUNH
AD73;HANGUL SYLLABLE GUD
AD74;HANGUL SYLLABLE GUL
AD75;HANGUL SYLLABLE GULG
AD76;HANGUL SYLLABLE GULM
AD77;HANGUL SYLLABLE GULB
AD78;HANGUL SYLLABLE GULS
AD79;HANGUL SYLLABLE GULT
AD7A;HANGUL SYLLABLE GULP
AD7B;HANGUL SYLLABLE GULH
AD7C;HANGUL SYLLABLE GUM
AD7D;HANGUL SYLLABLE GUB
AD7E;HANGUL SYLLABLE GUBS
AD7F;HANGUL SYLLABLE GUS
AD80;HANGUL SYLLABLE GUSS
AD81;HANGUL SYLLABLE GUNG
AD82;HANGUL SYLLABLE GUJ
AD83;HANGUL SYLLABLE GUC
AD84;HANGUL SYLLABLE GUK
AD85;HANGUL SYLLABLE GUT
AD86;HANGUL SYLLABLE GUP
AD87;HANGUL SYLLABLE GUH
AD88;HANGUL SYLLABLE GWEO
AD89;HANGUL SYLLABLE GWEOG
AD8A;HANGUL SYLLABLE GWEOGG
AD8B;HANGUL SYLLABLE GWEOGS
AD8C;HANGUL SYLLABLE GWEON
AD8D;HANGUL SYLLABLE GWEONJ
AD8E;HANGUL SYLLABLE GWEONH
AD8F;HANGUL SYLLABLE GWEOD
AD90;HANGUL SYLLABLE GWEOL
AD91;HANGUL SYLLABLE GWEOLG
AD92;HANGUL SYLLABLE GWEOLM
AD93;HANGUL SYLLABLE GWEOLB
AD94;HANGUL SYLLABLE GWEOLS
AD95;HANGUL SYLLABLE GWEOLT
AD96;HANGUL SYLLABLE GWEOLP
AD97;HANGUL SYLLABLE GWEOLH
AD98;HANGUL SYLLABLE GWEOM
AD99;HANGUL SYLLABLE GWEOB
AD9A;HANGUL SYLLABLE GWEOBS
AD9B;HANGUL SYLLABLE GWEOS
AD9C;HANGUL SYLLABLE GWEOSS
AD9D;HANGUL SYLLABLE GWEONG
AD9E;HANGUL SYLLABLE GWEOJ
AD9F;HANGUL SYLLABLE GWEOC
ADA0;HANGUL SYLLABLE GWEOK
ADA1;HANGUL SYLLABLE GWEOT
ADA2;HANGUL SYLLABLE GWEOP
ADA3;HANGUL SYLLABLE GWEOH
ADA4;HANGUL SYLLABLE GWE
ADA5;HANGUL SYLLABLE GWEG
ADA6;HANGUL SYLLABLE GWEGG
ADA7;HANGUL SYLLABLE GWEGS
ADA8;HANGUL SYLLABLE GWEN
ADA9;HANGUL SYLLABLE GWENJ
ADAA;HANGUL SYLLABLE GWENH
ADAB;HANGUL SYLLABLE GWED
ADAC;HANGUL SYLLABLE GWEL
ADAD;HANGUL SYLLABLE GWELG
ADAE;HANGUL SYLLABLE GWELM
ADAF;HANGUL SYLLABLE GWELB
ADB0;HANGUL SYLLABLE GWELS
ADB1;HANGUL SYLLABLE GWELT
ADB2;HANGUL SYLLABLE GWELP
ADB3;HANGUL SYLLABLE GWELH
ADB4;HANGUL SYLLABLE GWEM
ADB5;HANGUL SYLLABLE GWEB
ADB6;HANGUL SYLLABLE GWEBS
ADB7;HANGUL SYLLABLE GWES
ADB8;HANGUL SYLLABLE GWESS
ADB9;HANGUL SYLLABLE GWENG
ADBA;HANGUL SYLLABLE GWEJ
ADBB;HANGUL SYLLABLE GWEC
ADBC;HANGUL SYLLABLE GWEK
ADBD;HANGUL SYLLABLE GWET
ADBE;HANGUL SYLLABLE GWEP
ADBF;HANGUL SYLLABLE GWEH
ADC0;HANGUL SYLLABLE GWI
ADC1;HANGUL SYLLABLE GWIG
ADC2;HANGUL SYLLABLE GWIGG
ADC3;HANGUL SYLLABLE GWIGS
ADC4;HANGUL SYLLABLE GWIN
ADC5;HANGUL SYLLABLE GWINJ
ADC6;HANGUL SYLLABLE GWINH
ADC7;HANGUL SYLLABLE GWID
ADC8;HANGUL SYLLABLE GWIL
ADC9;HANGUL SYLLABLE GWILG
ADCA;HANGUL SYLLABLE GWILM
ADCB;HANGUL SYLLABLE GWILB
ADCC;HANGUL SYLLABLE GWILS
ADCD;HANGUL SYLLABLE GWILT
ADCE;HANGUL SYLLABLE GWILP
ADCF;HANGUL SYLLABLE GWILH
ADD0;HANGUL SYLLABLE GWIM
ADD1;HANGUL SYLLABLE GWIB
ADD2;HANGUL SYLLABLE GWIBS
ADD3;HANGUL SYLLABLE GWIS
ADD4;HANGUL SYLLABLE GWISS
ADD5;HANGUL SYLLABLE GWING
ADD6;HANGUL SYLLABLE GWIJ
ADD7;HANGUL SYLLABLE GWIC
ADD8;HANGUL SYLLABLE GWIK
ADD9;HANGUL SYLLABLE GWIT
ADDA;HANGUL SYLLABLE GWIP
ADDB;HANGUL SYLLABLE GWIH
ADDC;HANGUL SYLLABLE GYU
ADDD;HANGUL SYLLABLE GYUG
ADDE;HANGUL SYLLABLE GYUGG
ADDF;HANGUL SYLLABLE GYUGS
ADE0;HANGUL SYLLABLE GYUN
ADE1;HANGUL SYLLABLE GYUNJ
ADE2;HANGUL SYLLABLE GYUNH
ADE3;HANGUL SYLLABLE GYUD
ADE4;HANGUL SYLLABLE GYUL
ADE5;HANGUL SYLLABLE GYULG
ADE6;HANGUL SYLLABLE GYULM
ADE7;HANGUL SYLLABLE GYULB
ADE8;HANGUL SYLLABLE GYULS
ADE9;HANGUL SYLLABLE GYULT
ADEA;HANGUL SYLLABLE GYULP
ADEB;HANGUL SYLLABLE GYULH
ADEC;HANGUL SYLLABLE GYUM
ADED;HANGUL SYLLABLE GYUB
ADEE;HANGUL SYLLABLE GYUBS
ADEF;HANGUL SYLLABLE GYUS
ADF0;HANGUL SYLLABLE GYUSS
ADF1;HANGUL SYLLABLE GYUNG
ADF2;HANGUL SYLLABLE GYUJ
ADF3;HANGUL SYLLABLE GYUC
ADF4;HANGUL SYLLABLE GYUK
ADF5;HANGUL SYLLABLE GYUT
ADF6;HANGUL SYLLABLE GYUP
ADF7;HANGUL SYLLABLE GYUH
ADF8;HANGUL SYLLABLE GEU
ADF9;HANGUL SYLLABLE GEUG
ADFA;HANGUL SYLLABLE GEUGG
ADFB;HANGUL SYLLABLE GEUGS
ADFC;HANGUL SYLLABLE GEUN
ADFD;HANGUL SYLLABLE GEUNJ
ADFE;HANGUL SYLLABLE GEUNH
ADFF;HANGUL SYLLABLE GEUD
AE00;HANGUL SYLLABLE GEUL
AE01;HANGUL SYLLABLE GEULG
AE02;HANGUL SYLLABLE GEULM
AE03;HANGUL SYLLABLE GEULB
AE04;HANGUL SYLLABLE GEULS
AE05;HANGUL SYLLABLE GEULT
AE06;HANGUL SYLLABLE GEULP
AE07;HANGUL SYLLABLE GEULH
AE08;HANGUL SYLLABLE GEUM
AE09;HANGUL SYLLABLE GEUB
AE0A;HANGUL SYLLABLE GEUBS
AE0B;HANGUL SYLLABLE GEUS
AE0C;HANGUL SYLLABLE GEUSS
AE0D;HANGUL SYLLABLE GEUNG
AE0E;HANGUL SYLLABLE GEUJ
AE0F;HANGUL SYLLABLE GEUC
AE10;HANGUL SYLLABLE GEUK
AE11;HANGUL SYLLABLE GEUT
AE12;HANGUL SYLLABLE GEUP
AE13;HANGUL SYLLABLE GEUH
AE14;HANGUL SYLLABLE GYI
AE15;HANGUL SYLLABLE GYIG
AE16;HANGUL SYLLABLE GYIGG
AE17;HANGUL SYLLABLE GYIGS
AE18;HANGUL SYLLABLE GYIN
AE19;HANGUL SYLLABLE GYINJ
AE1A;HANGUL SYLLABLE GYINH
AE1B;HANGUL SYLLABLE GYID
AE1C;HANGUL SYLLABLE GYIL
AE1D;HANGUL SYLLABLE GYILG
AE1E;HANGUL SYLLABLE GYILM
AE1F;HANGUL SYLLABLE GYILB
AE20;HANGUL SYLLABLE GYILS
AE21;HANGUL SYLLABLE GYILT
AE22;HANGUL SYLLABLE GYILP
AE23;HANGUL SYLLABLE GYILH
AE24;HANGUL SYLLABLE GYIM
AE25;HANGUL SYLLABLE GYIB
AE26;HANGUL SYLLABLE GYIBS
AE27;HANGUL SYLLABLE GYIS
AE28;HANGUL SYLLABLE GYISS
AE29;HANGUL SYLLABLE GYING
AE2A;HANGUL SYLLABLE GYIJ
AE2B;HANGUL SYLLABLE GYIC
AE2C;HANGUL SYLLABLE GYIK
AE2D;HANGUL SYLLABLE GYIT
AE2E;HANGUL SYLLABLE GYIP
AE2F;HANGUL SYLLABLE GYIH
AE30;HANGUL SYLLABLE GI
AE31;HANGUL SYLLABLE GIG
AE32;HANGUL SYLLABLE GIGG
AE33;HANGUL SYLLABLE GIGS
AE34;HANGUL SYLLABLE GIN
AE35;HANGUL SYLLABLE GINJ
AE36;HANGUL SYLLABLE GINH
AE37;HANGUL SYLLABLE GID
AE38;HANGUL SYLLABLE GIL
AE39;HANGUL SYLLABLE GILG
AE3A;HANGUL SYLLABLE GILM
AE3B;HANGUL SYLLABLE GILB
AE3C;HANGUL SYLLABLE GILS
AE3D;HANGUL SYLLABLE GILT
AE3E;HANGUL SYLLABLE GILP
AE3F;HANGUL SYLLABLE GILH
AE40;HANGUL SYLLABLE GIM
AE41;HANGUL SYLLABLE GIB
AE42;HANGUL SYLLABLE GIBS
AE43;HANGUL SYLLABLE GIS
AE44;HANGUL SYLLABLE GISS
AE45;HANGUL SYLLABLE GING
AE46;HANGUL SYLLABLE GIJ
AE47;HANGUL SYLLABLE GIC
AE48;HANGUL SYLLABLE GIK
AE49;HANGUL SYLLABLE GIT
AE4A;HANGUL SYLLABLE GIP
AE4B;HANGUL SYLLABLE GIH
AE4C;HANGUL SYLLABLE GGA
AE4D;HANGUL SYLLABLE GGAG
AE4E;HANGUL SYLLABLE GGAGG
AE4F;HANGUL SYLLABLE GGAGS
AE50;HANGUL SYLLABLE GGAN
AE51;HANGUL SYLLABLE GGANJ
AE52;HANGUL SYLLABLE GGANH
AE53;HANGUL SYLLABLE GGAD
AE54;HANGUL SYLLABLE GGAL
AE55;HANGUL SYLLABLE GGALG
AE56;HANGUL SYLLABLE GGALM
AE57;HANGUL SYLLABLE GGALB
AE58;HANGUL SYLLABLE GGALS
AE59;HANGUL SYLLABLE GGALT
AE5A;HANGUL SYLLABLE GGALP
AE5B;HANGUL SYLLABLE GGALH
AE5C;HANGUL SYLLABLE GGAM
AE5D;HANGUL SYLLABLE GGAB
AE5E;HANGUL SYLLABLE GGABS
AE5F;HANGUL SYLLABLE GGAS
AE60;HANGUL SYLLABLE GGASS
AE61;HANGUL SYLLABLE GGANG
AE62;HANGUL SYLLABLE GGAJ
AE63;HANGUL SYLLABLE GGAC
AE64;HANGUL SYLLABLE GGAK
AE65;HANGUL SYLLABLE GGAT
AE66;HANGUL SYLLABLE GGAP
AE67;HANGUL SYLLABLE GGAH
AE68;HANGUL SYLLABLE GGAE
AE69;HANGUL SYLLABLE GGAEG
AE6A;HANGUL SYLLABLE GGAEGG
AE6B;HANGUL SYLLABLE GGAEGS
AE6C;HANGUL SYLLABLE GGAEN
AE6D;HANGUL SYLLABLE GGAENJ
AE6E;HANGUL SYLLABLE GGAENH
AE6F;HANGUL SYLLABLE GGAED
AE70;HANGUL SYLLABLE GGAEL
AE71;HANGUL SYLLABLE GGAELG
AE72;HANGUL SYLLABLE GGAELM
AE73;HANGUL SYLLABLE GGAELB
AE74;HANGUL SYLLABLE GGAELS
AE75;HANGUL SYLLABLE GGAELT
AE76;HANGUL SYLLABLE GGAELP
AE77;HANGUL SYLLABLE GGAELH
AE78;HANGUL SYLLABLE GGAEM
AE79;HANGUL SYLLABLE GGAEB
AE7A;HANGUL SYLLABLE GGAEBS
AE7B;HANGUL SYLLABLE GGAES
AE7C;HANGUL SYLLABLE GGAESS
AE7D;HANGUL SYLLABLE GGAENG
AE7E;HANGUL SYLLABLE GGAEJ
AE7F;HANGUL SYLLABLE GGAEC
AE80;HANGUL SYLLABLE GGAEK
AE81;HANGUL SYLLABLE GGAET
AE82;HANGUL SYLLABLE GGAEP
AE83;HANGUL SYLLABLE GGAEH
AE84;HANGUL SYLLABLE GGYA
AE85;HANGUL SYLLABLE GGYAG
AE86;HANGUL SYLLABLE GGYAGG
AE87;HANGUL SYLLABLE GGYAGS
AE88;HANGUL SYLLABLE GGYAN
AE89;HANGUL SYLLABLE GGYANJ
AE8A;HANGUL SYLLABLE GGYANH
AE8B;HANGUL SYLLABLE GGYAD
AE8C;HANGUL SYLLABLE GGYAL
AE8D;HANGUL SYLLABLE GGYALG
AE8E;HANGUL SYLLABLE GGYALM
AE8F;HANGUL SYLLABLE GGYALB
AE90;HANGUL SYLLABLE GGYALS
AE91;HANGUL SYLLABLE GGYALT
AE92;HANGUL SYLLABLE GGYALP
AE93;HANGUL SYLLABLE GGYALH
AE94;HANGUL SYLLABLE GGYAM
AE95;HANGUL SYLLABLE GGYAB
AE96;HANGUL SYLLABLE GGYABS
AE97;HANGUL SYLLABLE GGYAS
AE98;HANGUL SYLLABLE GGYASS
AE99;HANGUL SYLLABLE GGYANG
AE9A;HANGUL SYLLABLE GGYAJ
AE9B;HANGUL SYLLABLE GGYAC
AE9C;HANGUL SYLLABLE GGYAK
AE9D;HANGUL SYLLABLE GGYAT
AE9E;HANGUL SYLLABLE GGYAP
AE9F;HANGUL SYLLABLE GGYAH
AEA0;HANGUL SYLLABLE GGYAE
AEA1;HANGUL SYLLABLE GGYAEG
AEA2;HANGUL SYLLABLE GGYAEGG
AEA3;HANGUL SYLLABLE GGYAEGS
AEA4;HANGUL SYLLABLE GGYAEN
AEA5;HANGUL SYLLABLE GGYAENJ
AEA6;HANGUL SYLLABLE GGYAENH
AEA7;HANGUL SYLLABLE GGYAED
AEA8;HANGUL SYLLABLE GGYAEL
AEA9;HANGUL SYLLABLE GGYAELG
AEAA;HANGUL SYLLABLE GGYAELM
AEAB;HANGUL SYLLABLE GGYAELB
AEAC;HANGUL SYLLABLE GGYAELS
AEAD;HANGUL SYLLABLE GGYAELT
AEAE;HANGUL SYLLABLE GGYAELP
AEAF;HANGUL SYLLABLE GGYAELH
AEB0;HANGUL SYLLABLE GGYAEM
AEB1;HANGUL SYLLABLE GGYAEB
AEB2;HANGUL SYLLABLE GGYAEBS
AEB3;HANGUL SYLLABLE GGYAES
AEB4;HANGUL SYLLABLE GGYAESS
AEB5;HANGUL SYLLABLE GGYAENG
AEB6;HANGUL SYLLABLE GGYAEJ
AEB7;HANGUL SYLLABLE GGYAEC
AEB8;HANGUL SYLLABLE GGYAEK
AEB9;HANGUL SYLLABLE GGYAET
AEBA;HANGUL SYLLABLE GGYAEP
AEBB;HANGUL SYLLABLE GGYAEH
AEBC;HANGUL SYLLABLE GGEO
AEBD;HANGUL SYLLABLE GGEOG
AEBE;HANGUL SYLLABLE GGEOGG
AEBF;HANGUL SYLLABLE GGEOGS
AEC0;HANGUL SYLLABLE GGEON
AEC1;HANGUL SYLLABLE GGEONJ
AEC2;HANGUL SYLLABLE GGEONH
AEC3;HANGUL SYLLABLE GGEOD
AEC4;HANGUL SYLLABLE GGEOL
AEC5;HANGUL SYLLABLE GGEOLG
AEC6;HANGUL SYLLABLE GGEOLM
AEC7;HANGUL SYLLABLE GGEOLB
AEC8;HANGUL SYLLABLE GGEOLS
AEC9;HANGUL SYLLABLE GGEOLT
AECA;HANGUL SYLLABLE GGEOLP
AECB;HANGUL SYLLABLE GGEOLH
AECC;HANGUL SYLLABLE GGEOM
AECD;HANGUL SYLLABLE GGEOB
AECE;HANGUL SYLLABLE GGEOBS
AECF;HANGUL SYLLABLE GGEOS
AED0;HANGUL SYLLABLE GGEOSS
AED1;HANGUL SYLLABLE GGEONG
AED2;HANGUL SYLLABLE GGEOJ
AED3;HANGUL SYLLABLE GGEOC
AED4;HANGUL SYLLABLE GGEOK
AED5;HANGUL SYLLABLE GGEOT
AED6;HANGUL SYLLABLE GGEOP
AED7;HANGUL SYLLABLE GGEOH
AED8;HANGUL SYLLABLE GGE
AED9;HANGUL SYLLABLE GGEG
AEDA;HANGUL SYLLABLE GGEGG
AEDB;HANGUL SYLLABLE GGEGS
AEDC;HANGUL SYLLABLE GGEN
AEDD;HANGUL SYLLABLE GGENJ
AEDE;HANGUL SYLLABLE GGENH
AEDF;HANGUL SYLLABLE GGED
AEE0;HANGUL SYLLABLE GGEL
AEE1;HANGUL SYLLABLE GGELG
AEE2;HANGUL SYLLABLE GGELM
AEE3;HANGUL SYLLABLE GGELB
AEE4;HANGUL SYLLABLE GGELS
AEE5;HANGUL SYLLABLE GGELT
AEE6;HANGUL SYLLABLE GGELP
AEE7;HANGUL SYLLABLE GGELH
AEE8;HANGUL SYLLABLE GGEM
AEE9;HANGUL SYLLABLE GGEB
AEEA;HANGUL SYLLABLE GGEBS
AEEB;HANGUL SYLLABLE GGES
AEEC;HANGUL SYLLABLE GGESS
AEED;HANGUL SYLLABLE GGENG
AEEE;HANGUL SYLLABLE GGEJ
AEEF;HANGUL SYLLABLE GGEC
AEF0;HANGUL SYLLABLE GGEK
AEF1;HANGUL SYLLABLE GGET
AEF2;HANGUL SYLLABLE GGEP
AEF3;HANGUL SYLLABLE GGEH
AEF4;HANGUL SYLLABLE GGYEO
AEF5;HANGUL SYLLABLE GGYEOG
AEF6;HANGUL SYLLABLE GGYEOGG
AEF7;HANGUL SYLLABLE GGYEOGS
AEF8;HANGUL SYLLABLE GGYEON
AEF9;HANGUL SYLLABLE GGYEONJ
AEFA;HANGUL SYLLABLE GGYEONH
AEFB;HANGUL SYLLABLE GGYEOD
AEFC;HANGUL SYLLABLE GGYEOL
AEFD;HANGUL SYLLABLE GGYEOLG
AEFE;HANGUL SYLLABLE GGYEOLM
AEFF;HANGUL SYLLABLE GGYEOLB
AF00;HANGUL SYLLABLE GGYEOLS
AF01;HANGUL SYLLABLE GGYEOLT
AF02;HANGUL SYLLABLE GGYEOLP
AF03;HANGUL SYLLABLE GGYEOLH
AF04;HANGUL SYLLABLE GGYEOM
AF05;HANGUL SYLLABLE GGYEOB
AF06;HANGUL SYLLABLE GGYEOBS
AF07;HANGUL SYLLABLE GGYEOS
AF08;HANGUL SYLLABLE GGYEOSS
AF09;HANGUL SYLLABLE GGYEONG
AF0A;HANGUL SYLLABLE GGYEOJ
AF0B;HANGUL SYLLABLE GGYEOC
AF0C;HANGUL SYLLABLE GGYEOK
AF0D;HANGUL SYLLABLE GGYEOT
AF0E;HANGUL SYLLABLE GGYEOP
AF0F;HANGUL SYLLABLE GGYEOH
AF10;HANGUL SYLLABLE GGYE
AF11;HANGUL SYLLABLE GGYEG
AF12;HANGUL SYLLABLE GGYEGG
AF13;HANGUL SYLLABLE GGYEGS
AF14;HANGUL SYLLABLE GGYEN
AF15;HANGUL SYLLABLE GGYENJ
AF16;HANGUL SYLLABLE GGYENH
AF17;HANGUL SYLLABLE GGYED
AF18;HANGUL SYLLABLE GGYEL
AF19;HANGUL SYLLABLE GGYELG
AF1A;HANGUL SYLLABLE GGYELM
AF1B;HANGUL SYLLABLE GGYELB
AF1C;HANGUL SYLLABLE GGYELS
AF1D;HANGUL SYLLABLE GGYELT
AF1E;HANGUL SYLLABLE GGYELP
AF1F;HANGUL SYLLABLE GGYELH
AF20;HANGUL SYLLABLE GGYEM
AF21;HANGUL SYLLABLE GGYEB
AF22;HANGUL SYLLABLE GGYEBS
AF23;HANGUL SYLLABLE GGYES
AF24;HANGUL SYLLABLE GGYESS
AF25;HANGUL SYLLABLE GGYENG
AF26;HANGUL SYLLABLE GGYEJ
AF27;HANGUL SYLLABLE GGYEC
AF28;HANGUL SYLLABLE GGYEK
AF29;HANGUL SYLLABLE GGYET
AF2A;HANGUL SYLLABLE GGYEP
AF2B;HANGUL SYLLABLE GGYEH
AF2C;HANGUL SYLLABLE GGO
AF2D;HANGUL SYLLABLE GGOG
AF2E;HANGUL SYLLABLE GGOGG
AF2F;HANGUL SYLLABLE GGOGS
AF30;HANGUL SYLLABLE GGON
AF31;HANGUL SYLLABLE GGONJ
AF32;HANGUL SYLLABLE GGONH
AF33;HANGUL SYLLABLE GGOD
AF34;HANGUL SYLLABLE GGOL
AF35;HANGUL SYLLABLE GGOLG
AF36;HANGUL SYLLABLE GGOLM
AF37;HANGUL SYLLABLE GGOLB
AF38;HANGUL SYLLABLE GGOLS
AF39;HANGUL SYLLABLE GGOLT
AF3A;HANGUL SYLLABLE GGOLP
AF3B;HANGUL SYLLABLE GGOLH
AF3C;HANGUL SYLLABLE GGOM
AF3D;HANGUL SYLLABLE GGOB
AF3E;HANGUL SYLLABLE GGOBS
AF3F;HANGUL SYLLABLE GGOS
AF40;HANGUL SYLLABLE GGOSS
AF41;HANGUL SYLLABLE GGONG
AF42;HANGUL SYLLABLE GGOJ
AF43;HANGUL SYLLABLE GGOC
AF44;HANGUL SYLLABLE GGOK
AF45;HANGUL SYLLABLE GGOT
AF46;HANGUL SYLLABLE GGOP
AF47;HANGUL SYLLABLE GGOH
AF48;HANGUL SYLLABLE GGWA
AF49;HANGUL SYLLABLE GGWAG
AF4A;HANGUL SYLLABLE GGWAGG
AF4B;HANGUL SYLLABLE GGWAGS
AF4C;HANGUL SYLLABLE GGWAN
AF4D;HANGUL SYLLABLE GGWANJ
AF4E;HANGUL SYLLABLE GGWANH
AF4F;HANGUL SYLLABLE GGWAD
AF50;HANGUL SYLLABLE GGWAL
AF51;HANGUL SYLLABLE GGWALG
AF52;HANGUL SYLLABLE GGWALM
AF53;HANGUL SYLLABLE GGWALB
AF54;HANGUL SYLLABLE GGWALS
AF55;HANGUL SYLLABLE GGWALT
AF56;HANGUL SYLLABLE GGWALP
AF57;HANGUL SYLLABLE GGWALH
AF58;HANGUL SYLLABLE GGWAM
AF59;HANGUL SYLLABLE GGWAB
AF5A;HANGUL SYLLABLE GGWABS
AF5B;HANGUL SYLLABLE GGWAS
AF5C;HANGUL SYLLABLE GGWASS
AF5D;HANGUL SYLLABLE GGWANG
AF5E;HANGUL SYLLABLE GGWAJ
AF5F;HANGUL SYLLABLE GGWAC
AF60;HANGUL SYLLABLE GGWAK
AF61;HANGUL SYLLABLE GGWAT
AF62;HANGUL SYLLABLE GGWAP
AF63;HANGUL SYLLABLE GGWAH
AF64;HANGUL SYLLABLE GGWAE
AF65;HANGUL SYLLABLE GGWAEG
AF66;HANGUL SYLLABLE GGWAEGG
AF67;HANGUL SYLLABLE GGWAEGS
AF68;HANGUL SYLLABLE GGWAEN
AF69;HANGUL SYLLABLE GGWAENJ
AF6A;HANGUL SYLLABLE GGWAENH
AF6B;HANGUL SYLLABLE GGWAED
AF6C;HANGUL SYLLABLE GGWAEL
AF6D;HANGUL SYLLABLE GGWAELG
AF6E;HANGUL SYLLABLE GGWAELM
AF6F;HANGUL SYLLABLE GGWAELB
AF70;HANGUL SYLLABLE GGWAELS
AF71;HANGUL SYLLABLE GGWAELT
AF72;HANGUL SYLLABLE GGWAELP
AF73;HANGUL SYLLABLE GGWAELH
AF74;HANGUL SYLLABLE GGWAEM
AF75;HANGUL SYLLABLE GGWAEB
AF76;HANGUL SYLLABLE GGWAEBS
AF77;HANGUL SYLLABLE GGWAES
AF78;HANGUL SYLLABLE GGWAESS
AF79;HANGUL SYLLABLE GGWAENG
AF7A;HANGUL SYLLABLE GGWAEJ
AF7B;HANGUL SYLLABLE GGWAEC
AF7C;HANGUL SYLLABLE GGWAEK
AF7D;HANGUL SYLLABLE GGWAET
AF7E;HANGUL SYLLABLE GGWAEP
AF7F;HANGUL SYLLABLE GGWAEH
AF80;HANGUL SYLLABLE GGOE
AF81;HANGUL SYLLABLE GGOEG
AF82;HANGUL SYLLABLE GGOEGG
AF83;HANGUL SYLLABLE GGOEGS
AF84;HANGUL SYLLABLE GGOEN
AF85;HANGUL SYLLABLE GGOENJ
AF86;HANGUL SYLLABLE GGOENH
AF87;HANGUL SYLLABLE GGOED
AF88;HANGUL SYLLABLE GGOEL
AF89;HANGUL SYLLABLE GGOELG
AF8A;HANGUL SYLLABLE GGOELM
AF8B;HANGUL SYLLABLE GGOELB
AF8C;HANGUL SYLLABLE GGOELS
AF8D;HANGUL SYLLABLE GGOELT
AF8E;HANGUL SYLLABLE GGOELP
AF8F;HANGUL SYLLABLE GGOELH
AF90;HANGUL SYLLABLE GGOEM
AF91;HANGUL SYLLABLE GGOEB
AF92;HANGUL SYLLABLE GGOEBS
AF93;HANGUL SYLLABLE GGOES
AF94;HANGUL SYLLABLE GGOESS
AF95;HANGUL SYLLABLE GGOENG
AF96;HANGUL SYLLABLE GGOEJ
AF97;HANGUL SYLLABLE GGOEC
AF98;HANGUL SYLLABLE GGOEK
AF99;HANGUL SYLLABLE GGOET
AF9A;HANGUL SYLLABLE GGOEP
AF9B;HANGUL SYLLABLE GGOEH
AF9C;HANGUL SYLLABLE GGYO
AF9D;HANGUL SYLLABLE GGYOG
AF9E;HANGUL SYLLABLE GGYOGG
AF9F;HANGUL SYLLABLE GGYOGS
AFA0;HANGUL SYLLABLE GGYON
AFA1;HANGUL SYLLABLE GGYONJ
AFA2;HANGUL SYLLABLE GGYONH
AFA3;HANGUL SYLLABLE GGYOD
AFA4;HANGUL SYLLABLE GGYOL
AFA5;HANGUL SYLLABLE GGYOLG
AFA6;HANGUL SYLLABLE GGYOLM
AFA7;HANGUL SYLLABLE GGYOLB
AFA8;HANGUL SYLLABLE GGYOLS
AFA9;HANGUL SYLLABLE GGYOLT
AFAA;HANGUL SYLLABLE GGYOLP
AFAB;HANGUL SYLLABLE GGYOLH
AFAC;HANGUL SYLLABLE GGYOM
AFAD;HANGUL SYLLABLE GGYOB
AFAE;HANGUL SYLLABLE GGYOBS
AFAF;HANGUL SYLLABLE GGYOS
AFB0;HANGUL SYLLABLE GGYOSS
AFB1;HANGUL SYLLABLE GGYONG
AFB2;HANGUL SYLLABLE GGYOJ
AFB3;HANGUL SYLLABLE GGYOC
AFB4;HANGUL SYLLABLE GGYOK
AFB5;HANGUL SYLLABLE GGYOT
AFB6;HANGUL SYLLABLE GGYOP
AFB7;HANGUL SYLLABLE GGYOH
AFB8;HANGUL SYLLABLE GGU
AFB9;HANGUL SYLLABLE GGUG
AFBA;HANGUL SYLLABLE GGUGG
AFBB;HANGUL SYLLABLE GGUGS
AFBC;HANGUL SYLLABLE GGUN
AFBD;HANGUL SYLLABLE GGUNJ
AFBE;HANGUL SYLLABLE GGUNH
AFBF;HANGUL SYLLABLE GGUD
AFC0;HANGUL SYLLABLE GGUL
AFC1;HANGUL SYLLABLE GGULG
AFC2;HANGUL SYLLABLE GGULM
AFC3;HANGUL SYLLABLE GGULB
AFC4;HANGUL SYLLABLE GGULS
AFC5;HANGUL SYLLABLE GGULT
AFC6;HANGUL SYLLABLE GGULP
AFC7;HANGUL SYLLABLE GGULH
AFC8;HANGUL SYLLABLE GGUM
AFC9;HANGUL SYLLABLE GGUB
AFCA;HANGUL SYLLABLE GGUBS
AFCB;HANGUL SYLLABLE GGUS
AFCC;HANGUL SYLLABLE GGUSS
AFCD;HANGUL SYLLABLE GGUNG
AFCE;HANGUL SYLLABLE GGUJ
AFCF;HANGUL SYLLABLE GGUC
AFD0;HANGUL SYLLABLE GGUK
AFD1;HANGUL SYLLABLE GGUT
AFD2;HANGUL SYLLABLE GGUP
AFD3;HANGUL SYLLABLE GGUH
AFD4;HANGUL SYLLABLE GGWEO
AFD5;HANGUL SYLLABLE GGWEOG
AFD6;HANGUL SYLLABLE GGWEOGG
AFD7;HANGUL SYLLABLE GGWEOGS
AFD8;HANGUL SYLLABLE GGWEON
AFD9;HANGUL SYLLABLE GGWEONJ
AFDA;HANGUL SYLLABLE GGWEONH
AFDB;HANGUL SYLLABLE GGWEOD
AFDC;HANGUL SYLLABLE GGWEOL
AFDD;HANGUL SYLLABLE GGWEOLG
AFDE;HANGUL SYLLABLE GGWEOLM
AFDF;HANGUL SYLLABLE GGWEOLB
AFE0;HANGUL SYLLABLE GGWEOLS
AFE1;HANGUL SYLLABLE GGWEOLT
AFE2;HANGUL SYLLABLE GGWEOLP
AFE3;HANGUL SYLLABLE GGWEOLH
AFE4;HANGUL SYLLABLE GGWEOM
AFE5;HANGUL SYLLABLE GGWEOB
AFE6;HANGUL SYLLABLE GGWEOBS
AFE7;HANGUL SYLLABLE GGWEOS
AFE8;HANGUL SYLLABLE GGWEOSS
AFE9;HANGUL SYLLABLE GGWEONG
AFEA;HANGUL SYLLABLE GGWEOJ
AFEB;HANGUL SYLLABLE GGWEOC
AFEC;HANGUL SYLLABLE GGWEOK
AFED;HANGUL SYLLABLE GGWEOT
AFEE;HANGUL SYLLABLE GGWEOP
AFEF;HANGUL SYLLABLE GGWEOH
AFF0;HANGUL SYLLABLE GGWE
AFF1;HANGUL SYLLABLE GGWEG
AFF2;HANGUL SYLLABLE GGWEGG
AFF3;HANGUL SYLLABLE GGWEGS
AFF4;HANGUL SYLLABLE GGWEN
AFF5;HANGUL SYLLABLE GGWENJ
AFF6;HANGUL SYLLABLE GGWENH
AFF7;HANGUL SYLLABLE GGWED
AFF8;HANGUL SYLLABLE GGWEL
AFF9;HANGUL SYLLABLE GGWELG
AFFA;HANGUL SYLLABLE GGWELM
AFFB;HANGUL SYLLABLE GGWELB
AFFC;HANGUL SYLLABLE GGWELS
AFFD;HANGUL SYLLABLE GGWELT
AFFE;HANGUL SYLLABLE GGWELP
AFFF;HANGUL SYLLABLE GGWELH
B000;HANGUL SYLLABLE GGWEM
B001;HANGUL SYLLABLE GGWEB
B002;HANGUL SYLLABLE GGWEBS
B003;HANGUL SYLLABLE GGWES
B004;HANGUL SYLLABLE GGWESS
B005;HANGUL SYLLABLE GGWENG
B006;HANGUL SYLLABLE GGWEJ
B007;HANGUL SYLLABLE GGWEC
B008;HANGUL SYLLABLE GGWEK
B009;HANGUL SYLLABLE GGWET
B00A;HANGUL SYLLABLE GGWEP
B00B;HANGUL SYLLABLE GGWEH
B00C;HANGUL SYLLABLE GGWI
B00D;HANGUL SYLLABLE GGWIG
B00E;HANGUL SYLLABLE GGWIGG
B00F;HANGUL SYLLABLE GGWIGS
B010;HANGUL SYLLABLE GGWIN
B011;HANGUL SYLLABLE GGWINJ
B012;HANGUL SYLLABLE GGWINH
B013;HANGUL SYLLABLE GGWID
B014;HANGUL SYLLABLE GGWIL
B015;HANGUL SYLLABLE GGWILG
B016;HANGUL SYLLABLE GGWILM
B017;HANGUL SYLLABLE GGWILB
B018;HANGUL SYLLABLE GGWILS
B019;HANGUL SYLLABLE GGWILT
B01A;HANGUL SYLLABLE GGWILP
B01B;HANGUL SYLLABLE GGWILH
B01C;HANGUL SYLLABLE GGWIM
B01D;HANGUL SYLLABLE GGWIB
B01E;HANGUL SYLLABLE GGWIBS
B01F;HANGUL SYLLABLE GGWIS
B020;HANGUL SYLLABLE GGWISS
B021;HANGUL SYLLABLE GGWING
B022;HANGUL SYLLABLE GGWIJ
B023;HANGUL SYLLABLE GGWIC
B024;HANGUL SYLLABLE GGWIK
B025;HANGUL SYLLABLE GGWIT
B026;HANGUL SYLLABLE GGWIP
B027;HANGUL SYLLABLE GGWIH
B028;HANGUL SYLLABLE GGYU
B029;HANGUL SYLLABLE GGYUG
B02A;HANGUL SYLLABLE GGYUGG
B02B;HANGUL SYLLABLE GGYUGS
B02C;HANGUL SYLLABLE GGYUN
B02D;HANGUL SYLLABLE GGYUNJ
B02E;HANGUL SYLLABLE GGYUNH
B02F;HANGUL SYLLABLE GGYUD
B030;HANGUL SYLLABLE GGYUL
B031;HANGUL SYLLABLE GGYULG
B032;HANGUL SYLLABLE GGYULM
B033;HANGUL SYLLABLE GGYULB
B034;HANGUL SYLLABLE GGYULS
B035;HANGUL SYLLABLE GGYULT
B036;HANGUL SYLLABLE GGYULP
B037;HANGUL SYLLABLE GGYULH
B038;HANGUL SYLLABLE GGYUM
B039;HANGUL SYLLABLE GGYUB
B03A;HANGUL SYLLABLE GGYUBS
B03B;HANGUL SYLLABLE GGYUS
B03C;HANGUL SYLLABLE GGYUSS
B03D;HANGUL SYLLABLE GGYUNG
B03E;HANGUL SYLLABLE GGYUJ
B03F;HANGUL SYLLABLE GGYUC
B040;HANGUL SYLLABLE GGYUK
B041;HANGUL SYLLABLE GGYUT
B042;HANGUL SYLLABLE GGYUP
B043;HANGUL SYLLABLE GGYUH
B044;HANGUL SYLLABLE GGEU
B045;HANGUL SYLLABLE GGEUG
B046;HANGUL SYLLABLE GGEUGG
B047;HANGUL SYLLABLE GGEUGS
B048;HANGUL SYLLABLE GGEUN
B049;HANGUL SYLLABLE GGEUNJ
B04A;HANGUL SYLLABLE GGEUNH
B04B;HANGUL SYLLABLE GGEUD
B04C;HANGUL SYLLABLE GGEUL
B04D;HANGUL SYLLABLE GGEULG
B04E;HANGUL SYLLABLE GGEULM
B04F;HANGUL SYLLABLE GGEULB
B050;HANGUL SYLLABLE GGEULS
B051;HANGUL SYLLABLE GGEULT
B052;HANGUL SYLLABLE GGEULP
B053;HANGUL SYLLABLE GGEULH
B054;HANGUL SYLLABLE GGEUM
B055;HANGUL SYLLABLE GGEUB
B056;HANGUL SYLLABLE GGEUBS
B057;HANGUL SYLLABLE GGEUS
B058;HANGUL SYLLABLE GGEUSS
B059;HANGUL SYLLABLE GGEUNG
B05A;HANGUL SYLLABLE GGEUJ
B05B;HANGUL SYLLABLE GGEUC
B05C;HANGUL SYLLABLE GGEUK
B05D;HANGUL SYLLABLE GGEUT
B05E;HANGUL SYLLABLE GGEUP
B05F;HANGUL SYLLABLE GGEUH
B060;HANGUL SYLLABLE GGYI
B061;HANGUL SYLLABLE GGYIG
B062;HANGUL SYLLABLE GGYIGG
B063;HANGUL SYLLABLE GGYIGS
B064;HANGUL SYLLABLE GGYIN
B065;HANGUL SYLLABLE GGYINJ
B066;HANGUL SYLLABLE GGYINH
B067;HANGUL SYLLABLE GGYID
B068;HANGUL SYLLABLE GGYIL
B069;HANGUL SYLLABLE GGYILG
B06A;HANGUL SYLLABLE GGYILM
B06B;HANGUL SYLLABLE GGYILB
B06C;HANGUL SYLLABLE GGYILS
B06D;HANGUL SYLLABLE GGYILT
B06E;HANGUL SYLLABLE GGYILP
B06F;HANGUL SYLLABLE GGYILH
B070;HANGUL SYLLABLE GGYIM
B071;HANGUL SYLLABLE GGYIB
B072;HANGUL SYLLABLE GGYIBS
B073;HANGUL SYLLABLE GGYIS
B074;HANGUL SYLLABLE GGYISS
B075;HANGUL SYLLABLE GGYING
B076;HANGUL SYLLABLE GGYIJ
B077;HANGUL SYLLABLE GGYIC
B078;HANGUL SYLLABLE GGYIK
B079;HANGUL SYLLABLE GGYIT
B07A;HANGUL SYLLABLE GGYIP
B07B;HANGUL SYLLABLE GGYIH
B07C;HANGUL SYLLABLE GGI
B07D;HANGUL SYLLABLE GGIG
B07E;HANGUL SYLLABLE GGIGG
B07F;HANGUL SYLLABLE GGIGS
B080;HANGUL SYLLABLE GGIN
B081;HANGUL SYLLABLE GGINJ
B082;HANGUL SYLLABLE GGINH
B083;HANGUL SYLLABLE GGID
B084;HANGUL SYLLABLE GGIL
B085;HANGUL SYLLABLE GGILG
B086;HANGUL SYLLABLE GGILM
B087;HANGUL SYLLABLE GGILB
B088;HANGUL SYLLABLE GGILS
B089;HANGUL SYLLABLE GGILT
B08A;HANGUL SYLLABLE GGILP
B08B;HANGUL SYLLABLE GGILH
B08C;HANGUL SYLLABLE GGIM
B08D;HANGUL SYLLABLE GGIB
B08E;HANGUL SYLLABLE GGIBS
B08F;HANGUL SYLLABLE GGIS
B090;HANGUL SYLLABLE GGISS
B091;HANGUL SYLLABLE GGING
B092;HANGUL SYLLABLE GGIJ
B093;HANGUL SYLLABLE GGIC
B094;HANGUL SYLLABLE GGIK
B095;HANGUL SYLLABLE GGIT
B096;HANGUL SYLLABLE GGIP
B097;HANGUL SYLLABLE GGIH
B098;HANGUL SYLLABLE NA
B099;HANGUL SYLLABLE NAG
B09A;HANGUL SYLLABLE NAGG
B09B;HANGUL SYLLABLE NAGS
B09C;HANGUL SYLLABLE NAN
B09D;HANGUL SYLLABLE NANJ
B09E;HANGUL SYLLABLE NANH
B09F;HANGUL SYLLABLE NAD
B0A0;HANGUL SYLLABLE NAL
B0A1;HANGUL SYLLABLE NALG
B0A2;HANGUL SYLLABLE NALM
B0A3;HANGUL SYLLABLE NALB
B0A4;HANGUL SYLLABLE NALS
B0A5;HANGUL SYLLABLE NALT
B0A6;HANGUL SYLLABLE NALP
B0A7;HANGUL SYLLABLE NALH
B0A8;HANGUL SYLLABLE NAM
B0A9;HANGUL SYLLABLE NAB
B0AA;HANGUL SYLLABLE NABS
B0AB;HANGUL SYLLABLE NAS
B0AC;HANGUL SYLLABLE NASS
B0AD;HANGUL SYLLABLE NANG
B0AE;HANGUL SYLLABLE NAJ
B0AF;HANGUL SYLLABLE NAC
B0B0;HANGUL SYLLABLE NAK
B0B1;HANGUL SYLLABLE NAT
B0B2;HANGUL SYLLABLE NAP
B0B3;HANGUL SYLLABLE NAH
B0B4;HANGUL SYLLABLE NAE
B0B5;HANGUL SYLLABLE NAEG
B0B6;HANGUL SYLLABLE NAEGG
B0B7;HANGUL SYLLABLE NAEGS
B0B8;HANGUL SYLLABLE NAEN
B0B9;HANGUL SYLLABLE NAENJ
B0BA;HANGUL SYLLABLE NAENH
B0BB;HANGUL SYLLABLE NAED
B0BC;HANGUL SYLLABLE NAEL
B0BD;HANGUL SYLLABLE NAELG
B0BE;HANGUL SYLLABLE NAELM
B0BF;HANGUL SYLLABLE NAELB
B0C0;HANGUL SYLLABLE NAELS
B0C1;HANGUL SYLLABLE NAELT
B0C2;HANGUL SYLLABLE NAELP
B0C3;HANGUL SYLLABLE NAELH
B0C4;HANGUL SYLLABLE NAEM
B0C5;HANGUL SYLLABLE NAEB
B0C6;HANGUL SYLLABLE NAEBS
B0C7;HANGUL SYLLABLE NAES
B0C8;HANGUL SYLLABLE NAESS
B0C9;HANGUL SYLLABLE NAENG
B0CA;HANGUL SYLLABLE NAEJ
B0CB;HANGUL SYLLABLE NAEC
B0CC;HANGUL SYLLABLE NAEK
B0CD;HANGUL SYLLABLE NAET
B0CE;HANGUL SYLLABLE NAEP
B0CF;HANGUL SYLLABLE NAEH
B0D0;HANGUL SYLLABLE NYA
B0D1;HANGUL SYLLABLE NYAG
B0D2;HANGUL SYLLABLE NYAGG
B0D3;HANGUL SYLLABLE NYAGS
B0D4;HANGUL SYLLABLE NYAN
B0D5;HANGUL SYLLABLE NYANJ
B0D6;HANGUL SYLLABLE NYANH
B0D7;HANGUL SYLLABLE NYAD
B0D8;HANGUL SYLLABLE NYAL
B0D9;HANGUL SYLLABLE NYALG
B0DA;HANGUL SYLLABLE NYALM
B0DB;HANGUL SYLLABLE NYALB
B0DC;HANGUL SYLLABLE NYALS
B0DD;HANGUL SYLLABLE NYALT
B0DE;HANGUL SYLLABLE NYALP
B0DF;HANGUL SYLLABLE NYALH
B0E0;HANGUL SYLLABLE NYAM
B0E1;HANGUL SYLLABLE NYAB
B0E2;HANGUL SYLLABLE NYABS
B0E3;HANGUL SYLLABLE NYAS
B0E4;HANGUL SYLLABLE NYASS
B0E5;HANGUL SYLLABLE NYANG
B0E6;HANGUL SYLLABLE NYAJ
B0E7;HANGUL SYLLABLE NYAC
B0E8;HANGUL SYLLABLE NYAK
B0E9;HANGUL SYLLABLE NYAT
B0EA;HANGUL SYLLABLE NYAP
B0EB;HANGUL SYLLABLE NYAH
B0EC;HANGUL SYLLABLE NYAE
B0ED;HANGUL SYLLABLE NYAEG
B0EE;HANGUL SYLLABLE NYAEGG
B0EF;HANGUL SYLLABLE NYAEGS
B0F0;HANGUL SYLLABLE NYAEN
B0F1;HANGUL SYLLABLE NYAENJ
B0F2;HANGUL SYLLABLE NYAENH
B0F3;HANGUL SYLLABLE NYAED
B0F4;HANGUL SYLLABLE NYAEL
B0F5;HANGUL SYLLABLE NYAELG
B0F6;HANGUL SYLLABLE NYAELM
B0F7;HANGUL SYLLABLE NYAELB
B0F8;HANGUL SYLLABLE NYAELS
B0F9;HANGUL SYLLABLE NYAELT
B0FA;HANGUL SYLLABLE NYAELP
B0FB;HANGUL SYLLABLE NYAELH
B0FC;HANGUL SYLLABLE NYAEM
B0FD;HANGUL SYLLABLE NYAEB
B0FE;HANGUL SYLLABLE NYAEBS
B0FF;HANGUL SYLLABLE NYAES
B100;HANGUL SYLLABLE NYAESS
B101;HANGUL SYLLABLE NYAENG
B102;HANGUL SYLLABLE NYAEJ
B103;HANGUL SYLLABLE NYAEC
B104;HANGUL SYLLABLE NYAEK
B105;HANGUL SYLLABLE NYAET
B106;HANGUL SYLLABLE NYAEP
B107;HANGUL SYLLABLE NYAEH
B108;HANGUL SYLLABLE NEO
B109;HANGUL SYLLABLE NEOG
B10A;HANGUL SYLLABLE NEOGG
B10B;HANGUL SYLLABLE NEOGS
B10C;HANGUL SYLLABLE NEON
B10D;HANGUL SYLLABLE NEONJ
B10E;HANGUL SYLLABLE NEONH
B10F;HANGUL SYLLABLE NEOD
B110;HANGUL SYLLABLE NEOL
B111;HANGUL SYLLABLE NEOLG
B112;HANGUL SYLLABLE NEOLM
B113;HANGUL SYLLABLE NEOLB
B114;HANGUL SYLLABLE NEOLS
B115;HANGUL SYLLABLE NEOLT
B116;HANGUL SYLLABLE NEOLP
B117;HANGUL SYLLABLE NEOLH
B118;HANGUL SYLLABLE NEOM
B119;HANGUL SYLLABLE NEOB
B11A;HANGUL SYLLABLE NEOBS
B11B;HANGUL SYLLABLE NEOS
B11C;HANGUL SYLLABLE NEOSS
B11D;HANGUL SYLLABLE NEONG
B11E;HANGUL SYLLABLE NEOJ
B11F;HANGUL SYLLABLE NEOC
B120;HANGUL SYLLABLE NEOK
B121;HANGUL SYLLABLE NEOT
B122;HANGUL SYLLABLE NEOP
B123;HANGUL SYLLABLE NEOH
B124;HANGUL SYLLABLE NE
B125;HANGUL SYLLABLE NEG
B126;HANGUL SYLLABLE NEGG
B127;HANGUL SYLLABLE NEGS
B128;HANGUL SYLLABLE NEN
B129;HANGUL SYLLABLE NENJ
B12A;HANGUL SYLLABLE NENH
B12B;HANGUL SYLLABLE NED
B12C;HANGUL SYLLABLE NEL
B12D;HANGUL SYLLABLE NELG
B12E;HANGUL SYLLABLE NELM
B12F;HANGUL SYLLABLE NELB
B130;HANGUL SYLLABLE NELS
B131;HANGUL SYLLABLE NELT
B132;HANGUL SYLLABLE NELP
B133;HANGUL SYLLABLE NELH
B134;HANGUL SYLLABLE NEM
B135;HANGUL SYLLABLE NEB
B136;HANGUL SYLLABLE NEBS
B137;HANGUL SYLLABLE NES
B138;HANGUL SYLLABLE NESS
B139;HANGUL SYLLABLE NENG
B13A;HANGUL SYLLABLE NEJ
B13B;HANGUL SYLLABLE NEC
B13C;HANGUL SYLLABLE NEK
B13D;HANGUL SYLLABLE NET
B13E;HANGUL SYLLABLE NEP
B13F;HANGUL SYLLABLE NEH
B140;HANGUL SYLLABLE NYEO
B141;HANGUL SYLLABLE NYEOG
B142;HANGUL SYLLABLE NYEOGG
B143;HANGUL SYLLABLE NYEOGS
B144;HANGUL SYLLABLE NYEON
B145;HANGUL SYLLABLE NYEONJ
B146;HANGUL SYLLABLE NYEONH
B147;HANGUL SYLLABLE NYEOD
B148;HANGUL SYLLABLE NYEOL
B149;HANGUL SYLLABLE NYEOLG
B14A;HANGUL SYLLABLE NYEOLM
B14B;HANGUL SYLLABLE NYEOLB
B14C;HANGUL SYLLABLE NYEOLS
B14D;HANGUL SYLLABLE NYEOLT
B14E;HANGUL SYLLABLE NYEOLP
B14F;HANGUL SYLLABLE NYEOLH
B150;HANGUL SYLLABLE NYEOM
B151;HANGUL SYLLABLE NYEOB
B152;HANGUL SYLLABLE NYEOBS
B153;HANGUL SYLLABLE NYEOS
B154;HANGUL SYLLABLE NYEOSS
B155;HANGUL SYLLABLE NYEONG
B156;HANGUL SYLLABLE NYEOJ
B157;HANGUL SYLLABLE NYEOC
B158;HANGUL SYLLABLE NYEOK
B159;HANGUL SYLLABLE NYEOT
B15A;HANGUL SYLLABLE NYEOP
B15B;HANGUL SYLLABLE NYEOH
B15C;HANGUL SYLLABLE NYE
B15D;HANGUL SYLLABLE NYEG
B15E;HANGUL SYLLABLE NYEGG
B15F;HANGUL SYLLABLE NYEGS
B160;HANGUL SYLLABLE NYEN
B161;HANGUL SYLLABLE NYENJ
B162;HANGUL SYLLABLE NYENH
B163;HANGUL SYLLABLE NYED
B164;HANGUL SYLLABLE NYEL
B165;HANGUL SYLLABLE NYELG
B166;HANGUL SYLLABLE NYELM
B167;HANGUL SYLLABLE NYELB
B168;HANGUL SYLLABLE NYELS
B169;HANGUL SYLLABLE NYELT
B16A;HANGUL SYLLABLE NYELP
B16B;HANGUL SYLLABLE NYELH
B16C;HANGUL SYLLABLE NYEM
B16D;HANGUL SYLLABLE NYEB
B16E;HANGUL SYLLABLE NYEBS
B16F;HANGUL SYLLABLE NYES
B170;HANGUL SYLLABLE NYESS
B171;HANGUL SYLLABLE NYENG
B172;HANGUL SYLLABLE NYEJ
B173;HANGUL SYLLABLE NYEC
B174;HANGUL SYLLABLE NYEK
B175;HANGUL SYLLABLE NYET
B176;HANGUL SYLLABLE NYEP
B177;HANGUL SYLLABLE NYEH
B178;HANGUL SYLLABLE NO
B179;HANGUL SYLLABLE NOG
B17A;HANGUL SYLLABLE NOGG
B17B;HANGUL SYLLABLE NOGS
B17C;HANGUL SYLLABLE NON
B17D;HANGUL SYLLABLE NONJ
B17E;HANGUL SYLLABLE NONH
B17F;HANGUL SYLLABLE NOD
B180;HANGUL SYLLABLE NOL
B181;HANGUL SYLLABLE NOLG
B182;HANGUL SYLLABLE NOLM
B183;HANGUL SYLLABLE NOLB
B184;HANGUL SYLLABLE NOLS
B185;HANGUL SYLLABLE NOLT
B186;HANGUL SYLLABLE NOLP
B187;HANGUL SYLLABLE NOLH
B188;HANGUL SYLLABLE NOM
B189;HANGUL SYLLABLE NOB
B18A;HANGUL SYLLABLE NOBS
B18B;HANGUL SYLLABLE NOS
B18C;HANGUL SYLLABLE NOSS
B18D;HANGUL SYLLABLE NONG
B18E;HANGUL SYLLABLE NOJ
B18F;HANGUL SYLLABLE NOC
B190;HANGUL SYLLABLE NOK
B191;HANGUL SYLLABLE NOT
B192;HANGUL SYLLABLE NOP
B193;HANGUL SYLLABLE NOH
B194;HANGUL SYLLABLE NWA
B195;HANGUL SYLLABLE NWAG
B196;HANGUL SYLLABLE NWAGG
B197;HANGUL SYLLABLE NWAGS
B198;HANGUL SYLLABLE NWAN
B199;HANGUL SYLLABLE NWANJ
B19A;HANGUL SYLLABLE NWANH
B19B;HANGUL SYLLABLE NWAD
B19C;HANGUL SYLLABLE NWAL
B19D;HANGUL SYLLABLE NWALG
B19E;HANGUL SYLLABLE NWALM
B19F;HANGUL SYLLABLE NWALB
B1A0;HANGUL SYLLABLE NWALS
B1A1;HANGUL SYLLABLE NWALT
B1A2;HANGUL SYLLABLE NWALP
B1A3;HANGUL SYLLABLE NWALH
B1A4;HANGUL SYLLABLE NWAM
B1A5;HANGUL SYLLABLE NWAB
B1A6;HANGUL SYLLABLE NWABS
B1A7;HANGUL SYLLABLE NWAS
B1A8;HANGUL SYLLABLE NWASS
B1A9;HANGUL SYLLABLE NWANG
B1AA;HANGUL SYLLABLE NWAJ
B1AB;HANGUL SYLLABLE NWAC
B1AC;HANGUL SYLLABLE NWAK
B1AD;HANGUL SYLLABLE NWAT
B1AE;HANGUL SYLLABLE NWAP
B1AF;HANGUL SYLLABLE NWAH
B1B0;HANGUL SYLLABLE NWAE
B1B1;HANGUL SYLLABLE NWAEG
B1B2;HANGUL SYLLABLE NWAEGG
B1B3;HANGUL SYLLABLE NWAEGS
B1B4;HANGUL SYLLABLE NWAEN
B1B5;HANGUL SYLLABLE NWAENJ
B1B6;HANGUL SYLLABLE NWAENH
B1B7;HANGUL SYLLABLE NWAED
B1B8;HANGUL SYLLABLE NWAEL
B1B9;HANGUL SYLLABLE NWAELG
B1BA;HANGUL SYLLABLE NWAELM
B1BB;HANGUL SYLLABLE NWAELB
B1BC;HANGUL SYLLABLE NWAELS
B1BD;HANGUL SYLLABLE NWAELT
B1BE;HANGUL SYLLABLE NWAELP
B1BF;HANGUL SYLLABLE NWAELH
B1C0;HANGUL SYLLABLE NWAEM
B1C1;HANGUL SYLLABLE NWAEB
B1C2;HANGUL SYLLABLE NWAEBS
B1C3;HANGUL SYLLABLE NWAES
B1C4;HANGUL SYLLABLE NWAESS
B1C5;HANGUL SYLLABLE NWAENG
B1C6;HANGUL SYLLABLE NWAEJ
B1C7;HANGUL SYLLABLE NWAEC
B1C8;HANGUL SYLLABLE NWAEK
B1C9;HANGUL SYLLABLE NWAET
B1CA;HANGUL SYLLABLE NWAEP
B1CB;HANGUL SYLLABLE NWAEH
B1CC;HANGUL SYLLABLE NOE
B1CD;HANGUL SYLLABLE NOEG
B1CE;HANGUL SYLLABLE NOEGG
B1CF;HANGUL SYLLABLE NOEGS
B1D0;HANGUL SYLLABLE NOEN
B1D1;HANGUL SYLLABLE NOENJ
B1D2;HANGUL SYLLABLE NOENH
B1D3;HANGUL SYLLABLE NOED
B1D4;HANGUL SYLLABLE NOEL
B1D5;HANGUL SYLLABLE NOELG
B1D6;HANGUL SYLLABLE NOELM
B1D7;HANGUL SYLLABLE NOELB
B1D8;HANGUL SYLLABLE NOELS
B1D9;HANGUL SYLLABLE NOELT
B1DA;HANGUL SYLLABLE NOELP
B1DB;HANGUL SYLLABLE NOELH
B1DC;HANGUL SYLLABLE NOEM
B1DD;HANGUL SYLLABLE NOEB
B1DE;HANGUL SYLLABLE NOEBS
B1DF;HANGUL SYLLABLE NOES
B1E0;HANGUL SYLLABLE NOESS
B1E1;HANGUL SYLLABLE NOENG
B1E2;HANGUL SYLLABLE NOEJ
B1E3;HANGUL SYLLABLE NOEC
B1E4;HANGUL SYLLABLE NOEK
B1E5;HANGUL SYLLABLE NOET
B1E6;HANGUL SYLLABLE NOEP
B1E7;HANGUL SYLLABLE NOEH
B1E8;HANGUL SYLLABLE NYO
B1E9;HANGUL SYLLABLE NYOG
B1EA;HANGUL SYLLABLE NYOGG
B1EB;HANGUL SYLLABLE NYOGS
B1EC;HANGUL SYLLABLE NYON
B1ED;HANGUL SYLLABLE NYONJ
B1EE;HANGUL SYLLABLE NYONH
B1EF;HANGUL SYLLABLE NYOD
B1F0;HANGUL SYLLABLE NYOL
B1F1;HANGUL SYLLABLE NYOLG
B1F2;HANGUL SYLLABLE NYOLM
B1F3;HANGUL SYLLABLE NYOLB
B1F4;HANGUL SYLLABLE NYOLS
B1F5;HANGUL SYLLABLE NYOLT
B1F6;HANGUL SYLLABLE NYOLP
B1F7;HANGUL SYLLABLE NYOLH
B1F8;HANGUL SYLLABLE NYOM
B1F9;HANGUL SYLLABLE NYOB
B1FA;HANGUL SYLLABLE NYOBS
B1FB;HANGUL SYLLABLE NYOS
B1FC;HANGUL SYLLABLE NYOSS
B1FD;HANGUL SYLLABLE NYONG
B1FE;HANGUL SYLLABLE NYOJ
B1FF;HANGUL SYLLABLE NYOC
B200;HANGUL SYLLABLE NYOK
B201;HANGUL SYLLABLE NYOT
B202;HANGUL SYLLABLE NYOP
B203;HANGUL SYLLABLE NYOH
B204;HANGUL SYLLABLE NU
B205;HANGUL SYLLABLE NUG
B206;HANGUL SYLLABLE NUGG
B207;HANGUL SYLLABLE NUGS
B208;HANGUL SYLLABLE NUN
B209;HANGUL SYLLABLE NUNJ
B20A;HANGUL SYLLABLE NUNH
B20B;HANGUL SYLLABLE NUD
B20C;HANGUL SYLLABLE NUL
B20D;HANGUL SYLLABLE NULG
B20E;HANGUL SYLLABLE NULM
B20F;HANGUL SYLLABLE NULB
B210;HANGUL SYLLABLE NULS
B211;HANGUL SYLLABLE NULT
B212;HANGUL SYLLABLE NULP
B213;HANGUL SYLLABLE NULH
B214;HANGUL SYLLABLE NUM
B215;HANGUL SYLLABLE NUB
B216;HANGUL SYLLABLE NUBS
B217;HANGUL SYLLABLE NUS
B218;HANGUL SYLLABLE NUSS
B219;HANGUL SYLLABLE NUNG
B21A;HANGUL SYLLABLE NUJ
B21B;HANGUL SYLLABLE NUC
B21C;HANGUL SYLLABLE NUK
B21D;HANGUL SYLLABLE NUT
B21E;HANGUL SYLLABLE NUP
B21F;HANGUL SYLLABLE NUH
B220;HANGUL SYLLABLE NWEO
B221;HANGUL SYLLABLE NWEOG
B222;HANGUL SYLLABLE NWEOGG
B223;HANGUL SYLLABLE NWEOGS
B224;HANGUL SYLLABLE NWEON
B225;HANGUL SYLLABLE NWEONJ
B226;HANGUL SYLLABLE NWEONH
B227;HANGUL SYLLABLE NWEOD
B228;HANGUL SYLLABLE NWEOL
B229;HANGUL SYLLABLE NWEOLG
B22A;HANGUL SYLLABLE NWEOLM
B22B;HANGUL SYLLABLE NWEOLB
B22C;HANGUL SYLLABLE NWEOLS
B22D;HANGUL SYLLABLE NWEOLT
B22E;HANGUL SYLLABLE NWEOLP
B22F;HANGUL SYLLABLE NWEOLH
B230;HANGUL SYLLABLE NWEOM
B231;HANGUL SYLLABLE NWEOB
B232;HANGUL SYLLABLE NWEOBS
B233;HANGUL SYLLABLE NWEOS
B234;HANGUL SYLLABLE NWEOSS
B235;HANGUL SYLLABLE NWEONG
B236;HANGUL SYLLABLE NWEOJ
B237;HANGUL SYLLABLE NWEOC
B238;HANGUL SYLLABLE NWEOK
B239;HANGUL SYLLABLE NWEOT
B23A;HANGUL SYLLABLE NWEOP
B23B;HANGUL SYLLABLE NWEOH
B23C;HANGUL SYLLABLE NWE
B23D;HANGUL SYLLABLE NWEG
B23E;HANGUL SYLLABLE NWEGG
B23F;HANGUL SYLLABLE NWEGS
B240;HANGUL SYLLABLE NWEN
B241;HANGUL SYLLABLE NWENJ
B242;HANGUL SYLLABLE NWENH
B243;HANGUL SYLLABLE NWED
B244;HANGUL SYLLABLE NWEL
B245;HANGUL SYLLABLE NWELG
B246;HANGUL SYLLABLE NWELM
B247;HANGUL SYLLABLE NWELB
B248;HANGUL SYLLABLE NWELS
B249;HANGUL SYLLABLE NWELT
B24A;HANGUL SYLLABLE NWELP
B24B;HANGUL SYLLABLE NWELH
B24C;HANGUL SYLLABLE NWEM
B24D;HANGUL SYLLABLE NWEB
B24E;HANGUL SYLLABLE NWEBS
B24F;HANGUL SYLLABLE NWES
B250;HANGUL SYLLABLE NWESS
B251;HANGUL SYLLABLE NWENG
B252;HANGUL SYLLABLE NWEJ
B253;HANGUL SYLLABLE NWEC
B254;HANGUL SYLLABLE NWEK
B255;HANGUL SYLLABLE NWET
B256;HANGUL SYLLABLE NWEP
B257;HANGUL SYLLABLE NWEH
B258;HANGUL SYLLABLE NWI
B259;HANGUL SYLLABLE NWIG
B25A;HANGUL SYLLABLE NWIGG
B25B;HANGUL SYLLABLE NWIGS
B25C;HANGUL SYLLABLE NWIN
B25D;HANGUL SYLLABLE NWINJ
B25E;HANGUL SYLLABLE NWINH
B25F;HANGUL SYLLABLE NWID
B260;HANGUL SYLLABLE NWIL
B261;HANGUL SYLLABLE NWILG
B262;HANGUL SYLLABLE NWILM
B263;HANGUL SYLLABLE NWILB
B264;HANGUL SYLLABLE NWILS
B265;HANGUL SYLLABLE NWILT
B266;HANGUL SYLLABLE NWILP
B267;HANGUL SYLLABLE NWILH
B268;HANGUL SYLLABLE NWIM
B269;HANGUL SYLLABLE NWIB
B26A;HANGUL SYLLABLE NWIBS
B26B;HANGUL SYLLABLE NWIS
B26C;HANGUL SYLLABLE NWISS
B26D;HANGUL SYLLABLE NWING
B26E;HANGUL SYLLABLE NWIJ
B26F;HANGUL SYLLABLE NWIC
B270;HANGUL SYLLABLE NWIK
B271;HANGUL SYLLABLE NWIT
B272;HANGUL SYLLABLE NWIP
B273;HANGUL SYLLABLE NWIH
B274;HANGUL SYLLABLE NYU
B275;HANGUL SYLLABLE NYUG
B276;HANGUL SYLLABLE NYUGG
B277;HANGUL SYLLABLE NYUGS
B278;HANGUL SYLLABLE NYUN
B279;HANGUL SYLLABLE NYUNJ
B27A;HANGUL SYLLABLE NYUNH
B27B;HANGUL SYLLABLE NYUD
B27C;HANGUL SYLLABLE NYUL
B27D;HANGUL SYLLABLE NYULG
B27E;HANGUL SYLLABLE NYULM
B27F;HANGUL SYLLABLE NYULB
B280;HANGUL SYLLABLE NYULS
B281;HANGUL SYLLABLE NYULT
B282;HANGUL SYLLABLE NYULP
B283;HANGUL SYLLABLE NYULH
B284;HANGUL SYLLABLE NYUM
B285;HANGUL SYLLABLE NYUB
B286;HANGUL SYLLABLE NYUBS
B287;HANGUL SYLLABLE NYUS
B288;HANGUL SYLLABLE NYUSS
B289;HANGUL SYLLABLE NYUNG
B28A;HANGUL SYLLABLE NYUJ
B28B;HANGUL SYLLABLE NYUC
B28C;HANGUL SYLLABLE NYUK
B28D;HANGUL SYLLABLE NYUT
B28E;HANGUL SYLLABLE NYUP
B28F;HANGUL SYLLABLE NYUH
B290;HANGUL SYLLABLE NEU
B291;HANGUL SYLLABLE NEUG
B292;HANGUL SYLLABLE NEUGG
B293;HANGUL SYLLABLE NEUGS
B294;HANGUL SYLLABLE NEUN
B295;HANGUL SYLLABLE NEUNJ
B296;HANGUL SYLLABLE NEUNH
B297;HANGUL SYLLABLE NEUD
B298;HANGUL SYLLABLE NEUL
B299;HANGUL SYLLABLE NEULG
B29A;HANGUL SYLLABLE NEULM
B29B;HANGUL SYLLABLE NEULB
B29C;HANGUL SYLLABLE NEULS
B29D;HANGUL SYLLABLE NEULT
B29E;HANGUL SYLLABLE NEULP
B29F;HANGUL SYLLABLE NEULH
B2A0;HANGUL SYLLABLE NEUM
B2A1;HANGUL SYLLABLE NEUB
B2A2;HANGUL SYLLABLE NEUBS
B2A3;HANGUL SYLLABLE NEUS
B2A4;HANGUL SYLLABLE NEUSS
B2A5;HANGUL SYLLABLE NEUNG
B2A6;HANGUL SYLLABLE NEUJ
B2A7;HANGUL SYLLABLE NEUC
B2A8;HANGUL SYLLABLE NEUK
B2A9;HANGUL SYLLABLE NEUT
B2AA;HANGUL SYLLABLE NEUP
B2AB;HANGUL SYLLABLE NEUH
B2AC;HANGUL SYLLABLE NYI
B2AD;HANGUL SYLLABLE NYIG
B2AE;HANGUL SYLLABLE NYIGG
B2AF;HANGUL SYLLABLE NYIGS
B2B0;HANGUL SYLLABLE NYIN
B2B1;HANGUL SYLLABLE NYINJ
B2B2;HANGUL SYLLABLE NYINH
B2B3;HANGUL SYLLABLE NYID
B2B4;HANGUL SYLLABLE NYIL
B2B5;HANGUL SYLLABLE NYILG
B2B6;HANGUL SYLLABLE NYILM
B2B7;HANGUL SYLLABLE NYILB
B2B8;HANGUL SYLLABLE NYILS
B2B9;HANGUL SYLLABLE NYILT
B2BA;HANGUL SYLLABLE NYILP
B2BB;HANGUL SYLLABLE NYILH
B2BC;HANGUL SYLLABLE NYIM
B2BD;HANGUL SYLLABLE NYIB
B2BE;HANGUL SYLLABLE NYIBS
B2BF;HANGUL SYLLABLE NYIS
B2C0;HANGUL SYLLABLE NYISS
B2C1;HANGUL SYLLABLE NYING
B2C2;HANGUL SYLLABLE NYIJ
B2C3;HANGUL SYLLABLE NYIC
B2C4;HANGUL SYLLABLE NYIK
B2C5;HANGUL SYLLABLE NYIT
B2C6;HANGUL SYLLABLE NYIP
B2C7;HANGUL SYLLABLE NYIH
B2C8;HANGUL SYLLABLE NI
B2C9;HANGUL SYLLABLE NIG
B2CA;HANGUL SYLLABLE NIGG
B2CB;HANGUL SYLLABLE NIGS
B2CC;HANGUL SYLLABLE NIN
B2CD;HANGUL SYLLABLE NINJ
B2CE;HANGUL SYLLABLE NINH
B2CF;HANGUL SYLLABLE NID
B2D0;HANGUL SYLLABLE NIL
B2D1;HANGUL SYLLABLE NILG
B2D2;HANGUL SYLLABLE NILM
B2D3;HANGUL SYLLABLE NILB
B2D4;HANGUL SYLLABLE NILS
B2D5;HANGUL SYLLABLE NILT
B2D6;HANGUL SYLLABLE NILP
B2D7;HANGUL SYLLABLE NILH
B2D8;HANGUL SYLLABLE NIM
B2D9;HANGUL SYLLABLE NIB
B2DA;HANGUL SYLLABLE NIBS
B2DB;HANGUL SYLLABLE NIS
B2DC;HANGUL SYLLABLE NISS
B2DD;HANGUL SYLLABLE NING
B2DE;HANGUL SYLLABLE NIJ
B2DF;HANGUL SYLLABLE NIC
B2E0;HANGUL SYLLABLE NIK
B2E1;HANGUL SYLLABLE NIT
B2E2;HANGUL SYLLABLE NIP
B2E3;HANGUL SYLLABLE NIH
B2E4;HANGUL SYLLABLE DA
B2E5;HANGUL SYLLABLE DAG
B2E6;HANGUL SYLLABLE DAGG
B2E7;HANGUL SYLLABLE DAGS
B2E8;HANGUL SYLLABLE DAN
B2E9;HANGUL SYLLABLE DANJ
B2EA;HANGUL SYLLABLE DANH
B2EB;HANGUL SYLLABLE DAD
B2EC;HANGUL SYLLABLE DAL
B2ED;HANGUL SYLLABLE DALG
B2EE;HANGUL SYLLABLE DALM
B2EF;HANGUL SYLLABLE DALB
B2F0;HANGUL SYLLABLE DALS
B2F1;HANGUL SYLLABLE DALT
B2F2;HANGUL SYLLABLE DALP
B2F3;HANGUL SYLLABLE DALH
B2F4;HANGUL SYLLABLE DAM
B2F5;HANGUL SYLLABLE DAB
B2F6;HANGUL SYLLABLE DABS
B2F7;HANGUL SYLLABLE DAS
B2F8;HANGUL SYLLABLE DASS
B2F9;HANGUL SYLLABLE DANG
B2FA;HANGUL SYLLABLE DAJ
B2FB;HANGUL SYLLABLE DAC
B2FC;HANGUL SYLLABLE DAK
B2FD;HANGUL SYLLABLE DAT
B2FE;HANGUL SYLLABLE DAP
B2FF;HANGUL SYLLABLE DAH
B300;HANGUL SYLLABLE DAE
B301;HANGUL SYLLABLE DAEG
B302;HANGUL SYLLABLE DAEGG
B303;HANGUL SYLLABLE DAEGS
B304;HANGUL SYLLABLE DAEN
B305;HANGUL SYLLABLE DAENJ
B306;HANGUL SYLLABLE DAENH
B307;HANGUL SYLLABLE DAED
B308;HANGUL SYLLABLE DAEL
B309;HANGUL SYLLABLE DAELG
B30A;HANGUL SYLLABLE DAELM
B30B;HANGUL SYLLABLE DAELB
B30C;HANGUL SYLLABLE DAELS
B30D;HANGUL SYLLABLE DAELT
B30E;HANGUL SYLLABLE DAELP
B30F;HANGUL SYLLABLE DAELH
B310;HANGUL SYLLABLE DAEM
B311;HANGUL SYLLABLE DAEB
B312;HANGUL SYLLABLE DAEBS
B313;HANGUL SYLLABLE DAES
B314;HANGUL SYLLABLE DAESS
B315;HANGUL SYLLABLE DAENG
B316;HANGUL SYLLABLE DAEJ
B317;HANGUL SYLLABLE DAEC
B318;HANGUL SYLLABLE DAEK
B319;HANGUL SYLLABLE DAET
B31A;HANGUL SYLLABLE DAEP
B31B;HANGUL SYLLABLE DAEH
B31C;HANGUL SYLLABLE DYA
B31D;HANGUL SYLLABLE DYAG
B31E;HANGUL SYLLABLE DYAGG
B31F;HANGUL SYLLABLE DYAGS
B320;HANGUL SYLLABLE DYAN
B321;HANGUL SYLLABLE DYANJ
B322;HANGUL SYLLABLE DYANH
B323;HANGUL SYLLABLE DYAD
B324;HANGUL SYLLABLE DYAL
B325;HANGUL SYLLABLE DYALG
B326;HANGUL SYLLABLE DYALM
B327;HANGUL SYLLABLE DYALB
B328;HANGUL SYLLABLE DYALS
B329;HANGUL SYLLABLE DYALT
B32A;HANGUL SYLLABLE DYALP
B32B;HANGUL SYLLABLE DYALH
B32C;HANGUL SYLLABLE DYAM
B32D;HANGUL SYLLABLE DYAB
B32E;HANGUL SYLLABLE DYABS
B32F;HANGUL SYLLABLE DYAS
B330;HANGUL SYLLABLE DYASS
B331;HANGUL SYLLABLE DYANG
B332;HANGUL SYLLABLE DYAJ
B333;HANGUL SYLLABLE DYAC
B334;HANGUL SYLLABLE DYAK
B335;HANGUL SYLLABLE DYAT
B336;HANGUL SYLLABLE DYAP
B337;HANGUL SYLLABLE DYAH
B338;HANGUL SYLLABLE DYAE
B339;HANGUL SYLLABLE DYAEG
B33A;HANGUL SYLLABLE DYAEGG
B33B;HANGUL SYLLABLE DYAEGS
B33C;HANGUL SYLLABLE DYAEN
B33D;HANGUL SYLLABLE DYAENJ
B33E;HANGUL SYLLABLE DYAENH
B33F;HANGUL SYLLABLE DYAED
B340;HANGUL SYLLABLE DYAEL
B341;HANGUL SYLLABLE DYAELG
B342;HANGUL SYLLABLE DYAELM
B343;HANGUL SYLLABLE DYAELB
B344;HANGUL SYLLABLE DYAELS
B345;HANGUL SYLLABLE DYAELT
B346;HANGUL SYLLABLE DYAELP
B347;HANGUL SYLLABLE DYAELH
B348;HANGUL SYLLABLE DYAEM
B349;HANGUL SYLLABLE DYAEB
B34A;HANGUL SYLLABLE DYAEBS
B34B;HANGUL SYLLABLE DYAES
B34C;HANGUL SYLLABLE DYAESS
B34D;HANGUL SYLLABLE DYAENG
B34E;HANGUL SYLLABLE DYAEJ
B34F;HANGUL SYLLABLE DYAEC
B350;HANGUL SYLLABLE DYAEK
B351;HANGUL SYLLABLE DYAET
B352;HANGUL SYLLABLE DYAEP
B353;HANGUL SYLLABLE DYAEH
B354;HANGUL SYLLABLE DEO
B355;HANGUL SYLLABLE DEOG
B356;HANGUL SYLLABLE DEOGG
B357;HANGUL SYLLABLE DEOGS
B358;HANGUL SYLLABLE DEON
B359;HANGUL SYLLABLE DEONJ
B35A;HANGUL SYLLABLE DEONH
B35B;HANGUL SYLLABLE DEOD
B35C;HANGUL SYLLABLE DEOL
B35D;HANGUL SYLLABLE DEOLG
B35E;HANGUL SYLLABLE DEOLM
B35F;HANGUL SYLLABLE DEOLB
B360;HANGUL SYLLABLE DEOLS
B361;HANGUL SYLLABLE DEOLT
B362;HANGUL SYLLABLE DEOLP
B363;HANGUL SYLLABLE DEOLH
B364;HANGUL SYLLABLE DEOM
B365;HANGUL SYLLABLE DEOB
B366;HANGUL SYLLABLE DEOBS
B367;HANGUL SYLLABLE DEOS
B368;HANGUL SYLLABLE DEOSS
B369;HANGUL SYLLABLE DEONG
B36A;HANGUL SYLLABLE DEOJ
B36B;HANGUL SYLLABLE DEOC
B36C;HANGUL SYLLABLE DEOK
B36D;HANGUL SYLLABLE DEOT
B36E;HANGUL SYLLABLE DEOP
B36F;HANGUL SYLLABLE DEOH
B370;HANGUL SYLLABLE DE
B371;HANGUL SYLLABLE DEG
B372;HANGUL SYLLABLE DEGG
B373;HANGUL SYLLABLE DEGS
B374;HANGUL SYLLABLE DEN
B375;HANGUL SYLLABLE DENJ
B376;HANGUL SYLLABLE DENH
B377;HANGUL SYLLABLE DED
B378;HANGUL SYLLABLE DEL
B379;HANGUL SYLLABLE DELG
B37A;HANGUL SYLLABLE DELM
B37B;HANGUL SYLLABLE DELB
B37C;HANGUL SYLLABLE DELS
B37D;HANGUL SYLLABLE DELT
B37E;HANGUL SYLLABLE DELP
B37F;HANGUL SYLLABLE DELH
B380;HANGUL SYLLABLE DEM
B381;HANGUL SYLLABLE DEB
B382;HANGUL SYLLABLE DEBS
B383;HANGUL SYLLABLE DES
B384;HANGUL SYLLABLE DESS
B385;HANGUL SYLLABLE DENG
B386;HANGUL SYLLABLE DEJ
B387;HANGUL SYLLABLE DEC
B388;HANGUL SYLLABLE DEK
B389;HANGUL SYLLABLE DET
B38A;HANGUL SYLLABLE DEP
B38B;HANGUL SYLLABLE DEH
B38C;HANGUL SYLLABLE DYEO
B38D;HANGUL SYLLABLE DYEOG
B38E;HANGUL SYLLABLE DYEOGG
B38F;HANGUL SYLLABLE DYEOGS
B390;HANGUL SYLLABLE DYEON
B391;HANGUL SYLLABLE DYEONJ
B392;HANGUL SYLLABLE DYEONH
B393;HANGUL SYLLABLE DYEOD
B394;HANGUL SYLLABLE DYEOL
B395;HANGUL SYLLABLE DYEOLG
B396;HANGUL SYLLABLE DYEOLM
B397;HANGUL SYLLABLE DYEOLB
B398;HANGUL SYLLABLE DYEOLS
B399;HANGUL SYLLABLE DYEOLT
B39A;HANGUL SYLLABLE DYEOLP
B39B;HANGUL SYLLABLE DYEOLH
B39C;HANGUL SYLLABLE DYEOM
B39D;HANGUL SYLLABLE DYEOB
B39E;HANGUL SYLLABLE DYEOBS
B39F;HANGUL SYLLABLE DYEOS
B3A0;HANGUL SYLLABLE DYEOSS
B3A1;HANGUL SYLLABLE DYEONG
B3A2;HANGUL SYLLABLE DYEOJ
B3A3;HANGUL SYLLABLE DYEOC
B3A4;HANGUL SYLLABLE DYEOK
B3A5;HANGUL SYLLABLE DYEOT
B3A6;HANGUL SYLLABLE DYEOP
B3A7;HANGUL SYLLABLE DYEOH
B3A8;HANGUL SYLLABLE DYE
B3A9;HANGUL SYLLABLE DYEG
B3AA;HANGUL SYLLABLE DYEGG
B3AB;HANGUL SYLLABLE DYEGS
B3AC;HANGUL SYLLABLE DYEN
B3AD;HANGUL SYLLABLE DYENJ
B3AE;HANGUL SYLLABLE DYENH
B3AF;HANGUL SYLLABLE DYED
B3B0;HANGUL SYLLABLE DYEL
B3B1;HANGUL SYLLABLE DYELG
B3B2;HANGUL SYLLABLE DYELM
B3B3;HANGUL SYLLABLE DYELB
B3B4;HANGUL SYLLABLE DYELS
B3B5;HANGUL SYLLABLE DYELT
B3B6;HANGUL SYLLABLE DYELP
B3B7;HANGUL SYLLABLE DYELH
B3B8;HANGUL SYLLABLE DYEM
B3B9;HANGUL SYLLABLE DYEB
B3BA;HANGUL SYLLABLE DYEBS
B3BB;HANGUL SYLLABLE DYES
B3BC;HANGUL SYLLABLE DYESS
B3BD;HANGUL SYLLABLE DYENG
B3BE;HANGUL SYLLABLE DYEJ
B3BF;HANGUL SYLLABLE DYEC
B3C0;HANGUL SYLLABLE DYEK
B3C1;HANGUL SYLLABLE DYET
B3C2;HANGUL SYLLABLE DYEP
B3C3;HANGUL SYLLABLE DYEH
B3C4;HANGUL SYLLABLE DO
B3C5;HANGUL SYLLABLE DOG
B3C6;HANGUL SYLLABLE DOGG
B3C7;HANGUL SYLLABLE DOGS
B3C8;HANGUL SYLLABLE DON
B3C9;HANGUL SYLLABLE DONJ
B3CA;HANGUL SYLLABLE DONH
B3CB;HANGUL SYLLABLE DOD
B3CC;HANGUL SYLLABLE DOL
B3CD;HANGUL SYLLABLE DOLG
B3CE;HANGUL SYLLABLE DOLM
B3CF;HANGUL SYLLABLE DOLB
B3D0;HANGUL SYLLABLE DOLS
B3D1;HANGUL SYLLABLE DOLT
B3D2;HANGUL SYLLABLE DOLP
B3D3;HANGUL SYLLABLE DOLH
B3D4;HANGUL SYLLABLE DOM
B3D5;HANGUL SYLLABLE DOB
B3D6;HANGUL SYLLABLE DOBS
B3D7;HANGUL SYLLABLE DOS
B3D8;HANGUL SYLLABLE DOSS
B3D9;HANGUL SYLLABLE DONG
B3DA;HANGUL SYLLABLE DOJ
B3DB;HANGUL SYLLABLE DOC
B3DC;HANGUL SYLLABLE DOK
B3DD;HANGUL SYLLABLE DOT
B3DE;HANGUL SYLLABLE DOP
B3DF;HANGUL SYLLABLE DOH
B3E0;HANGUL SYLLABLE DWA
B3E1;HANGUL SYLLABLE DWAG
B3E2;HANGUL SYLLABLE DWAGG
B3E3;HANGUL SYLLABLE DWAGS
B3E4;HANGUL SYLLABLE DWAN
B3E5;HANGUL SYLLABLE DWANJ
B3E6;HANGUL SYLLABLE DWANH
B3E7;HANGUL SYLLABLE DWAD
B3E8;HANGUL SYLLABLE DWAL
B3E9;HANGUL SYLLABLE DWALG
B3EA;HANGUL SYLLABLE DWALM
B3EB;HANGUL SYLLABLE DWALB
B3EC;HANGUL SYLLABLE DWALS
B3ED;HANGUL SYLLABLE DWALT
B3EE;HANGUL SYLLABLE DWALP
B3EF;HANGUL SYLLABLE DWALH
B3F0;HANGUL SYLLABLE DWAM
B3F1;HANGUL SYLLABLE DWAB
B3F2;HANGUL SYLLABLE DWABS
B3F3;HANGUL SYLLABLE DWAS
B3F4;HANGUL SYLLABLE DWASS
B3F5;HANGUL SYLLABLE DWANG
B3F6;HANGUL SYLLABLE DWAJ
B3F7;HANGUL SYLLABLE DWAC
B3F8;HANGUL SYLLABLE DWAK
B3F9;HANGUL SYLLABLE DWAT
B3FA;HANGUL SYLLABLE DWAP
B3FB;HANGUL SYLLABLE DWAH
B3FC;HANGUL SYLLABLE DWAE
B3FD;HANGUL SYLLABLE DWAEG
B3FE;HANGUL SYLLABLE DWAEGG
B3FF;HANGUL SYLLABLE DWAEGS
B400;HANGUL SYLLABLE DWAEN
B401;HANGUL SYLLABLE DWAENJ
B402;HANGUL SYLLABLE DWAENH
B403;HANGUL SYLLABLE DWAED
B404;HANGUL SYLLABLE DWAEL
B405;HANGUL SYLLABLE DWAELG
B406;HANGUL SYLLABLE DWAELM
B407;HANGUL SYLLABLE DWAELB
B408;HANGUL SYLLABLE DWAELS
B409;HANGUL SYLLABLE DWAELT
B40A;HANGUL SYLLABLE DWAELP
B40B;HANGUL SYLLABLE DWAELH
B40C;HANGUL SYLLABLE DWAEM
B40D;HANGUL SYLLABLE DWAEB
B40E;HANGUL SYLLABLE DWAEBS
B40F;HANGUL SYLLABLE DWAES
B410;HANGUL SYLLABLE DWAESS
B411;HANGUL SYLLABLE DWAENG
B412;HANGUL SYLLABLE DWAEJ
B413;HANGUL SYLLABLE DWAEC
B414;HANGUL SYLLABLE DWAEK
B415;HANGUL SYLLABLE DWAET
B416;HANGUL SYLLABLE DWAEP
B417;HANGUL SYLLABLE DWAEH
B418;HANGUL SYLLABLE DOE
B419;HANGUL SYLLABLE DOEG
B41A;HANGUL SYLLABLE DOEGG
B41B;HANGUL SYLLABLE DOEGS
B41C;HANGUL SYLLABLE DOEN
B41D;HANGUL SYLLABLE DOENJ
B41E;HANGUL SYLLABLE DOENH
B41F;HANGUL SYLLABLE DOED
B420;HANGUL SYLLABLE DOEL
B421;HANGUL SYLLABLE DOELG
B422;HANGUL SYLLABLE DOELM
B423;HANGUL SYLLABLE DOELB
B424;HANGUL SYLLABLE DOELS
B425;HANGUL SYLLABLE DOELT
B426;HANGUL SYLLABLE DOELP
B427;HANGUL SYLLABLE DOELH
B428;HANGUL SYLLABLE DOEM
B429;HANGUL SYLLABLE DOEB
B42A;HANGUL SYLLABLE DOEBS
B42B;HANGUL SYLLABLE DOES
B42C;HANGUL SYLLABLE DOESS
B42D;HANGUL SYLLABLE DOENG
B42E;HANGUL SYLLABLE DOEJ
B42F;HANGUL SYLLABLE DOEC
B430;HANGUL SYLLABLE DOEK
B431;HANGUL SYLLABLE DOET
B432;HANGUL SYLLABLE DOEP
B433;HANGUL SYLLABLE DOEH
B434;HANGUL SYLLABLE DYO
B435;HANGUL SYLLABLE DYOG
B436;HANGUL SYLLABLE DYOGG
B437;HANGUL SYLLABLE DYOGS
B438;HANGUL SYLLABLE DYON
B439;HANGUL SYLLABLE DYONJ
B43A;HANGUL SYLLABLE DYONH
B43B;HANGUL SYLLABLE DYOD
B43C;HANGUL SYLLABLE DYOL
B43D;HANGUL SYLLABLE DYOLG
B43E;HANGUL SYLLABLE DYOLM
B43F;HANGUL SYLLABLE DYOLB
B440;HANGUL SYLLABLE DYOLS
B441;HANGUL SYLLABLE DYOLT
B442;HANGUL SYLLABLE DYOLP
B443;HANGUL SYLLABLE DYOLH
B444;HANGUL SYLLABLE DYOM
B445;HANGUL SYLLABLE DYOB
B446;HANGUL SYLLABLE DYOBS
B447;HANGUL SYLLABLE DYOS
B448;HANGUL SYLLABLE DYOSS
B449;HANGUL SYLLABLE DYONG
B44A;HANGUL SYLLABLE DYOJ
B44B;HANGUL SYLLABLE DYOC
B44C;HANGUL SYLLABLE DYOK
B44D;HANGUL SYLLABLE DYOT
B44E;HANGUL SYLLABLE DYOP
B44F;HANGUL SYLLABLE DYOH
B450;HANGUL SYLLABLE DU
B451;HANGUL SYLLABLE DUG
B452;HANGUL SYLLABLE DUGG
B453;HANGUL SYLLABLE DUGS
B454;HANGUL SYLLABLE DUN
B455;HANGUL SYLLABLE DUNJ
B456;HANGUL SYLLABLE DUNH
B457;HANGUL SYLLABLE DUD
B458;HANGUL SYLLABLE DUL
B459;HANGUL SYLLABLE DULG
B45A;HANGUL SYLLABLE DULM
B45B;HANGUL SYLLABLE DULB
B45C;HANGUL SYLLABLE DULS
B45D;HANGUL SYLLABLE DULT
B45E;HANGUL SYLLABLE DULP
B45F;HANGUL SYLLABLE DULH
B460;HANGUL SYLLABLE DUM
B461;HANGUL SYLLABLE DUB
B462;HANGUL SYLLABLE DUBS
B463;HANGUL SYLLABLE DUS
B464;HANGUL SYLLABLE DUSS
B465;HANGUL SYLLABLE DUNG
B466;HANGUL SYLLABLE DUJ
B467;HANGUL SYLLABLE DUC
B468;HANGUL SYLLABLE DUK
B469;HANGUL SYLLABLE DUT
B46A;HANGUL SYLLABLE DUP
B46B;HANGUL SYLLABLE DUH
B46C;HANGUL SYLLABLE DWEO
B46D;HANGUL SYLLABLE DWEOG
B46E;HANGUL SYLLABLE DWEOGG
B46F;HANGUL SYLLABLE DWEOGS
B470;HANGUL SYLLABLE DWEON
B471;HANGUL SYLLABLE DWEONJ
B472;HANGUL SYLLABLE DWEONH
B473;HANGUL SYLLABLE DWEOD
B474;HANGUL SYLLABLE DWEOL
B475;HANGUL SYLLABLE DWEOLG
B476;HANGUL SYLLABLE DWEOLM
B477;HANGUL SYLLABLE DWEOLB
B478;HANGUL SYLLABLE DWEOLS
B479;HANGUL SYLLABLE DWEOLT
B47A;HANGUL SYLLABLE DWEOLP
B47B;HANGUL SYLLABLE DWEOLH
B47C;HANGUL SYLLABLE DWEOM
B47D;HANGUL SYLLABLE DWEOB
B47E;HANGUL SYLLABLE DWEOBS
B47F;HANGUL SYLLABLE DWEOS
B480;HANGUL SYLLABLE DWEOSS
B481;HANGUL SYLLABLE DWEONG
B482;HANGUL SYLLABLE DWEOJ
B483;HANGUL SYLLABLE DWEOC
B484;HANGUL SYLLABLE DWEOK
B485;HANGUL SYLLABLE DWEOT
B486;HANGUL SYLLABLE DWEOP
B487;HANGUL SYLLABLE DWEOH
B488;HANGUL SYLLABLE DWE
B489;HANGUL SYLLABLE DWEG
B48A;HANGUL SYLLABLE DWEGG
B48B;HANGUL SYLLABLE DWEGS
B48C;HANGUL SYLLABLE DWEN
B48D;HANGUL SYLLABLE DWENJ
B48E;HANGUL SYLLABLE DWENH
B48F;HANGUL SYLLABLE DWED
B490;HANGUL SYLLABLE DWEL
B491;HANGUL SYLLABLE DWELG
B492;HANGUL SYLLABLE DWELM
B493;HANGUL SYLLABLE DWELB
B494;HANGUL SYLLABLE DWELS
B495;HANGUL SYLLABLE DWELT
B496;HANGUL SYLLABLE DWELP
B497;HANGUL SYLLABLE DWELH
B498;HANGUL SYLLABLE DWEM
B499;HANGUL SYLLABLE DWEB
B49A;HANGUL SYLLABLE DWEBS
B49B;HANGUL SYLLABLE DWES
B49C;HANGUL SYLLABLE DWESS
B49D;HANGUL SYLLABLE DWENG
B49E;HANGUL SYLLABLE DWEJ
B49F;HANGUL SYLLABLE DWEC
B4A0;HANGUL SYLLABLE DWEK
B4A1;HANGUL SYLLABLE DWET
B4A2;HANGUL SYLLABLE DWEP
B4A3;HANGUL SYLLABLE DWEH
B4A4;HANGUL SYLLABLE DWI
B4A5;HANGUL SYLLABLE DWIG
B4A6;HANGUL SYLLABLE DWIGG
B4A7;HANGUL SYLLABLE DWIGS
B4A8;HANGUL SYLLABLE DWIN
B4A9;HANGUL SYLLABLE DWINJ
B4AA;HANGUL SYLLABLE DWINH
B4AB;HANGUL SYLLABLE DWID
B4AC;HANGUL SYLLABLE DWIL
B4AD;HANGUL SYLLABLE DWILG
B4AE;HANGUL SYLLABLE DWILM
B4AF;HANGUL SYLLABLE DWILB
B4B0;HANGUL SYLLABLE DWILS
B4B1;HANGUL SYLLABLE DWILT
B4B2;HANGUL SYLLABLE DWILP
B4B3;HANGUL SYLLABLE DWILH
B4B4;HANGUL SYLLABLE DWIM
B4B5;HANGUL SYLLABLE DWIB
B4B6;HANGUL SYLLABLE DWIBS
B4B7;HANGUL SYLLABLE DWIS
B4B8;HANGUL SYLLABLE DWISS
B4B9;HANGUL SYLLABLE DWING
B4BA;HANGUL SYLLABLE DWIJ
B4BB;HANGUL SYLLABLE DWIC
B4BC;HANGUL SYLLABLE DWIK
B4BD;HANGUL SYLLABLE DWIT
B4BE;HANGUL SYLLABLE DWIP
B4BF;HANGUL SYLLABLE DWIH
B4C0;HANGUL SYLLABLE DYU
B4C1;HANGUL SYLLABLE DYUG
B4C2;HANGUL SYLLABLE DYUGG
B4C3;HANGUL SYLLABLE DYUGS
B4C4;HANGUL SYLLABLE DYUN
B4C5;HANGUL SYLLABLE DYUNJ
B4C6;HANGUL SYLLABLE DYUNH
B4C7;HANGUL SYLLABLE DYUD
B4C8;HANGUL SYLLABLE DYUL
B4C9;HANGUL SYLLABLE DYULG
B4CA;HANGUL SYLLABLE DYULM
B4CB;HANGUL SYLLABLE DYULB
B4CC;HANGUL SYLLABLE DYULS
B4CD;HANGUL SYLLABLE DYULT
B4CE;HANGUL SYLLABLE DYULP
B4CF;HANGUL SYLLABLE DYULH
B4D0;HANGUL SYLLABLE DYUM
B4D1;HANGUL SYLLABLE DYUB
B4D2;HANGUL SYLLABLE DYUBS
B4D3;HANGUL SYLLABLE DYUS
B4D4;HANGUL SYLLABLE DYUSS
B4D5;HANGUL SYLLABLE DYUNG
B4D6;HANGUL SYLLABLE DYUJ
B4D7;HANGUL SYLLABLE DYUC
B4D8;HANGUL SYLLABLE DYUK
B4D9;HANGUL SYLLABLE DYUT
B4DA;HANGUL SYLLABLE DYUP
B4DB;HANGUL SYLLABLE DYUH
B4DC;HANGUL SYLLABLE DEU
B4DD;HANGUL SYLLABLE DEUG
B4DE;HANGUL SYLLABLE DEUGG
B4DF;HANGUL SYLLABLE DEUGS
B4E0;HANGUL SYLLABLE DEUN
B4E1;HANGUL SYLLABLE DEUNJ
B4E2;HANGUL SYLLABLE DEUNH
B4E3;HANGUL SYLLABLE DEUD
B4E4;HANGUL SYLLABLE DEUL
B4E5;HANGUL SYLLABLE DEULG
B4E6;HANGUL SYLLABLE DEULM
B4E7;HANGUL SYLLABLE DEULB
B4E8;HANGUL SYLLABLE DEULS
B4E9;HANGUL SYLLABLE DEULT
B4EA;HANGUL SYLLABLE DEULP
B4EB;HANGUL SYLLABLE DEULH
B4EC;HANGUL SYLLABLE DEUM
B4ED;HANGUL SYLLABLE DEUB
B4EE;HANGUL SYLLABLE DEUBS
B4EF;HANGUL SYLLABLE DEUS
B4F0;HANGUL SYLLABLE DEUSS
B4F1;HANGUL SYLLABLE DEUNG
B4F2;HANGUL SYLLABLE DEUJ
B4F3;HANGUL SYLLABLE DEUC
B4F4;HANGUL SYLLABLE DEUK
B4F5;HANGUL SYLLABLE DEUT
B4F6;HANGUL SYLLABLE DEUP
B4F7;HANGUL SYLLABLE DEUH
B4F8;HANGUL SYLLABLE DYI
B4F9;HANGUL SYLLABLE DYIG
B4FA;HANGUL SYLLABLE DYIGG
B4FB;HANGUL SYLLABLE DYIGS
B4FC;HANGUL SYLLABLE DYIN
B4FD;HANGUL SYLLABLE DYINJ
B4FE;HANGUL SYLLABLE DYINH
B4FF;HANGUL SYLLABLE DYID
B500;HANGUL SYLLABLE DYIL
B501;HANGUL SYLLABLE DYILG
B502;HANGUL SYLLABLE DYILM
B503;HANGUL SYLLABLE DYILB
B504;HANGUL SYLLABLE DYILS
B505;HANGUL SYLLABLE DYILT
B506;HANGUL SYLLABLE DYILP
B507;HANGUL SYLLABLE DYILH
B508;HANGUL SYLLABLE DYIM
B509;HANGUL SYLLABLE DYIB
B50A;HANGUL SYLLABLE DYIBS
B50B;HANGUL SYLLABLE DYIS
B50C;HANGUL SYLLABLE DYISS
B50D;HANGUL SYLLABLE DYING
B50E;HANGUL SYLLABLE DYIJ
B50F;HANGUL SYLLABLE DYIC
B510;HANGUL SYLLABLE DYIK
B511;HANGUL SYLLABLE DYIT
B512;HANGUL SYLLABLE DYIP
B513;HANGUL SYLLABLE DYIH
B514;HANGUL SYLLABLE DI
B515;HANGUL SYLLABLE DIG
B516;HANGUL SYLLABLE DIGG
B517;HANGUL SYLLABLE DIGS
B518;HANGUL SYLLABLE DIN
B519;HANGUL SYLLABLE DINJ
B51A;HANGUL SYLLABLE DINH
B51B;HANGUL SYLLABLE DID
B51C;HANGUL SYLLABLE DIL
B51D;HANGUL SYLLABLE DILG
B51E;HANGUL SYLLABLE DILM
B51F;HANGUL SYLLABLE DILB
B520;HANGUL SYLLABLE DILS
B521;HANGUL SYLLABLE DILT
B522;HANGUL SYLLABLE DILP
B523;HANGUL SYLLABLE DILH
B524;HANGUL SYLLABLE DIM
B525;HANGUL SYLLABLE DIB
B526;HANGUL SYLLABLE DIBS
B527;HANGUL SYLLABLE DIS
B528;HANGUL SYLLABLE DISS
B529;HANGUL SYLLABLE DING
B52A;HANGUL SYLLABLE DIJ
B52B;HANGUL SYLLABLE DIC
B52C;HANGUL SYLLABLE DIK
B52D;HANGUL SYLLABLE DIT
B52E;HANGUL SYLLABLE DIP
B52F;HANGUL SYLLABLE DIH
B530;HANGUL SYLLABLE DDA
B531;HANGUL SYLLABLE DDAG
B532;HANGUL SYLLABLE DDAGG
B533;HANGUL SYLLABLE DDAGS
B534;HANGUL SYLLABLE DDAN
B535;HANGUL SYLLABLE DDANJ
B536;HANGUL SYLLABLE DDANH
B537;HANGUL SYLLABLE DDAD
B538;HANGUL SYLLABLE DDAL
B539;HANGUL SYLLABLE DDALG
B53A;HANGUL SYLLABLE DDALM
B53B;HANGUL SYLLABLE DDALB
B53C;HANGUL SYLLABLE DDALS
B53D;HANGUL SYLLABLE DDALT
B53E;HANGUL SYLLABLE DDALP
B53F;HANGUL SYLLABLE DDALH
B540;HANGUL SYLLABLE DDAM
B541;HANGUL SYLLABLE DDAB
B542;HANGUL SYLLABLE DDABS
B543;HANGUL SYLLABLE DDAS
B544;HANGUL SYLLABLE DDASS
B545;HANGUL SYLLABLE DDANG
B546;HANGUL SYLLABLE DDAJ
B547;HANGUL SYLLABLE DDAC
B548;HANGUL SYLLABLE DDAK
B549;HANGUL SYLLABLE DDAT
B54A;HANGUL SYLLABLE DDAP
B54B;HANGUL SYLLABLE DDAH
B54C;HANGUL SYLLABLE DDAE
B54D;HANGUL SYLLABLE DDAEG
B54E;HANGUL SYLLABLE DDAEGG
B54F;HANGUL SYLLABLE DDAEGS
B550;HANGUL SYLLABLE DDAEN
B551;HANGUL SYLLABLE DDAENJ
B552;HANGUL SYLLABLE DDAENH
B553;HANGUL SYLLABLE DDAED
B554;HANGUL SYLLABLE DDAEL
B555;HANGUL SYLLABLE DDAELG
B556;HANGUL SYLLABLE DDAELM
B557;HANGUL SYLLABLE DDAELB
B558;HANGUL SYLLABLE DDAELS
B559;HANGUL SYLLABLE DDAELT
B55A;HANGUL SYLLABLE DDAELP
B55B;HANGUL SYLLABLE DDAELH
B55C;HANGUL SYLLABLE DDAEM
B55D;HANGUL SYLLABLE DDAEB
B55E;HANGUL SYLLABLE DDAEBS
B55F;HANGUL SYLLABLE DDAES
B560;HANGUL SYLLABLE DDAESS
B561;HANGUL SYLLABLE DDAENG
B562;HANGUL SYLLABLE DDAEJ
B563;HANGUL SYLLABLE DDAEC
B564;HANGUL SYLLABLE DDAEK
B565;HANGUL SYLLABLE DDAET
B566;HANGUL SYLLABLE DDAEP
B567;HANGUL SYLLABLE DDAEH
B568;HANGUL SYLLABLE DDYA
B569;HANGUL SYLLABLE DDYAG
B56A;HANGUL SYLLABLE DDYAGG
B56B;HANGUL SYLLABLE DDYAGS
B56C;HANGUL SYLLABLE DDYAN
B56D;HANGUL SYLLABLE DDYANJ
B56E;HANGUL SYLLABLE DDYANH
B56F;HANGUL SYLLABLE DDYAD
B570;HANGUL SYLLABLE DDYAL
B571;HANGUL SYLLABLE DDYALG
B572;HANGUL SYLLABLE DDYALM
B573;HANGUL SYLLABLE DDYALB
B574;HANGUL SYLLABLE DDYALS
B575;HANGUL SYLLABLE DDYALT
B576;HANGUL SYLLABLE DDYALP
B577;HANGUL SYLLABLE DDYALH
B578;HANGUL SYLLABLE DDYAM
B579;HANGUL SYLLABLE DDYAB
B57A;HANGUL SYLLABLE DDYABS
B57B;HANGUL SYLLABLE DDYAS
B57C;HANGUL SYLLABLE DDYASS
B57D;HANGUL SYLLABLE DDYANG
B57E;HANGUL SYLLABLE DDYAJ
B57F;HANGUL SYLLABLE DDYAC
B580;HANGUL SYLLABLE DDYAK
B581;HANGUL SYLLABLE DDYAT
B582;HANGUL SYLLABLE DDYAP
B583;HANGUL SYLLABLE DDYAH
B584;HANGUL SYLLABLE DDYAE
B585;HANGUL SYLLABLE DDYAEG
B586;HANGUL SYLLABLE DDYAEGG
B587;HANGUL SYLLABLE DDYAEGS
B588;HANGUL SYLLABLE DDYAEN
B589;HANGUL SYLLABLE DDYAENJ
B58A;HANGUL SYLLABLE DDYAENH
B58B;HANGUL SYLLABLE DDYAED
B58C;HANGUL SYLLABLE DDYAEL
B58D;HANGUL SYLLABLE DDYAELG
B58E;HANGUL SYLLABLE DDYAELM
B58F;HANGUL SYLLABLE DDYAELB
B590;HANGUL SYLLABLE DDYAELS
B591;HANGUL SYLLABLE DDYAELT
B592;HANGUL SYLLABLE DDYAELP
B593;HANGUL SYLLABLE DDYAELH
B594;HANGUL SYLLABLE DDYAEM
B595;HANGUL SYLLABLE DDYAEB
B596;HANGUL SYLLABLE DDYAEBS
B597;HANGUL SYLLABLE DDYAES
B598;HANGUL SYLLABLE DDYAESS
B599;HANGUL SYLLABLE DDYAENG
B59A;HANGUL SYLLABLE DDYAEJ
B59B;HANGUL SYLLABLE DDYAEC
B59C;HANGUL SYLLABLE DDYAEK
B59D;HANGUL SYLLABLE DDYAET
B59E;HANGUL SYLLABLE DDYAEP
B59F;HANGUL SYLLABLE DDYAEH
B5A0;HANGUL SYLLABLE DDEO
B5A1;HANGUL SYLLABLE DDEOG
B5A2;HANGUL SYLLABLE DDEOGG
B5A3;HANGUL SYLLABLE DDEOGS
B5A4;HANGUL SYLLABLE DDEON
B5A5;HANGUL SYLLABLE DDEONJ
B5A6;HANGUL SYLLABLE DDEONH
B5A7;HANGUL SYLLABLE DDEOD
B5A8;HANGUL SYLLABLE DDEOL
B5A9;HANGUL SYLLABLE DDEOLG
B5AA;HANGUL SYLLABLE DDEOLM
B5AB;HANGUL SYLLABLE DDEOLB
B5AC;HANGUL SYLLABLE DDEOLS
B5AD;HANGUL SYLLABLE DDEOLT
B5AE;HANGUL SYLLABLE DDEOLP
B5AF;HANGUL SYLLABLE DDEOLH
B5B0;HANGUL SYLLABLE DDEOM
B5B1;HANGUL SYLLABLE DDEOB
B5B2;HANGUL SYLLABLE DDEOBS
B5B3;HANGUL SYLLABLE DDEOS
B5B4;HANGUL SYLLABLE DDEOSS
B5B5;HANGUL SYLLABLE DDEONG
B5B6;HANGUL SYLLABLE DDEOJ
B5B7;HANGUL SYLLABLE DDEOC
B5B8;HANGUL SYLLABLE DDEOK
B5B9;HANGUL SYLLABLE DDEOT
B5BA;HANGUL SYLLABLE DDEOP
B5BB;HANGUL SYLLABLE DDEOH
B5BC;HANGUL SYLLABLE DDE
B5BD;HANGUL SYLLABLE DDEG
B5BE;HANGUL SYLLABLE DDEGG
B5BF;HANGUL SYLLABLE DDEGS
B5C0;HANGUL SYLLABLE DDEN
B5C1;HANGUL SYLLABLE DDENJ
B5C2;HANGUL SYLLABLE DDENH
B5C3;HANGUL SYLLABLE DDED
B5C4;HANGUL SYLLABLE DDEL
B5C5;HANGUL SYLLABLE DDELG
B5C6;HANGUL SYLLABLE DDELM
B5C7;HANGUL SYLLABLE DDELB
B5C8;HANGUL SYLLABLE DDELS
B5C9;HANGUL SYLLABLE DDELT
B5CA;HANGUL SYLLABLE DDELP
B5CB;HANGUL SYLLABLE DDELH
B5CC;HANGUL SYLLABLE DDEM
B5CD;HANGUL SYLLABLE DDEB
B5CE;HANGUL SYLLABLE DDEBS
B5CF;HANGUL SYLLABLE DDES
B5D0;HANGUL SYLLABLE DDESS
B5D1;HANGUL SYLLABLE DDENG
B5D2;HANGUL SYLLABLE DDEJ
B5D3;HANGUL SYLLABLE DDEC
B5D4;HANGUL SYLLABLE DDEK
B5D5;HANGUL SYLLABLE DDET
B5D6;HANGUL SYLLABLE DDEP
B5D7;HANGUL SYLLABLE DDEH
B5D8;HANGUL SYLLABLE DDYEO
B5D9;HANGUL SYLLABLE DDYEOG
B5DA;HANGUL SYLLABLE DDYEOGG
B5DB;HANGUL SYLLABLE DDYEOGS
B5DC;HANGUL SYLLABLE DDYEON
B5DD;HANGUL SYLLABLE DDYEONJ
B5DE;HANGUL SYLLABLE DDYEONH
B5DF;HANGUL SYLLABLE DDYEOD
B5E0;HANGUL SYLLABLE DDYEOL
B5E1;HANGUL SYLLABLE DDYEOLG
B5E2;HANGUL SYLLABLE DDYEOLM
B5E3;HANGUL SYLLABLE DDYEOLB
B5E4;HANGUL SYLLABLE DDYEOLS
B5E5;HANGUL SYLLABLE DDYEOLT
B5E6;HANGUL SYLLABLE DDYEOLP
B5E7;HANGUL SYLLABLE DDYEOLH
B5E8;HANGUL SYLLABLE DDYEOM
B5E9;HANGUL SYLLABLE DDYEOB
B5EA;HANGUL SYLLABLE DDYEOBS
B5EB;HANGUL SYLLABLE DDYEOS
B5EC;HANGUL SYLLABLE DDYEOSS
B5ED;HANGUL SYLLABLE DDYEONG
B5EE;HANGUL SYLLABLE DDYEOJ
B5EF;HANGUL SYLLABLE DDYEOC
B5F0;HANGUL SYLLABLE DDYEOK
B5F1;HANGUL SYLLABLE DDYEOT
B5F2;HANGUL SYLLABLE DDYEOP
B5F3;HANGUL SYLLABLE DDYEOH
B5F4;HANGUL SYLLABLE DDYE
B5F5;HANGUL SYLLABLE DDYEG
B5F6;HANGUL SYLLABLE DDYEGG
B5F7;HANGUL SYLLABLE DDYEGS
B5F8;HANGUL SYLLABLE DDYEN
B5F9;HANGUL SYLLABLE DDYENJ
B5FA;HANGUL SYLLABLE DDYENH
B5FB;HANGUL SYLLABLE DDYED
B5FC;HANGUL SYLLABLE DDYEL
B5FD;HANGUL SYLLABLE DDYELG
B5FE;HANGUL SYLLABLE DDYELM
B5FF;HANGUL SYLLABLE DDYELB
B600;HANGUL SYLLABLE DDYELS
B601;HANGUL SYLLABLE DDYELT
B602;HANGUL SYLLABLE DDYELP
B603;HANGUL SYLLABLE DDYELH
B604;HANGUL SYLLABLE DDYEM
B605;HANGUL SYLLABLE DDYEB
B606;HANGUL SYLLABLE DDYEBS
B607;HANGUL SYLLABLE DDYES
B608;HANGUL SYLLABLE DDYESS
B609;HANGUL SYLLABLE DDYENG
B60A;HANGUL SYLLABLE DDYEJ
B60B;HANGUL SYLLABLE DDYEC
B60C;HANGUL SYLLABLE DDYEK
B60D;HANGUL SYLLABLE DDYET
B60E;HANGUL SYLLABLE DDYEP
B60F;HANGUL SYLLABLE DDYEH
B610;HANGUL SYLLABLE DDO
B611;HANGUL SYLLABLE DDOG
B612;HANGUL SYLLABLE DDOGG
B613;HANGUL SYLLABLE DDOGS
B614;HANGUL SYLLABLE DDON
B615;HANGUL SYLLABLE DDONJ
B616;HANGUL SYLLABLE DDONH
B617;HANGUL SYLLABLE DDOD
B618;HANGUL SYLLABLE DDOL
B619;HANGUL SYLLABLE DDOLG
B61A;HANGUL SYLLABLE DDOLM
B61B;HANGUL SYLLABLE DDOLB
B61C;HANGUL SYLLABLE DDOLS
B61D;HANGUL SYLLABLE DDOLT
B61E;HANGUL SYLLABLE DDOLP
B61F;HANGUL SYLLABLE DDOLH
B620;HANGUL SYLLABLE DDOM
B621;HANGUL SYLLABLE DDOB
B622;HANGUL SYLLABLE DDOBS
B623;HANGUL SYLLABLE DDOS
B624;HANGUL SYLLABLE DDOSS
B625;HANGUL SYLLABLE DDONG
B626;HANGUL SYLLABLE DDOJ
B627;HANGUL SYLLABLE DDOC
B628;HANGUL SYLLABLE DDOK
B629;HANGUL SYLLABLE DDOT
B62A;HANGUL SYLLABLE DDOP
B62B;HANGUL SYLLABLE DDOH
B62C;HANGUL SYLLABLE DDWA
B62D;HANGUL SYLLABLE DDWAG
B62E;HANGUL SYLLABLE DDWAGG
B62F;HANGUL SYLLABLE DDWAGS
B630;HANGUL SYLLABLE DDWAN
B631;HANGUL SYLLABLE DDWANJ
B632;HANGUL SYLLABLE DDWANH
B633;HANGUL SYLLABLE DDWAD
B634;HANGUL SYLLABLE DDWAL
B635;HANGUL SYLLABLE DDWALG
B636;HANGUL SYLLABLE DDWALM
B637;HANGUL SYLLABLE DDWALB
B638;HANGUL SYLLABLE DDWALS
B639;HANGUL SYLLABLE DDWALT
B63A;HANGUL SYLLABLE DDWALP
B63B;HANGUL SYLLABLE DDWALH
B63C;HANGUL SYLLABLE DDWAM
B63D;HANGUL SYLLABLE DDWAB
B63E;HANGUL SYLLABLE DDWABS
B63F;HANGUL SYLLABLE DDWAS
B640;HANGUL SYLLABLE DDWASS
B641;HANGUL SYLLABLE DDWANG
B642;HANGUL SYLLABLE DDWAJ
B643;HANGUL SYLLABLE DDWAC
B644;HANGUL SYLLABLE DDWAK
B645;HANGUL SYLLABLE DDWAT
B646;HANGUL SYLLABLE DDWAP
B647;HANGUL SYLLABLE DDWAH
B648;HANGUL SYLLABLE DDWAE
B649;HANGUL SYLLABLE DDWAEG
B64A;HANGUL SYLLABLE DDWAEGG
B64B;HANGUL SYLLABLE DDWAEGS
B64C;HANGUL SYLLABLE DDWAEN
B64D;HANGUL SYLLABLE DDWAENJ
B64E;HANGUL SYLLABLE DDWAENH
B64F;HANGUL SYLLABLE DDWAED
B650;HANGUL SYLLABLE DDWAEL
B651;HANGUL SYLLABLE DDWAELG
B652;HANGUL SYLLABLE DDWAELM
B653;HANGUL SYLLABLE DDWAELB
B654;HANGUL SYLLABLE DDWAELS
B655;HANGUL SYLLABLE DDWAELT
B656;HANGUL SYLLABLE DDWAELP
B657;HANGUL SYLLABLE DDWAELH
B658;HANGUL SYLLABLE DDWAEM
B659;HANGUL SYLLABLE DDWAEB
B65A;HANGUL SYLLABLE DDWAEBS
B65B;HANGUL SYLLABLE DDWAES
B65C;HANGUL SYLLABLE DDWAESS
B65D;HANGUL SYLLABLE DDWAENG
B65E;HANGUL SYLLABLE DDWAEJ
B65F;HANGUL SYLLABLE DDWAEC
B660;HANGUL SYLLABLE DDWAEK
B661;HANGUL SYLLABLE DDWAET
B662;HANGUL SYLLABLE DDWAEP
B663;HANGUL SYLLABLE DDWAEH
B664;HANGUL SYLLABLE DDOE
B665;HANGUL SYLLABLE DDOEG
B666;HANGUL SYLLABLE DDOEGG
B667;HANGUL SYLLABLE DDOEGS
B668;HANGUL SYLLABLE DDOEN
B669;HANGUL SYLLABLE DDOENJ
B66A;HANGUL SYLLABLE DDOENH
B66B;HANGUL SYLLABLE DDOED
B66C;HANGUL SYLLABLE DDOEL
B66D;HANGUL SYLLABLE DDOELG
B66E;HANGUL SYLLABLE DDOELM
B66F;HANGUL SYLLABLE DDOELB
B670;HANGUL SYLLABLE DDOELS
B671;HANGUL SYLLABLE DDOELT
B672;HANGUL SYLLABLE DDOELP
B673;HANGUL SYLLABLE DDOELH
B674;HANGUL SYLLABLE DDOEM
B675;HANGUL SYLLABLE DDOEB
B676;HANGUL SYLLABLE DDOEBS
B677;HANGUL SYLLABLE DDOES
B678;HANGUL SYLLABLE DDOESS
B679;HANGUL SYLLABLE DDOENG
B67A;HANGUL SYLLABLE DDOEJ
B67B;HANGUL SYLLABLE DDOEC
B67C;HANGUL SYLLABLE DDOEK
B67D;HANGUL SYLLABLE DDOET
B67E;HANGUL SYLLABLE DDOEP
B67F;HANGUL SYLLABLE DDOEH
B680;HANGUL SYLLABLE DDYO
B681;HANGUL SYLLABLE DDYOG
B682;HANGUL SYLLABLE DDYOGG
B683;HANGUL SYLLABLE DDYOGS
B684;HANGUL SYLLABLE DDYON
B685;HANGUL SYLLABLE DDYONJ
B686;HANGUL SYLLABLE DDYONH
B687;HANGUL SYLLABLE DDYOD
B688;HANGUL SYLLABLE DDYOL
B689;HANGUL SYLLABLE DDYOLG
B68A;HANGUL SYLLABLE DDYOLM
B68B;HANGUL SYLLABLE DDYOLB
B68C;HANGUL SYLLABLE DDYOLS
B68D;HANGUL SYLLABLE DDYOLT
B68E;HANGUL SYLLABLE DDYOLP
B68F;HANGUL SYLLABLE DDYOLH
B690;HANGUL SYLLABLE DDYOM
B691;HANGUL SYLLABLE DDYOB
B692;HANGUL SYLLABLE DDYOBS
B693;HANGUL SYLLABLE DDYOS
B694;HANGUL SYLLABLE DDYOSS
B695;HANGUL SYLLABLE DDYONG
B696;HANGUL SYLLABLE DDYOJ
B697;HANGUL SYLLABLE DDYOC
B698;HANGUL SYLLABLE DDYOK
B699;HANGUL SYLLABLE DDYOT
B69A;HANGUL SYLLABLE DDYOP
B69B;HANGUL SYLLABLE DDYOH
B69C;HANGUL SYLLABLE DDU
B69D;HANGUL SYLLABLE DDUG
B69E;HANGUL SYLLABLE DDUGG
B69F;HANGUL SYLLABLE DDUGS
B6A0;HANGUL SYLLABLE DDUN
B6A1;HANGUL SYLLABLE DDUNJ
B6A2;HANGUL SYLLABLE DDUNH
B6A3;HANGUL SYLLABLE DDUD
B6A4;HANGUL SYLLABLE DDUL
B6A5;HANGUL SYLLABLE DDULG
B6A6;HANGUL SYLLABLE DDULM
B6A7;HANGUL SYLLABLE DDULB
B6A8;HANGUL SYLLABLE DDULS
B6A9;HANGUL SYLLABLE DDULT
B6AA;HANGUL SYLLABLE DDULP
B6AB;HANGUL SYLLABLE DDULH
B6AC;HANGUL SYLLABLE DDUM
B6AD;HANGUL SYLLABLE DDUB
B6AE;HANGUL SYLLABLE DDUBS
B6AF;HANGUL SYLLABLE DDUS
B6B0;HANGUL SYLLABLE DDUSS
B6B1;HANGUL SYLLABLE DDUNG
B6B2;HANGUL SYLLABLE DDUJ
B6B3;HANGUL SYLLABLE DDUC
B6B4;HANGUL SYLLABLE DDUK
B6B5;HANGUL SYLLABLE DDUT
B6B6;HANGUL SYLLABLE DDUP
B6B7;HANGUL SYLLABLE DDUH
B6B8;HANGUL SYLLABLE DDWEO
B6B9;HANGUL SYLLABLE DDWEOG
B6BA;HANGUL SYLLABLE DDWEOGG
B6BB;HANGUL SYLLABLE DDWEOGS
B6BC;HANGUL SYLLABLE DDWEON
B6BD;HANGUL SYLLABLE DDWEONJ
B6BE;HANGUL SYLLABLE DDWEONH
B6BF;HANGUL SYLLABLE DDWEOD
B6C0;HANGUL SYLLABLE DDWEOL
B6C1;HANGUL SYLLABLE DDWEOLG
B6C2;HANGUL SYLLABLE DDWEOLM
B6C3;HANGUL SYLLABLE DDWEOLB
B6C4;HANGUL SYLLABLE DDWEOLS
B6C5;HANGUL SYLLABLE DDWEOLT
B6C6;HANGUL SYLLABLE DDWEOLP
B6C7;HANGUL SYLLABLE DDWEOLH
B6C8;HANGUL SYLLABLE DDWEOM
B6C9;HANGUL SYLLABLE DDWEOB
B6CA;HANGUL SYLLABLE DDWEOBS
B6CB;HANGUL SYLLABLE DDWEOS
B6CC;HANGUL SYLLABLE DDWEOSS
B6CD;HANGUL SYLLABLE DDWEONG
B6CE;HANGUL SYLLABLE DDWEOJ
B6CF;HANGUL SYLLABLE DDWEOC
B6D0;HANGUL SYLLABLE DDWEOK
B6D1;HANGUL SYLLABLE DDWEOT
B6D2;HANGUL SYLLABLE DDWEOP
B6D3;HANGUL SYLLABLE DDWEOH
B6D4;HANGUL SYLLABLE DDWE
B6D5;HANGUL SYLLABLE DDWEG
B6D6;HANGUL SYLLABLE DDWEGG
B6D7;HANGUL SYLLABLE DDWEGS
B6D8;HANGUL SYLLABLE DDWEN
B6D9;HANGUL SYLLABLE DDWENJ
B6DA;HANGUL SYLLABLE DDWENH
B6DB;HANGUL SYLLABLE DDWED
B6DC;HANGUL SYLLABLE DDWEL
B6DD;HANGUL SYLLABLE DDWELG
B6DE;HANGUL SYLLABLE DDWELM
B6DF;HANGUL SYLLABLE DDWELB
B6E0;HANGUL SYLLABLE DDWELS
B6E1;HANGUL SYLLABLE DDWELT
B6E2;HANGUL SYLLABLE DDWELP
B6E3;HANGUL SYLLABLE DDWELH
B6E4;HANGUL SYLLABLE DDWEM
B6E5;HANGUL SYLLABLE DDWEB
B6E6;HANGUL SYLLABLE DDWEBS
B6E7;HANGUL SYLLABLE DDWES
B6E8;HANGUL SYLLABLE DDWESS
B6E9;HANGUL SYLLABLE DDWENG
B6EA;HANGUL SYLLABLE DDWEJ
B6EB;HANGUL SYLLABLE DDWEC
B6EC;HANGUL SYLLABLE DDWEK
B6ED;HANGUL SYLLABLE DDWET
B6EE;HANGUL SYLLABLE DDWEP
B6EF;HANGUL SYLLABLE DDWEH
B6F0;HANGUL SYLLABLE DDWI
B6F1;HANGUL SYLLABLE DDWIG
B6F2;HANGUL SYLLABLE DDWIGG
B6F3;HANGUL SYLLABLE DDWIGS
B6F4;HANGUL SYLLABLE DDWIN
B6F5;HANGUL SYLLABLE DDWINJ
B6F6;HANGUL SYLLABLE DDWINH
B6F7;HANGUL SYLLABLE DDWID
B6F8;HANGUL SYLLABLE DDWIL
B6F9;HANGUL SYLLABLE DDWILG
B6FA;HANGUL SYLLABLE DDWILM
B6FB;HANGUL SYLLABLE DDWILB
B6FC;HANGUL SYLLABLE DDWILS
B6FD;HANGUL SYLLABLE DDWILT
B6FE;HANGUL SYLLABLE DDWILP
B6FF;HANGUL SYLLABLE DDWILH
B700;HANGUL SYLLABLE DDWIM
B701;HANGUL SYLLABLE DDWIB
B702;HANGUL SYLLABLE DDWIBS
B703;HANGUL SYLLABLE DDWIS
B704;HANGUL SYLLABLE DDWISS
B705;HANGUL SYLLABLE DDWING
B706;HANGUL SYLLABLE DDWIJ
B707;HANGUL SYLLABLE DDWIC
B708;HANGUL SYLLABLE DDWIK
B709;HANGUL SYLLABLE DDWIT
B70A;HANGUL SYLLABLE DDWIP
B70B;HANGUL SYLLABLE DDWIH
B70C;HANGUL SYLLABLE DDYU
B70D;HANGUL SYLLABLE DDYUG
B70E;HANGUL SYLLABLE DDYUGG
B70F;HANGUL SYLLABLE DDYUGS
B710;HANGUL SYLLABLE DDYUN
B711;HANGUL SYLLABLE DDYUNJ
B712;HANGUL SYLLABLE DDYUNH
B713;HANGUL SYLLABLE DDYUD
B714;HANGUL SYLLABLE DDYUL
B715;HANGUL SYLLABLE DDYULG
B716;HANGUL SYLLABLE DDYULM
B717;HANGUL SYLLABLE DDYULB
B718;HANGUL SYLLABLE DDYULS
B719;HANGUL SYLLABLE DDYULT
B71A;HANGUL SYLLABLE DDYULP
B71B;HANGUL SYLLABLE DDYULH
B71C;HANGUL SYLLABLE DDYUM
B71D;HANGUL SYLLABLE DDYUB
B71E;HANGUL SYLLABLE DDYUBS
B71F;HANGUL SYLLABLE DDYUS
B720;HANGUL SYLLABLE DDYUSS
B721;HANGUL SYLLABLE DDYUNG
B722;HANGUL SYLLABLE DDYUJ
B723;HANGUL SYLLABLE DDYUC
B724;HANGUL SYLLABLE DDYUK
B725;HANGUL SYLLABLE DDYUT
B726;HANGUL SYLLABLE DDYUP
B727;HANGUL SYLLABLE DDYUH
B728;HANGUL SYLLABLE DDEU
B729;HANGUL SYLLABLE DDEUG
B72A;HANGUL SYLLABLE DDEUGG
B72B;HANGUL SYLLABLE DDEUGS
B72C;HANGUL SYLLABLE DDEUN
B72D;HANGUL SYLLABLE DDEUNJ
B72E;HANGUL SYLLABLE DDEUNH
B72F;HANGUL SYLLABLE DDEUD
B730;HANGUL SYLLABLE DDEUL
B731;HANGUL SYLLABLE DDEULG
B732;HANGUL SYLLABLE DDEULM
B733;HANGUL SYLLABLE DDEULB
B734;HANGUL SYLLABLE DDEULS
B735;HANGUL SYLLABLE DDEULT
B736;HANGUL SYLLABLE DDEULP
B737;HANGUL SYLLABLE DDEULH
B738;HANGUL SYLLABLE DDEUM
B739;HANGUL SYLLABLE DDEUB
B73A;HANGUL SYLLABLE DDEUBS
B73B;HANGUL SYLLABLE DDEUS
B73C;HANGUL SYLLABLE DDEUSS
B73D;HANGUL SYLLABLE DDEUNG
B73E;HANGUL SYLLABLE DDEUJ
B73F;HANGUL SYLLABLE DDEUC
B740;HANGUL SYLLABLE DDEUK
B741;HANGUL SYLLABLE DDEUT
B742;HANGUL SYLLABLE DDEUP
B743;HANGUL SYLLABLE DDEUH
B744;HANGUL SYLLABLE DDYI
B745;HANGUL SYLLABLE DDYIG
B746;HANGUL SYLLABLE DDYIGG
B747;HANGUL SYLLABLE DDYIGS
B748;HANGUL SYLLABLE DDYIN
B749;HANGUL SYLLABLE DDYINJ
B74A;HANGUL SYLLABLE DDYINH
B74B;HANGUL SYLLABLE DDYID
B74C;HANGUL SYLLABLE DDYIL
B74D;HANGUL SYLLABLE DDYILG
B74E;HANGUL SYLLABLE DDYILM
B74F;HANGUL SYLLABLE DDYILB
B750;HANGUL SYLLABLE DDYILS
B751;HANGUL SYLLABLE DDYILT
B752;HANGUL SYLLABLE DDYILP
B753;HANGUL SYLLABLE DDYILH
B754;HANGUL SYLLABLE DDYIM
B755;HANGUL SYLLABLE DDYIB
B756;HANGUL SYLLABLE DDYIBS
B757;HANGUL SYLLABLE DDYIS
B758;HANGUL SYLLABLE DDYISS
B759;HANGUL SYLLABLE DDYING
B75A;HANGUL SYLLABLE DDYIJ
B75B;HANGUL SYLLABLE DDYIC
B75C;HANGUL SYLLABLE DDYIK
B75D;HANGUL SYLLABLE DDYIT
B75E;HANGUL SYLLABLE DDYIP
B75F;HANGUL SYLLABLE DDYIH
B760;HANGUL SYLLABLE DDI
B761;HANGUL SYLLABLE DDIG
B762;HANGUL SYLLABLE DDIGG
B763;HANGUL SYLLABLE DDIGS
B764;HANGUL SYLLABLE DDIN
B765;HANGUL SYLLABLE DDINJ
B766;HANGUL SYLLABLE DDINH
B767;HANGUL SYLLABLE DDID
B768;HANGUL SYLLABLE DDIL
B769;HANGUL SYLLABLE DDILG
B76A;HANGUL SYLLABLE DDILM
B76B;HANGUL SYLLABLE DDILB
B76C;HANGUL SYLLABLE DDILS
B76D;HANGUL SYLLABLE DDILT
B76E;HANGUL SYLLABLE DDILP
B76F;HANGUL SYLLABLE DDILH
B770;HANGUL SYLLABLE DDIM
B771;HANGUL SYLLABLE DDIB
B772;HANGUL SYLLABLE DDIBS
B773;HANGUL SYLLABLE DDIS
B774;HANGUL SYLLABLE DDISS
B775;HANGUL SYLLABLE DDING
B776;HANGUL SYLLABLE DDIJ
B777;HANGUL SYLLABLE DDIC
B778;HANGUL SYLLABLE DDIK
B779;HANGUL SYLLABLE DDIT
B77A;HANGUL SYLLABLE DDIP
B77B;HANGUL SYLLABLE DDIH
B77C;HANGUL SYLLABLE RA
B77D;HANGUL SYLLABLE RAG
B77E;HANGUL SYLLABLE RAGG
B77F;HANGUL SYLLABLE RAGS
B780;HANGUL SYLLABLE RAN
B781;HANGUL SYLLABLE RANJ
B782;HANGUL SYLLABLE RANH
B783;HANGUL SYLLABLE RAD
B784;HANGUL SYLLABLE RAL
B785;HANGUL SYLLABLE RALG
B786;HANGUL SYLLABLE RALM
B787;HANGUL SYLLABLE RALB
B788;HANGUL SYLLABLE RALS
B789;HANGUL SYLLABLE RALT
B78A;HANGUL SYLLABLE RALP
B78B;HANGUL SYLLABLE RALH
B78C;HANGUL SYLLABLE RAM
B78D;HANGUL SYLLABLE RAB
B78E;HANGUL SYLLABLE RABS
B78F;HANGUL SYLLABLE RAS
B790;HANGUL SYLLABLE RASS
B791;HANGUL SYLLABLE RANG
B792;HANGUL SYLLABLE RAJ
B793;HANGUL SYLLABLE RAC
B794;HANGUL SYLLABLE RAK
B795;HANGUL SYLLABLE RAT
B796;HANGUL SYLLABLE RAP
B797;HANGUL SYLLABLE RAH
B798;HANGUL SYLLABLE RAE
B799;HANGUL SYLLABLE RAEG
B79A;HANGUL SYLLABLE RAEGG
B79B;HANGUL SYLLABLE RAEGS
B79C;HANGUL SYLLABLE RAEN
B79D;HANGUL SYLLABLE RAENJ
B79E;HANGUL SYLLABLE RAENH
B79F;HANGUL SYLLABLE RAED
B7A0;HANGUL SYLLABLE RAEL
B7A1;HANGUL SYLLABLE RAELG
B7A2;HANGUL SYLLABLE RAELM
B7A3;HANGUL SYLLABLE RAELB
B7A4;HANGUL SYLLABLE RAELS
B7A5;HANGUL SYLLABLE RAELT
B7A6;HANGUL SYLLABLE RAELP
B7A7;HANGUL SYLLABLE RAELH
B7A8;HANGUL SYLLABLE RAEM
B7A9;HANGUL SYLLABLE RAEB
B7AA;HANGUL SYLLABLE RAEBS
B7AB;HANGUL SYLLABLE RAES
B7AC;HANGUL SYLLABLE RAESS
B7AD;HANGUL SYLLABLE RAENG
B7AE;HANGUL SYLLABLE RAEJ
B7AF;HANGUL SYLLABLE RAEC
B7B0;HANGUL SYLLABLE RAEK
B7B1;HANGUL SYLLABLE RAET
B7B2;HANGUL SYLLABLE RAEP
B7B3;HANGUL SYLLABLE RAEH
B7B4;HANGUL SYLLABLE RYA
B7B5;HANGUL SYLLABLE RYAG
B7B6;HANGUL SYLLABLE RYAGG
B7B7;HANGUL SYLLABLE RYAGS
B7B8;HANGUL SYLLABLE RYAN
B7B9;HANGUL SYLLABLE RYANJ
B7BA;HANGUL SYLLABLE RYANH
B7BB;HANGUL SYLLABLE RYAD
B7BC;HANGUL SYLLABLE RYAL
B7BD;HANGUL SYLLABLE RYALG
B7BE;HANGUL SYLLABLE RYALM
B7BF;HANGUL SYLLABLE RYALB
B7C0;HANGUL SYLLABLE RYALS
B7C1;HANGUL SYLLABLE RYALT
B7C2;HANGUL SYLLABLE RYALP
B7C3;HANGUL SYLLABLE RYALH
B7C4;HANGUL SYLLABLE RYAM
B7C5;HANGUL SYLLABLE RYAB
B7C6;HANGUL SYLLABLE RYABS
B7C7;HANGUL SYLLABLE RYAS
B7C8;HANGUL SYLLABLE RYASS
B7C9;HANGUL SYLLABLE RYANG
B7CA;HANGUL SYLLABLE RYAJ
B7CB;HANGUL SYLLABLE RYAC
B7CC;HANGUL SYLLABLE RYAK
B7CD;HANGUL SYLLABLE RYAT
B7CE;HANGUL SYLLABLE RYAP
B7CF;HANGUL SYLLABLE RYAH
B7D0;HANGUL SYLLABLE RYAE
B7D1;HANGUL SYLLABLE RYAEG
B7D2;HANGUL SYLLABLE RYAEGG
B7D3;HANGUL SYLLABLE RYAEGS
B7D4;HANGUL SYLLABLE RYAEN
B7D5;HANGUL SYLLABLE RYAENJ
B7D6;HANGUL SYLLABLE RYAENH
B7D7;HANGUL SYLLABLE RYAED
B7D8;HANGUL SYLLABLE RYAEL
B7D9;HANGUL SYLLABLE RYAELG
B7DA;HANGUL SYLLABLE RYAELM
B7DB;HANGUL SYLLABLE RYAELB
B7DC;HANGUL SYLLABLE RYAELS
B7DD;HANGUL SYLLABLE RYAELT
B7DE;HANGUL SYLLABLE RYAELP
B7DF;HANGUL SYLLABLE RYAELH
B7E0;HANGUL SYLLABLE RYAEM
B7E1;HANGUL SYLLABLE RYAEB
B7E2;HANGUL SYLLABLE RYAEBS
B7E3;HANGUL SYLLABLE RYAES
B7E4;HANGUL SYLLABLE RYAESS
B7E5;HANGUL SYLLABLE RYAENG
B7E6;HANGUL SYLLABLE RYAEJ
B7E7;HANGUL SYLLABLE RYAEC
B7E8;HANGUL SYLLABLE RYAEK
B7E9;HANGUL SYLLABLE RYAET
B7EA;HANGUL SYLLABLE RYAEP
B7EB;HANGUL SYLLABLE RYAEH
B7EC;HANGUL SYLLABLE REO
B7ED;HANGUL SYLLABLE REOG
B7EE;HANGUL SYLLABLE REOGG
B7EF;HANGUL SYLLABLE REOGS
B7F0;HANGUL SYLLABLE REON
B7F1;HANGUL SYLLABLE REONJ
B7F2;HANGUL SYLLABLE REONH
B7F3;HANGUL SYLLABLE REOD
B7F4;HANGUL SYLLABLE REOL
B7F5;HANGUL SYLLABLE REOLG
B7F6;HANGUL SYLLABLE REOLM
B7F7;HANGUL SYLLABLE REOLB
B7F8;HANGUL SYLLABLE REOLS
B7F9;HANGUL SYLLABLE REOLT
B7FA;HANGUL SYLLABLE REOLP
B7FB;HANGUL SYLLABLE REOLH
B7FC;HANGUL SYLLABLE REOM
B7FD;HANGUL SYLLABLE REOB
B7FE;HANGUL SYLLABLE REOBS
B7FF;HANGUL SYLLABLE REOS
B800;HANGUL SYLLABLE REOSS
B801;HANGUL SYLLABLE REONG
B802;HANGUL SYLLABLE REOJ
B803;HANGUL SYLLABLE REOC
B804;HANGUL SYLLABLE REOK
B805;HANGUL SYLLABLE REOT
B806;HANGUL SYLLABLE REOP
B807;HANGUL SYLLABLE REOH
B808;HANGUL SYLLABLE RE
B809;HANGUL SYLLABLE REG
B80A;HANGUL SYLLABLE REGG
B80B;HANGUL SYLLABLE REGS
B80C;HANGUL SYLLABLE REN
B80D;HANGUL SYLLABLE RENJ
B80E;HANGUL SYLLABLE RENH
B80F;HANGUL SYLLABLE RED
B810;HANGUL SYLLABLE REL
B811;HANGUL SYLLABLE RELG
B812;HANGUL SYLLABLE RELM
B813;HANGUL SYLLABLE RELB
B814;HANGUL SYLLABLE RELS
B815;HANGUL SYLLABLE RELT
B816;HANGUL SYLLABLE RELP
B817;HANGUL SYLLABLE RELH
B818;HANGUL SYLLABLE REM
B819;HANGUL SYLLABLE REB
B81A;HANGUL SYLLABLE REBS
B81B;HANGUL SYLLABLE RES
B81C;HANGUL SYLLABLE RESS
B81D;HANGUL SYLLABLE RENG
B81E;HANGUL SYLLABLE REJ
B81F;HANGUL SYLLABLE REC
B820;HANGUL SYLLABLE REK
B821;HANGUL SYLLABLE RET
B822;HANGUL SYLLABLE REP
B823;HANGUL SYLLABLE REH
B824;HANGUL SYLLABLE RYEO
B825;HANGUL SYLLABLE RYEOG
B826;HANGUL SYLLABLE RYEOGG
B827;HANGUL SYLLABLE RYEOGS
B828;HANGUL SYLLABLE RYEON
B829;HANGUL SYLLABLE RYEONJ
B82A;HANGUL SYLLABLE RYEONH
B82B;HANGUL SYLLABLE RYEOD
B82C;HANGUL SYLLABLE RYEOL
B82D;HANGUL SYLLABLE RYEOLG
B82E;HANGUL SYLLABLE RYEOLM
B82F;HANGUL SYLLABLE RYEOLB
B830;HANGUL SYLLABLE RYEOLS
B831;HANGUL SYLLABLE RYEOLT
B832;HANGUL SYLLABLE RYEOLP
B833;HANGUL SYLLABLE RYEOLH
B834;HANGUL SYLLABLE RYEOM
B835;HANGUL SYLLABLE RYEOB
B836;HANGUL SYLLABLE RYEOBS
B837;HANGUL SYLLABLE RYEOS
B838;HANGUL SYLLABLE RYEOSS
B839;HANGUL SYLLABLE RYEONG
B83A;HANGUL SYLLABLE RYEOJ
B83B;HANGUL SYLLABLE RYEOC
B83C;HANGUL SYLLABLE RYEOK
B83D;HANGUL SYLLABLE RYEOT
B83E;HANGUL SYLLABLE RYEOP
B83F;HANGUL SYLLABLE RYEOH
B840;HANGUL SYLLABLE RYE
B841;HANGUL SYLLABLE RYEG
B842;HANGUL SYLLABLE RYEGG
B843;HANGUL SYLLABLE RYEGS
B844;HANGUL SYLLABLE RYEN
B845;HANGUL SYLLABLE RYENJ
B846;HANGUL SYLLABLE RYENH
B847;HANGUL SYLLABLE RYED
B848;HANGUL SYLLABLE RYEL
B849;HANGUL SYLLABLE RYELG
B84A;HANGUL SYLLABLE RYELM
B84B;HANGUL SYLLABLE RYELB
B84C;HANGUL SYLLABLE RYELS
B84D;HANGUL SYLLABLE RYELT
B84E;HANGUL SYLLABLE RYELP
B84F;HANGUL SYLLABLE RYELH
B850;HANGUL SYLLABLE RYEM
B851;HANGUL SYLLABLE RYEB
B852;HANGUL SYLLABLE RYEBS
B853;HANGUL SYLLABLE RYES
B854;HANGUL SYLLABLE RYESS
B855;HANGUL SYLLABLE RYENG
B856;HANGUL SYLLABLE RYEJ
B857;HANGUL SYLLABLE RYEC
B858;HANGUL SYLLABLE RYEK
B859;HANGUL SYLLABLE RYET
B85A;HANGUL SYLLABLE RYEP
B85B;HANGUL SYLLABLE RYEH
B85C;HANGUL SYLLABLE RO
B85D;HANGUL SYLLABLE ROG
B85E;HANGUL SYLLABLE ROGG
B85F;HANGUL SYLLABLE ROGS
B860;HANGUL SYLLABLE RON
B861;HANGUL SYLLABLE RONJ
B862;HANGUL SYLLABLE RONH
B863;HANGUL SYLLABLE ROD
B864;HANGUL SYLLABLE ROL
B865;HANGUL SYLLABLE ROLG
B866;HANGUL SYLLABLE ROLM
B867;HANGUL SYLLABLE ROLB
B868;HANGUL SYLLABLE ROLS
B869;HANGUL SYLLABLE ROLT
B86A;HANGUL SYLLABLE ROLP
B86B;HANGUL SYLLABLE ROLH
B86C;HANGUL SYLLABLE ROM
B86D;HANGUL SYLLABLE ROB
B86E;HANGUL SYLLABLE ROBS
B86F;HANGUL SYLLABLE ROS
B870;HANGUL SYLLABLE ROSS
B871;HANGUL SYLLABLE RONG
B872;HANGUL SYLLABLE ROJ
B873;HANGUL SYLLABLE ROC
B874;HANGUL SYLLABLE ROK
B875;HANGUL SYLLABLE ROT
B876;HANGUL SYLLABLE ROP
B877;HANGUL SYLLABLE ROH
B878;HANGUL SYLLABLE RWA
B879;HANGUL SYLLABLE RWAG
B87A;HANGUL SYLLABLE RWAGG
B87B;HANGUL SYLLABLE RWAGS
B87C;HANGUL SYLLABLE RWAN
B87D;HANGUL SYLLABLE RWANJ
B87E;HANGUL SYLLABLE RWANH
B87F;HANGUL SYLLABLE RWAD
B880;HANGUL SYLLABLE RWAL
B881;HANGUL SYLLABLE RWALG
B882;HANGUL SYLLABLE RWALM
B883;HANGUL SYLLABLE RWALB
B884;HANGUL SYLLABLE RWALS
B885;HANGUL SYLLABLE RWALT
B886;HANGUL SYLLABLE RWALP
B887;HANGUL SYLLABLE RWALH
B888;HANGUL SYLLABLE RWAM
B889;HANGUL SYLLABLE RWAB
B88A;HANGUL SYLLABLE RWABS
B88B;HANGUL SYLLABLE RWAS
B88C;HANGUL SYLLABLE RWASS
B88D;HANGUL SYLLABLE RWANG
B88E;HANGUL SYLLABLE RWAJ
B88F;HANGUL SYLLABLE RWAC
B890;HANGUL SYLLABLE RWAK
B891;HANGUL SYLLABLE RWAT
B892;HANGUL SYLLABLE RWAP
B893;HANGUL SYLLABLE RWAH
B894;HANGUL SYLLABLE RWAE
B895;HANGUL SYLLABLE RWAEG
B896;HANGUL SYLLABLE RWAEGG
B897;HANGUL SYLLABLE RWAEGS
B898;HANGUL SYLLABLE RWAEN
B899;HANGUL SYLLABLE RWAENJ
B89A;HANGUL SYLLABLE RWAENH
B89B;HANGUL SYLLABLE RWAED
B89C;HANGUL SYLLABLE RWAEL
B89D;HANGUL SYLLABLE RWAELG
B89E;HANGUL SYLLABLE RWAELM
B89F;HANGUL SYLLABLE RWAELB
B8A0;HANGUL SYLLABLE RWAELS
B8A1;HANGUL SYLLABLE RWAELT
B8A2;HANGUL SYLLABLE RWAELP
B8A3;HANGUL SYLLABLE RWAELH
B8A4;HANGUL SYLLABLE RWAEM
B8A5;HANGUL SYLLABLE RWAEB
B8A6;HANGUL SYLLABLE RWAEBS
B8A7;HANGUL SYLLABLE RWAES
B8A8;HANGUL SYLLABLE RWAESS
B8A9;HANGUL SYLLABLE RWAENG
B8AA;HANGUL SYLLABLE RWAEJ
B8AB;HANGUL SYLLABLE RWAEC
B8AC;HANGUL SYLLABLE RWAEK
B8AD;HANGUL SYLLABLE RWAET
B8AE;HANGUL SYLLABLE RWAEP
B8AF;HANGUL SYLLABLE RWAEH
B8B0;HANGUL SYLLABLE ROE
B8B1;HANGUL SYLLABLE ROEG
B8B2;HANGUL SYLLABLE ROEGG
B8B3;HANGUL SYLLABLE ROEGS
B8B4;HANGUL SYLLABLE ROEN
B8B5;HANGUL SYLLABLE ROENJ
B8B6;HANGUL SYLLABLE ROENH
B8B7;HANGUL SYLLABLE ROED
B8B8;HANGUL SYLLABLE ROEL
B8B9;HANGUL SYLLABLE ROELG
B8BA;HANGUL SYLLABLE ROELM
B8BB;HANGUL SYLLABLE ROELB
B8BC;HANGUL SYLLABLE ROELS
B8BD;HANGUL SYLLABLE ROELT
B8BE;HANGUL SYLLABLE ROELP
B8BF;HANGUL SYLLABLE ROELH
B8C0;HANGUL SYLLABLE ROEM
B8C1;HANGUL SYLLABLE ROEB
B8C2;HANGUL SYLLABLE ROEBS
B8C3;HANGUL SYLLABLE ROES
B8C4;HANGUL SYLLABLE ROESS
B8C5;HANGUL SYLLABLE ROENG
B8C6;HANGUL SYLLABLE ROEJ
B8C7;HANGUL SYLLABLE ROEC
B8C8;HANGUL SYLLABLE ROEK
B8C9;HANGUL SYLLABLE ROET
B8CA;HANGUL SYLLABLE ROEP
B8CB;HANGUL SYLLABLE ROEH
B8CC;HANGUL SYLLABLE RYO
B8CD;HANGUL SYLLABLE RYOG
B8CE;HANGUL SYLLABLE RYOGG
B8CF;HANGUL SYLLABLE RYOGS
B8D0;HANGUL SYLLABLE RYON
B8D1;HANGUL SYLLABLE RYONJ
B8D2;HANGUL SYLLABLE RYONH
B8D3;HANGUL SYLLABLE RYOD
B8D4;HANGUL SYLLABLE RYOL
B8D5;HANGUL SYLLABLE RYOLG
B8D6;HANGUL SYLLABLE RYOLM
B8D7;HANGUL SYLLABLE RYOLB
B8D8;HANGUL SYLLABLE RYOLS
B8D9;HANGUL SYLLABLE RYOLT
B8DA;HANGUL SYLLABLE RYOLP
B8DB;HANGUL SYLLABLE RYOLH
B8DC;HANGUL SYLLABLE RYOM
B8DD;HANGUL SYLLABLE RYOB
B8DE;HANGUL SYLLABLE RYOBS
B8DF;HANGUL SYLLABLE RYOS
B8E0;HANGUL SYLLABLE RYOSS
B8E1;HANGUL SYLLABLE RYONG
B8E2;HANGUL SYLLABLE RYOJ
B8E3;HANGUL SYLLABLE RYOC
B8E4;HANGUL SYLLABLE RYOK
B8E5;HANGUL SYLLABLE RYOT
B8E6;HANGUL SYLLABLE RYOP
B8E7;HANGUL SYLLABLE RYOH
B8E8;HANGUL SYLLABLE RU
B8E9;HANGUL SYLLABLE RUG
B8EA;HANGUL SYLLABLE RUGG
B8EB;HANGUL SYLLABLE RUGS
B8EC;HANGUL SYLLABLE RUN
B8ED;HANGUL SYLLABLE RUNJ
B8EE;HANGUL SYLLABLE RUNH
B8EF;HANGUL SYLLABLE RUD
B8F0;HANGUL SYLLABLE RUL
B8F1;HANGUL SYLLABLE RULG
B8F2;HANGUL SYLLABLE RULM
B8F3;HANGUL SYLLABLE RULB
B8F4;HANGUL SYLLABLE RULS
B8F5;HANGUL SYLLABLE RULT
B8F6;HANGUL SYLLABLE RULP
B8F7;HANGUL SYLLABLE RULH
B8F8;HANGUL SYLLABLE RUM
B8F9;HANGUL SYLLABLE RUB
B8FA;HANGUL SYLLABLE RUBS
B8FB;HANGUL SYLLABLE RUS
B8FC;HANGUL SYLLABLE RUSS
B8FD;HANGUL SYLLABLE RUNG
B8FE;HANGUL SYLLABLE RUJ
B8FF;HANGUL SYLLABLE RUC
B900;HANGUL SYLLABLE RUK
B901;HANGUL SYLLABLE RUT
B902;HANGUL SYLLABLE RUP
B903;HANGUL SYLLABLE RUH
B904;HANGUL SYLLABLE RWEO
B905;HANGUL SYLLABLE RWEOG
B906;HANGUL SYLLABLE RWEOGG
B907;HANGUL SYLLABLE RWEOGS
B908;HANGUL SYLLABLE RWEON
B909;HANGUL SYLLABLE RWEONJ
B90A;HANGUL SYLLABLE RWEONH
B90B;HANGUL SYLLABLE RWEOD
B90C;HANGUL SYLLABLE RWEOL
B90D;HANGUL SYLLABLE RWEOLG
B90E;HANGUL SYLLABLE RWEOLM
B90F;HANGUL SYLLABLE RWEOLB
B910;HANGUL SYLLABLE RWEOLS
B911;HANGUL SYLLABLE RWEOLT
B912;HANGUL SYLLABLE RWEOLP
B913;HANGUL SYLLABLE RWEOLH
B914;HANGUL SYLLABLE RWEOM
B915;HANGUL SYLLABLE RWEOB
B916;HANGUL SYLLABLE RWEOBS
B917;HANGUL SYLLABLE RWEOS
B918;HANGUL SYLLABLE RWEOSS
B919;HANGUL SYLLABLE RWEONG
B91A;HANGUL SYLLABLE RWEOJ
B91B;HANGUL SYLLABLE RWEOC
B91C;HANGUL SYLLABLE RWEOK
B91D;HANGUL SYLLABLE RWEOT
B91E;HANGUL SYLLABLE RWEOP
B91F;HANGUL SYLLABLE RWEOH
B920;HANGUL SYLLABLE RWE
B921;HANGUL SYLLABLE RWEG
B922;HANGUL SYLLABLE RWEGG
B923;HANGUL SYLLABLE RWEGS
B924;HANGUL SYLLABLE RWEN
B925;HANGUL SYLLABLE RWENJ
B926;HANGUL SYLLABLE RWENH
B927;HANGUL SYLLABLE RWED
B928;HANGUL SYLLABLE RWEL
B929;HANGUL SYLLABLE RWELG
B92A;HANGUL SYLLABLE RWELM
B92B;HANGUL SYLLABLE RWELB
B92C;HANGUL SYLLABLE RWELS
B92D;HANGUL SYLLABLE RWELT
B92E;HANGUL SYLLABLE RWELP
B92F;HANGUL SYLLABLE RWELH
B930;HANGUL SYLLABLE RWEM
B931;HANGUL SYLLABLE RWEB
B932;HANGUL SYLLABLE RWEBS
B933;HANGUL SYLLABLE RWES
B934;HANGUL SYLLABLE RWESS
B935;HANGUL SYLLABLE RWENG
B936;HANGUL SYLLABLE RWEJ
B937;HANGUL SYLLABLE RWEC
B938;HANGUL SYLLABLE RWEK
B939;HANGUL SYLLABLE RWET
B93A;HANGUL SYLLABLE RWEP
B93B;HANGUL SYLLABLE RWEH
B93C;HANGUL SYLLABLE RWI
B93D;HANGUL SYLLABLE RWIG
B93E;HANGUL SYLLABLE RWIGG
B93F;HANGUL SYLLABLE RWIGS
B940;HANGUL SYLLABLE RWIN
B941;HANGUL SYLLABLE RWINJ
B942;HANGUL SYLLABLE RWINH
B943;HANGUL SYLLABLE RWID
B944;HANGUL SYLLABLE RWIL
B945;HANGUL SYLLABLE RWILG
B946;HANGUL SYLLABLE RWILM
B947;HANGUL SYLLABLE RWILB
B948;HANGUL SYLLABLE RWILS
B949;HANGUL SYLLABLE RWILT
B94A;HANGUL SYLLABLE RWILP
B94B;HANGUL SYLLABLE RWILH
B94C;HANGUL SYLLABLE RWIM
B94D;HANGUL SYLLABLE RWIB
B94E;HANGUL SYLLABLE RWIBS
B94F;HANGUL SYLLABLE RWIS
B950;HANGUL SYLLABLE RWISS
B951;HANGUL SYLLABLE RWING
B952;HANGUL SYLLABLE RWIJ
B953;HANGUL SYLLABLE RWIC
B954;HANGUL SYLLABLE RWIK
B955;HANGUL SYLLABLE RWIT
B956;HANGUL SYLLABLE RWIP
B957;HANGUL SYLLABLE RWIH
B958;HANGUL SYLLABLE RYU
B959;HANGUL SYLLABLE RYUG
B95A;HANGUL SYLLABLE RYUGG
B95B;HANGUL SYLLABLE RYUGS
B95C;HANGUL SYLLABLE RYUN
B95D;HANGUL SYLLABLE RYUNJ
B95E;HANGUL SYLLABLE RYUNH
B95F;HANGUL SYLLABLE RYUD
B960;HANGUL SYLLABLE RYUL
B961;HANGUL SYLLABLE RYULG
B962;HANGUL SYLLABLE RYULM
B963;HANGUL SYLLABLE RYULB
B964;HANGUL SYLLABLE RYULS
B965;HANGUL SYLLABLE RYULT
B966;HANGUL SYLLABLE RYULP
B967;HANGUL SYLLABLE RYULH
B968;HANGUL SYLLABLE RYUM
B969;HANGUL SYLLABLE RYUB
B96A;HANGUL SYLLABLE RYUBS
B96B;HANGUL SYLLABLE RYUS
B96C;HANGUL SYLLABLE RYUSS
B96D;HANGUL SYLLABLE RYUNG
B96E;HANGUL SYLLABLE RYUJ
B96F;HANGUL SYLLABLE RYUC
B970;HANGUL SYLLABLE RYUK
B971;HANGUL SYLLABLE RYUT
B972;HANGUL SYLLABLE RYUP
B973;HANGUL SYLLABLE RYUH
B974;HANGUL SYLLABLE REU
B975;HANGUL SYLLABLE REUG
B976;HANGUL SYLLABLE REUGG
B977;HANGUL SYLLABLE REUGS
B978;HANGUL SYLLABLE REUN
B979;HANGUL SYLLABLE REUNJ
B97A;HANGUL SYLLABLE REUNH
B97B;HANGUL SYLLABLE REUD
B97C;HANGUL SYLLABLE REUL
B97D;HANGUL SYLLABLE REULG
B97E;HANGUL SYLLABLE REULM
B97F;HANGUL SYLLABLE REULB
B980;HANGUL SYLLABLE REULS
B981;HANGUL SYLLABLE REULT
B982;HANGUL SYLLABLE REULP
B983;HANGUL SYLLABLE REULH
B984;HANGUL SYLLABLE REUM
B985;HANGUL SYLLABLE REUB
B986;HANGUL SYLLABLE REUBS
B987;HANGUL SYLLABLE REUS
B988;HANGUL SYLLABLE REUSS
B989;HANGUL SYLLABLE REUNG
B98A;HANGUL SYLLABLE REUJ
B98B;HANGUL SYLLABLE REUC
B98C;HANGUL SYLLABLE REUK
B98D;HANGUL SYLLABLE REUT
B98E;HANGUL SYLLABLE REUP
B98F;HANGUL SYLLABLE REUH
B990;HANGUL SYLLABLE RYI
B991;HANGUL SYLLABLE RYIG
B992;HANGUL SYLLABLE RYIGG
B993;HANGUL SYLLABLE RYIGS
B994;HANGUL SYLLABLE RYIN
B995;HANGUL SYLLABLE RYINJ
B996;HANGUL SYLLABLE RYINH
B997;HANGUL SYLLABLE RYID
B998;HANGUL SYLLABLE RYIL
B999;HANGUL SYLLABLE RYILG
B99A;HANGUL SYLLABLE RYILM
B99B;HANGUL SYLLABLE RYILB
B99C;HANGUL SYLLABLE RYILS
B99D;HANGUL SYLLABLE RYILT
B99E;HANGUL SYLLABLE RYILP
B99F;HANGUL SYLLABLE RYILH
B9A0;HANGUL SYLLABLE RYIM
B9A1;HANGUL SYLLABLE RYIB
B9A2;HANGUL SYLLABLE RYIBS
B9A3;HANGUL SYLLABLE RYIS
B9A4;HANGUL SYLLABLE RYISS
B9A5;HANGUL SYLLABLE RYING
B9A6;HANGUL SYLLABLE RYIJ
B9A7;HANGUL SYLLABLE RYIC
B9A8;HANGUL SYLLABLE RYIK
B9A9;HANGUL SYLLABLE RYIT
B9AA;HANGUL SYLLABLE RYIP
B9AB;HANGUL SYLLABLE RYIH
B9AC;HANGUL SYLLABLE RI
B9AD;HANGUL SYLLABLE RIG
B9AE;HANGUL SYLLABLE RIGG
B9AF;HANGUL SYLLABLE RIGS
B9B0;HANGUL SYLLABLE RIN
B9B1;HANGUL SYLLABLE RINJ
B9B2;HANGUL SYLLABLE RINH
B9B3;HANGUL SYLLABLE RID
B9B4;HANGUL SYLLABLE RIL
B9B5;HANGUL SYLLABLE RILG
B9B6;HANGUL SYLLABLE RILM
B9B7;HANGUL SYLLABLE RILB
B9B8;HANGUL SYLLABLE RILS
B9B9;HANGUL SYLLABLE RILT
B9BA;HANGUL SYLLABLE RILP
B9BB;HANGUL SYLLABLE RILH
B9BC;HANGUL SYLLABLE RIM
B9BD;HANGUL SYLLABLE RIB
B9BE;HANGUL SYLLABLE RIBS
B9BF;HANGUL SYLLABLE RIS
B9C0;HANGUL SYLLABLE RISS
B9C1;HANGUL SYLLABLE RING
B9C2;HANGUL SYLLABLE RIJ
B9C3;HANGUL SYLLABLE RIC
B9C4;HANGUL SYLLABLE RIK
B9C5;HANGUL SYLLABLE RIT
B9C6;HANGUL SYLLABLE RIP
B9C7;HANGUL SYLLABLE RIH
B9C8;HANGUL SYLLABLE MA
B9C9;HANGUL SYLLABLE MAG
B9CA;HANGUL SYLLABLE MAGG
B9CB;HANGUL SYLLABLE MAGS
B9CC;HANGUL SYLLABLE MAN
B9CD;HANGUL SYLLABLE MANJ
B9CE;HANGUL SYLLABLE MANH
B9CF;HANGUL SYLLABLE MAD
B9D0;HANGUL SYLLABLE MAL
B9D1;HANGUL SYLLABLE MALG
B9D2;HANGUL SYLLABLE MALM
B9D3;HANGUL SYLLABLE MALB
B9D4;HANGUL SYLLABLE MALS
B9D5;HANGUL SYLLABLE MALT
B9D6;HANGUL SYLLABLE MALP
B9D7;HANGUL SYLLABLE MALH
B9D8;HANGUL SYLLABLE MAM
B9D9;HANGUL SYLLABLE MAB
B9DA;HANGUL SYLLABLE MABS
B9DB;HANGUL SYLLABLE MAS
B9DC;HANGUL SYLLABLE MASS
B9DD;HANGUL SYLLABLE MANG
B9DE;HANGUL SYLLABLE MAJ
B9DF;HANGUL SYLLABLE MAC
B9E0;HANGUL SYLLABLE MAK
B9E1;HANGUL SYLLABLE MAT
B9E2;HANGUL SYLLABLE MAP
B9E3;HANGUL SYLLABLE MAH
B9E4;HANGUL SYLLABLE MAE
B9E5;HANGUL SYLLABLE MAEG
B9E6;HANGUL SYLLABLE MAEGG
B9E7;HANGUL SYLLABLE MAEGS
B9E8;HANGUL SYLLABLE MAEN
B9E9;HANGUL SYLLABLE MAENJ
B9EA;HANGUL SYLLABLE MAENH
B9EB;HANGUL SYLLABLE MAED
B9EC;HANGUL SYLLABLE MAEL
B9ED;HANGUL SYLLABLE MAELG
B9EE;HANGUL SYLLABLE MAELM
B9EF;HANGUL SYLLABLE MAELB
B9F0;HANGUL SYLLABLE MAELS
B9F1;HANGUL SYLLABLE MAELT
B9F2;HANGUL SYLLABLE MAELP
B9F3;HANGUL SYLLABLE MAELH
B9F4;HANGUL SYLLABLE MAEM
B9F5;HANGUL SYLLABLE MAEB
B9F6;HANGUL SYLLABLE MAEBS
B9F7;HANGUL SYLLABLE MAES
B9F8;HANGUL SYLLABLE MAESS
B9F9;HANGUL SYLLABLE MAENG
B9FA;HANGUL SYLLABLE MAEJ
B9FB;HANGUL SYLLABLE MAEC
B9FC;HANGUL SYLLABLE MAEK
B9FD;HANGUL SYLLABLE MAET
B9FE;HANGUL SYLLABLE MAEP
B9FF;HANGUL SYLLABLE MAEH
BA00;HANGUL SYLLABLE MYA
BA01;HANGUL SYLLABLE MYAG
BA02;HANGUL SYLLABLE MYAGG
BA03;HANGUL SYLLABLE MYAGS
BA04;HANGUL SYLLABLE MYAN
BA05;HANGUL SYLLABLE MYANJ
BA06;HANGUL SYLLABLE MYANH
BA07;HANGUL SYLLABLE MYAD
BA08;HANGUL SYLLABLE MYAL
BA09;HANGUL SYLLABLE MYALG
BA0A;HANGUL SYLLABLE MYALM
BA0B;HANGUL SYLLABLE MYALB
BA0C;HANGUL SYLLABLE MYALS
BA0D;HANGUL SYLLABLE MYALT
BA0E;HANGUL SYLLABLE MYALP
BA0F;HANGUL SYLLABLE MYALH
BA10;HANGUL SYLLABLE MYAM
BA11;HANGUL SYLLABLE MYAB
BA12;HANGUL SYLLABLE MYABS
BA13;HANGUL SYLLABLE MYAS
BA14;HANGUL SYLLABLE MYASS
BA15;HANGUL SYLLABLE MYANG
BA16;HANGUL SYLLABLE MYAJ
BA17;HANGUL SYLLABLE MYAC
BA18;HANGUL SYLLABLE MYAK
BA19;HANGUL SYLLABLE MYAT
BA1A;HANGUL SYLLABLE MYAP
BA1B;HANGUL SYLLABLE MYAH
BA1C;HANGUL SYLLABLE MYAE
BA1D;HANGUL SYLLABLE MYAEG
BA1E;HANGUL SYLLABLE MYAEGG
BA1F;HANGUL SYLLABLE MYAEGS
BA20;HANGUL SYLLABLE MYAEN
BA21;HANGUL SYLLABLE MYAENJ
BA22;HANGUL SYLLABLE MYAENH
BA23;HANGUL SYLLABLE MYAED
BA24;HANGUL SYLLABLE MYAEL
BA25;HANGUL SYLLABLE MYAELG
BA26;HANGUL SYLLABLE MYAELM
BA27;HANGUL SYLLABLE MYAELB
BA28;HANGUL SYLLABLE MYAELS
BA29;HANGUL SYLLABLE MYAELT
BA2A;HANGUL SYLLABLE MYAELP
BA2B;HANGUL SYLLABLE MYAELH
BA2C;HANGUL SYLLABLE MYAEM
BA2D;HANGUL SYLLABLE MYAEB
BA2E;HANGUL SYLLABLE MYAEBS
BA2F;HANGUL SYLLABLE MYAES
BA30;HANGUL SYLLABLE MYAESS
BA31;HANGUL SYLLABLE MYAENG
BA32;HANGUL SYLLABLE MYAEJ
BA33;HANGUL SYLLABLE MYAEC
BA34;HANGUL SYLLABLE MYAEK
BA35;HANGUL SYLLABLE MYAET
BA36;HANGUL SYLLABLE MYAEP
BA37;HANGUL SYLLABLE MYAEH
BA38;HANGUL SYLLABLE MEO
BA39;HANGUL SYLLABLE MEOG
BA3A;HANGUL SYLLABLE MEOGG
BA3B;HANGUL SYLLABLE MEOGS
BA3C;HANGUL SYLLABLE MEON
BA3D;HANGUL SYLLABLE MEONJ
BA3E;HANGUL SYLLABLE MEONH
BA3F;HANGUL SYLLABLE MEOD
BA40;HANGUL SYLLABLE MEOL
BA41;HANGUL SYLLABLE MEOLG
BA42;HANGUL SYLLABLE MEOLM
BA43;HANGUL SYLLABLE MEOLB
BA44;HANGUL SYLLABLE MEOLS
BA45;HANGUL SYLLABLE MEOLT
BA46;HANGUL SYLLABLE MEOLP
BA47;HANGUL SYLLABLE MEOLH
BA48;HANGUL SYLLABLE MEOM
BA49;HANGUL SYLLABLE MEOB
BA4A;HANGUL SYLLABLE MEOBS
BA4B;HANGUL SYLLABLE MEOS
BA4C;HANGUL SYLLABLE MEOSS
BA4D;HANGUL SYLLABLE MEONG
BA4E;HANGUL SYLLABLE MEOJ
BA4F;HANGUL SYLLABLE MEOC
BA50;HANGUL SYLLABLE MEOK
BA51;HANGUL SYLLABLE MEOT
BA52;HANGUL SYLLABLE MEOP
BA53;HANGUL SYLLABLE MEOH
BA54;HANGUL SYLLABLE ME
BA55;HANGUL SYLLABLE MEG
BA56;HANGUL SYLLABLE MEGG
BA57;HANGUL SYLLABLE MEGS
BA58;HANGUL SYLLABLE MEN
BA59;HANGUL SYLLABLE MENJ
BA5A;HANGUL SYLLABLE MENH
BA5B;HANGUL SYLLABLE MED
BA5C;HANGUL SYLLABLE MEL
BA5D;HANGUL SYLLABLE MELG
BA5E;HANGUL SYLLABLE MELM
BA5F;HANGUL SYLLABLE MELB
BA60;HANGUL SYLLABLE MELS
BA61;HANGUL SYLLABLE MELT
BA62;HANGUL SYLLABLE MELP
BA63;HANGUL SYLLABLE MELH
BA64;HANGUL SYLLABLE MEM
BA65;HANGUL SYLLABLE MEB
BA66;HANGUL SYLLABLE MEBS
BA67;HANGUL SYLLABLE MES
BA68;HANGUL SYLLABLE MESS
BA69;HANGUL SYLLABLE MENG
BA6A;HANGUL SYLLABLE MEJ
BA6B;HANGUL SYLLABLE MEC
BA6C;HANGUL SYLLABLE MEK
BA6D;HANGUL SYLLABLE MET
BA6E;HANGUL SYLLABLE MEP
BA6F;HANGUL SYLLABLE MEH
BA70;HANGUL SYLLABLE MYEO
BA71;HANGUL SYLLABLE MYEOG
BA72;HANGUL SYLLABLE MYEOGG
BA73;HANGUL SYLLABLE MYEOGS
BA74;HANGUL SYLLABLE MYEON
BA75;HANGUL SYLLABLE MYEONJ
BA76;HANGUL SYLLABLE MYEONH
BA77;HANGUL SYLLABLE MYEOD
BA78;HANGUL SYLLABLE MYEOL
BA79;HANGUL SYLLABLE MYEOLG
BA7A;HANGUL SYLLABLE MYEOLM
BA7B;HANGUL SYLLABLE MYEOLB
BA7C;HANGUL SYLLABLE MYEOLS
BA7D;HANGUL SYLLABLE MYEOLT
BA7E;HANGUL SYLLABLE MYEOLP
BA7F;HANGUL SYLLABLE MYEOLH
BA80;HANGUL SYLLABLE MYEOM
BA81;HANGUL SYLLABLE MYEOB
BA82;HANGUL SYLLABLE MYEOBS
BA83;HANGUL SYLLABLE MYEOS
BA84;HANGUL SYLLABLE MYEOSS
BA85;HANGUL SYLLABLE MYEONG
BA86;HANGUL SYLLABLE MYEOJ
BA87;HANGUL SYLLABLE MYEOC
BA88;HANGUL SYLLABLE MYEOK
BA89;HANGUL SYLLABLE MYEOT
BA8A;HANGUL SYLLABLE MYEOP
BA8B;HANGUL SYLLABLE MYEOH
BA8C;HANGUL SYLLABLE MYE
BA8D;HANGUL SYLLABLE MYEG
BA8E;HANGUL SYLLABLE MYEGG
BA8F;HANGUL SYLLABLE MYEGS
BA90;HANGUL SYLLABLE MYEN
BA91;HANGUL SYLLABLE MYENJ
BA92;HANGUL SYLLABLE MYENH
BA93;HANGUL SYLLABLE MYED
BA94;HANGUL SYLLABLE MYEL
BA95;HANGUL SYLLABLE MYELG
BA96;HANGUL SYLLABLE MYELM
BA97;HANGUL SYLLABLE MYELB
BA98;HANGUL SYLLABLE MYELS
BA99;HANGUL SYLLABLE MYELT
BA9A;HANGUL SYLLABLE MYELP
BA9B;HANGUL SYLLABLE MYELH
BA9C;HANGUL SYLLABLE MYEM
BA9D;HANGUL SYLLABLE MYEB
BA9E;HANGUL SYLLABLE MYEBS
BA9F;HANGUL SYLLABLE MYES
BAA0;HANGUL SYLLABLE MYESS
BAA1;HANGUL SYLLABLE MYENG
BAA2;HANGUL SYLLABLE MYEJ
BAA3;HANGUL SYLLABLE MYEC
BAA4;HANGUL SYLLABLE MYEK
BAA5;HANGUL SYLLABLE MYET
BAA6;HANGUL SYLLABLE MYEP
BAA7;HANGUL SYLLABLE MYEH
BAA8;HANGUL SYLLABLE MO
BAA9;HANGUL SYLLABLE MOG
BAAA;HANGUL SYLLABLE MOGG
BAAB;HANGUL SYLLABLE MOGS
BAAC;HANGUL SYLLABLE MON
BAAD;HANGUL SYLLABLE MONJ
BAAE;HANGUL SYLLABLE MONH
BAAF;HANGUL SYLLABLE MOD
BAB0;HANGUL SYLLABLE MOL
BAB1;HANGUL SYLLABLE MOLG
BAB2;HANGUL SYLLABLE MOLM
BAB3;HANGUL SYLLABLE MOLB
BAB4;HANGUL SYLLABLE MOLS
BAB5;HANGUL SYLLABLE MOLT
BAB6;HANGUL SYLLABLE MOLP
BAB7;HANGUL SYLLABLE MOLH
BAB8;HANGUL SYLLABLE MOM
BAB9;HANGUL SYLLABLE MOB
BABA;HANGUL SYLLABLE MOBS
BABB;HANGUL SYLLABLE MOS
BABC;HANGUL SYLLABLE MOSS
BABD;HANGUL SYLLABLE MONG
BABE;HANGUL SYLLABLE MOJ
BABF;HANGUL SYLLABLE MOC
BAC0;HANGUL SYLLABLE MOK
BAC1;HANGUL SYLLABLE MOT
BAC2;HANGUL SYLLABLE MOP
BAC3;HANGUL SYLLABLE MOH
BAC4;HANGUL SYLLABLE MWA
BAC5;HANGUL SYLLABLE MWAG
BAC6;HANGUL SYLLABLE MWAGG
BAC7;HANGUL SYLLABLE MWAGS
BAC8;HANGUL SYLLABLE MWAN
BAC9;HANGUL SYLLABLE MWANJ
BACA;HANGUL SYLLABLE MWANH
BACB;HANGUL SYLLABLE MWAD
BACC;HANGUL SYLLABLE MWAL
BACD;HANGUL SYLLABLE MWALG
BACE;HANGUL SYLLABLE MWALM
BACF;HANGUL SYLLABLE MWALB
BAD0;HANGUL SYLLABLE MWALS
BAD1;HANGUL SYLLABLE MWALT
BAD2;HANGUL SYLLABLE MWALP
BAD3;HANGUL SYLLABLE MWALH
BAD4;HANGUL SYLLABLE MWAM
BAD5;HANGUL SYLLABLE MWAB
BAD6;HANGUL SYLLABLE MWABS
BAD7;HANGUL SYLLABLE MWAS
BAD8;HANGUL SYLLABLE MWASS
BAD9;HANGUL SYLLABLE MWANG
BADA;HANGUL SYLLABLE MWAJ
BADB;HANGUL SYLLABLE MWAC
BADC;HANGUL SYLLABLE MWAK
BADD;HANGUL SYLLABLE MWAT
BADE;HANGUL SYLLABLE MWAP
BADF;HANGUL SYLLABLE MWAH
BAE0;HANGUL SYLLABLE MWAE
BAE1;HANGUL SYLLABLE MWAEG
BAE2;HANGUL SYLLABLE MWAEGG
BAE3;HANGUL SYLLABLE MWAEGS
BAE4;HANGUL SYLLABLE MWAEN
BAE5;HANGUL SYLLABLE MWAENJ
BAE6;HANGUL SYLLABLE MWAENH
BAE7;HANGUL SYLLABLE MWAED
BAE8;HANGUL SYLLABLE MWAEL
BAE9;HANGUL SYLLABLE MWAELG
BAEA;HANGUL SYLLABLE MWAELM
BAEB;HANGUL SYLLABLE MWAELB
BAEC;HANGUL SYLLABLE MWAELS
BAED;HANGUL SYLLABLE MWAELT
BAEE;HANGUL SYLLABLE MWAELP
BAEF;HANGUL SYLLABLE MWAELH
BAF0;HANGUL SYLLABLE MWAEM
BAF1;HANGUL SYLLABLE MWAEB
BAF2;HANGUL SYLLABLE MWAEBS
BAF3;HANGUL SYLLABLE MWAES
BAF4;HANGUL SYLLABLE MWAESS
BAF5;HANGUL SYLLABLE MWAENG
BAF6;HANGUL SYLLABLE MWAEJ
BAF7;HANGUL SYLLABLE MWAEC
BAF8;HANGUL SYLLABLE MWAEK
BAF9;HANGUL SYLLABLE MWAET
BAFA;HANGUL SYLLABLE MWAEP
BAFB;HANGUL SYLLABLE MWAEH
BAFC;HANGUL SYLLABLE MOE
BAFD;HANGUL SYLLABLE MOEG
BAFE;HANGUL SYLLABLE MOEGG
BAFF;HANGUL SYLLABLE MOEGS
BB00;HANGUL SYLLABLE MOEN
BB01;HANGUL SYLLABLE MOENJ
BB02;HANGUL SYLLABLE MOENH
BB03;HANGUL SYLLABLE MOED
BB04;HANGUL SYLLABLE MOEL
BB05;HANGUL SYLLABLE MOELG
BB06;HANGUL SYLLABLE MOELM
BB07;HANGUL SYLLABLE MOELB
BB08;HANGUL SYLLABLE MOELS
BB09;HANGUL SYLLABLE MOELT
BB0A;HANGUL SYLLABLE MOELP
BB0B;HANGUL SYLLABLE MOELH
BB0C;HANGUL SYLLABLE MOEM
BB0D;HANGUL SYLLABLE MOEB
BB0E;HANGUL SYLLABLE MOEBS
BB0F;HANGUL SYLLABLE MOES
BB10;HANGUL SYLLABLE MOESS
BB11;HANGUL SYLLABLE MOENG
BB12;HANGUL SYLLABLE MOEJ
BB13;HANGUL SYLLABLE MOEC
BB14;HANGUL SYLLABLE MOEK
BB15;HANGUL SYLLABLE MOET
BB16;HANGUL SYLLABLE MOEP
BB17;HANGUL SYLLABLE MOEH
BB18;HANGUL SYLLABLE MYO
BB19;HANGUL SYLLABLE MYOG
BB1A;HANGUL SYLLABLE MYOGG
BB1B;HANGUL SYLLABLE MYOGS
BB1C;HANGUL SYLLABLE MYON
BB1D;HANGUL SYLLABLE MYONJ
BB1E;HANGUL SYLLABLE MYONH
BB1F;HANGUL SYLLABLE MYOD
BB20;HANGUL SYLLABLE MYOL
BB21;HANGUL SYLLABLE MYOLG
BB22;HANGUL SYLLABLE MYOLM
BB23;HANGUL SYLLABLE MYOLB
BB24;HANGUL SYLLABLE MYOLS
BB25;HANGUL SYLLABLE MYOLT
BB26;HANGUL SYLLABLE MYOLP
BB27;HANGUL SYLLABLE MYOLH
BB28;HANGUL SYLLABLE MYOM
BB29;HANGUL SYLLABLE MYOB
BB2A;HANGUL SYLLABLE MYOBS
BB2B;HANGUL SYLLABLE MYOS
BB2C;HANGUL SYLLABLE MYOSS
BB2D;HANGUL SYLLABLE MYONG
BB2E;HANGUL SYLLABLE MYOJ
BB2F;HANGUL SYLLABLE MYOC
BB30;HANGUL SYLLABLE MYOK
BB31;HANGUL SYLLABLE MYOT
BB32;HANGUL SYLLABLE MYOP
BB33;HANGUL SYLLABLE MYOH
BB34;HANGUL SYLLABLE MU
BB35;HANGUL SYLLABLE MUG
BB36;HANGUL SYLLABLE MUGG
BB37;HANGUL SYLLABLE MUGS
BB38;HANGUL SYLLABLE MUN
BB39;HANGUL SYLLABLE MUNJ
BB3A;HANGUL SYLLABLE MUNH
BB3B;HANGUL SYLLABLE MUD
BB3C;HANGUL SYLLABLE MUL
BB3D;HANGUL SYLLABLE MULG
BB3E;HANGUL SYLLABLE MULM
BB3F;HANGUL SYLLABLE MULB
BB40;HANGUL SYLLABLE MULS
BB41;HANGUL SYLLABLE MULT
BB42;HANGUL SYLLABLE MULP
BB43;HANGUL SYLLABLE MULH
BB44;HANGUL SYLLABLE MUM
BB45;HANGUL SYLLABLE MUB
BB46;HANGUL SYLLABLE MUBS
BB47;HANGUL SYLLABLE MUS
BB48;HANGUL SYLLABLE MUSS
BB49;HANGUL SYLLABLE MUNG
BB4A;HANGUL SYLLABLE MUJ
BB4B;HANGUL SYLLABLE MUC
BB4C;HANGUL SYLLABLE MUK
BB4D;HANGUL SYLLABLE MUT
BB4E;HANGUL SYLLABLE MUP
BB4F;HANGUL SYLLABLE MUH
BB50;HANGUL SYLLABLE MWEO
BB51;HANGUL SYLLABLE MWEOG
BB52;HANGUL SYLLABLE MWEOGG
BB53;HANGUL SYLLABLE MWEOGS
BB54;HANGUL SYLLABLE MWEON
BB55;HANGUL SYLLABLE MWEONJ
BB56;HANGUL SYLLABLE MWEONH
BB57;HANGUL SYLLABLE MWEOD
BB58;HANGUL SYLLABLE MWEOL
BB59;HANGUL SYLLABLE MWEOLG
BB5A;HANGUL SYLLABLE MWEOLM
BB5B;HANGUL SYLLABLE MWEOLB
BB5C;HANGUL SYLLABLE MWEOLS
BB5D;HANGUL SYLLABLE MWEOLT
BB5E;HANGUL SYLLABLE MWEOLP
BB5F;HANGUL SYLLABLE MWEOLH
BB60;HANGUL SYLLABLE MWEOM
BB61;HANGUL SYLLABLE MWEOB
BB62;HANGUL SYLLABLE MWEOBS
BB63;HANGUL SYLLABLE MWEOS
BB64;HANGUL SYLLABLE MWEOSS
BB65;HANGUL SYLLABLE MWEONG
BB66;HANGUL SYLLABLE MWEOJ
BB67;HANGUL SYLLABLE MWEOC
BB68;HANGUL SYLLABLE MWEOK
BB69;HANGUL SYLLABLE MWEOT
BB6A;HANGUL SYLLABLE MWEOP
BB6B;HANGUL SYLLABLE MWEOH
BB6C;HANGUL SYLLABLE MWE
BB6D;HANGUL SYLLABLE MWEG
BB6E;HANGUL SYLLABLE MWEGG
BB6F;HANGUL SYLLABLE MWEGS
BB70;HANGUL SYLLABLE MWEN
BB71;HANGUL SYLLABLE MWENJ
BB72;HANGUL SYLLABLE MWENH
BB73;HANGUL SYLLABLE MWED
BB74;HANGUL SYLLABLE MWEL
BB75;HANGUL SYLLABLE MWELG
BB76;HANGUL SYLLABLE MWELM
BB77;HANGUL SYLLABLE MWELB
BB78;HANGUL SYLLABLE MWELS
BB79;HANGUL SYLLABLE MWELT
BB7A;HANGUL SYLLABLE MWELP
BB7B;HANGUL SYLLABLE MWELH
BB7C;HANGUL SYLLABLE MWEM
BB7D;HANGUL SYLLABLE MWEB
BB7E;HANGUL SYLLABLE MWEBS
BB7F;HANGUL SYLLABLE MWES
BB80;HANGUL SYLLABLE MWESS
BB81;HANGUL SYLLABLE MWENG
BB82;HANGUL SYLLABLE MWEJ
BB83;HANGUL SYLLABLE MWEC
BB84;HANGUL SYLLABLE MWEK
BB85;HANGUL SYLLABLE MWET
BB86;HANGUL SYLLABLE MWEP
BB87;HANGUL SYLLABLE MWEH
BB88;HANGUL SYLLABLE MWI
BB89;HANGUL SYLLABLE MWIG
BB8A;HANGUL SYLLABLE MWIGG
BB8B;HANGUL SYLLABLE MWIGS
BB8C;HANGUL SYLLABLE MWIN
BB8D;HANGUL SYLLABLE MWINJ
BB8E;HANGUL SYLLABLE MWINH
BB8F;HANGUL SYLLABLE MWID
BB90;HANGUL SYLLABLE MWIL
BB91;HANGUL SYLLABLE MWILG
BB92;HANGUL SYLLABLE MWILM
BB93;HANGUL SYLLABLE MWILB
BB94;HANGUL SYLLABLE MWILS
BB95;HANGUL SYLLABLE MWILT
BB96;HANGUL SYLLABLE MWILP
BB97;HANGUL SYLLABLE MWILH
BB98;HANGUL SYLLABLE MWIM
BB99;HANGUL SYLLABLE MWIB
BB9A;HANGUL SYLLABLE MWIBS
BB9B;HANGUL SYLLABLE MWIS
BB9C;HANGUL SYLLABLE MWISS
BB9D;HANGUL SYLLABLE MWING
BB9E;HANGUL SYLLABLE MWIJ
BB9F;HANGUL SYLLABLE MWIC
BBA0;HANGUL SYLLABLE MWIK
BBA1;HANGUL SYLLABLE MWIT
BBA2;HANGUL SYLLABLE MWIP
BBA3;HANGUL SYLLABLE MWIH
BBA4;HANGUL SYLLABLE MYU
BBA5;HANGUL SYLLABLE MYUG
BBA6;HANGUL SYLLABLE MYUGG
BBA7;HANGUL SYLLABLE MYUGS
BBA8;HANGUL SYLLABLE MYUN
BBA9;HANGUL SYLLABLE MYUNJ
BBAA;HANGUL SYLLABLE MYUNH
BBAB;HANGUL SYLLABLE MYUD
BBAC;HANGUL SYLLABLE MYUL
BBAD;HANGUL SYLLABLE MYULG
BBAE;HANGUL SYLLABLE MYULM
BBAF;HANGUL SYLLABLE MYULB
BBB0;HANGUL SYLLABLE MYULS
BBB1;HANGUL SYLLABLE MYULT
BBB2;HANGUL SYLLABLE MYULP
BBB3;HANGUL SYLLABLE MYULH
BBB4;HANGUL SYLLABLE MYUM
BBB5;HANGUL SYLLABLE MYUB
BBB6;HANGUL SYLLABLE MYUBS
BBB7;HANGUL SYLLABLE MYUS
BBB8;HANGUL SYLLABLE MYUSS
BBB9;HANGUL SYLLABLE MYUNG
BBBA;HANGUL SYLLABLE MYUJ
BBBB;HANGUL SYLLABLE MYUC
BBBC;HANGUL SYLLABLE MYUK
BBBD;HANGUL SYLLABLE MYUT
BBBE;HANGUL SYLLABLE MYUP
BBBF;HANGUL SYLLABLE MYUH
BBC0;HANGUL SYLLABLE MEU
BBC1;HANGUL SYLLABLE MEUG
BBC2;HANGUL SYLLABLE MEUGG
BBC3;HANGUL SYLLABLE MEUGS
BBC4;HANGUL SYLLABLE MEUN
BBC5;HANGUL SYLLABLE MEUNJ
BBC6;HANGUL SYLLABLE MEUNH
BBC7;HANGUL SYLLABLE MEUD
BBC8;HANGUL SYLLABLE MEUL
BBC9;HANGUL SYLLABLE MEULG
BBCA;HANGUL SYLLABLE MEULM
BBCB;HANGUL SYLLABLE MEULB
BBCC;HANGUL SYLLABLE MEULS
BBCD;HANGUL SYLLABLE MEULT
BBCE;HANGUL SYLLABLE MEULP
BBCF;HANGUL SYLLABLE MEULH
BBD0;HANGUL SYLLABLE MEUM
BBD1;HANGUL SYLLABLE MEUB
BBD2;HANGUL SYLLABLE MEUBS
BBD3;HANGUL SYLLABLE MEUS
BBD4;HANGUL SYLLABLE MEUSS
BBD5;HANGUL SYLLABLE MEUNG
BBD6;HANGUL SYLLABLE MEUJ
BBD7;HANGUL SYLLABLE MEUC
BBD8;HANGUL SYLLABLE MEUK
BBD9;HANGUL SYLLABLE MEUT
BBDA;HANGUL SYLLABLE MEUP
BBDB;HANGUL SYLLABLE MEUH
BBDC;HANGUL SYLLABLE MYI
BBDD;HANGUL SYLLABLE MYIG
BBDE;HANGUL SYLLABLE MYIGG
BBDF;HANGUL SYLLABLE MYIGS
BBE0;HANGUL SYLLABLE MYIN
BBE1;HANGUL SYLLABLE MYINJ
BBE2;HANGUL SYLLABLE MYINH
BBE3;HANGUL SYLLABLE MYID
BBE4;HANGUL SYLLABLE MYIL
BBE5;HANGUL SYLLABLE MYILG
BBE6;HANGUL SYLLABLE MYILM
BBE7;HANGUL SYLLABLE MYILB
BBE8;HANGUL SYLLABLE MYILS
BBE9;HANGUL SYLLABLE MYILT
BBEA;HANGUL SYLLABLE MYILP
BBEB;HANGUL SYLLABLE MYILH
BBEC;HANGUL SYLLABLE MYIM
BBED;HANGUL SYLLABLE MYIB
BBEE;HANGUL SYLLABLE MYIBS
BBEF;HANGUL SYLLABLE MYIS
BBF0;HANGUL SYLLABLE MYISS
BBF1;HANGUL SYLLABLE MYING
BBF2;HANGUL SYLLABLE MYIJ
BBF3;HANGUL SYLLABLE MYIC
BBF4;HANGUL SYLLABLE MYIK
BBF5;HANGUL SYLLABLE MYIT
BBF6;HANGUL SYLLABLE MYIP
BBF7;HANGUL SYLLABLE MYIH
BBF8;HANGUL SYLLABLE MI
BBF9;HANGUL SYLLABLE MIG
BBFA;HANGUL SYLLABLE MIGG
BBFB;HANGUL SYLLABLE MIGS
BBFC;HANGUL SYLLABLE MIN
BBFD;HANGUL SYLLABLE MINJ
BBFE;HANGUL SYLLABLE MINH
BBFF;HANGUL SYLLABLE MID
BC00;HANGUL SYLLABLE MIL
BC01;HANGUL SYLLABLE MILG
BC02;HANGUL SYLLABLE MILM
BC03;HANGUL SYLLABLE MILB
BC04;HANGUL SYLLABLE MILS
BC05;HANGUL SYLLABLE MILT
BC06;HANGUL SYLLABLE MILP
BC07;HANGUL SYLLABLE MILH
BC08;HANGUL SYLLABLE MIM
BC09;HANGUL SYLLABLE MIB
BC0A;HANGUL SYLLABLE MIBS
BC0B;HANGUL SYLLABLE MIS
BC0C;HANGUL SYLLABLE MISS
BC0D;HANGUL SYLLABLE MING
BC0E;HANGUL SYLLABLE MIJ
BC0F;HANGUL SYLLABLE MIC
BC10;HANGUL SYLLABLE MIK
BC11;HANGUL SYLLABLE MIT
BC12;HANGUL SYLLABLE MIP
BC13;HANGUL SYLLABLE MIH
BC14;HANGUL SYLLABLE BA
BC15;HANGUL SYLLABLE BAG
BC16;HANGUL SYLLABLE BAGG
BC17;HANGUL SYLLABLE BAGS
BC18;HANGUL SYLLABLE BAN
BC19;HANGUL SYLLABLE BANJ
BC1A;HANGUL SYLLABLE BANH
BC1B;HANGUL SYLLABLE BAD
BC1C;HANGUL SYLLABLE BAL
BC1D;HANGUL SYLLABLE BALG
BC1E;HANGUL SYLLABLE BALM
BC1F;HANGUL SYLLABLE BALB
BC20;HANGUL SYLLABLE BALS
BC21;HANGUL SYLLABLE BALT
BC22;HANGUL SYLLABLE BALP
BC23;HANGUL SYLLABLE BALH
BC24;HANGUL SYLLABLE BAM
BC25;HANGUL SYLLABLE BAB
BC26;HANGUL SYLLABLE BABS
BC27;HANGUL SYLLABLE BAS
BC28;HANGUL SYLLABLE BASS
BC29;HANGUL SYLLABLE BANG
BC2A;HANGUL SYLLABLE BAJ
BC2B;HANGUL SYLLABLE BAC
BC2C;HANGUL SYLLABLE BAK
BC2D;HANGUL SYLLABLE BAT
BC2E;HANGUL SYLLABLE BAP
BC2F;HANGUL SYLLABLE BAH
BC30;HANGUL SYLLABLE BAE
BC31;HANGUL SYLLABLE BAEG
BC32;HANGUL SYLLABLE BAEGG
BC33;HANGUL SYLLABLE BAEGS
BC34;HANGUL SYLLABLE BAEN
BC35;HANGUL SYLLABLE BAENJ
BC36;HANGUL SYLLABLE BAENH
BC37;HANGUL SYLLABLE BAED
BC38;HANGUL SYLLABLE BAEL
BC39;HANGUL SYLLABLE BAELG
BC3A;HANGUL SYLLABLE BAELM
BC3B;HANGUL SYLLABLE BAELB
BC3C;HANGUL SYLLABLE BAELS
BC3D;HANGUL SYLLABLE BAELT
BC3E;HANGUL SYLLABLE BAELP
BC3F;HANGUL SYLLABLE BAELH
BC40;HANGUL SYLLABLE BAEM
BC41;HANGUL SYLLABLE BAEB
BC42;HANGUL SYLLABLE BAEBS
BC43;HANGUL SYLLABLE BAES
BC44;HANGUL SYLLABLE BAESS
BC45;HANGUL SYLLABLE BAENG
BC46;HANGUL SYLLABLE BAEJ
BC47;HANGUL SYLLABLE BAEC
BC48;HANGUL SYLLABLE BAEK
BC49;HANGUL SYLLABLE BAET
BC4A;HANGUL SYLLABLE BAEP
BC4B;HANGUL SYLLABLE BAEH
BC4C;HANGUL SYLLABLE BYA
BC4D;HANGUL SYLLABLE BYAG
BC4E;HANGUL SYLLABLE BYAGG
BC4F;HANGUL SYLLABLE BYAGS
BC50;HANGUL SYLLABLE BYAN
BC51;HANGUL SYLLABLE BYANJ
BC52;HANGUL SYLLABLE BYANH
BC53;HANGUL SYLLABLE BYAD
BC54;HANGUL SYLLABLE BYAL
BC55;HANGUL SYLLABLE BYALG
BC56;HANGUL SYLLABLE BYALM
BC57;HANGUL SYLLABLE BYALB
BC58;HANGUL SYLLABLE BYALS
BC59;HANGUL SYLLABLE BYALT
BC5A;HANGUL SYLLABLE BYALP
BC5B;HANGUL SYLLABLE BYALH
BC5C;HANGUL SYLLABLE BYAM
BC5D;HANGUL SYLLABLE BYAB
BC5E;HANGUL SYLLABLE BYABS
BC5F;HANGUL SYLLABLE BYAS
BC60;HANGUL SYLLABLE BYASS
BC61;HANGUL SYLLABLE BYANG
BC62;HANGUL SYLLABLE BYAJ
BC63;HANGUL SYLLABLE BYAC
BC64;HANGUL SYLLABLE BYAK
BC65;HANGUL SYLLABLE BYAT
BC66;HANGUL SYLLABLE BYAP
BC67;HANGUL SYLLABLE BYAH
BC68;HANGUL SYLLABLE BYAE
BC69;HANGUL SYLLABLE BYAEG
BC6A;HANGUL SYLLABLE BYAEGG
BC6B;HANGUL SYLLABLE BYAEGS
BC6C;HANGUL SYLLABLE BYAEN
BC6D;HANGUL SYLLABLE BYAENJ
BC6E;HANGUL SYLLABLE BYAENH
BC6F;HANGUL SYLLABLE BYAED
BC70;HANGUL SYLLABLE BYAEL
BC71;HANGUL SYLLABLE BYAELG
BC72;HANGUL SYLLABLE BYAELM
BC73;HANGUL SYLLABLE BYAELB
BC74;HANGUL SYLLABLE BYAELS
BC75;HANGUL SYLLABLE BYAELT
BC76;HANGUL SYLLABLE BYAELP
BC77;HANGUL SYLLABLE BYAELH
BC78;HANGUL SYLLABLE BYAEM
BC79;HANGUL SYLLABLE BYAEB
BC7A;HANGUL SYLLABLE BYAEBS
BC7B;HANGUL SYLLABLE BYAES
BC7C;HANGUL SYLLABLE BYAESS
BC7D;HANGUL SYLLABLE BYAENG
BC7E;HANGUL SYLLABLE BYAEJ
BC7F;HANGUL SYLLABLE BYAEC
BC80;HANGUL SYLLABLE BYAEK
BC81;HANGUL SYLLABLE BYAET
BC82;HANGUL SYLLABLE BYAEP
BC83;HANGUL SYLLABLE BYAEH
BC84;HANGUL SYLLABLE BEO
BC85;HANGUL SYLLABLE BEOG
BC86;HANGUL SYLLABLE BEOGG
BC87;HANGUL SYLLABLE BEOGS
BC88;HANGUL SYLLABLE BEON
BC89;HANGUL SYLLABLE BEONJ
BC8A;HANGUL SYLLABLE BEONH
BC8B;HANGUL SYLLABLE BEOD
BC8C;HANGUL SYLLABLE BEOL
BC8D;HANGUL SYLLABLE BEOLG
BC8E;HANGUL SYLLABLE BEOLM
BC8F;HANGUL SYLLABLE BEOLB
BC90;HANGUL SYLLABLE BEOLS
BC91;HANGUL SYLLABLE BEOLT
BC92;HANGUL SYLLABLE BEOLP
BC93;HANGUL SYLLABLE BEOLH
BC94;HANGUL SYLLABLE BEOM
BC95;HANGUL SYLLABLE BEOB
BC96;HANGUL SYLLABLE BEOBS
BC97;HANGUL SYLLABLE BEOS
BC98;HANGUL SYLLABLE BEOSS
BC99;HANGUL SYLLABLE BEONG
BC9A;HANGUL SYLLABLE BEOJ
BC9B;HANGUL SYLLABLE BEOC
BC9C;HANGUL SYLLABLE BEOK
BC9D;HANGUL SYLLABLE BEOT
BC9E;HANGUL SYLLABLE BEOP
BC9F;HANGUL SYLLABLE BEOH
BCA0;HANGUL SYLLABLE BE
BCA1;HANGUL SYLLABLE BEG
BCA2;HANGUL SYLLABLE BEGG
BCA3;HANGUL SYLLABLE BEGS
BCA4;HANGUL SYLLABLE BEN
BCA5;HANGUL SYLLABLE BENJ
BCA6;HANGUL SYLLABLE BENH
BCA7;HANGUL SYLLABLE BED
BCA8;HANGUL SYLLABLE BEL
BCA9;HANGUL SYLLABLE BELG
BCAA;HANGUL SYLLABLE BELM
BCAB;HANGUL SYLLABLE BELB
BCAC;HANGUL SYLLABLE BELS
BCAD;HANGUL SYLLABLE BELT
BCAE;HANGUL SYLLABLE BELP
BCAF;HANGUL SYLLABLE BELH
BCB0;HANGUL SYLLABLE BEM
BCB1;HANGUL SYLLABLE BEB
BCB2;HANGUL SYLLABLE BEBS
BCB3;HANGUL SYLLABLE BES
BCB4;HANGUL SYLLABLE BESS
BCB5;HANGUL SYLLABLE BENG
BCB6;HANGUL SYLLABLE BEJ
BCB7;HANGUL SYLLABLE BEC
BCB8;HANGUL SYLLABLE BEK
BCB9;HANGUL SYLLABLE BET
BCBA;HANGUL SYLLABLE BEP
BCBB;HANGUL SYLLABLE BEH
BCBC;HANGUL SYLLABLE BYEO
BCBD;HANGUL SYLLABLE BYEOG
BCBE;HANGUL SYLLABLE BYEOGG
BCBF;HANGUL SYLLABLE BYEOGS
BCC0;HANGUL SYLLABLE BYEON
BCC1;HANGUL SYLLABLE BYEONJ
BCC2;HANGUL SYLLABLE BYEONH
BCC3;HANGUL SYLLABLE BYEOD
BCC4;HANGUL SYLLABLE BYEOL
BCC5;HANGUL SYLLABLE BYEOLG
BCC6;HANGUL SYLLABLE BYEOLM
BCC7;HANGUL SYLLABLE BYEOLB
BCC8;HANGUL SYLLABLE BYEOLS
BCC9;HANGUL SYLLABLE BYEOLT
BCCA;HANGUL SYLLABLE BYEOLP
BCCB;HANGUL SYLLABLE BYEOLH
BCCC;HANGUL SYLLABLE BYEOM
BCCD;HANGUL SYLLABLE BYEOB
BCCE;HANGUL SYLLABLE BYEOBS
BCCF;HANGUL SYLLABLE BYEOS
BCD0;HANGUL SYLLABLE BYEOSS
BCD1;HANGUL SYLLABLE BYEONG
BCD2;HANGUL SYLLABLE BYEOJ
BCD3;HANGUL SYLLABLE BYEOC
BCD4;HANGUL SYLLABLE BYEOK
BCD5;HANGUL SYLLABLE BYEOT
BCD6;HANGUL SYLLABLE BYEOP
BCD7;HANGUL SYLLABLE BYEOH
BCD8;HANGUL SYLLABLE BYE
BCD9;HANGUL SYLLABLE BYEG
BCDA;HANGUL SYLLABLE BYEGG
BCDB;HANGUL SYLLABLE BYEGS
BCDC;HANGUL SYLLABLE BYEN
BCDD;HANGUL SYLLABLE BYENJ
BCDE;HANGUL SYLLABLE BYENH
BCDF;HANGUL SYLLABLE BYED
BCE0;HANGUL SYLLABLE BYEL
BCE1;HANGUL SYLLABLE BYELG
BCE2;HANGUL SYLLABLE BYELM
BCE3;HANGUL SYLLABLE BYELB
BCE4;HANGUL SYLLABLE BYELS
BCE5;HANGUL SYLLABLE BYELT
BCE6;HANGUL SYLLABLE BYELP
BCE7;HANGUL SYLLABLE BYELH
BCE8;HANGUL SYLLABLE BYEM
BCE9;HANGUL SYLLABLE BYEB
BCEA;HANGUL SYLLABLE BYEBS
BCEB;HANGUL SYLLABLE BYES
BCEC;HANGUL SYLLABLE BYESS
BCED;HANGUL SYLLABLE BYENG
BCEE;HANGUL SYLLABLE BYEJ
BCEF;HANGUL SYLLABLE BYEC
BCF0;HANGUL SYLLABLE BYEK
BCF1;HANGUL SYLLABLE BYET
BCF2;HANGUL SYLLABLE BYEP
BCF3;HANGUL SYLLABLE BYEH
BCF4;HANGUL SYLLABLE BO
BCF5;HANGUL SYLLABLE BOG
BCF6;HANGUL SYLLABLE BOGG
BCF7;HANGUL SYLLABLE BOGS
BCF8;HANGUL SYLLABLE BON
BCF9;HANGUL SYLLABLE BONJ
BCFA;HANGUL SYLLABLE BONH
BCFB;HANGUL SYLLABLE BOD
BCFC;HANGUL SYLLABLE BOL
BCFD;HANGUL SYLLABLE BOLG
BCFE;HANGUL SYLLABLE BOLM
BCFF;HANGUL SYLLABLE BOLB
BD00;HANGUL SYLLABLE BOLS
BD01;HANGUL SYLLABLE BOLT
BD02;HANGUL SYLLABLE BOLP
BD03;HANGUL SYLLABLE BOLH
BD04;HANGUL SYLLABLE BOM
BD05;HANGUL SYLLABLE BOB
BD06;HANGUL SYLLABLE BOBS
BD07;HANGUL SYLLABLE BOS
BD08;HANGUL SYLLABLE BOSS
BD09;HANGUL SYLLABLE BONG
BD0A;HANGUL SYLLABLE BOJ
BD0B;HANGUL SYLLABLE BOC
BD0C;HANGUL SYLLABLE BOK
BD0D;HANGUL SYLLABLE BOT
BD0E;HANGUL SYLLABLE BOP
BD0F;HANGUL SYLLABLE BOH
BD10;HANGUL SYLLABLE BWA
BD11;HANGUL SYLLABLE BWAG
BD12;HANGUL SYLLABLE BWAGG
BD13;HANGUL SYLLABLE BWAGS
BD14;HANGUL SYLLABLE BWAN
BD15;HANGUL SYLLABLE BWANJ
BD16;HANGUL SYLLABLE BWANH
BD17;HANGUL SYLLABLE BWAD
BD18;HANGUL SYLLABLE BWAL
BD19;HANGUL SYLLABLE BWALG
BD1A;HANGUL SYLLABLE BWALM
BD1B;HANGUL SYLLABLE BWALB
BD1C;HANGUL SYLLABLE BWALS
BD1D;HANGUL SYLLABLE BWALT
BD1E;HANGUL SYLLABLE BWALP
BD1F;HANGUL SYLLABLE BWALH
BD20;HANGUL SYLLABLE BWAM
BD21;HANGUL SYLLABLE BWAB
BD22;HANGUL SYLLABLE BWABS
BD23;HANGUL SYLLABLE BWAS
BD24;HANGUL SYLLABLE BWASS
BD25;HANGUL SYLLABLE BWANG
BD26;HANGUL SYLLABLE BWAJ
BD27;HANGUL SYLLABLE BWAC
BD28;HANGUL SYLLABLE BWAK
BD29;HANGUL SYLLABLE BWAT
BD2A;HANGUL SYLLABLE BWAP
BD2B;HANGUL SYLLABLE BWAH
BD2C;HANGUL SYLLABLE BWAE
BD2D;HANGUL SYLLABLE BWAEG
BD2E;HANGUL SYLLABLE BWAEGG
BD2F;HANGUL SYLLABLE BWAEGS
BD30;HANGUL SYLLABLE BWAEN
BD31;HANGUL SYLLABLE BWAENJ
BD32;HANGUL SYLLABLE BWAENH
BD33;HANGUL SYLLABLE BWAED
BD34;HANGUL SYLLABLE BWAEL
BD35;HANGUL SYLLABLE BWAELG
BD36;HANGUL SYLLABLE BWAELM
BD37;HANGUL SYLLABLE BWAELB
BD38;HANGUL SYLLABLE BWAELS
BD39;HANGUL SYLLABLE BWAELT
BD3A;HANGUL SYLLABLE BWAELP
BD3B;HANGUL SYLLABLE BWAELH
BD3C;HANGUL SYLLABLE BWAEM
BD3D;HANGUL SYLLABLE BWAEB
BD3E;HANGUL SYLLABLE BWAEBS
BD3F;HANGUL SYLLABLE BWAES
BD40;HANGUL SYLLABLE BWAESS
BD41;HANGUL SYLLABLE BWAENG
BD42;HANGUL SYLLABLE BWAEJ
BD43;HANGUL SYLLABLE BWAEC
BD44;HANGUL SYLLABLE BWAEK
BD45;HANGUL SYLLABLE BWAET
BD46;HANGUL SYLLABLE BWAEP
BD47;HANGUL SYLLABLE BWAEH
BD48;HANGUL SYLLABLE BOE
BD49;HANGUL SYLLABLE BOEG
BD4A;HANGUL SYLLABLE BOEGG
BD4B;HANGUL SYLLABLE BOEGS
BD4C;HANGUL SYLLABLE BOEN
BD4D;HANGUL SYLLABLE BOENJ
BD4E;HANGUL SYLLABLE BOENH
BD4F;HANGUL SYLLABLE BOED
BD50;HANGUL SYLLABLE BOEL
BD51;HANGUL SYLLABLE BOELG
BD52;HANGUL SYLLABLE BOELM
BD53;HANGUL SYLLABLE BOELB
BD54;HANGUL SYLLABLE BOELS
BD55;HANGUL SYLLABLE BOELT
BD56;HANGUL SYLLABLE BOELP
BD57;HANGUL SYLLABLE BOELH
BD58;HANGUL SYLLABLE BOEM
BD59;HANGUL SYLLABLE BOEB
BD5A;HANGUL SYLLABLE BOEBS
BD5B;HANGUL SYLLABLE BOES
BD5C;HANGUL SYLLABLE BOESS
BD5D;HANGUL SYLLABLE BOENG
BD5E;HANGUL SYLLABLE BOEJ
BD5F;HANGUL SYLLABLE BOEC
BD60;HANGUL SYLLABLE BOEK
BD61;HANGUL SYLLABLE BOET
BD62;HANGUL SYLLABLE BOEP
BD63;HANGUL SYLLABLE BOEH
BD64;HANGUL SYLLABLE BYO
BD65;HANGUL SYLLABLE BYOG
BD66;HANGUL SYLLABLE BYOGG
BD67;HANGUL SYLLABLE BYOGS
BD68;HANGUL SYLLABLE BYON
BD69;HANGUL SYLLABLE BYONJ
BD6A;HANGUL SYLLABLE BYONH
BD6B;HANGUL SYLLABLE BYOD
BD6C;HANGUL SYLLABLE BYOL
BD6D;HANGUL SYLLABLE BYOLG
BD6E;HANGUL SYLLABLE BYOLM
BD6F;HANGUL SYLLABLE BYOLB
BD70;HANGUL SYLLABLE BYOLS
BD71;HANGUL SYLLABLE BYOLT
BD72;HANGUL SYLLABLE BYOLP
BD73;HANGUL SYLLABLE BYOLH
BD74;HANGUL SYLLABLE BYOM
BD75;HANGUL SYLLABLE BYOB
BD76;HANGUL SYLLABLE BYOBS
BD77;HANGUL SYLLABLE BYOS
BD78;HANGUL SYLLABLE BYOSS
BD79;HANGUL SYLLABLE BYONG
BD7A;HANGUL SYLLABLE BYOJ
BD7B;HANGUL SYLLABLE BYOC
BD7C;HANGUL SYLLABLE BYOK
BD7D;HANGUL SYLLABLE BYOT
BD7E;HANGUL SYLLABLE BYOP
BD7F;HANGUL SYLLABLE BYOH
BD80;HANGUL SYLLABLE BU
BD81;HANGUL SYLLABLE BUG
BD82;HANGUL SYLLABLE BUGG
BD83;HANGUL SYLLABLE BUGS
BD84;HANGUL SYLLABLE BUN
BD85;HANGUL SYLLABLE BUNJ
BD86;HANGUL SYLLABLE BUNH
BD87;HANGUL SYLLABLE BUD
BD88;HANGUL SYLLABLE BUL
BD89;HANGUL SYLLABLE BULG
BD8A;HANGUL SYLLABLE BULM
BD8B;HANGUL SYLLABLE BULB
BD8C;HANGUL SYLLABLE BULS
BD8D;HANGUL SYLLABLE BULT
BD8E;HANGUL SYLLABLE BULP
BD8F;HANGUL SYLLABLE BULH
BD90;HANGUL SYLLABLE BUM
BD91;HANGUL SYLLABLE BUB
BD92;HANGUL SYLLABLE BUBS
BD93;HANGUL SYLLABLE BUS
BD94;HANGUL SYLLABLE BUSS
BD95;HANGUL SYLLABLE BUNG
BD96;HANGUL SYLLABLE BUJ
BD97;HANGUL SYLLABLE BUC
BD98;HANGUL SYLLABLE BUK
BD99;HANGUL SYLLABLE BUT
BD9A;HANGUL SYLLABLE BUP
BD9B;HANGUL SYLLABLE BUH
BD9C;HANGUL SYLLABLE BWEO
BD9D;HANGUL SYLLABLE BWEOG
BD9E;HANGUL SYLLABLE BWEOGG
BD9F;HANGUL SYLLABLE BWEOGS
BDA0;HANGUL SYLLABLE BWEON
BDA1;HANGUL SYLLABLE BWEONJ
BDA2;HANGUL SYLLABLE BWEONH
BDA3;HANGUL SYLLABLE BWEOD
BDA4;HANGUL SYLLABLE BWEOL
BDA5;HANGUL SYLLABLE BWEOLG
BDA6;HANGUL SYLLABLE BWEOLM
BDA7;HANGUL SYLLABLE BWEOLB
BDA8;HANGUL SYLLABLE BWEOLS
BDA9;HANGUL SYLLABLE BWEOLT
BDAA;HANGUL SYLLABLE BWEOLP
BDAB;HANGUL SYLLABLE BWEOLH
BDAC;HANGUL SYLLABLE BWEOM
BDAD;HANGUL SYLLABLE BWEOB
BDAE;HANGUL SYLLABLE BWEOBS
BDAF;HANGUL SYLLABLE BWEOS
BDB0;HANGUL SYLLABLE BWEOSS
BDB1;HANGUL SYLLABLE BWEONG
BDB2;HANGUL SYLLABLE BWEOJ
BDB3;HANGUL SYLLABLE BWEOC
BDB4;HANGUL SYLLABLE BWEOK
BDB5;HANGUL SYLLABLE BWEOT
BDB6;HANGUL SYLLABLE BWEOP
BDB7;HANGUL SYLLABLE BWEOH
BDB8;HANGUL SYLLABLE BWE
BDB9;HANGUL SYLLABLE BWEG
BDBA;HANGUL SYLLABLE BWEGG
BDBB;HANGUL SYLLABLE BWEGS
BDBC;HANGUL SYLLABLE BWEN
BDBD;HANGUL SYLLABLE BWENJ
BDBE;HANGUL SYLLABLE BWENH
BDBF;HANGUL SYLLABLE BWED
BDC0;HANGUL SYLLABLE BWEL
BDC1;HANGUL SYLLABLE BWELG
BDC2;HANGUL SYLLABLE BWELM
BDC3;HANGUL SYLLABLE BWELB
BDC4;HANGUL SYLLABLE BWELS
BDC5;HANGUL SYLLABLE BWELT
BDC6;HANGUL SYLLABLE BWELP
BDC7;HANGUL SYLLABLE BWELH
BDC8;HANGUL SYLLABLE BWEM
BDC9;HANGUL SYLLABLE BWEB
BDCA;HANGUL SYLLABLE BWEBS
BDCB;HANGUL SYLLABLE BWES
BDCC;HANGUL SYLLABLE BWESS
BDCD;HANGUL SYLLABLE BWENG
BDCE;HANGUL SYLLABLE BWEJ
BDCF;HANGUL SYLLABLE BWEC
BDD0;HANGUL SYLLABLE BWEK
BDD1;HANGUL SYLLABLE BWET
BDD2;HANGUL SYLLABLE BWEP
BDD3;HANGUL SYLLABLE BWEH
BDD4;HANGUL SYLLABLE BWI
BDD5;HANGUL SYLLABLE BWIG
BDD6;HANGUL SYLLABLE BWIGG
BDD7;HANGUL SYLLABLE BWIGS
BDD8;HANGUL SYLLABLE BWIN
BDD9;HANGUL SYLLABLE BWINJ
BDDA;HANGUL SYLLABLE BWINH
BDDB;HANGUL SYLLABLE BWID
BDDC;HANGUL SYLLABLE BWIL
BDDD;HANGUL SYLLABLE BWILG
BDDE;HANGUL SYLLABLE BWILM
BDDF;HANGUL SYLLABLE BWILB
BDE0;HANGUL SYLLABLE BWILS
BDE1;HANGUL SYLLABLE BWILT
BDE2;HANGUL SYLLABLE BWILP
BDE3;HANGUL SYLLABLE BWILH
BDE4;HANGUL SYLLABLE BWIM
BDE5;HANGUL SYLLABLE BWIB
BDE6;HANGUL SYLLABLE BWIBS
BDE7;HANGUL SYLLABLE BWIS
BDE8;HANGUL SYLLABLE BWISS
BDE9;HANGUL SYLLABLE BWING
BDEA;HANGUL SYLLABLE BWIJ
BDEB;HANGUL SYLLABLE BWIC
BDEC;HANGUL SYLLABLE BWIK
BDED;HANGUL SYLLABLE BWIT
BDEE;HANGUL SYLLABLE BWIP
BDEF;HANGUL SYLLABLE BWIH
BDF0;HANGUL SYLLABLE BYU
BDF1;HANGUL SYLLABLE BYUG
BDF2;HANGUL SYLLABLE BYUGG
BDF3;HANGUL SYLLABLE BYUGS
BDF4;HANGUL SYLLABLE BYUN
BDF5;HANGUL SYLLABLE BYUNJ
BDF6;HANGUL SYLLABLE BYUNH
BDF7;HANGUL SYLLABLE BYUD
BDF8;HANGUL SYLLABLE BYUL
BDF9;HANGUL SYLLABLE BYULG
BDFA;HANGUL SYLLABLE BYULM
BDFB;HANGUL SYLLABLE BYULB
BDFC;HANGUL SYLLABLE BYULS
BDFD;HANGUL SYLLABLE BYULT
BDFE;HANGUL SYLLABLE BYULP
BDFF;HANGUL SYLLABLE BYULH
BE00;HANGUL SYLLABLE BYUM
BE01;HANGUL SYLLABLE BYUB
BE02;HANGUL SYLLABLE BYUBS
BE03;HANGUL SYLLABLE BYUS
BE04;HANGUL SYLLABLE BYUSS
BE05;HANGUL SYLLABLE BYUNG
BE06;HANGUL SYLLABLE BYUJ
BE07;HANGUL SYLLABLE BYUC
BE08;HANGUL SYLLABLE BYUK
BE09;HANGUL SYLLABLE BYUT
BE0A;HANGUL SYLLABLE BYUP
BE0B;HANGUL SYLLABLE BYUH
BE0C;HANGUL SYLLABLE BEU
BE0D;HANGUL SYLLABLE BEUG
BE0E;HANGUL SYLLABLE BEUGG
BE0F;HANGUL SYLLABLE BEUGS
BE10;HANGUL SYLLABLE BEUN
BE11;HANGUL SYLLABLE BEUNJ
BE12;HANGUL SYLLABLE BEUNH
BE13;HANGUL SYLLABLE BEUD
BE14;HANGUL SYLLABLE BEUL
BE15;HANGUL SYLLABLE BEULG
BE16;HANGUL SYLLABLE BEULM
BE17;HANGUL SYLLABLE BEULB
BE18;HANGUL SYLLABLE BEULS
BE19;HANGUL SYLLABLE BEULT
BE1A;HANGUL SYLLABLE BEULP
BE1B;HANGUL SYLLABLE BEULH
BE1C;HANGUL SYLLABLE BEUM
BE1D;HANGUL SYLLABLE BEUB
BE1E;HANGUL SYLLABLE BEUBS
BE1F;HANGUL SYLLABLE BEUS
BE20;HANGUL SYLLABLE BEUSS
BE21;HANGUL SYLLABLE BEUNG
BE22;HANGUL SYLLABLE BEUJ
BE23;HANGUL SYLLABLE BEUC
BE24;HANGUL SYLLABLE BEUK
BE25;HANGUL SYLLABLE BEUT
BE26;HANGUL SYLLABLE BEUP
BE27;HANGUL SYLLABLE BEUH
BE28;HANGUL SYLLABLE BYI
BE29;HANGUL SYLLABLE BYIG
BE2A;HANGUL SYLLABLE BYIGG
BE2B;HANGUL SYLLABLE BYIGS
BE2C;HANGUL SYLLABLE BYIN
BE2D;HANGUL SYLLABLE BYINJ
BE2E;HANGUL SYLLABLE BYINH
BE2F;HANGUL SYLLABLE BYID
BE30;HANGUL SYLLABLE BYIL
BE31;HANGUL SYLLABLE BYILG
BE32;HANGUL SYLLABLE BYILM
BE33;HANGUL SYLLABLE BYILB
BE34;HANGUL SYLLABLE BYILS
BE35;HANGUL SYLLABLE BYILT
BE36;HANGUL SYLLABLE BYILP
BE37;HANGUL SYLLABLE BYILH
BE38;HANGUL SYLLABLE BYIM
BE39;HANGUL SYLLABLE BYIB
BE3A;HANGUL SYLLABLE BYIBS
BE3B;HANGUL SYLLABLE BYIS
BE3C;HANGUL SYLLABLE BYISS
BE3D;HANGUL SYLLABLE BYING
BE3E;HANGUL SYLLABLE BYIJ
BE3F;HANGUL SYLLABLE BYIC
BE40;HANGUL SYLLABLE BYIK
BE41;HANGUL SYLLABLE BYIT
BE42;HANGUL SYLLABLE BYIP
BE43;HANGUL SYLLABLE BYIH
BE44;HANGUL SYLLABLE BI
BE45;HANGUL SYLLABLE BIG
BE46;HANGUL SYLLABLE BIGG
BE47;HANGUL SYLLABLE BIGS
BE48;HANGUL SYLLABLE BIN
BE49;HANGUL SYLLABLE BINJ
BE4A;HANGUL SYLLABLE BINH
BE4B;HANGUL SYLLABLE BID
BE4C;HANGUL SYLLABLE BIL
BE4D;HANGUL SYLLABLE BILG
BE4E;HANGUL SYLLABLE BILM
BE4F;HANGUL SYLLABLE BILB
BE50;HANGUL SYLLABLE BILS
BE51;HANGUL SYLLABLE BILT
BE52;HANGUL SYLLABLE BILP
BE53;HANGUL SYLLABLE BILH
BE54;HANGUL SYLLABLE BIM
BE55;HANGUL SYLLABLE BIB
BE56;HANGUL SYLLABLE BIBS
BE57;HANGUL SYLLABLE BIS
BE58;HANGUL SYLLABLE BISS
BE59;HANGUL SYLLABLE BING
BE5A;HANGUL SYLLABLE BIJ
BE5B;HANGUL SYLLABLE BIC
BE5C;HANGUL SYLLABLE BIK
BE5D;HANGUL SYLLABLE BIT
BE5E;HANGUL SYLLABLE BIP
BE5F;HANGUL SYLLABLE BIH
BE60;HANGUL SYLLABLE BBA
BE61;HANGUL SYLLABLE BBAG
BE62;HANGUL SYLLABLE BBAGG
BE63;HANGUL SYLLABLE BBAGS
BE64;HANGUL SYLLABLE BBAN
BE65;HANGUL SYLLABLE BBANJ
BE66;HANGUL SYLLABLE BBANH
BE67;HANGUL SYLLABLE BBAD
BE68;HANGUL SYLLABLE BBAL
BE69;HANGUL SYLLABLE BBALG
BE6A;HANGUL SYLLABLE BBALM
BE6B;HANGUL SYLLABLE BBALB
BE6C;HANGUL SYLLABLE BBALS
BE6D;HANGUL SYLLABLE BBALT
BE6E;HANGUL SYLLABLE BBALP
BE6F;HANGUL SYLLABLE BBALH
BE70;HANGUL SYLLABLE BBAM
BE71;HANGUL SYLLABLE BBAB
BE72;HANGUL SYLLABLE BBABS
BE73;HANGUL SYLLABLE BBAS
BE74;HANGUL SYLLABLE BBASS
BE75;HANGUL SYLLABLE BBANG
BE76;HANGUL SYLLABLE BBAJ
BE77;HANGUL SYLLABLE BBAC
BE78;HANGUL SYLLABLE BBAK
BE79;HANGUL SYLLABLE BBAT
BE7A;HANGUL SYLLABLE BBAP
BE7B;HANGUL SYLLABLE BBAH
BE7C;HANGUL SYLLABLE BBAE
BE7D;HANGUL SYLLABLE BBAEG
BE7E;HANGUL SYLLABLE BBAEGG
BE7F;HANGUL SYLLABLE BBAEGS
BE80;HANGUL SYLLABLE BBAEN
BE81;HANGUL SYLLABLE BBAENJ
BE82;HANGUL SYLLABLE BBAENH
BE83;HANGUL SYLLABLE BBAED
BE84;HANGUL SYLLABLE BBAEL
BE85;HANGUL SYLLABLE BBAELG
BE86;HANGUL SYLLABLE BBAELM
BE87;HANGUL SYLLABLE BBAELB
BE88;HANGUL SYLLABLE BBAELS
BE89;HANGUL SYLLABLE BBAELT
BE8A;HANGUL SYLLABLE BBAELP
BE8B;HANGUL SYLLABLE BBAELH
BE8C;HANGUL SYLLABLE BBAEM
BE8D;HANGUL SYLLABLE BBAEB
BE8E;HANGUL SYLLABLE BBAEBS
BE8F;HANGUL SYLLABLE BBAES
BE90;HANGUL SYLLABLE BBAESS
BE91;HANGUL SYLLABLE BBAENG
BE92;HANGUL SYLLABLE BBAEJ
BE93;HANGUL SYLLABLE BBAEC
BE94;HANGUL SYLLABLE BBAEK
BE95;HANGUL SYLLABLE BBAET
BE96;HANGUL SYLLABLE BBAEP
BE97;HANGUL SYLLABLE BBAEH
BE98;HANGUL SYLLABLE BBYA
BE99;HANGUL SYLLABLE BBYAG
BE9A;HANGUL SYLLABLE BBYAGG
BE9B;HANGUL SYLLABLE BBYAGS
BE9C;HANGUL SYLLABLE BBYAN
BE9D;HANGUL SYLLABLE BBYANJ
BE9E;HANGUL SYLLABLE BBYANH
BE9F;HANGUL SYLLABLE BBYAD
BEA0;HANGUL SYLLABLE BBYAL
BEA1;HANGUL SYLLABLE BBYALG
BEA2;HANGUL SYLLABLE BBYALM
BEA3;HANGUL SYLLABLE BBYALB
BEA4;HANGUL SYLLABLE BBYALS
BEA5;HANGUL SYLLABLE BBYALT
BEA6;HANGUL SYLLABLE BBYALP
BEA7;HANGUL SYLLABLE BBYALH
BEA8;HANGUL SYLLABLE BBYAM
BEA9;HANGUL SYLLABLE BBYAB
BEAA;HANGUL SYLLABLE BBYABS
BEAB;HANGUL SYLLABLE BBYAS
BEAC;HANGUL SYLLABLE BBYASS
BEAD;HANGUL SYLLABLE BBYANG
BEAE;HANGUL SYLLABLE BBYAJ
BEAF;HANGUL SYLLABLE BBYAC
BEB0;HANGUL SYLLABLE BBYAK
BEB1;HANGUL SYLLABLE BBYAT
BEB2;HANGUL SYLLABLE BBYAP
BEB3;HANGUL SYLLABLE BBYAH
BEB4;HANGUL SYLLABLE BBYAE
BEB5;HANGUL SYLLABLE BBYAEG
BEB6;HANGUL SYLLABLE BBYAEGG
BEB7;HANGUL SYLLABLE BBYAEGS
BEB8;HANGUL SYLLABLE BBYAEN
BEB9;HANGUL SYLLABLE BBYAENJ
BEBA;HANGUL SYLLABLE BBYAENH
BEBB;HANGUL SYLLABLE BBYAED
BEBC;HANGUL SYLLABLE BBYAEL
BEBD;HANGUL SYLLABLE BBYAELG
BEBE;HANGUL SYLLABLE BBYAELM
BEBF;HANGUL SYLLABLE BBYAELB
BEC0;HANGUL SYLLABLE BBYAELS
BEC1;HANGUL SYLLABLE BBYAELT
BEC2;HANGUL SYLLABLE BBYAELP
BEC3;HANGUL SYLLABLE BBYAELH
BEC4;HANGUL SYLLABLE BBYAEM
BEC5;HANGUL SYLLABLE BBYAEB
BEC6;HANGUL SYLLABLE BBYAEBS
BEC7;HANGUL SYLLABLE BBYAES
BEC8;HANGUL SYLLABLE BBYAESS
BEC9;HANGUL SYLLABLE BBYAENG
BECA;HANGUL SYLLABLE BBYAEJ
BECB;HANGUL SYLLABLE BBYAEC
BECC;HANGUL SYLLABLE BBYAEK
BECD;HANGUL SYLLABLE BBYAET
BECE;HANGUL SYLLABLE BBYAEP
BECF;HANGUL SYLLABLE BBYAEH
BED0;HANGUL SYLLABLE BBEO
BED1;HANGUL SYLLABLE BBEOG
BED2;HANGUL SYLLABLE BBEOGG
BED3;HANGUL SYLLABLE BBEOGS
BED4;HANGUL SYLLABLE BBEON
BED5;HANGUL SYLLABLE BBEONJ
BED6;HANGUL SYLLABLE BBEONH
BED7;HANGUL SYLLABLE BBEOD
BED8;HANGUL SYLLABLE BBEOL
BED9;HANGUL SYLLABLE BBEOLG
BEDA;HANGUL SYLLABLE BBEOLM
BEDB;HANGUL SYLLABLE BBEOLB
BEDC;HANGUL SYLLABLE BBEOLS
BEDD;HANGUL SYLLABLE BBEOLT
BEDE;HANGUL SYLLABLE BBEOLP
BEDF;HANGUL SYLLABLE BBEOLH
BEE0;HANGUL SYLLABLE BBEOM
BEE1;HANGUL SYLLABLE BBEOB
BEE2;HANGUL SYLLABLE BBEOBS
BEE3;HANGUL SYLLABLE BBEOS
BEE4;HANGUL SYLLABLE BBEOSS
BEE5;HANGUL SYLLABLE BBEONG
BEE6;HANGUL SYLLABLE BBEOJ
BEE7;HANGUL SYLLABLE BBEOC
BEE8;HANGUL SYLLABLE BBEOK
BEE9;HANGUL SYLLABLE BBEOT
BEEA;HANGUL SYLLABLE BBEOP
BEEB;HANGUL SYLLABLE BBEOH
BEEC;HANGUL SYLLABLE BBE
BEED;HANGUL SYLLABLE BBEG
BEEE;HANGUL SYLLABLE BBEGG
BEEF;HANGUL SYLLABLE BBEGS
BEF0;HANGUL SYLLABLE BBEN
BEF1;HANGUL SYLLABLE BBENJ
BEF2;HANGUL SYLLABLE BBENH
BEF3;HANGUL SYLLABLE BBED
BEF4;HANGUL SYLLABLE BBEL
BEF5;HANGUL SYLLABLE BBELG
BEF6;HANGUL SYLLABLE BBELM
BEF7;HANGUL SYLLABLE BBELB
BEF8;HANGUL SYLLABLE BBELS
BEF9;HANGUL SYLLABLE BBELT
BEFA;HANGUL SYLLABLE BBELP
BEFB;HANGUL SYLLABLE BBELH
BEFC;HANGUL SYLLABLE BBEM
BEFD;HANGUL SYLLABLE BBEB
BEFE;HANGUL SYLLABLE BBEBS
BEFF;HANGUL SYLLABLE BBES
BF00;HANGUL SYLLABLE BBESS
BF01;HANGUL SYLLABLE BBENG
BF02;HANGUL SYLLABLE BBEJ
BF03;HANGUL SYLLABLE BBEC
BF04;HANGUL SYLLABLE BBEK
BF05;HANGUL SYLLABLE BBET
BF06;HANGUL SYLLABLE BBEP
BF07;HANGUL SYLLABLE BBEH
BF08;HANGUL SYLLABLE BBYEO
BF09;HANGUL SYLLABLE BBYEOG
BF0A;HANGUL SYLLABLE BBYEOGG
BF0B;HANGUL SYLLABLE BBYEOGS
BF0C;HANGUL SYLLABLE BBYEON
BF0D;HANGUL SYLLABLE BBYEONJ
BF0E;HANGUL SYLLABLE BBYEONH
BF0F;HANGUL SYLLABLE BBYEOD
BF10;HANGUL SYLLABLE BBYEOL
BF11;HANGUL SYLLABLE BBYEOLG
BF12;HANGUL SYLLABLE BBYEOLM
BF13;HANGUL SYLLABLE BBYEOLB
BF14;HANGUL SYLLABLE BBYEOLS
BF15;HANGUL SYLLABLE BBYEOLT
BF16;HANGUL SYLLABLE BBYEOLP
BF17;HANGUL SYLLABLE BBYEOLH
BF18;HANGUL SYLLABLE BBYEOM
BF19;HANGUL SYLLABLE BBYEOB
BF1A;HANGUL SYLLABLE BBYEOBS
BF1B;HANGUL SYLLABLE BBYEOS
BF1C;HANGUL SYLLABLE BBYEOSS
BF1D;HANGUL SYLLABLE BBYEONG
BF1E;HANGUL SYLLABLE BBYEOJ
BF1F;HANGUL SYLLABLE BBYEOC
BF20;HANGUL SYLLABLE BBYEOK
BF21;HANGUL SYLLABLE BBYEOT
BF22;HANGUL SYLLABLE BBYEOP
BF23;HANGUL SYLLABLE BBYEOH
BF24;HANGUL SYLLABLE BBYE
BF25;HANGUL SYLLABLE BBYEG
BF26;HANGUL SYLLABLE BBYEGG
BF27;HANGUL SYLLABLE BBYEGS
BF28;HANGUL SYLLABLE BBYEN
BF29;HANGUL SYLLABLE BBYENJ
BF2A;HANGUL SYLLABLE BBYENH
BF2B;HANGUL SYLLABLE BBYED
BF2C;HANGUL SYLLABLE BBYEL
BF2D;HANGUL SYLLABLE BBYELG
BF2E;HANGUL SYLLABLE BBYELM
BF2F;HANGUL SYLLABLE BBYELB
BF30;HANGUL SYLLABLE BBYELS
BF31;HANGUL SYLLABLE BBYELT
BF32;HANGUL SYLLABLE BBYELP
BF33;HANGUL SYLLABLE BBYELH
BF34;HANGUL SYLLABLE BBYEM
BF35;HANGUL SYLLABLE BBYEB
BF36;HANGUL SYLLABLE BBYEBS
BF37;HANGUL SYLLABLE BBYES
BF38;HANGUL SYLLABLE BBYESS
BF39;HANGUL SYLLABLE BBYENG
BF3A;HANGUL SYLLABLE BBYEJ
BF3B;HANGUL SYLLABLE BBYEC
BF3C;HANGUL SYLLABLE BBYEK
BF3D;HANGUL SYLLABLE BBYET
BF3E;HANGUL SYLLABLE BBYEP
BF3F;HANGUL SYLLABLE BBYEH
BF40;HANGUL SYLLABLE BBO
BF41;HANGUL SYLLABLE BBOG
BF42;HANGUL SYLLABLE BBOGG
BF43;HANGUL SYLLABLE BBOGS
BF44;HANGUL SYLLABLE BBON
BF45;HANGUL SYLLABLE BBONJ
BF46;HANGUL SYLLABLE BBONH
BF47;HANGUL SYLLABLE BBOD
BF48;HANGUL SYLLABLE BBOL
BF49;HANGUL SYLLABLE BBOLG
BF4A;HANGUL SYLLABLE BBOLM
BF4B;HANGUL SYLLABLE BBOLB
BF4C;HANGUL SYLLABLE BBOLS
BF4D;HANGUL SYLLABLE BBOLT
BF4E;HANGUL SYLLABLE BBOLP
BF4F;HANGUL SYLLABLE BBOLH
BF50;HANGUL SYLLABLE BBOM
BF51;HANGUL SYLLABLE BBOB
BF52;HANGUL SYLLABLE BBOBS
BF53;HANGUL SYLLABLE BBOS
BF54;HANGUL SYLLABLE BBOSS
BF55;HANGUL SYLLABLE BBONG
BF56;HANGUL SYLLABLE BBOJ
BF57;HANGUL SYLLABLE BBOC
BF58;HANGUL SYLLABLE BBOK
BF59;HANGUL SYLLABLE BBOT
BF5A;HANGUL SYLLABLE BBOP
BF5B;HANGUL SYLLABLE BBOH
BF5C;HANGUL SYLLABLE BBWA
BF5D;HANGUL SYLLABLE BBWAG
BF5E;HANGUL SYLLABLE BBWAGG
BF5F;HANGUL SYLLABLE BBWAGS
BF60;HANGUL SYLLABLE BBWAN
BF61;HANGUL SYLLABLE BBWANJ
BF62;HANGUL SYLLABLE BBWANH
BF63;HANGUL SYLLABLE BBWAD
BF64;HANGUL SYLLABLE BBWAL
BF65;HANGUL SYLLABLE BBWALG
BF66;HANGUL SYLLABLE BBWALM
BF67;HANGUL SYLLABLE BBWALB
BF68;HANGUL SYLLABLE BBWALS
BF69;HANGUL SYLLABLE BBWALT
BF6A;HANGUL SYLLABLE BBWALP
BF6B;HANGUL SYLLABLE BBWALH
BF6C;HANGUL SYLLABLE BBWAM
BF6D;HANGUL SYLLABLE BBWAB
BF6E;HANGUL SYLLABLE BBWABS
BF6F;HANGUL SYLLABLE BBWAS
BF70;HANGUL SYLLABLE BBWASS
BF71;HANGUL SYLLABLE BBWANG
BF72;HANGUL SYLLABLE BBWAJ
BF73;HANGUL SYLLABLE BBWAC
BF74;HANGUL SYLLABLE BBWAK
BF75;HANGUL SYLLABLE BBWAT
BF76;HANGUL SYLLABLE BBWAP
BF77;HANGUL SYLLABLE BBWAH
BF78;HANGUL SYLLABLE BBWAE
BF79;HANGUL SYLLABLE BBWAEG
BF7A;HANGUL SYLLABLE BBWAEGG
BF7B;HANGUL SYLLABLE BBWAEGS
BF7C;HANGUL SYLLABLE BBWAEN
BF7D;HANGUL SYLLABLE BBWAENJ
BF7E;HANGUL SYLLABLE BBWAENH
BF7F;HANGUL SYLLABLE BBWAED
BF80;HANGUL SYLLABLE BBWAEL
BF81;HANGUL SYLLABLE BBWAELG
BF82;HANGUL SYLLABLE BBWAELM
BF83;HANGUL SYLLABLE BBWAELB
BF84;HANGUL SYLLABLE BBWAELS
BF85;HANGUL SYLLABLE BBWAELT
BF86;HANGUL SYLLABLE BBWAELP
BF87;HANGUL SYLLABLE BBWAELH
BF88;HANGUL SYLLABLE BBWAEM
BF89;HANGUL SYLLABLE BBWAEB
BF8A;HANGUL SYLLABLE BBWAEBS
BF8B;HANGUL SYLLABLE BBWAES
BF8C;HANGUL SYLLABLE BBWAESS
BF8D;HANGUL SYLLABLE BBWAENG
BF8E;HANGUL SYLLABLE BBWAEJ
BF8F;HANGUL SYLLABLE BBWAEC
BF90;HANGUL SYLLABLE BBWAEK
BF91;HANGUL SYLLABLE BBWAET
BF92;HANGUL SYLLABLE BBWAEP
BF93;HANGUL SYLLABLE BBWAEH
BF94;HANGUL SYLLABLE BBOE
BF95;HANGUL SYLLABLE BBOEG
BF96;HANGUL SYLLABLE BBOEGG
BF97;HANGUL SYLLABLE BBOEGS
BF98;HANGUL SYLLABLE BBOEN
BF99;HANGUL SYLLABLE BBOENJ
BF9A;HANGUL SYLLABLE BBOENH
BF9B;HANGUL SYLLABLE BBOED
BF9C;HANGUL SYLLABLE BBOEL
BF9D;HANGUL SYLLABLE BBOELG
BF9E;HANGUL SYLLABLE BBOELM
BF9F;HANGUL SYLLABLE BBOELB
BFA0;HANGUL SYLLABLE BBOELS
BFA1;HANGUL SYLLABLE BBOELT
BFA2;HANGUL SYLLABLE BBOELP
BFA3;HANGUL SYLLABLE BBOELH
BFA4;HANGUL SYLLABLE BBOEM
BFA5;HANGUL SYLLABLE BBOEB
BFA6;HANGUL SYLLABLE BBOEBS
BFA7;HANGUL SYLLABLE BBOES
BFA8;HANGUL SYLLABLE BBOESS
BFA9;HANGUL SYLLABLE BBOENG
BFAA;HANGUL SYLLABLE BBOEJ
BFAB;HANGUL SYLLABLE BBOEC
BFAC;HANGUL SYLLABLE BBOEK
BFAD;HANGUL SYLLABLE BBOET
BFAE;HANGUL SYLLABLE BBOEP
BFAF;HANGUL SYLLABLE BBOEH
BFB0;HANGUL SYLLABLE BBYO
BFB1;HANGUL SYLLABLE BBYOG
BFB2;HANGUL SYLLABLE BBYOGG
BFB3;HANGUL SYLLABLE BBYOGS
BFB4;HANGUL SYLLABLE BBYON
BFB5;HANGUL SYLLABLE BBYONJ
BFB6;HANGUL SYLLABLE BBYONH
BFB7;HANGUL SYLLABLE BBYOD
BFB8;HANGUL SYLLABLE BBYOL
BFB9;HANGUL SYLLABLE BBYOLG
BFBA;HANGUL SYLLABLE BBYOLM
BFBB;HANGUL SYLLABLE BBYOLB
BFBC;HANGUL SYLLABLE BBYOLS
BFBD;HANGUL SYLLABLE BBYOLT
BFBE;HANGUL SYLLABLE BBYOLP
BFBF;HANGUL SYLLABLE BBYOLH
BFC0;HANGUL SYLLABLE BBYOM
BFC1;HANGUL SYLLABLE BBYOB
BFC2;HANGUL SYLLABLE BBYOBS
BFC3;HANGUL SYLLABLE BBYOS
BFC4;HANGUL SYLLABLE BBYOSS
BFC5;HANGUL SYLLABLE BBYONG
BFC6;HANGUL SYLLABLE BBYOJ
BFC7;HANGUL SYLLABLE BBYOC
BFC8;HANGUL SYLLABLE BBYOK
BFC9;HANGUL SYLLABLE BBYOT
BFCA;HANGUL SYLLABLE BBYOP
BFCB;HANGUL SYLLABLE BBYOH
BFCC;HANGUL SYLLABLE BBU
BFCD;HANGUL SYLLABLE BBUG
BFCE;HANGUL SYLLABLE BBUGG
BFCF;HANGUL SYLLABLE BBUGS
BFD0;HANGUL SYLLABLE BBUN
BFD1;HANGUL SYLLABLE BBUNJ
BFD2;HANGUL SYLLABLE BBUNH
BFD3;HANGUL SYLLABLE BBUD
BFD4;HANGUL SYLLABLE BBUL
BFD5;HANGUL SYLLABLE BBULG
BFD6;HANGUL SYLLABLE BBULM
BFD7;HANGUL SYLLABLE BBULB
BFD8;HANGUL SYLLABLE BBULS
BFD9;HANGUL SYLLABLE BBULT
BFDA;HANGUL SYLLABLE BBULP
BFDB;HANGUL SYLLABLE BBULH
BFDC;HANGUL SYLLABLE BBUM
BFDD;HANGUL SYLLABLE BBUB
BFDE;HANGUL SYLLABLE BBUBS
BFDF;HANGUL SYLLABLE BBUS
BFE0;HANGUL SYLLABLE BBUSS
BFE1;HANGUL SYLLABLE BBUNG
BFE2;HANGUL SYLLABLE BBUJ
BFE3;HANGUL SYLLABLE BBUC
BFE4;HANGUL SYLLABLE BBUK
BFE5;HANGUL SYLLABLE BBUT
BFE6;HANGUL SYLLABLE BBUP
BFE7;HANGUL SYLLABLE BBUH
BFE8;HANGUL SYLLABLE BBWEO
BFE9;HANGUL SYLLABLE BBWEOG
BFEA;HANGUL SYLLABLE BBWEOGG
BFEB;HANGUL SYLLABLE BBWEOGS
BFEC;HANGUL SYLLABLE BBWEON
BFED;HANGUL SYLLABLE BBWEONJ
BFEE;HANGUL SYLLABLE BBWEONH
BFEF;HANGUL SYLLABLE BBWEOD
BFF0;HANGUL SYLLABLE BBWEOL
BFF1;HANGUL SYLLABLE BBWEOLG
BFF2;HANGUL SYLLABLE BBWEOLM
BFF3;HANGUL SYLLABLE BBWEOLB
BFF4;HANGUL SYLLABLE BBWEOLS
BFF5;HANGUL SYLLABLE BBWEOLT
BFF6;HANGUL SYLLABLE BBWEOLP
BFF7;HANGUL SYLLABLE BBWEOLH
BFF8;HANGUL SYLLABLE BBWEOM
BFF9;HANGUL SYLLABLE BBWEOB
BFFA;HANGUL SYLLABLE BBWEOBS
BFFB;HANGUL SYLLABLE BBWEOS
BFFC;HANGUL SYLLABLE BBWEOSS
BFFD;HANGUL SYLLABLE BBWEONG
BFFE;HANGUL SYLLABLE BBWEOJ
BFFF;HANGUL SYLLABLE BBWEOC
C000;HANGUL SYLLABLE BBWEOK
C001;HANGUL SYLLABLE BBWEOT
C002;HANGUL SYLLABLE BBWEOP
C003;HANGUL SYLLABLE BBWEOH
C004;HANGUL SYLLABLE BBWE
C005;HANGUL SYLLABLE BBWEG
C006;HANGUL SYLLABLE BBWEGG
C007;HANGUL SYLLABLE BBWEGS
C008;HANGUL SYLLABLE BBWEN
C009;HANGUL SYLLABLE BBWENJ
C00A;HANGUL SYLLABLE BBWENH
C00B;HANGUL SYLLABLE BBWED
C00C;HANGUL SYLLABLE BBWEL
C00D;HANGUL SYLLABLE BBWELG
C00E;HANGUL SYLLABLE BBWELM
C00F;HANGUL SYLLABLE BBWELB
C010;HANGUL SYLLABLE BBWELS
C011;HANGUL SYLLABLE BBWELT
C012;HANGUL SYLLABLE BBWELP
C013;HANGUL SYLLABLE BBWELH
C014;HANGUL SYLLABLE BBWEM
C015;HANGUL SYLLABLE BBWEB
C016;HANGUL SYLLABLE BBWEBS
C017;HANGUL SYLLABLE BBWES
C018;HANGUL SYLLABLE BBWESS
C019;HANGUL SYLLABLE BBWENG
C01A;HANGUL SYLLABLE BBWEJ
C01B;HANGUL SYLLABLE BBWEC
C01C;HANGUL SYLLABLE BBWEK
C01D;HANGUL SYLLABLE BBWET
C01E;HANGUL SYLLABLE BBWEP
C01F;HANGUL SYLLABLE BBWEH
C020;HANGUL SYLLABLE BBWI
C021;HANGUL SYLLABLE BBWIG
C022;HANGUL SYLLABLE BBWIGG
C023;HANGUL SYLLABLE BBWIGS
C024;HANGUL SYLLABLE BBWIN
C025;HANGUL SYLLABLE BBWINJ
C026;HANGUL SYLLABLE BBWINH
C027;HANGUL SYLLABLE BBWID
C028;HANGUL SYLLABLE BBWIL
C029;HANGUL SYLLABLE BBWILG
C02A;HANGUL SYLLABLE BBWILM
C02B;HANGUL SYLLABLE BBWILB
C02C;HANGUL SYLLABLE BBWILS
C02D;HANGUL SYLLABLE BBWILT
C02E;HANGUL SYLLABLE BBWILP
C02F;HANGUL SYLLABLE BBWILH
C030;HANGUL SYLLABLE BBWIM
C031;HANGUL SYLLABLE BBWIB
C032;HANGUL SYLLABLE BBWIBS
C033;HANGUL SYLLABLE BBWIS
C034;HANGUL SYLLABLE BBWISS
C035;HANGUL SYLLABLE BBWING
C036;HANGUL SYLLABLE BBWIJ
C037;HANGUL SYLLABLE BBWIC
C038;HANGUL SYLLABLE BBWIK
C039;HANGUL SYLLABLE BBWIT
C03A;HANGUL SYLLABLE BBWIP
C03B;HANGUL SYLLABLE BBWIH
C03C;HANGUL SYLLABLE BBYU
C03D;HANGUL SYLLABLE BBYUG
C03E;HANGUL SYLLABLE BBYUGG
C03F;HANGUL SYLLABLE BBYUGS
C040;HANGUL SYLLABLE BBYUN
C041;HANGUL SYLLABLE BBYUNJ
C042;HANGUL SYLLABLE BBYUNH
C043;HANGUL SYLLABLE BBYUD
C044;HANGUL SYLLABLE BBYUL
C045;HANGUL SYLLABLE BBYULG
C046;HANGUL SYLLABLE BBYULM
C047;HANGUL SYLLABLE BBYULB
C048;HANGUL SYLLABLE BBYULS
C049;HANGUL SYLLABLE BBYULT
C04A;HANGUL SYLLABLE BBYULP
C04B;HANGUL SYLLABLE BBYULH
C04C;HANGUL SYLLABLE BBYUM
C04D;HANGUL SYLLABLE BBYUB
C04E;HANGUL SYLLABLE BBYUBS
C04F;HANGUL SYLLABLE BBYUS
C050;HANGUL SYLLABLE BBYUSS
C051;HANGUL SYLLABLE BBYUNG
C052;HANGUL SYLLABLE BBYUJ
C053;HANGUL SYLLABLE BBYUC
C054;HANGUL SYLLABLE BBYUK
C055;HANGUL SYLLABLE BBYUT
C056;HANGUL SYLLABLE BBYUP
C057;HANGUL SYLLABLE BBYUH
C058;HANGUL SYLLABLE BBEU
C059;HANGUL SYLLABLE BBEUG
C05A;HANGUL SYLLABLE BBEUGG
C05B;HANGUL SYLLABLE BBEUGS
C05C;HANGUL SYLLABLE BBEUN
C05D;HANGUL SYLLABLE BBEUNJ
C05E;HANGUL SYLLABLE BBEUNH
C05F;HANGUL SYLLABLE BBEUD
C060;HANGUL SYLLABLE BBEUL
C061;HANGUL SYLLABLE BBEULG
C062;HANGUL SYLLABLE BBEULM
C063;HANGUL SYLLABLE BBEULB
C064;HANGUL SYLLABLE BBEULS
C065;HANGUL SYLLABLE BBEULT
C066;HANGUL SYLLABLE BBEULP
C067;HANGUL SYLLABLE BBEULH
C068;HANGUL SYLLABLE BBEUM
C069;HANGUL SYLLABLE BBEUB
C06A;HANGUL SYLLABLE BBEUBS
C06B;HANGUL SYLLABLE BBEUS
C06C;HANGUL SYLLABLE BBEUSS
C06D;HANGUL SYLLABLE BBEUNG
C06E;HANGUL SYLLABLE BBEUJ
C06F;HANGUL SYLLABLE BBEUC
C070;HANGUL SYLLABLE BBEUK
C071;HANGUL SYLLABLE BBEUT
C072;HANGUL SYLLABLE BBEUP
C073;HANGUL SYLLABLE BBEUH
C074;HANGUL SYLLABLE BBYI
C075;HANGUL SYLLABLE BBYIG
C076;HANGUL SYLLABLE BBYIGG
C077;HANGUL SYLLABLE BBYIGS
C078;HANGUL SYLLABLE BBYIN
C079;HANGUL SYLLABLE BBYINJ
C07A;HANGUL SYLLABLE BBYINH
C07B;HANGUL SYLLABLE BBYID
C07C;HANGUL SYLLABLE BBYIL
C07D;HANGUL SYLLABLE BBYILG
C07E;HANGUL SYLLABLE BBYILM
C07F;HANGUL SYLLABLE BBYILB
C080;HANGUL SYLLABLE BBYILS
C081;HANGUL SYLLABLE BBYILT
C082;HANGUL SYLLABLE BBYILP
C083;HANGUL SYLLABLE BBYILH
C084;HANGUL SYLLABLE BBYIM
C085;HANGUL SYLLABLE BBYIB
C086;HANGUL SYLLABLE BBYIBS
C087;HANGUL SYLLABLE BBYIS
C088;HANGUL SYLLABLE BBYISS
C089;HANGUL SYLLABLE BBYING
C08A;HANGUL SYLLABLE BBYIJ
C08B;HANGUL SYLLABLE BBYIC
C08C;HANGUL SYLLABLE BBYIK
C08D;HANGUL SYLLABLE BBYIT
C08E;HANGUL SYLLABLE BBYIP
C08F;HANGUL SYLLABLE BBYIH
C090;HANGUL SYLLABLE BBI
C091;HANGUL SYLLABLE BBIG
C092;HANGUL SYLLABLE BBIGG
C093;HANGUL SYLLABLE BBIGS
C094;HANGUL SYLLABLE BBIN
C095;HANGUL SYLLABLE BBINJ
C096;HANGUL SYLLABLE BBINH
C097;HANGUL SYLLABLE BBID
C098;HANGUL SYLLABLE BBIL
C099;HANGUL SYLLABLE BBILG
C09A;HANGUL SYLLABLE BBILM
C09B;HANGUL SYLLABLE BBILB
C09C;HANGUL SYLLABLE BBILS
C09D;HANGUL SYLLABLE BBILT
C09E;HANGUL SYLLABLE BBILP
C09F;HANGUL SYLLABLE BBILH
C0A0;HANGUL SYLLABLE BBIM
C0A1;HANGUL SYLLABLE BBIB
C0A2;HANGUL SYLLABLE BBIBS
C0A3;HANGUL SYLLABLE BBIS
C0A4;HANGUL SYLLABLE BBISS
C0A5;HANGUL SYLLABLE BBING
C0A6;HANGUL SYLLABLE BBIJ
C0A7;HANGUL SYLLABLE BBIC
C0A8;HANGUL SYLLABLE BBIK
C0A9;HANGUL SYLLABLE BBIT
C0AA;HANGUL SYLLABLE BBIP
C0AB;HANGUL SYLLABLE BBIH
C0AC;HANGUL SYLLABLE SA
C0AD;HANGUL SYLLABLE SAG
C0AE;HANGUL SYLLABLE SAGG
C0AF;HANGUL SYLLABLE SAGS
C0B0;HANGUL SYLLABLE SAN
C0B1;HANGUL SYLLABLE SANJ
C0B2;HANGUL SYLLABLE SANH
C0B3;HANGUL SYLLABLE SAD
C0B4;HANGUL SYLLABLE SAL
C0B5;HANGUL SYLLABLE SALG
C0B6;HANGUL SYLLABLE SALM
C0B7;HANGUL SYLLABLE SALB
C0B8;HANGUL SYLLABLE SALS
C0B9;HANGUL SYLLABLE SALT
C0BA;HANGUL SYLLABLE SALP
C0BB;HANGUL SYLLABLE SALH
C0BC;HANGUL SYLLABLE SAM
C0BD;HANGUL SYLLABLE SAB
C0BE;HANGUL SYLLABLE SABS
C0BF;HANGUL SYLLABLE SAS
C0C0;HANGUL SYLLABLE SASS
C0C1;HANGUL SYLLABLE SANG
C0C2;HANGUL SYLLABLE SAJ
C0C3;HANGUL SYLLABLE SAC
C0C4;HANGUL SYLLABLE SAK
C0C5;HANGUL SYLLABLE SAT
C0C6;HANGUL SYLLABLE SAP
C0C7;HANGUL SYLLABLE SAH
C0C8;HANGUL SYLLABLE SAE
C0C9;HANGUL SYLLABLE SAEG
C0CA;HANGUL SYLLABLE SAEGG
C0CB;HANGUL SYLLABLE SAEGS
C0CC;HANGUL SYLLABLE SAEN
C0CD;HANGUL SYLLABLE SAENJ
C0CE;HANGUL SYLLABLE SAENH
C0CF;HANGUL SYLLABLE SAED
C0D0;HANGUL SYLLABLE SAEL
C0D1;HANGUL SYLLABLE SAELG
C0D2;HANGUL SYLLABLE SAELM
C0D3;HANGUL SYLLABLE SAELB
C0D4;HANGUL SYLLABLE SAELS
C0D5;HANGUL SYLLABLE SAELT
C0D6;HANGUL SYLLABLE SAELP
C0D7;HANGUL SYLLABLE SAELH
C0D8;HANGUL SYLLABLE SAEM
C0D9;HANGUL SYLLABLE SAEB
C0DA;HANGUL SYLLABLE SAEBS
C0DB;HANGUL SYLLABLE SAES
C0DC;HANGUL SYLLABLE SAESS
C0DD;HANGUL SYLLABLE SAENG
C0DE;HANGUL SYLLABLE SAEJ
C0DF;HANGUL SYLLABLE SAEC
C0E0;HANGUL SYLLABLE SAEK
C0E1;HANGUL SYLLABLE SAET
C0E2;HANGUL SYLLABLE SAEP
C0E3;HANGUL SYLLABLE SAEH
C0E4;HANGUL SYLLABLE SYA
C0E5;HANGUL SYLLABLE SYAG
C0E6;HANGUL SYLLABLE SYAGG
C0E7;HANGUL SYLLABLE SYAGS
C0E8;HANGUL SYLLABLE SYAN
C0E9;HANGUL SYLLABLE SYANJ
C0EA;HANGUL SYLLABLE SYANH
C0EB;HANGUL SYLLABLE SYAD
C0EC;HANGUL SYLLABLE SYAL
C0ED;HANGUL SYLLABLE SYALG
C0EE;HANGUL SYLLABLE SYALM
C0EF;HANGUL SYLLABLE SYALB
C0F0;HANGUL SYLLABLE SYALS
C0F1;HANGUL SYLLABLE SYALT
C0F2;HANGUL SYLLABLE SYALP
C0F3;HANGUL SYLLABLE SYALH
C0F4;HANGUL SYLLABLE SYAM
C0F5;HANGUL SYLLABLE SYAB
C0F6;HANGUL SYLLABLE SYABS
C0F7;HANGUL SYLLABLE SYAS
C0F8;HANGUL SYLLABLE SYASS
C0F9;HANGUL SYLLABLE SYANG
C0FA;HANGUL SYLLABLE SYAJ
C0FB;HANGUL SYLLABLE SYAC
C0FC;HANGUL SYLLABLE SYAK
C0FD;HANGUL SYLLABLE SYAT
C0FE;HANGUL SYLLABLE SYAP
C0FF;HANGUL SYLLABLE SYAH
C100;HANGUL SYLLABLE SYAE
C101;HANGUL SYLLABLE SYAEG
C102;HANGUL SYLLABLE SYAEGG
C103;HANGUL SYLLABLE SYAEGS
C104;HANGUL SYLLABLE SYAEN
C105;HANGUL SYLLABLE SYAENJ
C106;HANGUL SYLLABLE SYAENH
C107;HANGUL SYLLABLE SYAED
C108;HANGUL SYLLABLE SYAEL
C109;HANGUL SYLLABLE SYAELG
C10A;HANGUL SYLLABLE SYAELM
C10B;HANGUL SYLLABLE SYAELB
C10C;HANGUL SYLLABLE SYAELS
C10D;HANGUL SYLLABLE SYAELT
C10E;HANGUL SYLLABLE SYAELP
C10F;HANGUL SYLLABLE SYAELH
C110;HANGUL SYLLABLE SYAEM
C111;HANGUL SYLLABLE SYAEB
C112;HANGUL SYLLABLE SYAEBS
C113;HANGUL SYLLABLE SYAES
C114;HANGUL SYLLABLE SYAESS
C115;HANGUL SYLLABLE SYAENG
C116;HANGUL SYLLABLE SYAEJ
C117;HANGUL SYLLABLE SYAEC
C118;HANGUL SYLLABLE SYAEK
C119;HANGUL SYLLABLE SYAET
C11A;HANGUL SYLLABLE SYAEP
C11B;HANGUL SYLLABLE SYAEH
C11C;HANGUL SYLLABLE SEO
C11D;HANGUL SYLLABLE SEOG
C11E;HANGUL SYLLABLE SEOGG
C11F;HANGUL SYLLABLE SEOGS
C120;HANGUL SYLLABLE SEON
C121;HANGUL SYLLABLE SEONJ
C122;HANGUL SYLLABLE SEONH
C123;HANGUL SYLLABLE SEOD
C124;HANGUL SYLLABLE SEOL
C125;HANGUL SYLLABLE SEOLG
C126;HANGUL SYLLABLE SEOLM
C127;HANGUL SYLLABLE SEOLB
C128;HANGUL SYLLABLE SEOLS
C129;HANGUL SYLLABLE SEOLT
C12A;HANGUL SYLLABLE SEOLP
C12B;HANGUL SYLLABLE SEOLH
C12C;HANGUL SYLLABLE SEOM
C12D;HANGUL SYLLABLE SEOB
C12E;HANGUL SYLLABLE SEOBS
C12F;HANGUL SYLLABLE SEOS
C130;HANGUL SYLLABLE SEOSS
C131;HANGUL SYLLABLE SEONG
C132;HANGUL SYLLABLE SEOJ
C133;HANGUL SYLLABLE SEOC
C134;HANGUL SYLLABLE SEOK
C135;HANGUL SYLLABLE SEOT
C136;HANGUL SYLLABLE SEOP
C137;HANGUL SYLLABLE SEOH
C138;HANGUL SYLLABLE SE
C139;HANGUL SYLLABLE SEG
C13A;HANGUL SYLLABLE SEGG
C13B;HANGUL SYLLABLE SEGS
C13C;HANGUL SYLLABLE SEN
C13D;HANGUL SYLLABLE SENJ
C13E;HANGUL SYLLABLE SENH
C13F;HANGUL SYLLABLE SED
C140;HANGUL SYLLABLE SEL
C141;HANGUL SYLLABLE SELG
C142;HANGUL SYLLABLE SELM
C143;HANGUL SYLLABLE SELB
C144;HANGUL SYLLABLE SELS
C145;HANGUL SYLLABLE SELT
C146;HANGUL SYLLABLE SELP
C147;HANGUL SYLLABLE SELH
C148;HANGUL SYLLABLE SEM
C149;HANGUL SYLLABLE SEB
C14A;HANGUL SYLLABLE SEBS
C14B;HANGUL SYLLABLE SES
C14C;HANGUL SYLLABLE SESS
C14D;HANGUL SYLLABLE SENG
C14E;HANGUL SYLLABLE SEJ
C14F;HANGUL SYLLABLE SEC
C150;HANGUL SYLLABLE SEK
C151;HANGUL SYLLABLE SET
C152;HANGUL SYLLABLE SEP
C153;HANGUL SYLLABLE SEH
C154;HANGUL SYLLABLE SYEO
C155;HANGUL SYLLABLE SYEOG
C156;HANGUL SYLLABLE SYEOGG
C157;HANGUL SYLLABLE SYEOGS
C158;HANGUL SYLLABLE SYEON
C159;HANGUL SYLLABLE SYEONJ
C15A;HANGUL SYLLABLE SYEONH
C15B;HANGUL SYLLABLE SYEOD
C15C;HANGUL SYLLABLE SYEOL
C15D;HANGUL SYLLABLE SYEOLG
C15E;HANGUL SYLLABLE SYEOLM
C15F;HANGUL SYLLABLE SYEOLB
C160;HANGUL SYLLABLE SYEOLS
C161;HANGUL SYLLABLE SYEOLT
C162;HANGUL SYLLABLE SYEOLP
C163;HANGUL SYLLABLE SYEOLH
C164;HANGUL SYLLABLE SYEOM
C165;HANGUL SYLLABLE SYEOB
C166;HANGUL SYLLABLE SYEOBS
C167;HANGUL SYLLABLE SYEOS
C168;HANGUL SYLLABLE SYEOSS
C169;HANGUL SYLLABLE SYEONG
C16A;HANGUL SYLLABLE SYEOJ
C16B;HANGUL SYLLABLE SYEOC
C16C;HANGUL SYLLABLE SYEOK
C16D;HANGUL SYLLABLE SYEOT
C16E;HANGUL SYLLABLE SYEOP
C16F;HANGUL SYLLABLE SYEOH
C170;HANGUL SYLLABLE SYE
C171;HANGUL SYLLABLE SYEG
C172;HANGUL SYLLABLE SYEGG
C173;HANGUL SYLLABLE SYEGS
C174;HANGUL SYLLABLE SYEN
C175;HANGUL SYLLABLE SYENJ
C176;HANGUL SYLLABLE SYENH
C177;HANGUL SYLLABLE SYED
C178;HANGUL SYLLABLE SYEL
C179;HANGUL SYLLABLE SYELG
C17A;HANGUL SYLLABLE SYELM
C17B;HANGUL SYLLABLE SYELB
C17C;HANGUL SYLLABLE SYELS
C17D;HANGUL SYLLABLE SYELT
C17E;HANGUL SYLLABLE SYELP
C17F;HANGUL SYLLABLE SYELH
C180;HANGUL SYLLABLE SYEM
C181;HANGUL SYLLABLE SYEB
C182;HANGUL SYLLABLE SYEBS
C183;HANGUL SYLLABLE SYES
C184;HANGUL SYLLABLE SYESS
C185;HANGUL SYLLABLE SYENG
C186;HANGUL SYLLABLE SYEJ
C187;HANGUL SYLLABLE SYEC
C188;HANGUL SYLLABLE SYEK
C189;HANGUL SYLLABLE SYET
C18A;HANGUL SYLLABLE SYEP
C18B;HANGUL SYLLABLE SYEH
C18C;HANGUL SYLLABLE SO
C18D;HANGUL SYLLABLE SOG
C18E;HANGUL SYLLABLE SOGG
C18F;HANGUL SYLLABLE SOGS
C190;HANGUL SYLLABLE SON
C191;HANGUL SYLLABLE SONJ
C192;HANGUL SYLLABLE SONH
C193;HANGUL SYLLABLE SOD
C194;HANGUL SYLLABLE SOL
C195;HANGUL SYLLABLE SOLG
C196;HANGUL SYLLABLE SOLM
C197;HANGUL SYLLABLE SOLB
C198;HANGUL SYLLABLE SOLS
C199;HANGUL SYLLABLE SOLT
C19A;HANGUL SYLLABLE SOLP
C19B;HANGUL SYLLABLE SOLH
C19C;HANGUL SYLLABLE SOM
C19D;HANGUL SYLLABLE SOB
C19E;HANGUL SYLLABLE SOBS
C19F;HANGUL SYLLABLE SOS
C1A0;HANGUL SYLLABLE SOSS
C1A1;HANGUL SYLLABLE SONG
C1A2;HANGUL SYLLABLE SOJ
C1A3;HANGUL SYLLABLE SOC
C1A4;HANGUL SYLLABLE SOK
C1A5;HANGUL SYLLABLE SOT
C1A6;HANGUL SYLLABLE SOP
C1A7;HANGUL SYLLABLE SOH
C1A8;HANGUL SYLLABLE SWA
C1A9;HANGUL SYLLABLE SWAG
C1AA;HANGUL SYLLABLE SWAGG
C1AB;HANGUL SYLLABLE SWAGS
C1AC;HANGUL SYLLABLE SWAN
C1AD;HANGUL SYLLABLE SWANJ
C1AE;HANGUL SYLLABLE SWANH
C1AF;HANGUL SYLLABLE SWAD
C1B0;HANGUL SYLLABLE SWAL
C1B1;HANGUL SYLLABLE SWALG
C1B2;HANGUL SYLLABLE SWALM
C1B3;HANGUL SYLLABLE SWALB
C1B4;HANGUL SYLLABLE SWALS
C1B5;HANGUL SYLLABLE SWALT
C1B6;HANGUL SYLLABLE SWALP
C1B7;HANGUL SYLLABLE SWALH
C1B8;HANGUL SYLLABLE SWAM
C1B9;HANGUL SYLLABLE SWAB
C1BA;HANGUL SYLLABLE SWABS
C1BB;HANGUL SYLLABLE SWAS
C1BC;HANGUL SYLLABLE SWASS
C1BD;HANGUL SYLLABLE SWANG
C1BE;HANGUL SYLLABLE SWAJ
C1BF;HANGUL SYLLABLE SWAC
C1C0;HANGUL SYLLABLE SWAK
C1C1;HANGUL SYLLABLE SWAT
C1C2;HANGUL SYLLABLE SWAP
C1C3;HANGUL SYLLABLE SWAH
C1C4;HANGUL SYLLABLE SWAE
C1C5;HANGUL SYLLABLE SWAEG
C1C6;HANGUL SYLLABLE SWAEGG
C1C7;HANGUL SYLLABLE SWAEGS
C1C8;HANGUL SYLLABLE SWAEN
C1C9;HANGUL SYLLABLE SWAENJ
C1CA;HANGUL SYLLABLE SWAENH
C1CB;HANGUL SYLLABLE SWAED
C1CC;HANGUL SYLLABLE SWAEL
C1CD;HANGUL SYLLABLE SWAELG
C1CE;HANGUL SYLLABLE SWAELM
C1CF;HANGUL SYLLABLE SWAELB
C1D0;HANGUL SYLLABLE SWAELS
C1D1;HANGUL SYLLABLE SWAELT
C1D2;HANGUL SYLLABLE SWAELP
C1D3;HANGUL SYLLABLE SWAELH
C1D4;HANGUL SYLLABLE SWAEM
C1D5;HANGUL SYLLABLE SWAEB
C1D6;HANGUL SYLLABLE SWAEBS
C1D7;HANGUL SYLLABLE SWAES
C1D8;HANGUL SYLLABLE SWAESS
C1D9;HANGUL SYLLABLE SWAENG
C1DA;HANGUL SYLLABLE SWAEJ
C1DB;HANGUL SYLLABLE SWAEC
C1DC;HANGUL SYLLABLE SWAEK
C1DD;HANGUL SYLLABLE SWAET
C1DE;HANGUL SYLLABLE SWAEP
C1DF;HANGUL SYLLABLE SWAEH
C1E0;HANGUL SYLLABLE SOE
C1E1;HANGUL SYLLABLE SOEG
C1E2;HANGUL SYLLABLE SOEGG
C1E3;HANGUL SYLLABLE SOEGS
C1E4;HANGUL SYLLABLE SOEN
C1E5;HANGUL SYLLABLE SOENJ
C1E6;HANGUL SYLLABLE SOENH
C1E7;HANGUL SYLLABLE SOED
C1E8;HANGUL SYLLABLE SOEL
C1E9;HANGUL SYLLABLE SOELG
C1EA;HANGUL SYLLABLE SOELM
C1EB;HANGUL SYLLABLE SOELB
C1EC;HANGUL SYLLABLE SOELS
C1ED;HANGUL SYLLABLE SOELT
C1EE;HANGUL SYLLABLE SOELP
C1EF;HANGUL SYLLABLE SOELH
C1F0;HANGUL SYLLABLE SOEM
C1F1;HANGUL SYLLABLE SOEB
C1F2;HANGUL SYLLABLE SOEBS
C1F3;HANGUL SYLLABLE SOES
C1F4;HANGUL SYLLABLE SOESS
C1F5;HANGUL SYLLABLE SOENG
C1F6;HANGUL SYLLABLE SOEJ
C1F7;HANGUL SYLLABLE SOEC
C1F8;HANGUL SYLLABLE SOEK
C1F9;HANGUL SYLLABLE SOET
C1FA;HANGUL SYLLABLE SOEP
C1FB;HANGUL SYLLABLE SOEH
C1FC;HANGUL SYLLABLE SYO
C1FD;HANGUL SYLLABLE SYOG
C1FE;HANGUL SYLLABLE SYOGG
C1FF;HANGUL SYLLABLE SYOGS
C200;HANGUL SYLLABLE SYON
C201;HANGUL SYLLABLE SYONJ
C202;HANGUL SYLLABLE SYONH
C203;HANGUL SYLLABLE SYOD
C204;HANGUL SYLLABLE SYOL
C205;HANGUL SYLLABLE SYOLG
C206;HANGUL SYLLABLE SYOLM
C207;HANGUL SYLLABLE SYOLB
C208;HANGUL SYLLABLE SYOLS
C209;HANGUL SYLLABLE SYOLT
C20A;HANGUL SYLLABLE SYOLP
C20B;HANGUL SYLLABLE SYOLH
C20C;HANGUL SYLLABLE SYOM
C20D;HANGUL SYLLABLE SYOB
C20E;HANGUL SYLLABLE SYOBS
C20F;HANGUL SYLLABLE SYOS
C210;HANGUL SYLLABLE SYOSS
C211;HANGUL SYLLABLE SYONG
C212;HANGUL SYLLABLE SYOJ
C213;HANGUL SYLLABLE SYOC
C214;HANGUL SYLLABLE SYOK
C215;HANGUL SYLLABLE SYOT
C216;HANGUL SYLLABLE SYOP
C217;HANGUL SYLLABLE SYOH
C218;HANGUL SYLLABLE SU
C219;HANGUL SYLLABLE SUG
C21A;HANGUL SYLLABLE SUGG
C21B;HANGUL SYLLABLE SUGS
C21C;HANGUL SYLLABLE SUN
C21D;HANGUL SYLLABLE SUNJ
C21E;HANGUL SYLLABLE SUNH
C21F;HANGUL SYLLABLE SUD
C220;HANGUL SYLLABLE SUL
C221;HANGUL SYLLABLE SULG
C222;HANGUL SYLLABLE SULM
C223;HANGUL SYLLABLE SULB
C224;HANGUL SYLLABLE SULS
C225;HANGUL SYLLABLE SULT
C226;HANGUL SYLLABLE SULP
C227;HANGUL SYLLABLE SULH
C228;HANGUL SYLLABLE SUM
C229;HANGUL SYLLABLE SUB
C22A;HANGUL SYLLABLE SUBS
C22B;HANGUL SYLLABLE SUS
C22C;HANGUL SYLLABLE SUSS
C22D;HANGUL SYLLABLE SUNG
C22E;HANGUL SYLLABLE SUJ
C22F;HANGUL SYLLABLE SUC
C230;HANGUL SYLLABLE SUK
C231;HANGUL SYLLABLE SUT
C232;HANGUL SYLLABLE SUP
C233;HANGUL SYLLABLE SUH
C234;HANGUL SYLLABLE SWEO
C235;HANGUL SYLLABLE SWEOG
C236;HANGUL SYLLABLE SWEOGG
C237;HANGUL SYLLABLE SWEOGS
C238;HANGUL SYLLABLE SWEON
C239;HANGUL SYLLABLE SWEONJ
C23A;HANGUL SYLLABLE SWEONH
C23B;HANGUL SYLLABLE SWEOD
C23C;HANGUL SYLLABLE SWEOL
C23D;HANGUL SYLLABLE SWEOLG
C23E;HANGUL SYLLABLE SWEOLM
C23F;HANGUL SYLLABLE SWEOLB
C240;HANGUL SYLLABLE SWEOLS
C241;HANGUL SYLLABLE SWEOLT
C242;HANGUL SYLLABLE SWEOLP
C243;HANGUL SYLLABLE SWEOLH
C244;HANGUL SYLLABLE SWEOM
C245;HANGUL SYLLABLE SWEOB
C246;HANGUL SYLLABLE SWEOBS
C247;HANGUL SYLLABLE SWEOS
C248;HANGUL SYLLABLE SWEOSS
C249;HANGUL SYLLABLE SWEONG
C24A;HANGUL SYLLABLE SWEOJ
C24B;HANGUL SYLLABLE SWEOC
C24C;HANGUL SYLLABLE SWEOK
C24D;HANGUL SYLLABLE SWEOT
C24E;HANGUL SYLLABLE SWEOP
C24F;HANGUL SYLLABLE SWEOH
C250;HANGUL SYLLABLE SWE
C251;HANGUL SYLLABLE SWEG
C252;HANGUL SYLLABLE SWEGG
C253;HANGUL SYLLABLE SWEGS
C254;HANGUL SYLLABLE SWEN
C255;HANGUL SYLLABLE SWENJ
C256;HANGUL SYLLABLE SWENH
C257;HANGUL SYLLABLE SWED
C258;HANGUL SYLLABLE SWEL
C259;HANGUL SYLLABLE SWELG
C25A;HANGUL SYLLABLE SWELM
C25B;HANGUL SYLLABLE SWELB
C25C;HANGUL SYLLABLE SWELS
C25D;HANGUL SYLLABLE SWELT
C25E;HANGUL SYLLABLE SWELP
C25F;HANGUL SYLLABLE SWELH
C260;HANGUL SYLLABLE SWEM
C261;HANGUL SYLLABLE SWEB
C262;HANGUL SYLLABLE SWEBS
C263;HANGUL SYLLABLE SWES
C264;HANGUL SYLLABLE SWESS
C265;HANGUL SYLLABLE SWENG
C266;HANGUL SYLLABLE SWEJ
C267;HANGUL SYLLABLE SWEC
C268;HANGUL SYLLABLE SWEK
C269;HANGUL SYLLABLE SWET
C26A;HANGUL SYLLABLE SWEP
C26B;HANGUL SYLLABLE SWEH
C26C;HANGUL SYLLABLE SWI
C26D;HANGUL SYLLABLE SWIG
C26E;HANGUL SYLLABLE SWIGG
C26F;HANGUL SYLLABLE SWIGS
C270;HANGUL SYLLABLE SWIN
C271;HANGUL SYLLABLE SWINJ
C272;HANGUL SYLLABLE SWINH
C273;HANGUL SYLLABLE SWID
C274;HANGUL SYLLABLE SWIL
C275;HANGUL SYLLABLE SWILG
C276;HANGUL SYLLABLE SWILM
C277;HANGUL SYLLABLE SWILB
C278;HANGUL SYLLABLE SWILS
C279;HANGUL SYLLABLE SWILT
C27A;HANGUL SYLLABLE SWILP
C27B;HANGUL SYLLABLE SWILH
C27C;HANGUL SYLLABLE SWIM
C27D;HANGUL SYLLABLE SWIB
C27E;HANGUL SYLLABLE SWIBS
C27F;HANGUL SYLLABLE SWIS
C280;HANGUL SYLLABLE SWISS
C281;HANGUL SYLLABLE SWING
C282;HANGUL SYLLABLE SWIJ
C283;HANGUL SYLLABLE SWIC
C284;HANGUL SYLLABLE SWIK
C285;HANGUL SYLLABLE SWIT
C286;HANGUL SYLLABLE SWIP
C287;HANGUL SYLLABLE SWIH
C288;HANGUL SYLLABLE SYU
C289;HANGUL SYLLABLE SYUG
C28A;HANGUL SYLLABLE SYUGG
C28B;HANGUL SYLLABLE SYUGS
C28C;HANGUL SYLLABLE SYUN
C28D;HANGUL SYLLABLE SYUNJ
C28E;HANGUL SYLLABLE SYUNH
C28F;HANGUL SYLLABLE SYUD
C290;HANGUL SYLLABLE SYUL
C291;HANGUL SYLLABLE SYULG
C292;HANGUL SYLLABLE SYULM
C293;HANGUL SYLLABLE SYULB
C294;HANGUL SYLLABLE SYULS
C295;HANGUL SYLLABLE SYULT
C296;HANGUL SYLLABLE SYULP
C297;HANGUL SYLLABLE SYULH
C298;HANGUL SYLLABLE SYUM
C299;HANGUL SYLLABLE SYUB
C29A;HANGUL SYLLABLE SYUBS
C29B;HANGUL SYLLABLE SYUS
C29C;HANGUL SYLLABLE SYUSS
C29D;HANGUL SYLLABLE SYUNG
C29E;HANGUL SYLLABLE SYUJ
C29F;HANGUL SYLLABLE SYUC
C2A0;HANGUL SYLLABLE SYUK
C2A1;HANGUL SYLLABLE SYUT
C2A2;HANGUL SYLLABLE SYUP
C2A3;HANGUL SYLLABLE SYUH
C2A4;HANGUL SYLLABLE SEU
C2A5;HANGUL SYLLABLE SEUG
C2A6;HANGUL SYLLABLE SEUGG
C2A7;HANGUL SYLLABLE SEUGS
C2A8;HANGUL SYLLABLE SEUN
C2A9;HANGUL SYLLABLE SEUNJ
C2AA;HANGUL SYLLABLE SEUNH
C2AB;HANGUL SYLLABLE SEUD
C2AC;HANGUL SYLLABLE SEUL
C2AD;HANGUL SYLLABLE SEULG
C2AE;HANGUL SYLLABLE SEULM
C2AF;HANGUL SYLLABLE SEULB
C2B0;HANGUL SYLLABLE SEULS
C2B1;HANGUL SYLLABLE SEULT
C2B2;HANGUL SYLLABLE SEULP
C2B3;HANGUL SYLLABLE SEULH
C2B4;HANGUL SYLLABLE SEUM
C2B5;HANGUL SYLLABLE SEUB
C2B6;HANGUL SYLLABLE SEUBS
C2B7;HANGUL SYLLABLE SEUS
C2B8;HANGUL SYLLABLE SEUSS
C2B9;HANGUL SYLLABLE SEUNG
C2BA;HANGUL SYLLABLE SEUJ
C2BB;HANGUL SYLLABLE SEUC
C2BC;HANGUL SYLLABLE SEUK
C2BD;HANGUL SYLLABLE SEUT
C2BE;HANGUL SYLLABLE SEUP
C2BF;HANGUL SYLLABLE SEUH
C2C0;HANGUL SYLLABLE SYI
C2C1;HANGUL SYLLABLE SYIG
C2C2;HANGUL SYLLABLE SYIGG
C2C3;HANGUL SYLLABLE SYIGS
C2C4;HANGUL SYLLABLE SYIN
C2C5;HANGUL SYLLABLE SYINJ
C2C6;HANGUL SYLLABLE SYINH
C2C7;HANGUL SYLLABLE SYID
C2C8;HANGUL SYLLABLE SYIL
C2C9;HANGUL SYLLABLE SYILG
C2CA;HANGUL SYLLABLE SYILM
C2CB;HANGUL SYLLABLE SYILB
C2CC;HANGUL SYLLABLE SYILS
C2CD;HANGUL SYLLABLE SYILT
C2CE;HANGUL SYLLABLE SYILP
C2CF;HANGUL SYLLABLE SYILH
C2D0;HANGUL SYLLABLE SYIM
C2D1;HANGUL SYLLABLE SYIB
C2D2;HANGUL SYLLABLE SYIBS
C2D3;HANGUL SYLLABLE SYIS
C2D4;HANGUL SYLLABLE SYISS
C2D5;HANGUL SYLLABLE SYING
C2D6;HANGUL SYLLABLE SYIJ
C2D7;HANGUL SYLLABLE SYIC
C2D8;HANGUL SYLLABLE SYIK
C2D9;HANGUL SYLLABLE SYIT
C2DA;HANGUL SYLLABLE SYIP
C2DB;HANGUL SYLLABLE SYIH
C2DC;HANGUL SYLLABLE SI
C2DD;HANGUL SYLLABLE SIG
C2DE;HANGUL SYLLABLE SIGG
C2DF;HANGUL SYLLABLE SIGS
C2E0;HANGUL SYLLABLE SIN
C2E1;HANGUL SYLLABLE SINJ
C2E2;HANGUL SYLLABLE SINH
C2E3;HANGUL SYLLABLE SID
C2E4;HANGUL SYLLABLE SIL
C2E5;HANGUL SYLLABLE SILG
C2E6;HANGUL SYLLABLE SILM
C2E7;HANGUL SYLLABLE SILB
C2E8;HANGUL SYLLABLE SILS
C2E9;HANGUL SYLLABLE SILT
C2EA;HANGUL SYLLABLE SILP
C2EB;HANGUL SYLLABLE SILH
C2EC;HANGUL SYLLABLE SIM
C2ED;HANGUL SYLLABLE SIB
C2EE;HANGUL SYLLABLE SIBS
C2EF;HANGUL SYLLABLE SIS
C2F0;HANGUL SYLLABLE SISS
C2F1;HANGUL SYLLABLE SING
C2F2;HANGUL SYLLABLE SIJ
C2F3;HANGUL SYLLABLE SIC
C2F4;HANGUL SYLLABLE SIK
C2F5;HANGUL SYLLABLE SIT
C2F6;HANGUL SYLLABLE SIP
C2F7;HANGUL SYLLABLE SIH
C2F8;HANGUL SYLLABLE SSA
C2F9;HANGUL SYLLABLE SSAG
C2FA;HANGUL SYLLABLE SSAGG
C2FB;HANGUL SYLLABLE SSAGS
C2FC;HANGUL SYLLABLE SSAN
C2FD;HANGUL SYLLABLE SSANJ
C2FE;HANGUL SYLLABLE SSANH
C2FF;HANGUL SYLLABLE SSAD
C300;HANGUL SYLLABLE SSAL
C301;HANGUL SYLLABLE SSALG
C302;HANGUL SYLLABLE SSALM
C303;HANGUL SYLLABLE SSALB
C304;HANGUL SYLLABLE SSALS
C305;HANGUL SYLLABLE SSALT
C306;HANGUL SYLLABLE SSALP
C307;HANGUL SYLLABLE SSALH
C308;HANGUL SYLLABLE SSAM
C309;HANGUL SYLLABLE SSAB
C30A;HANGUL SYLLABLE SSABS
C30B;HANGUL SYLLABLE SSAS
C30C;HANGUL SYLLABLE SSASS
C30D;HANGUL SYLLABLE SSANG
C30E;HANGUL SYLLABLE SSAJ
C30F;HANGUL SYLLABLE SSAC
C310;HANGUL SYLLABLE SSAK
C311;HANGUL SYLLABLE SSAT
C312;HANGUL SYLLABLE SSAP
C313;HANGUL SYLLABLE SSAH
C314;HANGUL SYLLABLE SSAE
C315;HANGUL SYLLABLE SSAEG
C316;HANGUL SYLLABLE SSAEGG
C317;HANGUL SYLLABLE SSAEGS
C318;HANGUL SYLLABLE SSAEN
C319;HANGUL SYLLABLE SSAENJ
C31A;HANGUL SYLLABLE SSAENH
C31B;HANGUL SYLLABLE SSAED
C31C;HANGUL SYLLABLE SSAEL
C31D;HANGUL SYLLABLE SSAELG
C31E;HANGUL SYLLABLE SSAELM
C31F;HANGUL SYLLABLE SSAELB
C320;HANGUL SYLLABLE SSAELS
C321;HANGUL SYLLABLE SSAELT
C322;HANGUL SYLLABLE SSAELP
C323;HANGUL SYLLABLE SSAELH
C324;HANGUL SYLLABLE SSAEM
C325;HANGUL SYLLABLE SSAEB
C326;HANGUL SYLLABLE SSAEBS
C327;HANGUL SYLLABLE SSAES
C328;HANGUL SYLLABLE SSAESS
C329;HANGUL SYLLABLE SSAENG
C32A;HANGUL SYLLABLE SSAEJ
C32B;HANGUL SYLLABLE SSAEC
C32C;HANGUL SYLLABLE SSAEK
C32D;HANGUL SYLLABLE SSAET
C32E;HANGUL SYLLABLE SSAEP
C32F;HANGUL SYLLABLE SSAEH
C330;HANGUL SYLLABLE SSYA
C331;HANGUL SYLLABLE SSYAG
C332;HANGUL SYLLABLE SSYAGG
C333;HANGUL SYLLABLE SSYAGS
C334;HANGUL SYLLABLE SSYAN
C335;HANGUL SYLLABLE SSYANJ
C336;HANGUL SYLLABLE SSYANH
C337;HANGUL SYLLABLE SSYAD
C338;HANGUL SYLLABLE SSYAL
C339;HANGUL SYLLABLE SSYALG
C33A;HANGUL SYLLABLE SSYALM
C33B;HANGUL SYLLABLE SSYALB
C33C;HANGUL SYLLABLE SSYALS
C33D;HANGUL SYLLABLE SSYALT
C33E;HANGUL SYLLABLE SSYALP
C33F;HANGUL SYLLABLE SSYALH
C340;HANGUL SYLLABLE SSYAM
C341;HANGUL SYLLABLE SSYAB
C342;HANGUL SYLLABLE SSYABS
C343;HANGUL SYLLABLE SSYAS
C344;HANGUL SYLLABLE SSYASS
C345;HANGUL SYLLABLE SSYANG
C346;HANGUL SYLLABLE SSYAJ
C347;HANGUL SYLLABLE SSYAC
C348;HANGUL SYLLABLE SSYAK
C349;HANGUL SYLLABLE SSYAT
C34A;HANGUL SYLLABLE SSYAP
C34B;HANGUL SYLLABLE SSYAH
C34C;HANGUL SYLLABLE SSYAE
C34D;HANGUL SYLLABLE SSYAEG
C34E;HANGUL SYLLABLE SSYAEGG
C34F;HANGUL SYLLABLE SSYAEGS
C350;HANGUL SYLLABLE SSYAEN
C351;HANGUL SYLLABLE SSYAENJ
C352;HANGUL SYLLABLE SSYAENH
C353;HANGUL SYLLABLE SSYAED
C354;HANGUL SYLLABLE SSYAEL
C355;HANGUL SYLLABLE SSYAELG
C356;HANGUL SYLLABLE SSYAELM
C357;HANGUL SYLLABLE SSYAELB
C358;HANGUL SYLLABLE SSYAELS
C359;HANGUL SYLLABLE SSYAELT
C35A;HANGUL SYLLABLE SSYAELP
C35B;HANGUL SYLLABLE SSYAELH
C35C;HANGUL SYLLABLE SSYAEM
C35D;HANGUL SYLLABLE SSYAEB
C35E;HANGUL SYLLABLE SSYAEBS
C35F;HANGUL SYLLABLE SSYAES
C360;HANGUL SYLLABLE SSYAESS
C361;HANGUL SYLLABLE SSYAENG
C362;HANGUL SYLLABLE SSYAEJ
C363;HANGUL SYLLABLE SSYAEC
C364;HANGUL SYLLABLE SSYAEK
C365;HANGUL SYLLABLE SSYAET
C366;HANGUL SYLLABLE SSYAEP
C367;HANGUL SYLLABLE SSYAEH
C368;HANGUL SYLLABLE SSEO
C369;HANGUL SYLLABLE SSEOG
C36A;HANGUL SYLLABLE SSEOGG
C36B;HANGUL SYLLABLE SSEOGS
C36C;HANGUL SYLLABLE SSEON
C36D;HANGUL SYLLABLE SSEONJ
C36E;HANGUL SYLLABLE SSEONH
C36F;HANGUL SYLLABLE SSEOD
C370;HANGUL SYLLABLE SSEOL
C371;HANGUL SYLLABLE SSEOLG
C372;HANGUL SYLLABLE SSEOLM
C373;HANGUL SYLLABLE SSEOLB
C374;HANGUL SYLLABLE SSEOLS
C375;HANGUL SYLLABLE SSEOLT
C376;HANGUL SYLLABLE SSEOLP
C377;HANGUL SYLLABLE SSEOLH
C378;HANGUL SYLLABLE SSEOM
C379;HANGUL SYLLABLE SSEOB
C37A;HANGUL SYLLABLE SSEOBS
C37B;HANGUL SYLLABLE SSEOS
C37C;HANGUL SYLLABLE SSEOSS
C37D;HANGUL SYLLABLE SSEONG
C37E;HANGUL SYLLABLE SSEOJ
C37F;HANGUL SYLLABLE SSEOC
C380;HANGUL SYLLABLE SSEOK
C381;HANGUL SYLLABLE SSEOT
C382;HANGUL SYLLABLE SSEOP
C383;HANGUL SYLLABLE SSEOH
C384;HANGUL SYLLABLE SSE
C385;HANGUL SYLLABLE SSEG
C386;HANGUL SYLLABLE SSEGG
C387;HANGUL SYLLABLE SSEGS
C388;HANGUL SYLLABLE SSEN
C389;HANGUL SYLLABLE SSENJ
C38A;HANGUL SYLLABLE SSENH
C38B;HANGUL SYLLABLE SSED
C38C;HANGUL SYLLABLE SSEL
C38D;HANGUL SYLLABLE SSELG
C38E;HANGUL SYLLABLE SSELM
C38F;HANGUL SYLLABLE SSELB
C390;HANGUL SYLLABLE SSELS
C391;HANGUL SYLLABLE SSELT
C392;HANGUL SYLLABLE SSELP
C393;HANGUL SYLLABLE SSELH
C394;HANGUL SYLLABLE SSEM
C395;HANGUL SYLLABLE SSEB
C396;HANGUL SYLLABLE SSEBS
C397;HANGUL SYLLABLE SSES
C398;HANGUL SYLLABLE SSESS
C399;HANGUL SYLLABLE SSENG
C39A;HANGUL SYLLABLE SSEJ
C39B;HANGUL SYLLABLE SSEC
C39C;HANGUL SYLLABLE SSEK
C39D;HANGUL SYLLABLE SSET
C39E;HANGUL SYLLABLE SSEP
C39F;HANGUL SYLLABLE SSEH
C3A0;HANGUL SYLLABLE SSYEO
C3A1;HANGUL SYLLABLE SSYEOG
C3A2;HANGUL SYLLABLE SSYEOGG
C3A3;HANGUL SYLLABLE SSYEOGS
C3A4;HANGUL SYLLABLE SSYEON
C3A5;HANGUL SYLLABLE SSYEONJ
C3A6;HANGUL SYLLABLE SSYEONH
C3A7;HANGUL SYLLABLE SSYEOD
C3A8;HANGUL SYLLABLE SSYEOL
C3A9;HANGUL SYLLABLE SSYEOLG
C3AA;HANGUL SYLLABLE SSYEOLM
C3AB;HANGUL SYLLABLE SSYEOLB
C3AC;HANGUL SYLLABLE SSYEOLS
C3AD;HANGUL SYLLABLE SSYEOLT
C3AE;HANGUL SYLLABLE SSYEOLP
C3AF;HANGUL SYLLABLE SSYEOLH
C3B0;HANGUL SYLLABLE SSYEOM
C3B1;HANGUL SYLLABLE SSYEOB
C3B2;HANGUL SYLLABLE SSYEOBS
C3B3;HANGUL SYLLABLE SSYEOS
C3B4;HANGUL SYLLABLE SSYEOSS
C3B5;HANGUL SYLLABLE SSYEONG
C3B6;HANGUL SYLLABLE SSYEOJ
C3B7;HANGUL SYLLABLE SSYEOC
C3B8;HANGUL SYLLABLE SSYEOK
C3B9;HANGUL SYLLABLE SSYEOT
C3BA;HANGUL SYLLABLE SSYEOP
C3BB;HANGUL SYLLABLE SSYEOH
C3BC;HANGUL SYLLABLE SSYE
C3BD;HANGUL SYLLABLE SSYEG
C3BE;HANGUL SYLLABLE SSYEGG
C3BF;HANGUL SYLLABLE SSYEGS
C3C0;HANGUL SYLLABLE SSYEN
C3C1;HANGUL SYLLABLE SSYENJ
C3C2;HANGUL SYLLABLE SSYENH
C3C3;HANGUL SYLLABLE SSYED
C3C4;HANGUL SYLLABLE SSYEL
C3C5;HANGUL SYLLABLE SSYELG
C3C6;HANGUL SYLLABLE SSYELM
C3C7;HANGUL SYLLABLE SSYELB
C3C8;HANGUL SYLLABLE SSYELS
C3C9;HANGUL SYLLABLE SSYELT
C3CA;HANGUL SYLLABLE SSYELP
C3CB;HANGUL SYLLABLE SSYELH
C3CC;HANGUL SYLLABLE SSYEM
C3CD;HANGUL SYLLABLE SSYEB
C3CE;HANGUL SYLLABLE SSYEBS
C3CF;HANGUL SYLLABLE SSYES
C3D0;HANGUL SYLLABLE SSYESS
C3D1;HANGUL SYLLABLE SSYENG
C3D2;HANGUL SYLLABLE SSYEJ
C3D3;HANGUL SYLLABLE SSYEC
C3D4;HANGUL SYLLABLE SSYEK
C3D5;HANGUL SYLLABLE SSYET
C3D6;HANGUL SYLLABLE SSYEP
C3D7;HANGUL SYLLABLE SSYEH
C3D8;HANGUL SYLLABLE SSO
C3D9;HANGUL SYLLABLE SSOG
C3DA;HANGUL SYLLABLE SSOGG
C3DB;HANGUL SYLLABLE SSOGS
C3DC;HANGUL SYLLABLE SSON
C3DD;HANGUL SYLLABLE SSONJ
C3DE;HANGUL SYLLABLE SSONH
C3DF;HANGUL SYLLABLE SSOD
C3E0;HANGUL SYLLABLE SSOL
C3E1;HANGUL SYLLABLE SSOLG
C3E2;HANGUL SYLLABLE SSOLM
C3E3;HANGUL SYLLABLE SSOLB
C3E4;HANGUL SYLLABLE SSOLS
C3E5;HANGUL SYLLABLE SSOLT
C3E6;HANGUL SYLLABLE SSOLP
C3E7;HANGUL SYLLABLE SSOLH
C3E8;HANGUL SYLLABLE SSOM
C3E9;HANGUL SYLLABLE SSOB
C3EA;HANGUL SYLLABLE SSOBS
C3EB;HANGUL SYLLABLE SSOS
C3EC;HANGUL SYLLABLE SSOSS
C3ED;HANGUL SYLLABLE SSONG
C3EE;HANGUL SYLLABLE SSOJ
C3EF;HANGUL SYLLABLE SSOC
C3F0;HANGUL SYLLABLE SSOK
C3F1;HANGUL SYLLABLE SSOT
C3F2;HANGUL SYLLABLE SSOP
C3F3;HANGUL SYLLABLE SSOH
C3F4;HANGUL SYLLABLE SSWA
C3F5;HANGUL SYLLABLE SSWAG
C3F6;HANGUL SYLLABLE SSWAGG
C3F7;HANGUL SYLLABLE SSWAGS
C3F8;HANGUL SYLLABLE SSWAN
C3F9;HANGUL SYLLABLE SSWANJ
C3FA;HANGUL SYLLABLE SSWANH
C3FB;HANGUL SYLLABLE SSWAD
C3FC;HANGUL SYLLABLE SSWAL
C3FD;HANGUL SYLLABLE SSWALG
C3FE;HANGUL SYLLABLE SSWALM
C3FF;HANGUL SYLLABLE SSWALB
C400;HANGUL SYLLABLE SSWALS
C401;HANGUL SYLLABLE SSWALT
C402;HANGUL SYLLABLE SSWALP
C403;HANGUL SYLLABLE SSWALH
C404;HANGUL SYLLABLE SSWAM
C405;HANGUL SYLLABLE SSWAB
C406;HANGUL SYLLABLE SSWABS
C407;HANGUL SYLLABLE SSWAS
C408;HANGUL SYLLABLE SSWASS
C409;HANGUL SYLLABLE SSWANG
C40A;HANGUL SYLLABLE SSWAJ
C40B;HANGUL SYLLABLE SSWAC
C40C;HANGUL SYLLABLE SSWAK
C40D;HANGUL SYLLABLE SSWAT
C40E;HANGUL SYLLABLE SSWAP
C40F;HANGUL SYLLABLE SSWAH
C410;HANGUL SYLLABLE SSWAE
C411;HANGUL SYLLABLE SSWAEG
C412;HANGUL SYLLABLE SSWAEGG
C413;HANGUL SYLLABLE SSWAEGS
C414;HANGUL SYLLABLE SSWAEN
C415;HANGUL SYLLABLE SSWAENJ
C416;HANGUL SYLLABLE SSWAENH
C417;HANGUL SYLLABLE SSWAED
C418;HANGUL SYLLABLE SSWAEL
C419;HANGUL SYLLABLE SSWAELG
C41A;HANGUL SYLLABLE SSWAELM
C41B;HANGUL SYLLABLE SSWAELB
C41C;HANGUL SYLLABLE SSWAELS
C41D;HANGUL SYLLABLE SSWAELT
C41E;HANGUL SYLLABLE SSWAELP
C41F;HANGUL SYLLABLE SSWAELH
C420;HANGUL SYLLABLE SSWAEM
C421;HANGUL SYLLABLE SSWAEB
C422;HANGUL SYLLABLE SSWAEBS
C423;HANGUL SYLLABLE SSWAES
C424;HANGUL SYLLABLE SSWAESS
C425;HANGUL SYLLABLE SSWAENG
C426;HANGUL SYLLABLE SSWAEJ
C427;HANGUL SYLLABLE SSWAEC
C428;HANGUL SYLLABLE SSWAEK
C429;HANGUL SYLLABLE SSWAET
C42A;HANGUL SYLLABLE SSWAEP
C42B;HANGUL SYLLABLE SSWAEH
C42C;HANGUL SYLLABLE SSOE
C42D;HANGUL SYLLABLE SSOEG
C42E;HANGUL SYLLABLE SSOEGG
C42F;HANGUL SYLLABLE SSOEGS
C430;HANGUL SYLLABLE SSOEN
C431;HANGUL SYLLABLE SSOENJ
C432;HANGUL SYLLABLE SSOENH
C433;HANGUL SYLLABLE SSOED
C434;HANGUL SYLLABLE SSOEL
C435;HANGUL SYLLABLE SSOELG
C436;HANGUL SYLLABLE SSOELM
C437;HANGUL SYLLABLE SSOELB
C438;HANGUL SYLLABLE SSOELS
C439;HANGUL SYLLABLE SSOELT
C43A;HANGUL SYLLABLE SSOELP
C43B;HANGUL SYLLABLE SSOELH
C43C;HANGUL SYLLABLE SSOEM
C43D;HANGUL SYLLABLE SSOEB
C43E;HANGUL SYLLABLE SSOEBS
C43F;HANGUL SYLLABLE SSOES
C440;HANGUL SYLLABLE SSOESS
C441;HANGUL SYLLABLE SSOENG
C442;HANGUL SYLLABLE SSOEJ
C443;HANGUL SYLLABLE SSOEC
C444;HANGUL SYLLABLE SSOEK
C445;HANGUL SYLLABLE SSOET
C446;HANGUL SYLLABLE SSOEP
C447;HANGUL SYLLABLE SSOEH
C448;HANGUL SYLLABLE SSYO
C449;HANGUL SYLLABLE SSYOG
C44A;HANGUL SYLLABLE SSYOGG
C44B;HANGUL SYLLABLE SSYOGS
C44C;HANGUL SYLLABLE SSYON
C44D;HANGUL SYLLABLE SSYONJ
C44E;HANGUL SYLLABLE SSYONH
C44F;HANGUL SYLLABLE SSYOD
C450;HANGUL SYLLABLE SSYOL
C451;HANGUL SYLLABLE SSYOLG
C452;HANGUL SYLLABLE SSYOLM
C453;HANGUL SYLLABLE SSYOLB
C454;HANGUL SYLLABLE SSYOLS
C455;HANGUL SYLLABLE SSYOLT
C456;HANGUL SYLLABLE SSYOLP
C457;HANGUL SYLLABLE SSYOLH
C458;HANGUL SYLLABLE SSYOM
C459;HANGUL SYLLABLE SSYOB
C45A;HANGUL SYLLABLE SSYOBS
C45B;HANGUL SYLLABLE SSYOS
C45C;HANGUL SYLLABLE SSYOSS
C45D;HANGUL SYLLABLE SSYONG
C45E;HANGUL SYLLABLE SSYOJ
C45F;HANGUL SYLLABLE SSYOC
C460;HANGUL SYLLABLE SSYOK
C461;HANGUL SYLLABLE SSYOT
C462;HANGUL SYLLABLE SSYOP
C463;HANGUL SYLLABLE SSYOH
C464;HANGUL SYLLABLE SSU
C465;HANGUL SYLLABLE SSUG
C466;HANGUL SYLLABLE SSUGG
C467;HANGUL SYLLABLE SSUGS
C468;HANGUL SYLLABLE SSUN
C469;HANGUL SYLLABLE SSUNJ
C46A;HANGUL SYLLABLE SSUNH
C46B;HANGUL SYLLABLE SSUD
C46C;HANGUL SYLLABLE SSUL
C46D;HANGUL SYLLABLE SSULG
C46E;HANGUL SYLLABLE SSULM
C46F;HANGUL SYLLABLE SSULB
C470;HANGUL SYLLABLE SSULS
C471;HANGUL SYLLABLE SSULT
C472;HANGUL SYLLABLE SSULP
C473;HANGUL SYLLABLE SSULH
C474;HANGUL SYLLABLE SSUM
C475;HANGUL SYLLABLE SSUB
C476;HANGUL SYLLABLE SSUBS
C477;HANGUL SYLLABLE SSUS
C478;HANGUL SYLLABLE SSUSS
C479;HANGUL SYLLABLE SSUNG
C47A;HANGUL SYLLABLE SSUJ
C47B;HANGUL SYLLABLE SSUC
C47C;HANGUL SYLLABLE SSUK
C47D;HANGUL SYLLABLE SSUT
C47E;HANGUL SYLLABLE SSUP
C47F;HANGUL SYLLABLE SSUH
C480;HANGUL SYLLABLE SSWEO
C481;HANGUL SYLLABLE SSWEOG
C482;HANGUL SYLLABLE SSWEOGG
C483;HANGUL SYLLABLE SSWEOGS
C484;HANGUL SYLLABLE SSWEON
C485;HANGUL SYLLABLE SSWEONJ
C486;HANGUL SYLLABLE SSWEONH
C487;HANGUL SYLLABLE SSWEOD
C488;HANGUL SYLLABLE SSWEOL
C489;HANGUL SYLLABLE SSWEOLG
C48A;HANGUL SYLLABLE SSWEOLM
C48B;HANGUL SYLLABLE SSWEOLB
C48C;HANGUL SYLLABLE SSWEOLS
C48D;HANGUL SYLLABLE SSWEOLT
C48E;HANGUL SYLLABLE SSWEOLP
C48F;HANGUL SYLLABLE SSWEOLH
C490;HANGUL SYLLABLE SSWEOM
C491;HANGUL SYLLABLE SSWEOB
C492;HANGUL SYLLABLE SSWEOBS
C493;HANGUL SYLLABLE SSWEOS
C494;HANGUL SYLLABLE SSWEOSS
C495;HANGUL SYLLABLE SSWEONG
C496;HANGUL SYLLABLE SSWEOJ
C497;HANGUL SYLLABLE SSWEOC
C498;HANGUL SYLLABLE SSWEOK
C499;HANGUL SYLLABLE SSWEOT
C49A;HANGUL SYLLABLE SSWEOP
C49B;HANGUL SYLLABLE SSWEOH
C49C;HANGUL SYLLABLE SSWE
C49D;HANGUL SYLLABLE SSWEG
C49E;HANGUL SYLLABLE SSWEGG
C49F;HANGUL SYLLABLE SSWEGS
C4A0;HANGUL SYLLABLE SSWEN
C4A1;HANGUL SYLLABLE SSWENJ
C4A2;HANGUL SYLLABLE SSWENH
C4A3;HANGUL SYLLABLE SSWED
C4A4;HANGUL SYLLABLE SSWEL
C4A5;HANGUL SYLLABLE SSWELG
C4A6;HANGUL SYLLABLE SSWELM
C4A7;HANGUL SYLLABLE SSWELB
C4A8;HANGUL SYLLABLE SSWELS
C4A9;HANGUL SYLLABLE SSWELT
C4AA;HANGUL SYLLABLE SSWELP
C4AB;HANGUL SYLLABLE SSWELH
C4AC;HANGUL SYLLABLE SSWEM
C4AD;HANGUL SYLLABLE SSWEB
C4AE;HANGUL SYLLABLE SSWEBS
C4AF;HANGUL SYLLABLE SSWES
C4B0;HANGUL SYLLABLE SSWESS
C4B1;HANGUL SYLLABLE SSWENG
C4B2;HANGUL SYLLABLE SSWEJ
C4B3;HANGUL SYLLABLE SSWEC
C4B4;HANGUL SYLLABLE SSWEK
C4B5;HANGUL SYLLABLE SSWET
C4B6;HANGUL SYLLABLE SSWEP
C4B7;HANGUL SYLLABLE SSWEH
C4B8;HANGUL SYLLABLE SSWI
C4B9;HANGUL SYLLABLE SSWIG
C4BA;HANGUL SYLLABLE SSWIGG
C4BB;HANGUL SYLLABLE SSWIGS
C4BC;HANGUL SYLLABLE SSWIN
C4BD;HANGUL SYLLABLE SSWINJ
C4BE;HANGUL SYLLABLE SSWINH
C4BF;HANGUL SYLLABLE SSWID
C4C0;HANGUL SYLLABLE SSWIL
C4C1;HANGUL SYLLABLE SSWILG
C4C2;HANGUL SYLLABLE SSWILM
C4C3;HANGUL SYLLABLE SSWILB
C4C4;HANGUL SYLLABLE SSWILS
C4C5;HANGUL SYLLABLE SSWILT
C4C6;HANGUL SYLLABLE SSWILP
C4C7;HANGUL SYLLABLE SSWILH
C4C8;HANGUL SYLLABLE SSWIM
C4C9;HANGUL SYLLABLE SSWIB
C4CA;HANGUL SYLLABLE SSWIBS
C4CB;HANGUL SYLLABLE SSWIS
C4CC;HANGUL SYLLABLE SSWISS
C4CD;HANGUL SYLLABLE SSWING
C4CE;HANGUL SYLLABLE SSWIJ
C4CF;HANGUL SYLLABLE SSWIC
C4D0;HANGUL SYLLABLE SSWIK
C4D1;HANGUL SYLLABLE SSWIT
C4D2;HANGUL SYLLABLE SSWIP
C4D3;HANGUL SYLLABLE SSWIH
C4D4;HANGUL SYLLABLE SSYU
C4D5;HANGUL SYLLABLE SSYUG
C4D6;HANGUL SYLLABLE SSYUGG
C4D7;HANGUL SYLLABLE SSYUGS
C4D8;HANGUL SYLLABLE SSYUN
C4D9;HANGUL SYLLABLE SSYUNJ
C4DA;HANGUL SYLLABLE SSYUNH
C4DB;HANGUL SYLLABLE SSYUD
C4DC;HANGUL SYLLABLE SSYUL
C4DD;HANGUL SYLLABLE SSYULG
C4DE;HANGUL SYLLABLE SSYULM
C4DF;HANGUL SYLLABLE SSYULB
C4E0;HANGUL SYLLABLE SSYULS
C4E1;HANGUL SYLLABLE SSYULT
C4E2;HANGUL SYLLABLE SSYULP
C4E3;HANGUL SYLLABLE SSYULH
C4E4;HANGUL SYLLABLE SSYUM
C4E5;HANGUL SYLLABLE SSYUB
C4E6;HANGUL SYLLABLE SSYUBS
C4E7;HANGUL SYLLABLE SSYUS
C4E8;HANGUL SYLLABLE SSYUSS
C4E9;HANGUL SYLLABLE SSYUNG
C4EA;HANGUL SYLLABLE SSYUJ
C4EB;HANGUL SYLLABLE SSYUC
C4EC;HANGUL SYLLABLE SSYUK
C4ED;HANGUL SYLLABLE SSYUT
C4EE;HANGUL SYLLABLE SSYUP
C4EF;HANGUL SYLLABLE SSYUH
C4F0;HANGUL SYLLABLE SSEU
C4F1;HANGUL SYLLABLE SSEUG
C4F2;HANGUL SYLLABLE SSEUGG
C4F3;HANGUL SYLLABLE SSEUGS
C4F4;HANGUL SYLLABLE SSEUN
C4F5;HANGUL SYLLABLE SSEUNJ
C4F6;HANGUL SYLLABLE SSEUNH
C4F7;HANGUL SYLLABLE SSEUD
C4F8;HANGUL SYLLABLE SSEUL
C4F9;HANGUL SYLLABLE SSEULG
C4FA;HANGUL SYLLABLE SSEULM
C4FB;HANGUL SYLLABLE SSEULB
C4FC;HANGUL SYLLABLE SSEULS
C4FD;HANGUL SYLLABLE SSEULT
C4FE;HANGUL SYLLABLE SSEULP
C4FF;HANGUL SYLLABLE SSEULH
C500;HANGUL SYLLABLE SSEUM
C501;HANGUL SYLLABLE SSEUB
C502;HANGUL SYLLABLE SSEUBS
C503;HANGUL SYLLABLE SSEUS
C504;HANGUL SYLLABLE SSEUSS
C505;HANGUL SYLLABLE SSEUNG
C506;HANGUL SYLLABLE SSEUJ
C507;HANGUL SYLLABLE SSEUC
C508;HANGUL SYLLABLE SSEUK
C509;HANGUL SYLLABLE SSEUT
C50A;HANGUL SYLLABLE SSEUP
C50B;HANGUL SYLLABLE SSEUH
C50C;HANGUL SYLLABLE SSYI
C50D;HANGUL SYLLABLE SSYIG
C50E;HANGUL SYLLABLE SSYIGG
C50F;HANGUL SYLLABLE SSYIGS
C510;HANGUL SYLLABLE SSYIN
C511;HANGUL SYLLABLE SSYINJ
C512;HANGUL SYLLABLE SSYINH
C513;HANGUL SYLLABLE SSYID
C514;HANGUL SYLLABLE SSYIL
C515;HANGUL SYLLABLE SSYILG
C516;HANGUL SYLLABLE SSYILM
C517;HANGUL SYLLABLE SSYILB
C518;HANGUL SYLLABLE SSYILS
C519;HANGUL SYLLABLE SSYILT
C51A;HANGUL SYLLABLE SSYILP
C51B;HANGUL SYLLABLE SSYILH
C51C;HANGUL SYLLABLE SSYIM
C51D;HANGUL SYLLABLE SSYIB
C51E;HANGUL SYLLABLE SSYIBS
C51F;HANGUL SYLLABLE SSYIS
C520;HANGUL SYLLABLE SSYISS
C521;HANGUL SYLLABLE SSYING
C522;HANGUL SYLLABLE SSYIJ
C523;HANGUL SYLLABLE SSYIC
C524;HANGUL SYLLABLE SSYIK
C525;HANGUL SYLLABLE SSYIT
C526;HANGUL SYLLABLE SSYIP
C527;HANGUL SYLLABLE SSYIH
C528;HANGUL SYLLABLE SSI
C529;HANGUL SYLLABLE SSIG
C52A;HANGUL SYLLABLE SSIGG
C52B;HANGUL SYLLABLE SSIGS
C52C;HANGUL SYLLABLE SSIN
C52D;HANGUL SYLLABLE SSINJ
C52E;HANGUL SYLLABLE SSINH
C52F;HANGUL SYLLABLE SSID
C530;HANGUL SYLLABLE SSIL
C531;HANGUL SYLLABLE SSILG
C532;HANGUL SYLLABLE SSILM
C533;HANGUL SYLLABLE SSILB
C534;HANGUL SYLLABLE SSILS
C535;HANGUL SYLLABLE SSILT
C536;HANGUL SYLLABLE SSILP
C537;HANGUL SYLLABLE SSILH
C538;HANGUL SYLLABLE SSIM
C539;HANGUL SYLLABLE SSIB
C53A;HANGUL SYLLABLE SSIBS
C53B;HANGUL SYLLABLE SSIS
C53C;HANGUL SYLLABLE SSISS
C53D;HANGUL SYLLABLE SSING
C53E;HANGUL SYLLABLE SSIJ
C53F;HANGUL SYLLABLE SSIC
C540;HANGUL SYLLABLE SSIK
C541;HANGUL SYLLABLE SSIT
C542;HANGUL SYLLABLE SSIP
C543;HANGUL SYLLABLE SSIH
C544;HANGUL SYLLABLE A
C545;HANGUL SYLLABLE AG
C546;HANGUL SYLLABLE AGG
C547;HANGUL SYLLABLE AGS
C548;HANGUL SYLLABLE AN
C549;HANGUL SYLLABLE ANJ
C54A;HANGUL SYLLABLE ANH
C54B;HANGUL SYLLABLE AD
C54C;HANGUL SYLLABLE AL
C54D;HANGUL SYLLABLE ALG
C54E;HANGUL SYLLABLE ALM
C54F;HANGUL SYLLABLE ALB
C550;HANGUL SYLLABLE ALS
C551;HANGUL SYLLABLE ALT
C552;HANGUL SYLLABLE ALP
C553;HANGUL SYLLABLE ALH
C554;HANGUL SYLLABLE AM
C555;HANGUL SYLLABLE AB
C556;HANGUL SYLLABLE ABS
C557;HANGUL SYLLABLE AS
C558;HANGUL SYLLABLE ASS
C559;HANGUL SYLLABLE ANG
C55A;HANGUL SYLLABLE AJ
C55B;HANGUL SYLLABLE AC
C55C;HANGUL SYLLABLE AK
C55D;HANGUL SYLLABLE AT
C55E;HANGUL SYLLABLE AP
C55F;HANGUL SYLLABLE AH
C560;HANGUL SYLLABLE AE
C561;HANGUL SYLLABLE AEG
C562;HANGUL SYLLABLE AEGG
C563;HANGUL SYLLABLE AEGS
C564;HANGUL SYLLABLE AEN
C565;HANGUL SYLLABLE AENJ
C566;HANGUL SYLLABLE AENH
C567;HANGUL SYLLABLE AED
C568;HANGUL SYLLABLE AEL
C569;HANGUL SYLLABLE AELG
C56A;HANGUL SYLLABLE AELM
C56B;HANGUL SYLLABLE AELB
C56C;HANGUL SYLLABLE AELS
C56D;HANGUL SYLLABLE AELT
C56E;HANGUL SYLLABLE AELP
C56F;HANGUL SYLLABLE AELH
C570;HANGUL SYLLABLE AEM
C571;HANGUL SYLLABLE AEB
C572;HANGUL SYLLABLE AEBS
C573;HANGUL SYLLABLE AES
C574;HANGUL SYLLABLE AESS
C575;HANGUL SYLLABLE AENG
C576;HANGUL SYLLABLE AEJ
C577;HANGUL SYLLABLE AEC
C578;HANGUL SYLLABLE AEK
C579;HANGUL SYLLABLE AET
C57A;HANGUL SYLLABLE AEP
C57B;HANGUL SYLLABLE AEH
C57C;HANGUL SYLLABLE YA
C57D;HANGUL SYLLABLE YAG
C57E;HANGUL SYLLABLE YAGG
C57F;HANGUL SYLLABLE YAGS
C580;HANGUL SYLLABLE YAN
C581;HANGUL SYLLABLE YANJ
C582;HANGUL SYLLABLE YANH
C583;HANGUL SYLLABLE YAD
C584;HANGUL SYLLABLE YAL
C585;HANGUL SYLLABLE YALG
C586;HANGUL SYLLABLE YALM
C587;HANGUL SYLLABLE YALB
C588;HANGUL SYLLABLE YALS
C589;HANGUL SYLLABLE YALT
C58A;HANGUL SYLLABLE YALP
C58B;HANGUL SYLLABLE YALH
C58C;HANGUL SYLLABLE YAM
C58D;HANGUL SYLLABLE YAB
C58E;HANGUL SYLLABLE YABS
C58F;HANGUL SYLLABLE YAS
C590;HANGUL SYLLABLE YASS
C591;HANGUL SYLLABLE YANG
C592;HANGUL SYLLABLE YAJ
C593;HANGUL SYLLABLE YAC
C594;HANGUL SYLLABLE YAK
C595;HANGUL SYLLABLE YAT
C596;HANGUL SYLLABLE YAP
C597;HANGUL SYLLABLE YAH
C598;HANGUL SYLLABLE YAE
C599;HANGUL SYLLABLE YAEG
C59A;HANGUL SYLLABLE YAEGG
C59B;HANGUL SYLLABLE YAEGS
C59C;HANGUL SYLLABLE YAEN
C59D;HANGUL SYLLABLE YAENJ
C59E;HANGUL SYLLABLE YAENH
C59F;HANGUL SYLLABLE YAED
C5A0;HANGUL SYLLABLE YAEL
C5A1;HANGUL SYLLABLE YAELG
C5A2;HANGUL SYLLABLE YAELM
C5A3;HANGUL SYLLABLE YAELB
C5A4;HANGUL SYLLABLE YAELS
C5A5;HANGUL SYLLABLE YAELT
C5A6;HANGUL SYLLABLE YAELP
C5A7;HANGUL SYLLABLE YAELH
C5A8;HANGUL SYLLABLE YAEM
C5A9;HANGUL SYLLABLE YAEB
C5AA;HANGUL SYLLABLE YAEBS
C5AB;HANGUL SYLLABLE YAES
C5AC;HANGUL SYLLABLE YAESS
C5AD;HANGUL SYLLABLE YAENG
C5AE;HANGUL SYLLABLE YAEJ
C5AF;HANGUL SYLLABLE YAEC
C5B0;HANGUL SYLLABLE YAEK
C5B1;HANGUL SYLLABLE YAET
C5B2;HANGUL SYLLABLE YAEP
C5B3;HANGUL SYLLABLE YAEH
C5B4;HANGUL SYLLABLE EO
C5B5;HANGUL SYLLABLE EOG
C5B6;HANGUL SYLLABLE EOGG
C5B7;HANGUL SYLLABLE EOGS
C5B8;HANGUL SYLLABLE EON
C5B9;HANGUL SYLLABLE EONJ
C5BA;HANGUL SYLLABLE EONH
C5BB;HANGUL SYLLABLE EOD
C5BC;HANGUL SYLLABLE EOL
C5BD;HANGUL SYLLABLE EOLG
C5BE;HANGUL SYLLABLE EOLM
C5BF;HANGUL SYLLABLE EOLB
C5C0;HANGUL SYLLABLE EOLS
C5C1;HANGUL SYLLABLE EOLT
C5C2;HANGUL SYLLABLE EOLP
C5C3;HANGUL SYLLABLE EOLH
C5C4;HANGUL SYLLABLE EOM
C5C5;HANGUL SYLLABLE EOB
C5C6;HANGUL SYLLABLE EOBS
C5C7;HANGUL SYLLABLE EOS
C5C8;HANGUL SYLLABLE EOSS
C5C9;HANGUL SYLLABLE EONG
C5CA;HANGUL SYLLABLE EOJ
C5CB;HANGUL SYLLABLE EOC
C5CC;HANGUL SYLLABLE EOK
C5CD;HANGUL SYLLABLE EOT
C5CE;HANGUL SYLLABLE EOP
C5CF;HANGUL SYLLABLE EOH
C5D0;HANGUL SYLLABLE E
C5D1;HANGUL SYLLABLE EG
C5D2;HANGUL SYLLABLE EGG
C5D3;HANGUL SYLLABLE EGS
C5D4;HANGUL SYLLABLE EN
C5D5;HANGUL SYLLABLE ENJ
C5D6;HANGUL SYLLABLE ENH
C5D7;HANGUL SYLLABLE ED
C5D8;HANGUL SYLLABLE EL
C5D9;HANGUL SYLLABLE ELG
C5DA;HANGUL SYLLABLE ELM
C5DB;HANGUL SYLLABLE ELB
C5DC;HANGUL SYLLABLE ELS
C5DD;HANGUL SYLLABLE ELT
C5DE;HANGUL SYLLABLE ELP
C5DF;HANGUL SYLLABLE ELH
C5E0;HANGUL SYLLABLE EM
C5E1;HANGUL SYLLABLE EB
C5E2;HANGUL SYLLABLE EBS
C5E3;HANGUL SYLLABLE ES
C5E4;HANGUL SYLLABLE ESS
C5E5;HANGUL SYLLABLE ENG
C5E6;HANGUL SYLLABLE EJ
C5E7;HANGUL SYLLABLE EC
C5E8;HANGUL SYLLABLE EK
C5E9;HANGUL SYLLABLE ET
C5EA;HANGUL SYLLABLE EP
C5EB;HANGUL SYLLABLE EH
C5EC;HANGUL SYLLABLE YEO
C5ED;HANGUL SYLLABLE YEOG
C5EE;HANGUL SYLLABLE YEOGG
C5EF;HANGUL SYLLABLE YEOGS
C5F0;HANGUL SYLLABLE YEON
C5F1;HANGUL SYLLABLE YEONJ
C5F2;HANGUL SYLLABLE YEONH
C5F3;HANGUL SYLLABLE YEOD
C5F4;HANGUL SYLLABLE YEOL
C5F5;HANGUL SYLLABLE YEOLG
C5F6;HANGUL SYLLABLE YEOLM
C5F7;HANGUL SYLLABLE YEOLB
C5F8;HANGUL SYLLABLE YEOLS
C5F9;HANGUL SYLLABLE YEOLT
C5FA;HANGUL SYLLABLE YEOLP
C5FB;HANGUL SYLLABLE YEOLH
C5FC;HANGUL SYLLABLE YEOM
C5FD;HANGUL SYLLABLE YEOB
C5FE;HANGUL SYLLABLE YEOBS
C5FF;HANGUL SYLLABLE YEOS
C600;HANGUL SYLLABLE YEOSS
C601;HANGUL SYLLABLE YEONG
C602;HANGUL SYLLABLE YEOJ
C603;HANGUL SYLLABLE YEOC
C604;HANGUL SYLLABLE YEOK
C605;HANGUL SYLLABLE YEOT
C606;HANGUL SYLLABLE YEOP
C607;HANGUL SYLLABLE YEOH
C608;HANGUL SYLLABLE YE
C609;HANGUL SYLLABLE YEG
C60A;HANGUL SYLLABLE YEGG
C60B;HANGUL SYLLABLE YEGS
C60C;HANGUL SYLLABLE YEN
C60D;HANGUL SYLLABLE YENJ
C60E;HANGUL SYLLABLE YENH
C60F;HANGUL SYLLABLE YED
C610;HANGUL SYLLABLE YEL
C611;HANGUL SYLLABLE YELG
C612;HANGUL SYLLABLE YELM
C613;HANGUL SYLLABLE YELB
C614;HANGUL SYLLABLE YELS
C615;HANGUL SYLLABLE YELT
C616;HANGUL SYLLABLE YELP
C617;HANGUL SYLLABLE YELH
C618;HANGUL SYLLABLE YEM
C619;HANGUL SYLLABLE YEB
C61A;HANGUL SYLLABLE YEBS
C61B;HANGUL SYLLABLE YES
C61C;HANGUL SYLLABLE YESS
C61D;HANGUL SYLLABLE YENG
C61E;HANGUL SYLLABLE YEJ
C61F;HANGUL SYLLABLE YEC
C620;HANGUL SYLLABLE YEK
C621;HANGUL SYLLABLE YET
C622;HANGUL SYLLABLE YEP
C623;HANGUL SYLLABLE YEH
C624;HANGUL SYLLABLE O
C625;HANGUL SYLLABLE OG
C626;HANGUL SYLLABLE OGG
C627;HANGUL SYLLABLE OGS
C628;HANGUL SYLLABLE ON
C629;HANGUL SYLLABLE ONJ
C62A;HANGUL SYLLABLE ONH
C62B;HANGUL SYLLABLE OD
C62C;HANGUL SYLLABLE OL
C62D;HANGUL SYLLABLE OLG
C62E;HANGUL SYLLABLE OLM
C62F;HANGUL SYLLABLE OLB
C630;HANGUL SYLLABLE OLS
C631;HANGUL SYLLABLE OLT
C632;HANGUL SYLLABLE OLP
C633;HANGUL SYLLABLE OLH
C634;HANGUL SYLLABLE OM
C635;HANGUL SYLLABLE OB
C636;HANGUL SYLLABLE OBS
C637;HANGUL SYLLABLE OS
C638;HANGUL SYLLABLE OSS
C639;HANGUL SYLLABLE ONG
C63A;HANGUL SYLLABLE OJ
C63B;HANGUL SYLLABLE OC
C63C;HANGUL SYLLABLE OK
C63D;HANGUL SYLLABLE OT
C63E;HANGUL SYLLABLE OP
C63F;HANGUL SYLLABLE OH
C640;HANGUL SYLLABLE WA
C641;HANGUL SYLLABLE WAG
C642;HANGUL SYLLABLE WAGG
C643;HANGUL SYLLABLE WAGS
C644;HANGUL SYLLABLE WAN
C645;HANGUL SYLLABLE WANJ
C646;HANGUL SYLLABLE WANH
C647;HANGUL SYLLABLE WAD
C648;HANGUL SYLLABLE WAL
C649;HANGUL SYLLABLE WALG
C64A;HANGUL SYLLABLE WALM
C64B;HANGUL SYLLABLE WALB
C64C;HANGUL SYLLABLE WALS
C64D;HANGUL SYLLABLE WALT
C64E;HANGUL SYLLABLE WALP
C64F;HANGUL SYLLABLE WALH
C650;HANGUL SYLLABLE WAM
C651;HANGUL SYLLABLE WAB
C652;HANGUL SYLLABLE WABS
C653;HANGUL SYLLABLE WAS
C654;HANGUL SYLLABLE WASS
C655;HANGUL SYLLABLE WANG
C656;HANGUL SYLLABLE WAJ
C657;HANGUL SYLLABLE WAC
C658;HANGUL SYLLABLE WAK
C659;HANGUL SYLLABLE WAT
C65A;HANGUL SYLLABLE WAP
C65B;HANGUL SYLLABLE WAH
C65C;HANGUL SYLLABLE WAE
C65D;HANGUL SYLLABLE WAEG
C65E;HANGUL SYLLABLE WAEGG
C65F;HANGUL SYLLABLE WAEGS
C660;HANGUL SYLLABLE WAEN
C661;HANGUL SYLLABLE WAENJ
C662;HANGUL SYLLABLE WAENH
C663;HANGUL SYLLABLE WAED
C664;HANGUL SYLLABLE WAEL
C665;HANGUL SYLLABLE WAELG
C666;HANGUL SYLLABLE WAELM
C667;HANGUL SYLLABLE WAELB
C668;HANGUL SYLLABLE WAELS
C669;HANGUL SYLLABLE WAELT
C66A;HANGUL SYLLABLE WAELP
C66B;HANGUL SYLLABLE WAELH
C66C;HANGUL SYLLABLE WAEM
C66D;HANGUL SYLLABLE WAEB
C66E;HANGUL SYLLABLE WAEBS
C66F;HANGUL SYLLABLE WAES
C670;HANGUL SYLLABLE WAESS
C671;HANGUL SYLLABLE WAENG
C672;HANGUL SYLLABLE WAEJ
C673;HANGUL SYLLABLE WAEC
C674;HANGUL SYLLABLE WAEK
C675;HANGUL SYLLABLE WAET
C676;HANGUL SYLLABLE WAEP
C677;HANGUL SYLLABLE WAEH
C678;HANGUL SYLLABLE OE
C679;HANGUL SYLLABLE OEG
C67A;HANGUL SYLLABLE OEGG
C67B;HANGUL SYLLABLE OEGS
C67C;HANGUL SYLLABLE OEN
C67D;HANGUL SYLLABLE OENJ
C67E;HANGUL SYLLABLE OENH
C67F;HANGUL SYLLABLE OED
C680;HANGUL SYLLABLE OEL
C681;HANGUL SYLLABLE OELG
C682;HANGUL SYLLABLE OELM
C683;HANGUL SYLLABLE OELB
C684;HANGUL SYLLABLE OELS
C685;HANGUL SYLLABLE OELT
C686;HANGUL SYLLABLE OELP
C687;HANGUL SYLLABLE OELH
C688;HANGUL SYLLABLE OEM
C689;HANGUL SYLLABLE OEB
C68A;HANGUL SYLLABLE OEBS
C68B;HANGUL SYLLABLE OES
C68C;HANGUL SYLLABLE OESS
C68D;HANGUL SYLLABLE OENG
C68E;HANGUL SYLLABLE OEJ
C68F;HANGUL SYLLABLE OEC
C690;HANGUL SYLLABLE OEK
C691;HANGUL SYLLABLE OET
C692;HANGUL SYLLABLE OEP
C693;HANGUL SYLLABLE OEH
C694;HANGUL SYLLABLE YO
C695;HANGUL SYLLABLE YOG
C696;HANGUL SYLLABLE YOGG
C697;HANGUL SYLLABLE YOGS
C698;HANGUL SYLLABLE YON
C699;HANGUL SYLLABLE YONJ
C69A;HANGUL SYLLABLE YONH
C69B;HANGUL SYLLABLE YOD
C69C;HANGUL SYLLABLE YOL
C69D;HANGUL SYLLABLE YOLG
C69E;HANGUL SYLLABLE YOLM
C69F;HANGUL SYLLABLE YOLB
C6A0;HANGUL SYLLABLE YOLS
C6A1;HANGUL SYLLABLE YOLT
C6A2;HANGUL SYLLABLE YOLP
C6A3;HANGUL SYLLABLE YOLH
C6A4;HANGUL SYLLABLE YOM
C6A5;HANGUL SYLLABLE YOB
C6A6;HANGUL SYLLABLE YOBS
C6A7;HANGUL SYLLABLE YOS
C6A8;HANGUL SYLLABLE YOSS
C6A9;HANGUL SYLLABLE YONG
C6AA;HANGUL SYLLABLE YOJ
C6AB;HANGUL SYLLABLE YOC
C6AC;HANGUL SYLLABLE YOK
C6AD;HANGUL SYLLABLE YOT
C6AE;HANGUL SYLLABLE YOP
C6AF;HANGUL SYLLABLE YOH
C6B0;HANGUL SYLLABLE U
C6B1;HANGUL SYLLABLE UG
C6B2;HANGUL SYLLABLE UGG
C6B3;HANGUL SYLLABLE UGS
C6B4;HANGUL SYLLABLE UN
C6B5;HANGUL SYLLABLE UNJ
C6B6;HANGUL SYLLABLE UNH
C6B7;HANGUL SYLLABLE UD
C6B8;HANGUL SYLLABLE UL
C6B9;HANGUL SYLLABLE ULG
C6BA;HANGUL SYLLABLE ULM
C6BB;HANGUL SYLLABLE ULB
C6BC;HANGUL SYLLABLE ULS
C6BD;HANGUL SYLLABLE ULT
C6BE;HANGUL SYLLABLE ULP
C6BF;HANGUL SYLLABLE ULH
C6C0;HANGUL SYLLABLE UM
C6C1;HANGUL SYLLABLE UB
C6C2;HANGUL SYLLABLE UBS
C6C3;HANGUL SYLLABLE US
C6C4;HANGUL SYLLABLE USS
C6C5;HANGUL SYLLABLE UNG
C6C6;HANGUL SYLLABLE UJ
C6C7;HANGUL SYLLABLE UC
C6C8;HANGUL SYLLABLE UK
C6C9;HANGUL SYLLABLE UT
C6CA;HANGUL SYLLABLE UP
C6CB;HANGUL SYLLABLE UH
C6CC;HANGUL SYLLABLE WEO
C6CD;HANGUL SYLLABLE WEOG
C6CE;HANGUL SYLLABLE WEOGG
C6CF;HANGUL SYLLABLE WEOGS
C6D0;HANGUL SYLLABLE WEON
C6D1;HANGUL SYLLABLE WEONJ
C6D2;HANGUL SYLLABLE WEONH
C6D3;HANGUL SYLLABLE WEOD
C6D4;HANGUL SYLLABLE WEOL
C6D5;HANGUL SYLLABLE WEOLG
C6D6;HANGUL SYLLABLE WEOLM
C6D7;HANGUL SYLLABLE WEOLB
C6D8;HANGUL SYLLABLE WEOLS
C6D9;HANGUL SYLLABLE WEOLT
C6DA;HANGUL SYLLABLE WEOLP
C6DB;HANGUL SYLLABLE WEOLH
C6DC;HANGUL SYLLABLE WEOM
C6DD;HANGUL SYLLABLE WEOB
C6DE;HANGUL SYLLABLE WEOBS
C6DF;HANGUL SYLLABLE WEOS
C6E0;HANGUL SYLLABLE WEOSS
C6E1;HANGUL SYLLABLE WEONG
C6E2;HANGUL SYLLABLE WEOJ
C6E3;HANGUL SYLLABLE WEOC
C6E4;HANGUL SYLLABLE WEOK
C6E5;HANGUL SYLLABLE WEOT
C6E6;HANGUL SYLLABLE WEOP
C6E7;HANGUL SYLLABLE WEOH
C6E8;HANGUL SYLLABLE WE
C6E9;HANGUL SYLLABLE WEG
C6EA;HANGUL SYLLABLE WEGG
C6EB;HANGUL SYLLABLE WEGS
C6EC;HANGUL SYLLABLE WEN
C6ED;HANGUL SYLLABLE WENJ
C6EE;HANGUL SYLLABLE WENH
C6EF;HANGUL SYLLABLE WED
C6F0;HANGUL SYLLABLE WEL
C6F1;HANGUL SYLLABLE WELG
C6F2;HANGUL SYLLABLE WELM
C6F3;HANGUL SYLLABLE WELB
C6F4;HANGUL SYLLABLE WELS
C6F5;HANGUL SYLLABLE WELT
C6F6;HANGUL SYLLABLE WELP
C6F7;HANGUL SYLLABLE WELH
C6F8;HANGUL SYLLABLE WEM
C6F9;HANGUL SYLLABLE WEB
C6FA;HANGUL SYLLABLE WEBS
C6FB;HANGUL SYLLABLE WES
C6FC;HANGUL SYLLABLE WESS
C6FD;HANGUL SYLLABLE WENG
C6FE;HANGUL SYLLABLE WEJ
C6FF;HANGUL SYLLABLE WEC
C700;HANGUL SYLLABLE WEK
C701;HANGUL SYLLABLE WET
C702;HANGUL SYLLABLE WEP
C703;HANGUL SYLLABLE WEH
C704;HANGUL SYLLABLE WI
C705;HANGUL SYLLABLE WIG
C706;HANGUL SYLLABLE WIGG
C707;HANGUL SYLLABLE WIGS
C708;HANGUL SYLLABLE WIN
C709;HANGUL SYLLABLE WINJ
C70A;HANGUL SYLLABLE WINH
C70B;HANGUL SYLLABLE WID
C70C;HANGUL SYLLABLE WIL
C70D;HANGUL SYLLABLE WILG
C70E;HANGUL SYLLABLE WILM
C70F;HANGUL SYLLABLE WILB
C710;HANGUL SYLLABLE WILS
C711;HANGUL SYLLABLE WILT
C712;HANGUL SYLLABLE WILP
C713;HANGUL SYLLABLE WILH
C714;HANGUL SYLLABLE WIM
C715;HANGUL SYLLABLE WIB
C716;HANGUL SYLLABLE WIBS
C717;HANGUL SYLLABLE WIS
C718;HANGUL SYLLABLE WISS
C719;HANGUL SYLLABLE WING
C71A;HANGUL SYLLABLE WIJ
C71B;HANGUL SYLLABLE WIC
C71C;HANGUL SYLLABLE WIK
C71D;HANGUL SYLLABLE WIT
C71E;HANGUL SYLLABLE WIP
C71F;HANGUL SYLLABLE WIH
C720;HANGUL SYLLABLE YU
C721;HANGUL SYLLABLE YUG
C722;HANGUL SYLLABLE YUGG
C723;HANGUL SYLLABLE YUGS
C724;HANGUL SYLLABLE YUN
C725;HANGUL SYLLABLE YUNJ
C726;HANGUL SYLLABLE YUNH
C727;HANGUL SYLLABLE YUD
C728;HANGUL SYLLABLE YUL
C729;HANGUL SYLLABLE YULG
C72A;HANGUL SYLLABLE YULM
C72B;HANGUL SYLLABLE YULB
C72C;HANGUL SYLLABLE YULS
C72D;HANGUL SYLLABLE YULT
C72E;HANGUL SYLLABLE YULP
C72F;HANGUL SYLLABLE YULH
C730;HANGUL SYLLABLE YUM
C731;HANGUL SYLLABLE YUB
C732;HANGUL SYLLABLE YUBS
C733;HANGUL SYLLABLE YUS
C734;HANGUL SYLLABLE YUSS
C735;HANGUL SYLLABLE YUNG
C736;HANGUL SYLLABLE YUJ
C737;HANGUL SYLLABLE YUC
C738;HANGUL SYLLABLE YUK
C739;HANGUL SYLLABLE YUT
C73A;HANGUL SYLLABLE YUP
C73B;HANGUL SYLLABLE YUH
C73C;HANGUL SYLLABLE EU
C73D;HANGUL SYLLABLE EUG
C73E;HANGUL SYLLABLE EUGG
C73F;HANGUL SYLLABLE EUGS
C740;HANGUL SYLLABLE EUN
C741;HANGUL SYLLABLE EUNJ
C742;HANGUL SYLLABLE EUNH
C743;HANGUL SYLLABLE EUD
C744;HANGUL SYLLABLE EUL
C745;HANGUL SYLLABLE EULG
C746;HANGUL SYLLABLE EULM
C747;HANGUL SYLLABLE EULB
C748;HANGUL SYLLABLE EULS
C749;HANGUL SYLLABLE EULT
C74A;HANGUL SYLLABLE EULP
C74B;HANGUL SYLLABLE EULH
C74C;HANGUL SYLLABLE EUM
C74D;HANGUL SYLLABLE EUB
C74E;HANGUL SYLLABLE EUBS
C74F;HANGUL SYLLABLE EUS
C750;HANGUL SYLLABLE EUSS
C751;HANGUL SYLLABLE EUNG
C752;HANGUL SYLLABLE EUJ
C753;HANGUL SYLLABLE EUC
C754;HANGUL SYLLABLE EUK
C755;HANGUL SYLLABLE EUT
C756;HANGUL SYLLABLE EUP
C757;HANGUL SYLLABLE EUH
C758;HANGUL SYLLABLE YI
C759;HANGUL SYLLABLE YIG
C75A;HANGUL SYLLABLE YIGG
C75B;HANGUL SYLLABLE YIGS
C75C;HANGUL SYLLABLE YIN
C75D;HANGUL SYLLABLE YINJ
C75E;HANGUL SYLLABLE YINH
C75F;HANGUL SYLLABLE YID
C760;HANGUL SYLLABLE YIL
C761;HANGUL SYLLABLE YILG
C762;HANGUL SYLLABLE YILM
C763;HANGUL SYLLABLE YILB
C764;HANGUL SYLLABLE YILS
C765;HANGUL SYLLABLE YILT
C766;HANGUL SYLLABLE YILP
C767;HANGUL SYLLABLE YILH
C768;HANGUL SYLLABLE YIM
C769;HANGUL SYLLABLE YIB
C76A;HANGUL SYLLABLE YIBS
C76B;HANGUL SYLLABLE YIS
C76C;HANGUL SYLLABLE YISS
C76D;HANGUL SYLLABLE YING
C76E;HANGUL SYLLABLE YIJ
C76F;HANGUL SYLLABLE YIC
C770;HANGUL SYLLABLE YIK
C771;HANGUL SYLLABLE YIT
C772;HANGUL SYLLABLE YIP
C773;HANGUL SYLLABLE YIH
C774;HANGUL SYLLABLE I
C775;HANGUL SYLLABLE IG
C776;HANGUL SYLLABLE IGG
C777;HANGUL SYLLABLE IGS
C778;HANGUL SYLLABLE IN
C779;HANGUL SYLLABLE INJ
C77A;HANGUL SYLLABLE INH
C77B;HANGUL SYLLABLE ID
C77C;HANGUL SYLLABLE IL
C77D;HANGUL SYLLABLE ILG
C77E;HANGUL SYLLABLE ILM
C77F;HANGUL SYLLABLE ILB
C780;HANGUL SYLLABLE ILS
C781;HANGUL SYLLABLE ILT
C782;HANGUL SYLLABLE ILP
C783;HANGUL SYLLABLE ILH
C784;HANGUL SYLLABLE IM
C785;HANGUL SYLLABLE IB
C786;HANGUL SYLLABLE IBS
C787;HANGUL SYLLABLE IS
C788;HANGUL SYLLABLE ISS
C789;HANGUL SYLLABLE ING
C78A;HANGUL SYLLABLE IJ
C78B;HANGUL SYLLABLE IC
C78C;HANGUL SYLLABLE IK
C78D;HANGUL SYLLABLE IT
C78E;HANGUL SYLLABLE IP
C78F;HANGUL SYLLABLE IH
C790;HANGUL SYLLABLE JA
C791;HANGUL SYLLABLE JAG
C792;HANGUL SYLLABLE JAGG
C793;HANGUL SYLLABLE JAGS
C794;HANGUL SYLLABLE JAN
C795;HANGUL SYLLABLE JANJ
C796;HANGUL SYLLABLE JANH
C797;HANGUL SYLLABLE JAD
C798;HANGUL SYLLABLE JAL
C799;HANGUL SYLLABLE JALG
C79A;HANGUL SYLLABLE JALM
C79B;HANGUL SYLLABLE JALB
C79C;HANGUL SYLLABLE JALS
C79D;HANGUL SYLLABLE JALT
C79E;HANGUL SYLLABLE JALP
C79F;HANGUL SYLLABLE JALH
C7A0;HANGUL SYLLABLE JAM
C7A1;HANGUL SYLLABLE JAB
C7A2;HANGUL SYLLABLE JABS
C7A3;HANGUL SYLLABLE JAS
C7A4;HANGUL SYLLABLE JASS
C7A5;HANGUL SYLLABLE JANG
C7A6;HANGUL SYLLABLE JAJ
C7A7;HANGUL SYLLABLE JAC
C7A8;HANGUL SYLLABLE JAK
C7A9;HANGUL SYLLABLE JAT
C7AA;HANGUL SYLLABLE JAP
C7AB;HANGUL SYLLABLE JAH
C7AC;HANGUL SYLLABLE JAE
C7AD;HANGUL SYLLABLE JAEG
C7AE;HANGUL SYLLABLE JAEGG
C7AF;HANGUL SYLLABLE JAEGS
C7B0;HANGUL SYLLABLE JAEN
C7B1;HANGUL SYLLABLE JAENJ
C7B2;HANGUL SYLLABLE JAENH
C7B3;HANGUL SYLLABLE JAED
C7B4;HANGUL SYLLABLE JAEL
C7B5;HANGUL SYLLABLE JAELG
C7B6;HANGUL SYLLABLE JAELM
C7B7;HANGUL SYLLABLE JAELB
C7B8;HANGUL SYLLABLE JAELS
C7B9;HANGUL SYLLABLE JAELT
C7BA;HANGUL SYLLABLE JAELP
C7BB;HANGUL SYLLABLE JAELH
C7BC;HANGUL SYLLABLE JAEM
C7BD;HANGUL SYLLABLE JAEB
C7BE;HANGUL SYLLABLE JAEBS
C7BF;HANGUL SYLLABLE JAES
C7C0;HANGUL SYLLABLE JAESS
C7C1;HANGUL SYLLABLE JAENG
C7C2;HANGUL SYLLABLE JAEJ
C7C3;HANGUL SYLLABLE JAEC
C7C4;HANGUL SYLLABLE JAEK
C7C5;HANGUL SYLLABLE JAET
C7C6;HANGUL SYLLABLE JAEP
C7C7;HANGUL SYLLABLE JAEH
C7C8;HANGUL SYLLABLE JYA
C7C9;HANGUL SYLLABLE JYAG
C7CA;HANGUL SYLLABLE JYAGG
C7CB;HANGUL SYLLABLE JYAGS
C7CC;HANGUL SYLLABLE JYAN
C7CD;HANGUL SYLLABLE JYANJ
C7CE;HANGUL SYLLABLE JYANH
C7CF;HANGUL SYLLABLE JYAD
C7D0;HANGUL SYLLABLE JYAL
C7D1;HANGUL SYLLABLE JYALG
C7D2;HANGUL SYLLABLE JYALM
C7D3;HANGUL SYLLABLE JYALB
C7D4;HANGUL SYLLABLE JYALS
C7D5;HANGUL SYLLABLE JYALT
C7D6;HANGUL SYLLABLE JYALP
C7D7;HANGUL SYLLABLE JYALH
C7D8;HANGUL SYLLABLE JYAM
C7D9;HANGUL SYLLABLE JYAB
C7DA;HANGUL SYLLABLE JYABS
C7DB;HANGUL SYLLABLE JYAS
C7DC;HANGUL SYLLABLE JYASS
C7DD;HANGUL SYLLABLE JYANG
C7DE;HANGUL SYLLABLE JYAJ
C7DF;HANGUL SYLLABLE JYAC
C7E0;HANGUL SYLLABLE JYAK
C7E1;HANGUL SYLLABLE JYAT
C7E2;HANGUL SYLLABLE JYAP
C7E3;HANGUL SYLLABLE JYAH
C7E4;HANGUL SYLLABLE JYAE
C7E5;HANGUL SYLLABLE JYAEG
C7E6;HANGUL SYLLABLE JYAEGG
C7E7;HANGUL SYLLABLE JYAEGS
C7E8;HANGUL SYLLABLE JYAEN
C7E9;HANGUL SYLLABLE JYAENJ
C7EA;HANGUL SYLLABLE JYAENH
C7EB;HANGUL SYLLABLE JYAED
C7EC;HANGUL SYLLABLE JYAEL
C7ED;HANGUL SYLLABLE JYAELG
C7EE;HANGUL SYLLABLE JYAELM
C7EF;HANGUL SYLLABLE JYAELB
C7F0;HANGUL SYLLABLE JYAELS
C7F1;HANGUL SYLLABLE JYAELT
C7F2;HANGUL SYLLABLE JYAELP
C7F3;HANGUL SYLLABLE JYAELH
C7F4;HANGUL SYLLABLE JYAEM
C7F5;HANGUL SYLLABLE JYAEB
C7F6;HANGUL SYLLABLE JYAEBS
C7F7;HANGUL SYLLABLE JYAES
C7F8;HANGUL SYLLABLE JYAESS
C7F9;HANGUL SYLLABLE JYAENG
C7FA;HANGUL SYLLABLE JYAEJ
C7FB;HANGUL SYLLABLE JYAEC
C7FC;HANGUL SYLLABLE JYAEK
C7FD;HANGUL SYLLABLE JYAET
C7FE;HANGUL SYLLABLE JYAEP
C7FF;HANGUL SYLLABLE JYAEH
C800;HANGUL SYLLABLE JEO
C801;HANGUL SYLLABLE JEOG
C802;HANGUL SYLLABLE JEOGG
C803;HANGUL SYLLABLE JEOGS
C804;HANGUL SYLLABLE JEON
C805;HANGUL SYLLABLE JEONJ
C806;HANGUL SYLLABLE JEONH
C807;HANGUL SYLLABLE JEOD
C808;HANGUL SYLLABLE JEOL
C809;HANGUL SYLLABLE JEOLG
C80A;HANGUL SYLLABLE JEOLM
C80B;HANGUL SYLLABLE JEOLB
C80C;HANGUL SYLLABLE JEOLS
C80D;HANGUL SYLLABLE JEOLT
C80E;HANGUL SYLLABLE JEOLP
C80F;HANGUL SYLLABLE JEOLH
C810;HANGUL SYLLABLE JEOM
C811;HANGUL SYLLABLE JEOB
C812;HANGUL SYLLABLE JEOBS
C813;HANGUL SYLLABLE JEOS
C814;HANGUL SYLLABLE JEOSS
C815;HANGUL SYLLABLE JEONG
C816;HANGUL SYLLABLE JEOJ
C817;HANGUL SYLLABLE JEOC
C818;HANGUL SYLLABLE JEOK
C819;HANGUL SYLLABLE JEOT
C81A;HANGUL SYLLABLE JEOP
C81B;HANGUL SYLLABLE JEOH
C81C;HANGUL SYLLABLE JE
C81D;HANGUL SYLLABLE JEG
C81E;HANGUL SYLLABLE JEGG
C81F;HANGUL SYLLABLE JEGS
C820;HANGUL SYLLABLE JEN
C821;HANGUL SYLLABLE JENJ
C822;HANGUL SYLLABLE JENH
C823;HANGUL SYLLABLE JED
C824;HANGUL SYLLABLE JEL
C825;HANGUL SYLLABLE JELG
C826;HANGUL SYLLABLE JELM
C827;HANGUL SYLLABLE JELB
C828;HANGUL SYLLABLE JELS
C829;HANGUL SYLLABLE JELT
C82A;HANGUL SYLLABLE JELP
C82B;HANGUL SYLLABLE JELH
C82C;HANGUL SYLLABLE JEM
C82D;HANGUL SYLLABLE JEB
C82E;HANGUL SYLLABLE JEBS
C82F;HANGUL SYLLABLE JES
C830;HANGUL SYLLABLE JESS
C831;HANGUL SYLLABLE JENG
C832;HANGUL SYLLABLE JEJ
C833;HANGUL SYLLABLE JEC
C834;HANGUL SYLLABLE JEK
C835;HANGUL SYLLABLE JET
C836;HANGUL SYLLABLE JEP
C837;HANGUL SYLLABLE JEH
C838;HANGUL SYLLABLE JYEO
C839;HANGUL SYLLABLE JYEOG
C83A;HANGUL SYLLABLE JYEOGG
C83B;HANGUL SYLLABLE JYEOGS
C83C;HANGUL SYLLABLE JYEON
C83D;HANGUL SYLLABLE JYEONJ
C83E;HANGUL SYLLABLE JYEONH
C83F;HANGUL SYLLABLE JYEOD
C840;HANGUL SYLLABLE JYEOL
C841;HANGUL SYLLABLE JYEOLG
C842;HANGUL SYLLABLE JYEOLM
C843;HANGUL SYLLABLE JYEOLB
C844;HANGUL SYLLABLE JYEOLS
C845;HANGUL SYLLABLE JYEOLT
C846;HANGUL SYLLABLE JYEOLP
C847;HANGUL SYLLABLE JYEOLH
C848;HANGUL SYLLABLE JYEOM
C849;HANGUL SYLLABLE JYEOB
C84A;HANGUL SYLLABLE JYEOBS
C84B;HANGUL SYLLABLE JYEOS
C84C;HANGUL SYLLABLE JYEOSS
C84D;HANGUL SYLLABLE JYEONG
C84E;HANGUL SYLLABLE JYEOJ
C84F;HANGUL SYLLABLE JYEOC
C850;HANGUL SYLLABLE JYEOK
C851;HANGUL SYLLABLE JYEOT
C852;HANGUL SYLLABLE JYEOP
C853;HANGUL SYLLABLE JYEOH
C854;HANGUL SYLLABLE JYE
C855;HANGUL SYLLABLE JYEG
C856;HANGUL SYLLABLE JYEGG
C857;HANGUL SYLLABLE JYEGS
C858;HANGUL SYLLABLE JYEN
C859;HANGUL SYLLABLE JYENJ
C85A;HANGUL SYLLABLE JYENH
C85B;HANGUL SYLLABLE JYED
C85C;HANGUL SYLLABLE JYEL
C85D;HANGUL SYLLABLE JYELG
C85E;HANGUL SYLLABLE JYELM
C85F;HANGUL SYLLABLE JYELB
C860;HANGUL SYLLABLE JYELS
C861;HANGUL SYLLABLE JYELT
C862;HANGUL SYLLABLE JYELP
C863;HANGUL SYLLABLE JYELH
C864;HANGUL SYLLABLE JYEM
C865;HANGUL SYLLABLE JYEB
C866;HANGUL SYLLABLE JYEBS
C867;HANGUL SYLLABLE JYES
C868;HANGUL SYLLABLE JYESS
C869;HANGUL SYLLABLE JYENG
C86A;HANGUL SYLLABLE JYEJ
C86B;HANGUL SYLLABLE JYEC
C86C;HANGUL SYLLABLE JYEK
C86D;HANGUL SYLLABLE JYET
C86E;HANGUL SYLLABLE JYEP
C86F;HANGUL SYLLABLE JYEH
C870;HANGUL SYLLABLE JO
C871;HANGUL SYLLABLE JOG
C872;HANGUL SYLLABLE JOGG
C873;HANGUL SYLLABLE JOGS
C874;HANGUL SYLLABLE JON
C875;HANGUL SYLLABLE JONJ
C876;HANGUL SYLLABLE JONH
C877;HANGUL SYLLABLE JOD
C878;HANGUL SYLLABLE JOL
C879;HANGUL SYLLABLE JOLG
C87A;HANGUL SYLLABLE JOLM
C87B;HANGUL SYLLABLE JOLB
C87C;HANGUL SYLLABLE JOLS
C87D;HANGUL SYLLABLE JOLT
C87E;HANGUL SYLLABLE JOLP
C87F;HANGUL SYLLABLE JOLH
C880;HANGUL SYLLABLE JOM
C881;HANGUL SYLLABLE JOB
C882;HANGUL SYLLABLE JOBS
C883;HANGUL SYLLABLE JOS
C884;HANGUL SYLLABLE JOSS
C885;HANGUL SYLLABLE JONG
C886;HANGUL SYLLABLE JOJ
C887;HANGUL SYLLABLE JOC
C888;HANGUL SYLLABLE JOK
C889;HANGUL SYLLABLE JOT
C88A;HANGUL SYLLABLE JOP
C88B;HANGUL SYLLABLE JOH
C88C;HANGUL SYLLABLE JWA
C88D;HANGUL SYLLABLE JWAG
C88E;HANGUL SYLLABLE JWAGG
C88F;HANGUL SYLLABLE JWAGS
C890;HANGUL SYLLABLE JWAN
C891;HANGUL SYLLABLE JWANJ
C892;HANGUL SYLLABLE JWANH
C893;HANGUL SYLLABLE JWAD
C894;HANGUL SYLLABLE JWAL
C895;HANGUL SYLLABLE JWALG
C896;HANGUL SYLLABLE JWALM
C897;HANGUL SYLLABLE JWALB
C898;HANGUL SYLLABLE JWALS
C899;HANGUL SYLLABLE JWALT
C89A;HANGUL SYLLABLE JWALP
C89B;HANGUL SYLLABLE JWALH
C89C;HANGUL SYLLABLE JWAM
C89D;HANGUL SYLLABLE JWAB
C89E;HANGUL SYLLABLE JWABS
C89F;HANGUL SYLLABLE JWAS
C8A0;HANGUL SYLLABLE JWASS
C8A1;HANGUL SYLLABLE JWANG
C8A2;HANGUL SYLLABLE JWAJ
C8A3;HANGUL SYLLABLE JWAC
C8A4;HANGUL SYLLABLE JWAK
C8A5;HANGUL SYLLABLE JWAT
C8A6;HANGUL SYLLABLE JWAP
C8A7;HANGUL SYLLABLE JWAH
C8A8;HANGUL SYLLABLE JWAE
C8A9;HANGUL SYLLABLE JWAEG
C8AA;HANGUL SYLLABLE JWAEGG
C8AB;HANGUL SYLLABLE JWAEGS
C8AC;HANGUL SYLLABLE JWAEN
C8AD;HANGUL SYLLABLE JWAENJ
C8AE;HANGUL SYLLABLE JWAENH
C8AF;HANGUL SYLLABLE JWAED
C8B0;HANGUL SYLLABLE JWAEL
C8B1;HANGUL SYLLABLE JWAELG
C8B2;HANGUL SYLLABLE JWAELM
C8B3;HANGUL SYLLABLE JWAELB
C8B4;HANGUL SYLLABLE JWAELS
C8B5;HANGUL SYLLABLE JWAELT
C8B6;HANGUL SYLLABLE JWAELP
C8B7;HANGUL SYLLABLE JWAELH
C8B8;HANGUL SYLLABLE JWAEM
C8B9;HANGUL SYLLABLE JWAEB
C8BA;HANGUL SYLLABLE JWAEBS
C8BB;HANGUL SYLLABLE JWAES
C8BC;HANGUL SYLLABLE JWAESS
C8BD;HANGUL SYLLABLE JWAENG
C8BE;HANGUL SYLLABLE JWAEJ
C8BF;HANGUL SYLLABLE JWAEC
C8C0;HANGUL SYLLABLE JWAEK
C8C1;HANGUL SYLLABLE JWAET
C8C2;HANGUL SYLLABLE JWAEP
C8C3;HANGUL SYLLABLE JWAEH
C8C4;HANGUL SYLLABLE JOE
C8C5;HANGUL SYLLABLE JOEG
C8C6;HANGUL SYLLABLE JOEGG
C8C7;HANGUL SYLLABLE JOEGS
C8C8;HANGUL SYLLABLE JOEN
C8C9;HANGUL SYLLABLE JOENJ
C8CA;HANGUL SYLLABLE JOENH
C8CB;HANGUL SYLLABLE JOED
C8CC;HANGUL SYLLABLE JOEL
C8CD;HANGUL SYLLABLE JOELG
C8CE;HANGUL SYLLABLE JOELM
C8CF;HANGUL SYLLABLE JOELB
C8D0;HANGUL SYLLABLE JOELS
C8D1;HANGUL SYLLABLE JOELT
C8D2;HANGUL SYLLABLE JOELP
C8D3;HANGUL SYLLABLE JOELH
C8D4;HANGUL SYLLABLE JOEM
C8D5;HANGUL SYLLABLE JOEB
C8D6;HANGUL SYLLABLE JOEBS
C8D7;HANGUL SYLLABLE JOES
C8D8;HANGUL SYLLABLE JOESS
C8D9;HANGUL SYLLABLE JOENG
C8DA;HANGUL SYLLABLE JOEJ
C8DB;HANGUL SYLLABLE JOEC
C8DC;HANGUL SYLLABLE JOEK
C8DD;HANGUL SYLLABLE JOET
C8DE;HANGUL SYLLABLE JOEP
C8DF;HANGUL SYLLABLE JOEH
C8E0;HANGUL SYLLABLE JYO
C8E1;HANGUL SYLLABLE JYOG
C8E2;HANGUL SYLLABLE JYOGG
C8E3;HANGUL SYLLABLE JYOGS
C8E4;HANGUL SYLLABLE JYON
C8E5;HANGUL SYLLABLE JYONJ
C8E6;HANGUL SYLLABLE JYONH
C8E7;HANGUL SYLLABLE JYOD
C8E8;HANGUL SYLLABLE JYOL
C8E9;HANGUL SYLLABLE JYOLG
C8EA;HANGUL SYLLABLE JYOLM
C8EB;HANGUL SYLLABLE JYOLB
C8EC;HANGUL SYLLABLE JYOLS
C8ED;HANGUL SYLLABLE JYOLT
C8EE;HANGUL SYLLABLE JYOLP
C8EF;HANGUL SYLLABLE JYOLH
C8F0;HANGUL SYLLABLE JYOM
C8F1;HANGUL SYLLABLE JYOB
C8F2;HANGUL SYLLABLE JYOBS
C8F3;HANGUL SYLLABLE JYOS
C8F4;HANGUL SYLLABLE JYOSS
C8F5;HANGUL SYLLABLE JYONG
C8F6;HANGUL SYLLABLE JYOJ
C8F7;HANGUL SYLLABLE JYOC
C8F8;HANGUL SYLLABLE JYOK
C8F9;HANGUL SYLLABLE JYOT
C8FA;HANGUL SYLLABLE JYOP
C8FB;HANGUL SYLLABLE JYOH
C8FC;HANGUL SYLLABLE JU
C8FD;HANGUL SYLLABLE JUG
C8FE;HANGUL SYLLABLE JUGG
C8FF;HANGUL SYLLABLE JUGS
C900;HANGUL SYLLABLE JUN
C901;HANGUL SYLLABLE JUNJ
C902;HANGUL SYLLABLE JUNH
C903;HANGUL SYLLABLE JUD
C904;HANGUL SYLLABLE JUL
C905;HANGUL SYLLABLE JULG
C906;HANGUL SYLLABLE JULM
C907;HANGUL SYLLABLE JULB
C908;HANGUL SYLLABLE JULS
C909;HANGUL SYLLABLE JULT
C90A;HANGUL SYLLABLE JULP
C90B;HANGUL SYLLABLE JULH
C90C;HANGUL SYLLABLE JUM
C90D;HANGUL SYLLABLE JUB
C90E;HANGUL SYLLABLE JUBS
C90F;HANGUL SYLLABLE JUS
C910;HANGUL SYLLABLE JUSS
C911;HANGUL SYLLABLE JUNG
C912;HANGUL SYLLABLE JUJ
C913;HANGUL SYLLABLE JUC
C914;HANGUL SYLLABLE JUK
C915;HANGUL SYLLABLE JUT
C916;HANGUL SYLLABLE JUP
C917;HANGUL SYLLABLE JUH
C918;HANGUL SYLLABLE JWEO
C919;HANGUL SYLLABLE JWEOG
C91A;HANGUL SYLLABLE JWEOGG
C91B;HANGUL SYLLABLE JWEOGS
C91C;HANGUL SYLLABLE JWEON
C91D;HANGUL SYLLABLE JWEONJ
C91E;HANGUL SYLLABLE JWEONH
C91F;HANGUL SYLLABLE JWEOD
C920;HANGUL SYLLABLE JWEOL
C921;HANGUL SYLLABLE JWEOLG
C922;HANGUL SYLLABLE JWEOLM
C923;HANGUL SYLLABLE JWEOLB
C924;HANGUL SYLLABLE JWEOLS
C925;HANGUL SYLLABLE JWEOLT
C926;HANGUL SYLLABLE JWEOLP
C927;HANGUL SYLLABLE JWEOLH
C928;HANGUL SYLLABLE JWEOM
C929;HANGUL SYLLABLE JWEOB
C92A;HANGUL SYLLABLE JWEOBS
C92B;HANGUL SYLLABLE JWEOS
C92C;HANGUL SYLLABLE JWEOSS
C92D;HANGUL SYLLABLE JWEONG
C92E;HANGUL SYLLABLE JWEOJ
C92F;HANGUL SYLLABLE JWEOC
C930;HANGUL SYLLABLE JWEOK
C931;HANGUL SYLLABLE JWEOT
C932;HANGUL SYLLABLE JWEOP
C933;HANGUL SYLLABLE JWEOH
C934;HANGUL SYLLABLE JWE
C935;HANGUL SYLLABLE JWEG
C936;HANGUL SYLLABLE JWEGG
C937;HANGUL SYLLABLE JWEGS
C938;HANGUL SYLLABLE JWEN
C939;HANGUL SYLLABLE JWENJ
C93A;HANGUL SYLLABLE JWENH
C93B;HANGUL SYLLABLE JWED
C93C;HANGUL SYLLABLE JWEL
C93D;HANGUL SYLLABLE JWELG
C93E;HANGUL SYLLABLE JWELM
C93F;HANGUL SYLLABLE JWELB
C940;HANGUL SYLLABLE JWELS
C941;HANGUL SYLLABLE JWELT
C942;HANGUL SYLLABLE JWELP
C943;HANGUL SYLLABLE JWELH
C944;HANGUL SYLLABLE JWEM
C945;HANGUL SYLLABLE JWEB
C946;HANGUL SYLLABLE JWEBS
C947;HANGUL SYLLABLE JWES
C948;HANGUL SYLLABLE JWESS
C949;HANGUL SYLLABLE JWENG
C94A;HANGUL SYLLABLE JWEJ
C94B;HANGUL SYLLABLE JWEC
C94C;HANGUL SYLLABLE JWEK
C94D;HANGUL SYLLABLE JWET
C94E;HANGUL SYLLABLE JWEP
C94F;HANGUL SYLLABLE JWEH
C950;HANGUL SYLLABLE JWI
C951;HANGUL SYLLABLE JWIG
C952;HANGUL SYLLABLE JWIGG
C953;HANGUL SYLLABLE JWIGS
C954;HANGUL SYLLABLE JWIN
C955;HANGUL SYLLABLE JWINJ
C956;HANGUL SYLLABLE JWINH
C957;HANGUL SYLLABLE JWID
C958;HANGUL SYLLABLE JWIL
C959;HANGUL SYLLABLE JWILG
C95A;HANGUL SYLLABLE JWILM
C95B;HANGUL SYLLABLE JWILB
C95C;HANGUL SYLLABLE JWILS
C95D;HANGUL SYLLABLE JWILT
C95E;HANGUL SYLLABLE JWILP
C95F;HANGUL SYLLABLE JWILH
C960;HANGUL SYLLABLE JWIM
C961;HANGUL SYLLABLE JWIB
C962;HANGUL SYLLABLE JWIBS
C963;HANGUL SYLLABLE JWIS
C964;HANGUL SYLLABLE JWISS
C965;HANGUL SYLLABLE JWING
C966;HANGUL SYLLABLE JWIJ
C967;HANGUL SYLLABLE JWIC
C968;HANGUL SYLLABLE JWIK
C969;HANGUL SYLLABLE JWIT
C96A;HANGUL SYLLABLE JWIP
C96B;HANGUL SYLLABLE JWIH
C96C;HANGUL SYLLABLE JYU
C96D;HANGUL SYLLABLE JYUG
C96E;HANGUL SYLLABLE JYUGG
C96F;HANGUL SYLLABLE JYUGS
C970;HANGUL SYLLABLE JYUN
C971;HANGUL SYLLABLE JYUNJ
C972;HANGUL SYLLABLE JYUNH
C973;HANGUL SYLLABLE JYUD
C974;HANGUL SYLLABLE JYUL
C975;HANGUL SYLLABLE JYULG
C976;HANGUL SYLLABLE JYULM
C977;HANGUL SYLLABLE JYULB
C978;HANGUL SYLLABLE JYULS
C979;HANGUL SYLLABLE JYULT
C97A;HANGUL SYLLABLE JYULP
C97B;HANGUL SYLLABLE JYULH
C97C;HANGUL SYLLABLE JYUM
C97D;HANGUL SYLLABLE JYUB
C97E;HANGUL SYLLABLE JYUBS
C97F;HANGUL SYLLABLE JYUS
C980;HANGUL SYLLABLE JYUSS
C981;HANGUL SYLLABLE JYUNG
C982;HANGUL SYLLABLE JYUJ
C983;HANGUL SYLLABLE JYUC
C984;HANGUL SYLLABLE JYUK
C985;HANGUL SYLLABLE JYUT
C986;HANGUL SYLLABLE JYUP
C987;HANGUL SYLLABLE JYUH
C988;HANGUL SYLLABLE JEU
C989;HANGUL SYLLABLE JEUG
C98A;HANGUL SYLLABLE JEUGG
C98B;HANGUL SYLLABLE JEUGS
C98C;HANGUL SYLLABLE JEUN
C98D;HANGUL SYLLABLE JEUNJ
C98E;HANGUL SYLLABLE JEUNH
C98F;HANGUL SYLLABLE JEUD
C990;HANGUL SYLLABLE JEUL
C991;HANGUL SYLLABLE JEULG
C992;HANGUL SYLLABLE JEULM
C993;HANGUL SYLLABLE JEULB
C994;HANGUL SYLLABLE JEULS
C995;HANGUL SYLLABLE JEULT
C996;HANGUL SYLLABLE JEULP
C997;HANGUL SYLLABLE JEULH
C998;HANGUL SYLLABLE JEUM
C999;HANGUL SYLLABLE JEUB
C99A;HANGUL SYLLABLE JEUBS
C99B;HANGUL SYLLABLE JEUS
C99C;HANGUL SYLLABLE JEUSS
C99D;HANGUL SYLLABLE JEUNG
C99E;HANGUL SYLLABLE JEUJ
C99F;HANGUL SYLLABLE JEUC
C9A0;HANGUL SYLLABLE JEUK
C9A1;HANGUL SYLLABLE JEUT
C9A2;HANGUL SYLLABLE JEUP
C9A3;HANGUL SYLLABLE JEUH
C9A4;HANGUL SYLLABLE JYI
C9A5;HANGUL SYLLABLE JYIG
C9A6;HANGUL SYLLABLE JYIGG
C9A7;HANGUL SYLLABLE JYIGS
C9A8;HANGUL SYLLABLE JYIN
C9A9;HANGUL SYLLABLE JYINJ
C9AA;HANGUL SYLLABLE JYINH
C9AB;HANGUL SYLLABLE JYID
C9AC;HANGUL SYLLABLE JYIL
C9AD;HANGUL SYLLABLE JYILG
C9AE;HANGUL SYLLABLE JYILM
C9AF;HANGUL SYLLABLE JYILB
C9B0;HANGUL SYLLABLE JYILS
C9B1;HANGUL SYLLABLE JYILT
C9B2;HANGUL SYLLABLE JYILP
C9B3;HANGUL SYLLABLE JYILH
C9B4;HANGUL SYLLABLE JYIM
C9B5;HANGUL SYLLABLE JYIB
C9B6;HANGUL SYLLABLE JYIBS
C9B7;HANGUL SYLLABLE JYIS
C9B8;HANGUL SYLLABLE JYISS
C9B9;HANGUL SYLLABLE JYING
C9BA;HANGUL SYLLABLE JYIJ
C9BB;HANGUL SYLLABLE JYIC
C9BC;HANGUL SYLLABLE JYIK
C9BD;HANGUL SYLLABLE JYIT
C9BE;HANGUL SYLLABLE JYIP
C9BF;HANGUL SYLLABLE JYIH
C9C0;HANGUL SYLLABLE JI
C9C1;HANGUL SYLLABLE JIG
C9C2;HANGUL SYLLABLE JIGG
C9C3;HANGUL SYLLABLE JIGS
C9C4;HANGUL SYLLABLE JIN
C9C5;HANGUL SYLLABLE JINJ
C9C6;HANGUL SYLLABLE JINH
C9C7;HANGUL SYLLABLE JID
C9C8;HANGUL SYLLABLE JIL
C9C9;HANGUL SYLLABLE JILG
C9CA;HANGUL SYLLABLE JILM
C9CB;HANGUL SYLLABLE JILB
C9CC;HANGUL SYLLABLE JILS
C9CD;HANGUL SYLLABLE JILT
C9CE;HANGUL SYLLABLE JILP
C9CF;HANGUL SYLLABLE JILH
C9D0;HANGUL SYLLABLE JIM
C9D1;HANGUL SYLLABLE JIB
C9D2;HANGUL SYLLABLE JIBS
C9D3;HANGUL SYLLABLE JIS
C9D4;HANGUL SYLLABLE JISS
C9D5;HANGUL SYLLABLE JING
C9D6;HANGUL SYLLABLE JIJ
C9D7;HANGUL SYLLABLE JIC
C9D8;HANGUL SYLLABLE JIK
C9D9;HANGUL SYLLABLE JIT
C9DA;HANGUL SYLLABLE JIP
C9DB;HANGUL SYLLABLE JIH
C9DC;HANGUL SYLLABLE JJA
C9DD;HANGUL SYLLABLE JJAG
C9DE;HANGUL SYLLABLE JJAGG
C9DF;HANGUL SYLLABLE JJAGS
C9E0;HANGUL SYLLABLE JJAN
C9E1;HANGUL SYLLABLE JJANJ
C9E2;HANGUL SYLLABLE JJANH
C9E3;HANGUL SYLLABLE JJAD
C9E4;HANGUL SYLLABLE JJAL
C9E5;HANGUL SYLLABLE JJALG
C9E6;HANGUL SYLLABLE JJALM
C9E7;HANGUL SYLLABLE JJALB
C9E8;HANGUL SYLLABLE JJALS
C9E9;HANGUL SYLLABLE JJALT
C9EA;HANGUL SYLLABLE JJALP
C9EB;HANGUL SYLLABLE JJALH
C9EC;HANGUL SYLLABLE JJAM
C9ED;HANGUL SYLLABLE JJAB
C9EE;HANGUL SYLLABLE JJABS
C9EF;HANGUL SYLLABLE JJAS
C9F0;HANGUL SYLLABLE JJASS
C9F1;HANGUL SYLLABLE JJANG
C9F2;HANGUL SYLLABLE JJAJ
C9F3;HANGUL SYLLABLE JJAC
C9F4;HANGUL SYLLABLE JJAK
C9F5;HANGUL SYLLABLE JJAT
C9F6;HANGUL SYLLABLE JJAP
C9F7;HANGUL SYLLABLE JJAH
C9F8;HANGUL SYLLABLE JJAE
C9F9;HANGUL SYLLABLE JJAEG
C9FA;HANGUL SYLLABLE JJAEGG
C9FB;HANGUL SYLLABLE JJAEGS
C9FC;HANGUL SYLLABLE JJAEN
C9FD;HANGUL SYLLABLE JJAENJ
C9FE;HANGUL SYLLABLE JJAENH
C9FF;HANGUL SYLLABLE JJAED
CA00;HANGUL SYLLABLE JJAEL
CA01;HANGUL SYLLABLE JJAELG
CA02;HANGUL SYLLABLE JJAELM
CA03;HANGUL SYLLABLE JJAELB
CA04;HANGUL SYLLABLE JJAELS
CA05;HANGUL SYLLABLE JJAELT
CA06;HANGUL SYLLABLE JJAELP
CA07;HANGUL SYLLABLE JJAELH
CA08;HANGUL SYLLABLE JJAEM
CA09;HANGUL SYLLABLE JJAEB
CA0A;HANGUL SYLLABLE JJAEBS
CA0B;HANGUL SYLLABLE JJAES
CA0C;HANGUL SYLLABLE JJAESS
CA0D;HANGUL SYLLABLE JJAENG
CA0E;HANGUL SYLLABLE JJAEJ
CA0F;HANGUL SYLLABLE JJAEC
CA10;HANGUL SYLLABLE JJAEK
CA11;HANGUL SYLLABLE JJAET
CA12;HANGUL SYLLABLE JJAEP
CA13;HANGUL SYLLABLE JJAEH
CA14;HANGUL SYLLABLE JJYA
CA15;HANGUL SYLLABLE JJYAG
CA16;HANGUL SYLLABLE JJYAGG
CA17;HANGUL SYLLABLE JJYAGS
CA18;HANGUL SYLLABLE JJYAN
CA19;HANGUL SYLLABLE JJYANJ
CA1A;HANGUL SYLLABLE JJYANH
CA1B;HANGUL SYLLABLE JJYAD
CA1C;HANGUL SYLLABLE JJYAL
CA1D;HANGUL SYLLABLE JJYALG
CA1E;HANGUL SYLLABLE JJYALM
CA1F;HANGUL SYLLABLE JJYALB
CA20;HANGUL SYLLABLE JJYALS
CA21;HANGUL SYLLABLE JJYALT
CA22;HANGUL SYLLABLE JJYALP
CA23;HANGUL SYLLABLE JJYALH
CA24;HANGUL SYLLABLE JJYAM
CA25;HANGUL SYLLABLE JJYAB
CA26;HANGUL SYLLABLE JJYABS
CA27;HANGUL SYLLABLE JJYAS
CA28;HANGUL SYLLABLE JJYASS
CA29;HANGUL SYLLABLE JJYANG
CA2A;HANGUL SYLLABLE JJYAJ
CA2B;HANGUL SYLLABLE JJYAC
CA2C;HANGUL SYLLABLE JJYAK
CA2D;HANGUL SYLLABLE JJYAT
CA2E;HANGUL SYLLABLE JJYAP
CA2F;HANGUL SYLLABLE JJYAH
CA30;HANGUL SYLLABLE JJYAE
CA31;HANGUL SYLLABLE JJYAEG
CA32;HANGUL SYLLABLE JJYAEGG
CA33;HANGUL SYLLABLE JJYAEGS
CA34;HANGUL SYLLABLE JJYAEN
CA35;HANGUL SYLLABLE JJYAENJ
CA36;HANGUL SYLLABLE JJYAENH
CA37;HANGUL SYLLABLE JJYAED
CA38;HANGUL SYLLABLE JJYAEL
CA39;HANGUL SYLLABLE JJYAELG
CA3A;HANGUL SYLLABLE JJYAELM
CA3B;HANGUL SYLLABLE JJYAELB
CA3C;HANGUL SYLLABLE JJYAELS
CA3D;HANGUL SYLLABLE JJYAELT
CA3E;HANGUL SYLLABLE JJYAELP
CA3F;HANGUL SYLLABLE JJYAELH
CA40;HANGUL SYLLABLE JJYAEM
CA41;HANGUL SYLLABLE JJYAEB
CA42;HANGUL SYLLABLE JJYAEBS
CA43;HANGUL SYLLABLE JJYAES
CA44;HANGUL SYLLABLE JJYAESS
CA45;HANGUL SYLLABLE JJYAENG
CA46;HANGUL SYLLABLE JJYAEJ
CA47;HANGUL SYLLABLE JJYAEC
CA48;HANGUL SYLLABLE JJYAEK
CA49;HANGUL SYLLABLE JJYAET
CA4A;HANGUL SYLLABLE JJYAEP
CA4B;HANGUL SYLLABLE JJYAEH
CA4C;HANGUL SYLLABLE JJEO
CA4D;HANGUL SYLLABLE JJEOG
CA4E;HANGUL SYLLABLE JJEOGG
CA4F;HANGUL SYLLABLE JJEOGS
CA50;HANGUL SYLLABLE JJEON
CA51;HANGUL SYLLABLE JJEONJ
CA52;HANGUL SYLLABLE JJEONH
CA53;HANGUL SYLLABLE JJEOD
CA54;HANGUL SYLLABLE JJEOL
CA55;HANGUL SYLLABLE JJEOLG
CA56;HANGUL SYLLABLE JJEOLM
CA57;HANGUL SYLLABLE JJEOLB
CA58;HANGUL SYLLABLE JJEOLS
CA59;HANGUL SYLLABLE JJEOLT
CA5A;HANGUL SYLLABLE JJEOLP
CA5B;HANGUL SYLLABLE JJEOLH
CA5C;HANGUL SYLLABLE JJEOM
CA5D;HANGUL SYLLABLE JJEOB
CA5E;HANGUL SYLLABLE JJEOBS
CA5F;HANGUL SYLLABLE JJEOS
CA60;HANGUL SYLLABLE JJEOSS
CA61;HANGUL SYLLABLE JJEONG
CA62;HANGUL SYLLABLE JJEOJ
CA63;HANGUL SYLLABLE JJEOC
CA64;HANGUL SYLLABLE JJEOK
CA65;HANGUL SYLLABLE JJEOT
CA66;HANGUL SYLLABLE JJEOP
CA67;HANGUL SYLLABLE JJEOH
CA68;HANGUL SYLLABLE JJE
CA69;HANGUL SYLLABLE JJEG
CA6A;HANGUL SYLLABLE JJEGG
CA6B;HANGUL SYLLABLE JJEGS
CA6C;HANGUL SYLLABLE JJEN
CA6D;HANGUL SYLLABLE JJENJ
CA6E;HANGUL SYLLABLE JJENH
CA6F;HANGUL SYLLABLE JJED
CA70;HANGUL SYLLABLE JJEL
CA71;HANGUL SYLLABLE JJELG
CA72;HANGUL SYLLABLE JJELM
CA73;HANGUL SYLLABLE JJELB
CA74;HANGUL SYLLABLE JJELS
CA75;HANGUL SYLLABLE JJELT
CA76;HANGUL SYLLABLE JJELP
CA77;HANGUL SYLLABLE JJELH
CA78;HANGUL SYLLABLE JJEM
CA79;HANGUL SYLLABLE JJEB
CA7A;HANGUL SYLLABLE JJEBS
CA7B;HANGUL SYLLABLE JJES
CA7C;HANGUL SYLLABLE JJESS
CA7D;HANGUL SYLLABLE JJENG
CA7E;HANGUL SYLLABLE JJEJ
CA7F;HANGUL SYLLABLE JJEC
CA80;HANGUL SYLLABLE JJEK
CA81;HANGUL SYLLABLE JJET
CA82;HANGUL SYLLABLE JJEP
CA83;HANGUL SYLLABLE JJEH
CA84;HANGUL SYLLABLE JJYEO
CA85;HANGUL SYLLABLE JJYEOG
CA86;HANGUL SYLLABLE JJYEOGG
CA87;HANGUL SYLLABLE JJYEOGS
CA88;HANGUL SYLLABLE JJYEON
CA89;HANGUL SYLLABLE JJYEONJ
CA8A;HANGUL SYLLABLE JJYEONH
CA8B;HANGUL SYLLABLE JJYEOD
CA8C;HANGUL SYLLABLE JJYEOL
CA8D;HANGUL SYLLABLE JJYEOLG
CA8E;HANGUL SYLLABLE JJYEOLM
CA8F;HANGUL SYLLABLE JJYEOLB
CA90;HANGUL SYLLABLE JJYEOLS
CA91;HANGUL SYLLABLE JJYEOLT
CA92;HANGUL SYLLABLE JJYEOLP
CA93;HANGUL SYLLABLE JJYEOLH
CA94;HANGUL SYLLABLE JJYEOM
CA95;HANGUL SYLLABLE JJYEOB
CA96;HANGUL SYLLABLE JJYEOBS
CA97;HANGUL SYLLABLE JJYEOS
CA98;HANGUL SYLLABLE JJYEOSS
CA99;HANGUL SYLLABLE JJYEONG
CA9A;HANGUL SYLLABLE JJYEOJ
CA9B;HANGUL SYLLABLE JJYEOC
CA9C;HANGUL SYLLABLE JJYEOK
CA9D;HANGUL SYLLABLE JJYEOT
CA9E;HANGUL SYLLABLE JJYEOP
CA9F;HANGUL SYLLABLE JJYEOH
CAA0;HANGUL SYLLABLE JJYE
CAA1;HANGUL SYLLABLE JJYEG
CAA2;HANGUL SYLLABLE JJYEGG
CAA3;HANGUL SYLLABLE JJYEGS
CAA4;HANGUL SYLLABLE JJYEN
CAA5;HANGUL SYLLABLE JJYENJ
CAA6;HANGUL SYLLABLE JJYENH
CAA7;HANGUL SYLLABLE JJYED
CAA8;HANGUL SYLLABLE JJYEL
CAA9;HANGUL SYLLABLE JJYELG
CAAA;HANGUL SYLLABLE JJYELM
CAAB;HANGUL SYLLABLE JJYELB
CAAC;HANGUL SYLLABLE JJYELS
CAAD;HANGUL SYLLABLE JJYELT
CAAE;HANGUL SYLLABLE JJYELP
CAAF;HANGUL SYLLABLE JJYELH
CAB0;HANGUL SYLLABLE JJYEM
CAB1;HANGUL SYLLABLE JJYEB
CAB2;HANGUL SYLLABLE JJYEBS
CAB3;HANGUL SYLLABLE JJYES
CAB4;HANGUL SYLLABLE JJYESS
CAB5;HANGUL SYLLABLE JJYENG
CAB6;HANGUL SYLLABLE JJYEJ
CAB7;HANGUL SYLLABLE JJYEC
CAB8;HANGUL SYLLABLE JJYEK
CAB9;HANGUL SYLLABLE JJYET
CABA;HANGUL SYLLABLE JJYEP
CABB;HANGUL SYLLABLE JJYEH
CABC;HANGUL SYLLABLE JJO
CABD;HANGUL SYLLABLE JJOG
CABE;HANGUL SYLLABLE JJOGG
CABF;HANGUL SYLLABLE JJOGS
CAC0;HANGUL SYLLABLE JJON
CAC1;HANGUL SYLLABLE JJONJ
CAC2;HANGUL SYLLABLE JJONH
CAC3;HANGUL SYLLABLE JJOD
CAC4;HANGUL SYLLABLE JJOL
CAC5;HANGUL SYLLABLE JJOLG
CAC6;HANGUL SYLLABLE JJOLM
CAC7;HANGUL SYLLABLE JJOLB
CAC8;HANGUL SYLLABLE JJOLS
CAC9;HANGUL SYLLABLE JJOLT
CACA;HANGUL SYLLABLE JJOLP
CACB;HANGUL SYLLABLE JJOLH
CACC;HANGUL SYLLABLE JJOM
CACD;HANGUL SYLLABLE JJOB
CACE;HANGUL SYLLABLE JJOBS
CACF;HANGUL SYLLABLE JJOS
CAD0;HANGUL SYLLABLE JJOSS
CAD1;HANGUL SYLLABLE JJONG
CAD2;HANGUL SYLLABLE JJOJ
CAD3;HANGUL SYLLABLE JJOC
CAD4;HANGUL SYLLABLE JJOK
CAD5;HANGUL SYLLABLE JJOT
CAD6;HANGUL SYLLABLE JJOP
CAD7;HANGUL SYLLABLE JJOH
CAD8;HANGUL SYLLABLE JJWA
CAD9;HANGUL SYLLABLE JJWAG
CADA;HANGUL SYLLABLE JJWAGG
CADB;HANGUL SYLLABLE JJWAGS
CADC;HANGUL SYLLABLE JJWAN
CADD;HANGUL SYLLABLE JJWANJ
CADE;HANGUL SYLLABLE JJWANH
CADF;HANGUL SYLLABLE JJWAD
CAE0;HANGUL SYLLABLE JJWAL
CAE1;HANGUL SYLLABLE JJWALG
CAE2;HANGUL SYLLABLE JJWALM
CAE3;HANGUL SYLLABLE JJWALB
CAE4;HANGUL SYLLABLE JJWALS
CAE5;HANGUL SYLLABLE JJWALT
CAE6;HANGUL SYLLABLE JJWALP
CAE7;HANGUL SYLLABLE JJWALH
CAE8;HANGUL SYLLABLE JJWAM
CAE9;HANGUL SYLLABLE JJWAB
CAEA;HANGUL SYLLABLE JJWABS
CAEB;HANGUL SYLLABLE JJWAS
CAEC;HANGUL SYLLABLE JJWASS
CAED;HANGUL SYLLABLE JJWANG
CAEE;HANGUL SYLLABLE JJWAJ
CAEF;HANGUL SYLLABLE JJWAC
CAF0;HANGUL SYLLABLE JJWAK
CAF1;HANGUL SYLLABLE JJWAT
CAF2;HANGUL SYLLABLE JJWAP
CAF3;HANGUL SYLLABLE JJWAH
CAF4;HANGUL SYLLABLE JJWAE
CAF5;HANGUL SYLLABLE JJWAEG
CAF6;HANGUL SYLLABLE JJWAEGG
CAF7;HANGUL SYLLABLE JJWAEGS
CAF8;HANGUL SYLLABLE JJWAEN
CAF9;HANGUL SYLLABLE JJWAENJ
CAFA;HANGUL SYLLABLE JJWAENH
CAFB;HANGUL SYLLABLE JJWAED
CAFC;HANGUL SYLLABLE JJWAEL
CAFD;HANGUL SYLLABLE JJWAELG
CAFE;HANGUL SYLLABLE JJWAELM
CAFF;HANGUL SYLLABLE JJWAELB
CB00;HANGUL SYLLABLE JJWAELS
CB01;HANGUL SYLLABLE JJWAELT
CB02;HANGUL SYLLABLE JJWAELP
CB03;HANGUL SYLLABLE JJWAELH
CB04;HANGUL SYLLABLE JJWAEM
CB05;HANGUL SYLLABLE JJWAEB
CB06;HANGUL SYLLABLE JJWAEBS
CB07;HANGUL SYLLABLE JJWAES
CB08;HANGUL SYLLABLE JJWAESS
CB09;HANGUL SYLLABLE JJWAENG
CB0A;HANGUL SYLLABLE JJWAEJ
CB0B;HANGUL SYLLABLE JJWAEC
CB0C;HANGUL SYLLABLE JJWAEK
CB0D;HANGUL SYLLABLE JJWAET
CB0E;HANGUL SYLLABLE JJWAEP
CB0F;HANGUL SYLLABLE JJWAEH
CB10;HANGUL SYLLABLE JJOE
CB11;HANGUL SYLLABLE JJOEG
CB12;HANGUL SYLLABLE JJOEGG
CB13;HANGUL SYLLABLE JJOEGS
CB14;HANGUL SYLLABLE JJOEN
CB15;HANGUL SYLLABLE JJOENJ
CB16;HANGUL SYLLABLE JJOENH
CB17;HANGUL SYLLABLE JJOED
CB18;HANGUL SYLLABLE JJOEL
CB19;HANGUL SYLLABLE JJOELG
CB1A;HANGUL SYLLABLE JJOELM
CB1B;HANGUL SYLLABLE JJOELB
CB1C;HANGUL SYLLABLE JJOELS
CB1D;HANGUL SYLLABLE JJOELT
CB1E;HANGUL SYLLABLE JJOELP
CB1F;HANGUL SYLLABLE JJOELH
CB20;HANGUL SYLLABLE JJOEM
CB21;HANGUL SYLLABLE JJOEB
CB22;HANGUL SYLLABLE JJOEBS
CB23;HANGUL SYLLABLE JJOES
CB24;HANGUL SYLLABLE JJOESS
CB25;HANGUL SYLLABLE JJOENG
CB26;HANGUL SYLLABLE JJOEJ
CB27;HANGUL SYLLABLE JJOEC
CB28;HANGUL SYLLABLE JJOEK
CB29;HANGUL SYLLABLE JJOET
CB2A;HANGUL SYLLABLE JJOEP
CB2B;HANGUL SYLLABLE JJOEH
CB2C;HANGUL SYLLABLE JJYO
CB2D;HANGUL SYLLABLE JJYOG
CB2E;HANGUL SYLLABLE JJYOGG
CB2F;HANGUL SYLLABLE JJYOGS
CB30;HANGUL SYLLABLE JJYON
CB31;HANGUL SYLLABLE JJYONJ
CB32;HANGUL SYLLABLE JJYONH
CB33;HANGUL SYLLABLE JJYOD
CB34;HANGUL SYLLABLE JJYOL
CB35;HANGUL SYLLABLE JJYOLG
CB36;HANGUL SYLLABLE JJYOLM
CB37;HANGUL SYLLABLE JJYOLB
CB38;HANGUL SYLLABLE JJYOLS
CB39;HANGUL SYLLABLE JJYOLT
CB3A;HANGUL SYLLABLE JJYOLP
CB3B;HANGUL SYLLABLE JJYOLH
CB3C;HANGUL SYLLABLE JJYOM
CB3D;HANGUL SYLLABLE JJYOB
CB3E;HANGUL SYLLABLE JJYOBS
CB3F;HANGUL SYLLABLE JJYOS
CB40;HANGUL SYLLABLE JJYOSS
CB41;HANGUL SYLLABLE JJYONG
CB42;HANGUL SYLLABLE JJYOJ
CB43;HANGUL SYLLABLE JJYOC
CB44;HANGUL SYLLABLE JJYOK
CB45;HANGUL SYLLABLE JJYOT
CB46;HANGUL SYLLABLE JJYOP
CB47;HANGUL SYLLABLE JJYOH
CB48;HANGUL SYLLABLE JJU
CB49;HANGUL SYLLABLE JJUG
CB4A;HANGUL SYLLABLE JJUGG
CB4B;HANGUL SYLLABLE JJUGS
CB4C;HANGUL SYLLABLE JJUN
CB4D;HANGUL SYLLABLE JJUNJ
CB4E;HANGUL SYLLABLE JJUNH
CB4F;HANGUL SYLLABLE JJUD
CB50;HANGUL SYLLABLE JJUL
CB51;HANGUL SYLLABLE JJULG
CB52;HANGUL SYLLABLE JJULM
CB53;HANGUL SYLLABLE JJULB
CB54;HANGUL SYLLABLE JJULS
CB55;HANGUL SYLLABLE JJULT
CB56;HANGUL SYLLABLE JJULP
CB57;HANGUL SYLLABLE JJULH
CB58;HANGUL SYLLABLE JJUM
CB59;HANGUL SYLLABLE JJUB
CB5A;HANGUL SYLLABLE JJUBS
CB5B;HANGUL SYLLABLE JJUS
CB5C;HANGUL SYLLABLE JJUSS
CB5D;HANGUL SYLLABLE JJUNG
CB5E;HANGUL SYLLABLE JJUJ
CB5F;HANGUL SYLLABLE JJUC
CB60;HANGUL SYLLABLE JJUK
CB61;HANGUL SYLLABLE JJUT
CB62;HANGUL SYLLABLE JJUP
CB63;HANGUL SYLLABLE JJUH
CB64;HANGUL SYLLABLE JJWEO
CB65;HANGUL SYLLABLE JJWEOG
CB66;HANGUL SYLLABLE JJWEOGG
CB67;HANGUL SYLLABLE JJWEOGS
CB68;HANGUL SYLLABLE JJWEON
CB69;HANGUL SYLLABLE JJWEONJ
CB6A;HANGUL SYLLABLE JJWEONH
CB6B;HANGUL SYLLABLE JJWEOD
CB6C;HANGUL SYLLABLE JJWEOL
CB6D;HANGUL SYLLABLE JJWEOLG
CB6E;HANGUL SYLLABLE JJWEOLM
CB6F;HANGUL SYLLABLE JJWEOLB
CB70;HANGUL SYLLABLE JJWEOLS
CB71;HANGUL SYLLABLE JJWEOLT
CB72;HANGUL SYLLABLE JJWEOLP
CB73;HANGUL SYLLABLE JJWEOLH
CB74;HANGUL SYLLABLE JJWEOM
CB75;HANGUL SYLLABLE JJWEOB
CB76;HANGUL SYLLABLE JJWEOBS
CB77;HANGUL SYLLABLE JJWEOS
CB78;HANGUL SYLLABLE JJWEOSS
CB79;HANGUL SYLLABLE JJWEONG
CB7A;HANGUL SYLLABLE JJWEOJ
CB7B;HANGUL SYLLABLE JJWEOC
CB7C;HANGUL SYLLABLE JJWEOK
CB7D;HANGUL SYLLABLE JJWEOT
CB7E;HANGUL SYLLABLE JJWEOP
CB7F;HANGUL SYLLABLE JJWEOH
CB80;HANGUL SYLLABLE JJWE
CB81;HANGUL SYLLABLE JJWEG
CB82;HANGUL SYLLABLE JJWEGG
CB83;HANGUL SYLLABLE JJWEGS
CB84;HANGUL SYLLABLE JJWEN
CB85;HANGUL SYLLABLE JJWENJ
CB86;HANGUL SYLLABLE JJWENH
CB87;HANGUL SYLLABLE JJWED
CB88;HANGUL SYLLABLE JJWEL
CB89;HANGUL SYLLABLE JJWELG
CB8A;HANGUL SYLLABLE JJWELM
CB8B;HANGUL SYLLABLE JJWELB
CB8C;HANGUL SYLLABLE JJWELS
CB8D;HANGUL SYLLABLE JJWELT
CB8E;HANGUL SYLLABLE JJWELP
CB8F;HANGUL SYLLABLE JJWELH
CB90;HANGUL SYLLABLE JJWEM
CB91;HANGUL SYLLABLE JJWEB
CB92;HANGUL SYLLABLE JJWEBS
CB93;HANGUL SYLLABLE JJWES
CB94;HANGUL SYLLABLE JJWESS
CB95;HANGUL SYLLABLE JJWENG
CB96;HANGUL SYLLABLE JJWEJ
CB97;HANGUL SYLLABLE JJWEC
CB98;HANGUL SYLLABLE JJWEK
CB99;HANGUL SYLLABLE JJWET
CB9A;HANGUL SYLLABLE JJWEP
CB9B;HANGUL SYLLABLE JJWEH
CB9C;HANGUL SYLLABLE JJWI
CB9D;HANGUL SYLLABLE JJWIG
CB9E;HANGUL SYLLABLE JJWIGG
CB9F;HANGUL SYLLABLE JJWIGS
CBA0;HANGUL SYLLABLE JJWIN
CBA1;HANGUL SYLLABLE JJWINJ
CBA2;HANGUL SYLLABLE JJWINH
CBA3;HANGUL SYLLABLE JJWID
CBA4;HANGUL SYLLABLE JJWIL
CBA5;HANGUL SYLLABLE JJWILG
CBA6;HANGUL SYLLABLE JJWILM
CBA7;HANGUL SYLLABLE JJWILB
CBA8;HANGUL SYLLABLE JJWILS
CBA9;HANGUL SYLLABLE JJWILT
CBAA;HANGUL SYLLABLE JJWILP
CBAB;HANGUL SYLLABLE JJWILH
CBAC;HANGUL SYLLABLE JJWIM
CBAD;HANGUL SYLLABLE JJWIB
CBAE;HANGUL SYLLABLE JJWIBS
CBAF;HANGUL SYLLABLE JJWIS
CBB0;HANGUL SYLLABLE JJWISS
CBB1;HANGUL SYLLABLE JJWING
CBB2;HANGUL SYLLABLE JJWIJ
CBB3;HANGUL SYLLABLE JJWIC
CBB4;HANGUL SYLLABLE JJWIK
CBB5;HANGUL SYLLABLE JJWIT
CBB6;HANGUL SYLLABLE JJWIP
CBB7;HANGUL SYLLABLE JJWIH
CBB8;HANGUL SYLLABLE JJYU
CBB9;HANGUL SYLLABLE JJYUG
CBBA;HANGUL SYLLABLE JJYUGG
CBBB;HANGUL SYLLABLE JJYUGS
CBBC;HANGUL SYLLABLE JJYUN
CBBD;HANGUL SYLLABLE JJYUNJ
CBBE;HANGUL SYLLABLE JJYUNH
CBBF;HANGUL SYLLABLE JJYUD
CBC0;HANGUL SYLLABLE JJYUL
CBC1;HANGUL SYLLABLE JJYULG
CBC2;HANGUL SYLLABLE JJYULM
CBC3;HANGUL SYLLABLE JJYULB
CBC4;HANGUL SYLLABLE JJYULS
CBC5;HANGUL SYLLABLE JJYULT
CBC6;HANGUL SYLLABLE JJYULP
CBC7;HANGUL SYLLABLE JJYULH
CBC8;HANGUL SYLLABLE JJYUM
CBC9;HANGUL SYLLABLE JJYUB
CBCA;HANGUL SYLLABLE JJYUBS
CBCB;HANGUL SYLLABLE JJYUS
CBCC;HANGUL SYLLABLE JJYUSS
CBCD;HANGUL SYLLABLE JJYUNG
CBCE;HANGUL SYLLABLE JJYUJ
CBCF;HANGUL SYLLABLE JJYUC
CBD0;HANGUL SYLLABLE JJYUK
CBD1;HANGUL SYLLABLE JJYUT
CBD2;HANGUL SYLLABLE JJYUP
CBD3;HANGUL SYLLABLE JJYUH
CBD4;HANGUL SYLLABLE JJEU
CBD5;HANGUL SYLLABLE JJEUG
CBD6;HANGUL SYLLABLE JJEUGG
CBD7;HANGUL SYLLABLE JJEUGS
CBD8;HANGUL SYLLABLE JJEUN
CBD9;HANGUL SYLLABLE JJEUNJ
CBDA;HANGUL SYLLABLE JJEUNH
CBDB;HANGUL SYLLABLE JJEUD
CBDC;HANGUL SYLLABLE JJEUL
CBDD;HANGUL SYLLABLE JJEULG
CBDE;HANGUL SYLLABLE JJEULM
CBDF;HANGUL SYLLABLE JJEULB
CBE0;HANGUL SYLLABLE JJEULS
CBE1;HANGUL SYLLABLE JJEULT
CBE2;HANGUL SYLLABLE JJEULP
CBE3;HANGUL SYLLABLE JJEULH
CBE4;HANGUL SYLLABLE JJEUM
CBE5;HANGUL SYLLABLE JJEUB
CBE6;HANGUL SYLLABLE JJEUBS
CBE7;HANGUL SYLLABLE JJEUS
CBE8;HANGUL SYLLABLE JJEUSS
CBE9;HANGUL SYLLABLE JJEUNG
CBEA;HANGUL SYLLABLE JJEUJ
CBEB;HANGUL SYLLABLE JJEUC
CBEC;HANGUL SYLLABLE JJEUK
CBED;HANGUL SYLLABLE JJEUT
CBEE;HANGUL SYLLABLE JJEUP
CBEF;HANGUL SYLLABLE JJEUH
CBF0;HANGUL SYLLABLE JJYI
CBF1;HANGUL SYLLABLE JJYIG
CBF2;HANGUL SYLLABLE JJYIGG
CBF3;HANGUL SYLLABLE JJYIGS
CBF4;HANGUL SYLLABLE JJYIN
CBF5;HANGUL SYLLABLE JJYINJ
CBF6;HANGUL SYLLABLE JJYINH
CBF7;HANGUL SYLLABLE JJYID
CBF8;HANGUL SYLLABLE JJYIL
CBF9;HANGUL SYLLABLE JJYILG
CBFA;HANGUL SYLLABLE JJYILM
CBFB;HANGUL SYLLABLE JJYILB
CBFC;HANGUL SYLLABLE JJYILS
CBFD;HANGUL SYLLABLE JJYILT
CBFE;HANGUL SYLLABLE JJYILP
CBFF;HANGUL SYLLABLE JJYILH
CC00;HANGUL SYLLABLE JJYIM
CC01;HANGUL SYLLABLE JJYIB
CC02;HANGUL SYLLABLE JJYIBS
CC03;HANGUL SYLLABLE JJYIS
CC04;HANGUL SYLLABLE JJYISS
CC05;HANGUL SYLLABLE JJYING
CC06;HANGUL SYLLABLE JJYIJ
CC07;HANGUL SYLLABLE JJYIC
CC08;HANGUL SYLLABLE JJYIK
CC09;HANGUL SYLLABLE JJYIT
CC0A;HANGUL SYLLABLE JJYIP
CC0B;HANGUL SYLLABLE JJYIH
CC0C;HANGUL SYLLABLE JJI
CC0D;HANGUL SYLLABLE JJIG
CC0E;HANGUL SYLLABLE JJIGG
CC0F;HANGUL SYLLABLE JJIGS
CC10;HANGUL SYLLABLE JJIN
CC11;HANGUL SYLLABLE JJINJ
CC12;HANGUL SYLLABLE JJINH
CC13;HANGUL SYLLABLE JJID
CC14;HANGUL SYLLABLE JJIL
CC15;HANGUL SYLLABLE JJILG
CC16;HANGUL SYLLABLE JJILM
CC17;HANGUL SYLLABLE JJILB
CC18;HANGUL SYLLABLE JJILS
CC19;HANGUL SYLLABLE JJILT
CC1A;HANGUL SYLLABLE JJILP
CC1B;HANGUL SYLLABLE JJILH
CC1C;HANGUL SYLLABLE JJIM
CC1D;HANGUL SYLLABLE JJIB
CC1E;HANGUL SYLLABLE JJIBS
CC1F;HANGUL SYLLABLE JJIS
CC20;HANGUL SYLLABLE JJISS
CC21;HANGUL SYLLABLE JJING
CC22;HANGUL SYLLABLE JJIJ
CC23;HANGUL SYLLABLE JJIC
CC24;HANGUL SYLLABLE JJIK
CC25;HANGUL SYLLABLE JJIT
CC26;HANGUL SYLLABLE JJIP
CC27;HANGUL SYLLABLE JJIH
CC28;HANGUL SYLLABLE CA
CC29;HANGUL SYLLABLE CAG
CC2A;HANGUL SYLLABLE CAGG
CC2B;HANGUL SYLLABLE CAGS
CC2C;HANGUL SYLLABLE CAN
CC2D;HANGUL SYLLABLE CANJ
CC2E;HANGUL SYLLABLE CANH
CC2F;HANGUL SYLLABLE CAD
CC30;HANGUL SYLLABLE CAL
CC31;HANGUL SYLLABLE CALG
CC32;HANGUL SYLLABLE CALM
CC33;HANGUL SYLLABLE CALB
CC34;HANGUL SYLLABLE CALS
CC35;HANGUL SYLLABLE CALT
CC36;HANGUL SYLLABLE CALP
CC37;HANGUL SYLLABLE CALH
CC38;HANGUL SYLLABLE CAM
CC39;HANGUL SYLLABLE CAB
CC3A;HANGUL SYLLABLE CABS
CC3B;HANGUL SYLLABLE CAS
CC3C;HANGUL SYLLABLE CASS
CC3D;HANGUL SYLLABLE CANG
CC3E;HANGUL SYLLABLE CAJ
CC3F;HANGUL SYLLABLE CAC
CC40;HANGUL SYLLABLE CAK
CC41;HANGUL SYLLABLE CAT
CC42;HANGUL SYLLABLE CAP
CC43;HANGUL SYLLABLE CAH
CC44;HANGUL SYLLABLE CAE
CC45;HANGUL SYLLABLE CAEG
CC46;HANGUL SYLLABLE CAEGG
CC47;HANGUL SYLLABLE CAEGS
CC48;HANGUL SYLLABLE CAEN
CC49;HANGUL SYLLABLE CAENJ
CC4A;HANGUL SYLLABLE CAENH
CC4B;HANGUL SYLLABLE CAED
CC4C;HANGUL SYLLABLE CAEL
CC4D;HANGUL SYLLABLE CAELG
CC4E;HANGUL SYLLABLE CAELM
CC4F;HANGUL SYLLABLE CAELB
CC50;HANGUL SYLLABLE CAELS
CC51;HANGUL SYLLABLE CAELT
CC52;HANGUL SYLLABLE CAELP
CC53;HANGUL SYLLABLE CAELH
CC54;HANGUL SYLLABLE CAEM
CC55;HANGUL SYLLABLE CAEB
CC56;HANGUL SYLLABLE CAEBS
CC57;HANGUL SYLLABLE CAES
CC58;HANGUL SYLLABLE CAESS
CC59;HANGUL SYLLABLE CAENG
CC5A;HANGUL SYLLABLE CAEJ
CC5B;HANGUL SYLLABLE CAEC
CC5C;HANGUL SYLLABLE CAEK
CC5D;HANGUL SYLLABLE CAET
CC5E;HANGUL SYLLABLE CAEP
CC5F;HANGUL SYLLABLE CAEH
CC60;HANGUL SYLLABLE CYA
CC61;HANGUL SYLLABLE CYAG
CC62;HANGUL SYLLABLE CYAGG
CC63;HANGUL SYLLABLE CYAGS
CC64;HANGUL SYLLABLE CYAN
CC65;HANGUL SYLLABLE CYANJ
CC66;HANGUL SYLLABLE CYANH
CC67;HANGUL SYLLABLE CYAD
CC68;HANGUL SYLLABLE CYAL
CC69;HANGUL SYLLABLE CYALG
CC6A;HANGUL SYLLABLE CYALM
CC6B;HANGUL SYLLABLE CYALB
CC6C;HANGUL SYLLABLE CYALS
CC6D;HANGUL SYLLABLE CYALT
CC6E;HANGUL SYLLABLE CYALP
CC6F;HANGUL SYLLABLE CYALH
CC70;HANGUL SYLLABLE CYAM
CC71;HANGUL SYLLABLE CYAB
CC72;HANGUL SYLLABLE CYABS
CC73;HANGUL SYLLABLE CYAS
CC74;HANGUL SYLLABLE CYASS
CC75;HANGUL SYLLABLE CYANG
CC76;HANGUL SYLLABLE CYAJ
CC77;HANGUL SYLLABLE CYAC
CC78;HANGUL SYLLABLE CYAK
CC79;HANGUL SYLLABLE CYAT
CC7A;HANGUL SYLLABLE CYAP
CC7B;HANGUL SYLLABLE CYAH
CC7C;HANGUL SYLLABLE CYAE
CC7D;HANGUL SYLLABLE CYAEG
CC7E;HANGUL SYLLABLE CYAEGG
CC7F;HANGUL SYLLABLE CYAEGS
CC80;HANGUL SYLLABLE CYAEN
CC81;HANGUL SYLLABLE CYAENJ
CC82;HANGUL SYLLABLE CYAENH
CC83;HANGUL SYLLABLE CYAED
CC84;HANGUL SYLLABLE CYAEL
CC85;HANGUL SYLLABLE CYAELG
CC86;HANGUL SYLLABLE CYAELM
CC87;HANGUL SYLLABLE CYAELB
CC88;HANGUL SYLLABLE CYAELS
CC89;HANGUL SYLLABLE CYAELT
CC8A;HANGUL SYLLABLE CYAELP
CC8B;HANGUL SYLLABLE CYAELH
CC8C;HANGUL SYLLABLE CYAEM
CC8D;HANGUL SYLLABLE CYAEB
CC8E;HANGUL SYLLABLE CYAEBS
CC8F;HANGUL SYLLABLE CYAES
CC90;HANGUL SYLLABLE CYAESS
CC91;HANGUL SYLLABLE CYAENG
CC92;HANGUL SYLLABLE CYAEJ
CC93;HANGUL SYLLABLE CYAEC
CC94;HANGUL SYLLABLE CYAEK
CC95;HANGUL SYLLABLE CYAET
CC96;HANGUL SYLLABLE CYAEP
CC97;HANGUL SYLLABLE CYAEH
CC98;HANGUL SYLLABLE CEO
CC99;HANGUL SYLLABLE CEOG
CC9A;HANGUL SYLLABLE CEOGG
CC9B;HANGUL SYLLABLE CEOGS
CC9C;HANGUL SYLLABLE CEON
CC9D;HANGUL SYLLABLE CEONJ
CC9E;HANGUL SYLLABLE CEONH
CC9F;HANGUL SYLLABLE CEOD
CCA0;HANGUL SYLLABLE CEOL
CCA1;HANGUL SYLLABLE CEOLG
CCA2;HANGUL SYLLABLE CEOLM
CCA3;HANGUL SYLLABLE CEOLB
CCA4;HANGUL SYLLABLE CEOLS
CCA5;HANGUL SYLLABLE CEOLT
CCA6;HANGUL SYLLABLE CEOLP
CCA7;HANGUL SYLLABLE CEOLH
CCA8;HANGUL SYLLABLE CEOM
CCA9;HANGUL SYLLABLE CEOB
CCAA;HANGUL SYLLABLE CEOBS
CCAB;HANGUL SYLLABLE CEOS
CCAC;HANGUL SYLLABLE CEOSS
CCAD;HANGUL SYLLABLE CEONG
CCAE;HANGUL SYLLABLE CEOJ
CCAF;HANGUL SYLLABLE CEOC
CCB0;HANGUL SYLLABLE CEOK
CCB1;HANGUL SYLLABLE CEOT
CCB2;HANGUL SYLLABLE CEOP
CCB3;HANGUL SYLLABLE CEOH
CCB4;HANGUL SYLLABLE CE
CCB5;HANGUL SYLLABLE CEG
CCB6;HANGUL SYLLABLE CEGG
CCB7;HANGUL SYLLABLE CEGS
CCB8;HANGUL SYLLABLE CEN
CCB9;HANGUL SYLLABLE CENJ
CCBA;HANGUL SYLLABLE CENH
CCBB;HANGUL SYLLABLE CED
CCBC;HANGUL SYLLABLE CEL
CCBD;HANGUL SYLLABLE CELG
CCBE;HANGUL SYLLABLE CELM
CCBF;HANGUL SYLLABLE CELB
CCC0;HANGUL SYLLABLE CELS
CCC1;HANGUL SYLLABLE CELT
CCC2;HANGUL SYLLABLE CELP
CCC3;HANGUL SYLLABLE CELH
CCC4;HANGUL SYLLABLE CEM
CCC5;HANGUL SYLLABLE CEB
CCC6;HANGUL SYLLABLE CEBS
CCC7;HANGUL SYLLABLE CES
CCC8;HANGUL SYLLABLE CESS
CCC9;HANGUL SYLLABLE CENG
CCCA;HANGUL SYLLABLE CEJ
CCCB;HANGUL SYLLABLE CEC
CCCC;HANGUL SYLLABLE CEK
CCCD;HANGUL SYLLABLE CET
CCCE;HANGUL SYLLABLE CEP
CCCF;HANGUL SYLLABLE CEH
CCD0;HANGUL SYLLABLE CYEO
CCD1;HANGUL SYLLABLE CYEOG
CCD2;HANGUL SYLLABLE CYEOGG
CCD3;HANGUL SYLLABLE CYEOGS
CCD4;HANGUL SYLLABLE CYEON
CCD5;HANGUL SYLLABLE CYEONJ
CCD6;HANGUL SYLLABLE CYEONH
CCD7;HANGUL SYLLABLE CYEOD
CCD8;HANGUL SYLLABLE CYEOL
CCD9;HANGUL SYLLABLE CYEOLG
CCDA;HANGUL SYLLABLE CYEOLM
CCDB;HANGUL SYLLABLE CYEOLB
CCDC;HANGUL SYLLABLE CYEOLS
CCDD;HANGUL SYLLABLE CYEOLT
CCDE;HANGUL SYLLABLE CYEOLP
CCDF;HANGUL SYLLABLE CYEOLH
CCE0;HANGUL SYLLABLE CYEOM
CCE1;HANGUL SYLLABLE CYEOB
CCE2;HANGUL SYLLABLE CYEOBS
CCE3;HANGUL SYLLABLE CYEOS
CCE4;HANGUL SYLLABLE CYEOSS
CCE5;HANGUL SYLLABLE CYEONG
CCE6;HANGUL SYLLABLE CYEOJ
CCE7;HANGUL SYLLABLE CYEOC
CCE8;HANGUL SYLLABLE CYEOK
CCE9;HANGUL SYLLABLE CYEOT
CCEA;HANGUL SYLLABLE CYEOP
CCEB;HANGUL SYLLABLE CYEOH
CCEC;HANGUL SYLLABLE CYE
CCED;HANGUL SYLLABLE CYEG
CCEE;HANGUL SYLLABLE CYEGG
CCEF;HANGUL SYLLABLE CYEGS
CCF0;HANGUL SYLLABLE CYEN
CCF1;HANGUL SYLLABLE CYENJ
CCF2;HANGUL SYLLABLE CYENH
CCF3;HANGUL SYLLABLE CYED
CCF4;HANGUL SYLLABLE CYEL
CCF5;HANGUL SYLLABLE CYELG
CCF6;HANGUL SYLLABLE CYELM
CCF7;HANGUL SYLLABLE CYELB
CCF8;HANGUL SYLLABLE CYELS
CCF9;HANGUL SYLLABLE CYELT
CCFA;HANGUL SYLLABLE CYELP
CCFB;HANGUL SYLLABLE CYELH
CCFC;HANGUL SYLLABLE CYEM
CCFD;HANGUL SYLLABLE CYEB
CCFE;HANGUL SYLLABLE CYEBS
CCFF;HANGUL SYLLABLE CYES
CD00;HANGUL SYLLABLE CYESS
CD01;HANGUL SYLLABLE CYENG
CD02;HANGUL SYLLABLE CYEJ
CD03;HANGUL SYLLABLE CYEC
CD04;HANGUL SYLLABLE CYEK
CD05;HANGUL SYLLABLE CYET
CD06;HANGUL SYLLABLE CYEP
CD07;HANGUL SYLLABLE CYEH
CD08;HANGUL SYLLABLE CO
CD09;HANGUL SYLLABLE COG
CD0A;HANGUL SYLLABLE COGG
CD0B;HANGUL SYLLABLE COGS
CD0C;HANGUL SYLLABLE CON
CD0D;HANGUL SYLLABLE CONJ
CD0E;HANGUL SYLLABLE CONH
CD0F;HANGUL SYLLABLE COD
CD10;HANGUL SYLLABLE COL
CD11;HANGUL SYLLABLE COLG
CD12;HANGUL SYLLABLE COLM
CD13;HANGUL SYLLABLE COLB
CD14;HANGUL SYLLABLE COLS
CD15;HANGUL SYLLABLE COLT
CD16;HANGUL SYLLABLE COLP
CD17;HANGUL SYLLABLE COLH
CD18;HANGUL SYLLABLE COM
CD19;HANGUL SYLLABLE COB
CD1A;HANGUL SYLLABLE COBS
CD1B;HANGUL SYLLABLE COS
CD1C;HANGUL SYLLABLE COSS
CD1D;HANGUL SYLLABLE CONG
CD1E;HANGUL SYLLABLE COJ
CD1F;HANGUL SYLLABLE COC
CD20;HANGUL SYLLABLE COK
CD21;HANGUL SYLLABLE COT
CD22;HANGUL SYLLABLE COP
CD23;HANGUL SYLLABLE COH
CD24;HANGUL SYLLABLE CWA
CD25;HANGUL SYLLABLE CWAG
CD26;HANGUL SYLLABLE CWAGG
CD27;HANGUL SYLLABLE CWAGS
CD28;HANGUL SYLLABLE CWAN
CD29;HANGUL SYLLABLE CWANJ
CD2A;HANGUL SYLLABLE CWANH
CD2B;HANGUL SYLLABLE CWAD
CD2C;HANGUL SYLLABLE CWAL
CD2D;HANGUL SYLLABLE CWALG
CD2E;HANGUL SYLLABLE CWALM
CD2F;HANGUL SYLLABLE CWALB
CD30;HANGUL SYLLABLE CWALS
CD31;HANGUL SYLLABLE CWALT
CD32;HANGUL SYLLABLE CWALP
CD33;HANGUL SYLLABLE CWALH
CD34;HANGUL SYLLABLE CWAM
CD35;HANGUL SYLLABLE CWAB
CD36;HANGUL SYLLABLE CWABS
CD37;HANGUL SYLLABLE CWAS
CD38;HANGUL SYLLABLE CWASS
CD39;HANGUL SYLLABLE CWANG
CD3A;HANGUL SYLLABLE CWAJ
CD3B;HANGUL SYLLABLE CWAC
CD3C;HANGUL SYLLABLE CWAK
CD3D;HANGUL SYLLABLE CWAT
CD3E;HANGUL SYLLABLE CWAP
CD3F;HANGUL SYLLABLE CWAH
CD40;HANGUL SYLLABLE CWAE
CD41;HANGUL SYLLABLE CWAEG
CD42;HANGUL SYLLABLE CWAEGG
CD43;HANGUL SYLLABLE CWAEGS
CD44;HANGUL SYLLABLE CWAEN
CD45;HANGUL SYLLABLE CWAENJ
CD46;HANGUL SYLLABLE CWAENH
CD47;HANGUL SYLLABLE CWAED
CD48;HANGUL SYLLABLE CWAEL
CD49;HANGUL SYLLABLE CWAELG
CD4A;HANGUL SYLLABLE CWAELM
CD4B;HANGUL SYLLABLE CWAELB
CD4C;HANGUL SYLLABLE CWAELS
CD4D;HANGUL SYLLABLE CWAELT
CD4E;HANGUL SYLLABLE CWAELP
CD4F;HANGUL SYLLABLE CWAELH
CD50;HANGUL SYLLABLE CWAEM
CD51;HANGUL SYLLABLE CWAEB
CD52;HANGUL SYLLABLE CWAEBS
CD53;HANGUL SYLLABLE CWAES
CD54;HANGUL SYLLABLE CWAESS
CD55;HANGUL SYLLABLE CWAENG
CD56;HANGUL SYLLABLE CWAEJ
CD57;HANGUL SYLLABLE CWAEC
CD58;HANGUL SYLLABLE CWAEK
CD59;HANGUL SYLLABLE CWAET
CD5A;HANGUL SYLLABLE CWAEP
CD5B;HANGUL SYLLABLE CWAEH
CD5C;HANGUL SYLLABLE COE
CD5D;HANGUL SYLLABLE COEG
CD5E;HANGUL SYLLABLE COEGG
CD5F;HANGUL SYLLABLE COEGS
CD60;HANGUL SYLLABLE COEN
CD61;HANGUL SYLLABLE COENJ
CD62;HANGUL SYLLABLE COENH
CD63;HANGUL SYLLABLE COED
CD64;HANGUL SYLLABLE COEL
CD65;HANGUL SYLLABLE COELG
CD66;HANGUL SYLLABLE COELM
CD67;HANGUL SYLLABLE COELB
CD68;HANGUL SYLLABLE COELS
CD69;HANGUL SYLLABLE COELT
CD6A;HANGUL SYLLABLE COELP
CD6B;HANGUL SYLLABLE COELH
CD6C;HANGUL SYLLABLE COEM
CD6D;HANGUL SYLLABLE COEB
CD6E;HANGUL SYLLABLE COEBS
CD6F;HANGUL SYLLABLE COES
CD70;HANGUL SYLLABLE COESS
CD71;HANGUL SYLLABLE COENG
CD72;HANGUL SYLLABLE COEJ
CD73;HANGUL SYLLABLE COEC
CD74;HANGUL SYLLABLE COEK
CD75;HANGUL SYLLABLE COET
CD76;HANGUL SYLLABLE COEP
CD77;HANGUL SYLLABLE COEH
CD78;HANGUL SYLLABLE CYO
CD79;HANGUL SYLLABLE CYOG
CD7A;HANGUL SYLLABLE CYOGG
CD7B;HANGUL SYLLABLE CYOGS
CD7C;HANGUL SYLLABLE CYON
CD7D;HANGUL SYLLABLE CYONJ
CD7E;HANGUL SYLLABLE CYONH
CD7F;HANGUL SYLLABLE CYOD
CD80;HANGUL SYLLABLE CYOL
CD81;HANGUL SYLLABLE CYOLG
CD82;HANGUL SYLLABLE CYOLM
CD83;HANGUL SYLLABLE CYOLB
CD84;HANGUL SYLLABLE CYOLS
CD85;HANGUL SYLLABLE CYOLT
CD86;HANGUL SYLLABLE CYOLP
CD87;HANGUL SYLLABLE CYOLH
CD88;HANGUL SYLLABLE CYOM
CD89;HANGUL SYLLABLE CYOB
CD8A;HANGUL SYLLABLE CYOBS
CD8B;HANGUL SYLLABLE CYOS
CD8C;HANGUL SYLLABLE CYOSS
CD8D;HANGUL SYLLABLE CYONG
CD8E;HANGUL SYLLABLE CYOJ
CD8F;HANGUL SYLLABLE CYOC
CD90;HANGUL SYLLABLE CYOK
CD91;HANGUL SYLLABLE CYOT
CD92;HANGUL SYLLABLE CYOP
CD93;HANGUL SYLLABLE CYOH
CD94;HANGUL SYLLABLE CU
CD95;HANGUL SYLLABLE CUG
CD96;HANGUL SYLLABLE CUGG
CD97;HANGUL SYLLABLE CUGS
CD98;HANGUL SYLLABLE CUN
CD99;HANGUL SYLLABLE CUNJ
CD9A;HANGUL SYLLABLE CUNH
CD9B;HANGUL SYLLABLE CUD
CD9C;HANGUL SYLLABLE CUL
CD9D;HANGUL SYLLABLE CULG
CD9E;HANGUL SYLLABLE CULM
CD9F;HANGUL SYLLABLE CULB
CDA0;HANGUL SYLLABLE CULS
CDA1;HANGUL SYLLABLE CULT
CDA2;HANGUL SYLLABLE CULP
CDA3;HANGUL SYLLABLE CULH
CDA4;HANGUL SYLLABLE CUM
CDA5;HANGUL SYLLABLE CUB
CDA6;HANGUL SYLLABLE CUBS
CDA7;HANGUL SYLLABLE CUS
CDA8;HANGUL SYLLABLE CUSS
CDA9;HANGUL SYLLABLE CUNG
CDAA;HANGUL SYLLABLE CUJ
CDAB;HANGUL SYLLABLE CUC
CDAC;HANGUL SYLLABLE CUK
CDAD;HANGUL SYLLABLE CUT
CDAE;HANGUL SYLLABLE CUP
CDAF;HANGUL SYLLABLE CUH
CDB0;HANGUL SYLLABLE CWEO
CDB1;HANGUL SYLLABLE CWEOG
CDB2;HANGUL SYLLABLE CWEOGG
CDB3;HANGUL SYLLABLE CWEOGS
CDB4;HANGUL SYLLABLE CWEON
CDB5;HANGUL SYLLABLE CWEONJ
CDB6;HANGUL SYLLABLE CWEONH
CDB7;HANGUL SYLLABLE CWEOD
CDB8;HANGUL SYLLABLE CWEOL
CDB9;HANGUL SYLLABLE CWEOLG
CDBA;HANGUL SYLLABLE CWEOLM
CDBB;HANGUL SYLLABLE CWEOLB
CDBC;HANGUL SYLLABLE CWEOLS
CDBD;HANGUL SYLLABLE CWEOLT
CDBE;HANGUL SYLLABLE CWEOLP
CDBF;HANGUL SYLLABLE CWEOLH
CDC0;HANGUL SYLLABLE CWEOM
CDC1;HANGUL SYLLABLE CWEOB
CDC2;HANGUL SYLLABLE CWEOBS
CDC3;HANGUL SYLLABLE CWEOS
CDC4;HANGUL SYLLABLE CWEOSS
CDC5;HANGUL SYLLABLE CWEONG
CDC6;HANGUL SYLLABLE CWEOJ
CDC7;HANGUL SYLLABLE CWEOC
CDC8;HANGUL SYLLABLE CWEOK
CDC9;HANGUL SYLLABLE CWEOT
CDCA;HANGUL SYLLABLE CWEOP
CDCB;HANGUL SYLLABLE CWEOH
CDCC;HANGUL SYLLABLE CWE
CDCD;HANGUL SYLLABLE CWEG
CDCE;HANGUL SYLLABLE CWEGG
CDCF;HANGUL SYLLABLE CWEGS
CDD0;HANGUL SYLLABLE CWEN
CDD1;HANGUL SYLLABLE CWENJ
CDD2;HANGUL SYLLABLE CWENH
CDD3;HANGUL SYLLABLE CWED
CDD4;HANGUL SYLLABLE CWEL
CDD5;HANGUL SYLLABLE CWELG
CDD6;HANGUL SYLLABLE CWELM
CDD7;HANGUL SYLLABLE CWELB
CDD8;HANGUL SYLLABLE CWELS
CDD9;HANGUL SYLLABLE CWELT
CDDA;HANGUL SYLLABLE CWELP
CDDB;HANGUL SYLLABLE CWELH
CDDC;HANGUL SYLLABLE CWEM
CDDD;HANGUL SYLLABLE CWEB
CDDE;HANGUL SYLLABLE CWEBS
CDDF;HANGUL SYLLABLE CWES
CDE0;HANGUL SYLLABLE CWESS
CDE1;HANGUL SYLLABLE CWENG
CDE2;HANGUL SYLLABLE CWEJ
CDE3;HANGUL SYLLABLE CWEC
CDE4;HANGUL SYLLABLE CWEK
CDE5;HANGUL SYLLABLE CWET
CDE6;HANGUL SYLLABLE CWEP
CDE7;HANGUL SYLLABLE CWEH
CDE8;HANGUL SYLLABLE CWI
CDE9;HANGUL SYLLABLE CWIG
CDEA;HANGUL SYLLABLE CWIGG
CDEB;HANGUL SYLLABLE CWIGS
CDEC;HANGUL SYLLABLE CWIN
CDED;HANGUL SYLLABLE CWINJ
CDEE;HANGUL SYLLABLE CWINH
CDEF;HANGUL SYLLABLE CWID
CDF0;HANGUL SYLLABLE CWIL
CDF1;HANGUL SYLLABLE CWILG
CDF2;HANGUL SYLLABLE CWILM
CDF3;HANGUL SYLLABLE CWILB
CDF4;HANGUL SYLLABLE CWILS
CDF5;HANGUL SYLLABLE CWILT
CDF6;HANGUL SYLLABLE CWILP
CDF7;HANGUL SYLLABLE CWILH
CDF8;HANGUL SYLLABLE CWIM
CDF9;HANGUL SYLLABLE CWIB
CDFA;HANGUL SYLLABLE CWIBS
CDFB;HANGUL SYLLABLE CWIS
CDFC;HANGUL SYLLABLE CWISS
CDFD;HANGUL SYLLABLE CWING
CDFE;HANGUL SYLLABLE CWIJ
CDFF;HANGUL SYLLABLE CWIC
CE00;HANGUL SYLLABLE CWIK
CE01;HANGUL SYLLABLE CWIT
CE02;HANGUL SYLLABLE CWIP
CE03;HANGUL SYLLABLE CWIH
CE04;HANGUL SYLLABLE CYU
CE05;HANGUL SYLLABLE CYUG
CE06;HANGUL SYLLABLE CYUGG
CE07;HANGUL SYLLABLE CYUGS
CE08;HANGUL SYLLABLE CYUN
CE09;HANGUL SYLLABLE CYUNJ
CE0A;HANGUL SYLLABLE CYUNH
CE0B;HANGUL SYLLABLE CYUD
CE0C;HANGUL SYLLABLE CYUL
CE0D;HANGUL SYLLABLE CYULG
CE0E;HANGUL SYLLABLE CYULM
CE0F;HANGUL SYLLABLE CYULB
CE10;HANGUL SYLLABLE CYULS
CE11;HANGUL SYLLABLE CYULT
CE12;HANGUL SYLLABLE CYULP
CE13;HANGUL SYLLABLE CYULH
CE14;HANGUL SYLLABLE CYUM
CE15;HANGUL SYLLABLE CYUB
CE16;HANGUL SYLLABLE CYUBS
CE17;HANGUL SYLLABLE CYUS
CE18;HANGUL SYLLABLE CYUSS
CE19;HANGUL SYLLABLE CYUNG
CE1A;HANGUL SYLLABLE CYUJ
CE1B;HANGUL SYLLABLE CYUC
CE1C;HANGUL SYLLABLE CYUK
CE1D;HANGUL SYLLABLE CYUT
CE1E;HANGUL SYLLABLE CYUP
CE1F;HANGUL SYLLABLE CYUH
CE20;HANGUL SYLLABLE CEU
CE21;HANGUL SYLLABLE CEUG
CE22;HANGUL SYLLABLE CEUGG
CE23;HANGUL SYLLABLE CEUGS
CE24;HANGUL SYLLABLE CEUN
CE25;HANGUL SYLLABLE CEUNJ
CE26;HANGUL SYLLABLE CEUNH
CE27;HANGUL SYLLABLE CEUD
CE28;HANGUL SYLLABLE CEUL
CE29;HANGUL SYLLABLE CEULG
CE2A;HANGUL SYLLABLE CEULM
CE2B;HANGUL SYLLABLE CEULB
CE2C;HANGUL SYLLABLE CEULS
CE2D;HANGUL SYLLABLE CEULT
CE2E;HANGUL SYLLABLE CEULP
CE2F;HANGUL SYLLABLE CEULH
CE30;HANGUL SYLLABLE CEUM
CE31;HANGUL SYLLABLE CEUB
CE32;HANGUL SYLLABLE CEUBS
CE33;HANGUL SYLLABLE CEUS
CE34;HANGUL SYLLABLE CEUSS
CE35;HANGUL SYLLABLE CEUNG
CE36;HANGUL SYLLABLE CEUJ
CE37;HANGUL SYLLABLE CEUC
CE38;HANGUL SYLLABLE CEUK
CE39;HANGUL SYLLABLE CEUT
CE3A;HANGUL SYLLABLE CEUP
CE3B;HANGUL SYLLABLE CEUH
CE3C;HANGUL SYLLABLE CYI
CE3D;HANGUL SYLLABLE CYIG
CE3E;HANGUL SYLLABLE CYIGG
CE3F;HANGUL SYLLABLE CYIGS
CE40;HANGUL SYLLABLE CYIN
CE41;HANGUL SYLLABLE CYINJ
CE42;HANGUL SYLLABLE CYINH
CE43;HANGUL SYLLABLE CYID
CE44;HANGUL SYLLABLE CYIL
CE45;HANGUL SYLLABLE CYILG
CE46;HANGUL SYLLABLE CYILM
CE47;HANGUL SYLLABLE CYILB
CE48;HANGUL SYLLABLE CYILS
CE49;HANGUL SYLLABLE CYILT
CE4A;HANGUL SYLLABLE CYILP
CE4B;HANGUL SYLLABLE CYILH
CE4C;HANGUL SYLLABLE CYIM
CE4D;HANGUL SYLLABLE CYIB
CE4E;HANGUL SYLLABLE CYIBS
CE4F;HANGUL SYLLABLE CYIS
CE50;HANGUL SYLLABLE CYISS
CE51;HANGUL SYLLABLE CYING
CE52;HANGUL SYLLABLE CYIJ
CE53;HANGUL SYLLABLE CYIC
CE54;HANGUL SYLLABLE CYIK
CE55;HANGUL SYLLABLE CYIT
CE56;HANGUL SYLLABLE CYIP
CE57;HANGUL SYLLABLE CYIH
CE58;HANGUL SYLLABLE CI
CE59;HANGUL SYLLABLE CIG
CE5A;HANGUL SYLLABLE CIGG
CE5B;HANGUL SYLLABLE CIGS
CE5C;HANGUL SYLLABLE CIN
CE5D;HANGUL SYLLABLE CINJ
CE5E;HANGUL SYLLABLE CINH
CE5F;HANGUL SYLLABLE CID
CE60;HANGUL SYLLABLE CIL
CE61;HANGUL SYLLABLE CILG
CE62;HANGUL SYLLABLE CILM
CE63;HANGUL SYLLABLE CILB
CE64;HANGUL SYLLABLE CILS
CE65;HANGUL SYLLABLE CILT
CE66;HANGUL SYLLABLE CILP
CE67;HANGUL SYLLABLE CILH
CE68;HANGUL SYLLABLE CIM
CE69;HANGUL SYLLABLE CIB
CE6A;HANGUL SYLLABLE CIBS
CE6B;HANGUL SYLLABLE CIS
CE6C;HANGUL SYLLABLE CISS
CE6D;HANGUL SYLLABLE CING
CE6E;HANGUL SYLLABLE CIJ
CE6F;HANGUL SYLLABLE CIC
CE70;HANGUL SYLLABLE CIK
CE71;HANGUL SYLLABLE CIT
CE72;HANGUL SYLLABLE CIP
CE73;HANGUL SYLLABLE CIH
CE74;HANGUL SYLLABLE KA
CE75;HANGUL SYLLABLE KAG
CE76;HANGUL SYLLABLE KAGG
CE77;HANGUL SYLLABLE KAGS
CE78;HANGUL SYLLABLE KAN
CE79;HANGUL SYLLABLE KANJ
CE7A;HANGUL SYLLABLE KANH
CE7B;HANGUL SYLLABLE KAD
CE7C;HANGUL SYLLABLE KAL
CE7D;HANGUL SYLLABLE KALG
CE7E;HANGUL SYLLABLE KALM
CE7F;HANGUL SYLLABLE KALB
CE80;HANGUL SYLLABLE KALS
CE81;HANGUL SYLLABLE KALT
CE82;HANGUL SYLLABLE KALP
CE83;HANGUL SYLLABLE KALH
CE84;HANGUL SYLLABLE KAM
CE85;HANGUL SYLLABLE KAB
CE86;HANGUL SYLLABLE KABS
CE87;HANGUL SYLLABLE KAS
CE88;HANGUL SYLLABLE KASS
CE89;HANGUL SYLLABLE KANG
CE8A;HANGUL SYLLABLE KAJ
CE8B;HANGUL SYLLABLE KAC
CE8C;HANGUL SYLLABLE KAK
CE8D;HANGUL SYLLABLE KAT
CE8E;HANGUL SYLLABLE KAP
CE8F;HANGUL SYLLABLE KAH
CE90;HANGUL SYLLABLE KAE
CE91;HANGUL SYLLABLE KAEG
CE92;HANGUL SYLLABLE KAEGG
CE93;HANGUL SYLLABLE KAEGS
CE94;HANGUL SYLLABLE KAEN
CE95;HANGUL SYLLABLE KAENJ
CE96;HANGUL SYLLABLE KAENH
CE97;HANGUL SYLLABLE KAED
CE98;HANGUL SYLLABLE KAEL
CE99;HANGUL SYLLABLE KAELG
CE9A;HANGUL SYLLABLE KAELM
CE9B;HANGUL SYLLABLE KAELB
CE9C;HANGUL SYLLABLE KAELS
CE9D;HANGUL SYLLABLE KAELT
CE9E;HANGUL SYLLABLE KAELP
CE9F;HANGUL SYLLABLE KAELH
CEA0;HANGUL SYLLABLE KAEM
CEA1;HANGUL SYLLABLE KAEB
CEA2;HANGUL SYLLABLE KAEBS
CEA3;HANGUL SYLLABLE KAES
CEA4;HANGUL SYLLABLE KAESS
CEA5;HANGUL SYLLABLE KAENG
CEA6;HANGUL SYLLABLE KAEJ
CEA7;HANGUL SYLLABLE KAEC
CEA8;HANGUL SYLLABLE KAEK
CEA9;HANGUL SYLLABLE KAET
CEAA;HANGUL SYLLABLE KAEP
CEAB;HANGUL SYLLABLE KAEH
CEAC;HANGUL SYLLABLE KYA
CEAD;HANGUL SYLLABLE KYAG
CEAE;HANGUL SYLLABLE KYAGG
CEAF;HANGUL SYLLABLE KYAGS
CEB0;HANGUL SYLLABLE KYAN
CEB1;HANGUL SYLLABLE KYANJ
CEB2;HANGUL SYLLABLE KYANH
CEB3;HANGUL SYLLABLE KYAD
CEB4;HANGUL SYLLABLE KYAL
CEB5;HANGUL SYLLABLE KYALG
CEB6;HANGUL SYLLABLE KYALM
CEB7;HANGUL SYLLABLE KYALB
CEB8;HANGUL SYLLABLE KYALS
CEB9;HANGUL SYLLABLE KYALT
CEBA;HANGUL SYLLABLE KYALP
CEBB;HANGUL SYLLABLE KYALH
CEBC;HANGUL SYLLABLE KYAM
CEBD;HANGUL SYLLABLE KYAB
CEBE;HANGUL SYLLABLE KYABS
CEBF;HANGUL SYLLABLE KYAS
CEC0;HANGUL SYLLABLE KYASS
CEC1;HANGUL SYLLABLE KYANG
CEC2;HANGUL SYLLABLE KYAJ
CEC3;HANGUL SYLLABLE KYAC
CEC4;HANGUL SYLLABLE KYAK
CEC5;HANGUL SYLLABLE KYAT
CEC6;HANGUL SYLLABLE KYAP
CEC7;HANGUL SYLLABLE KYAH
CEC8;HANGUL SYLLABLE KYAE
CEC9;HANGUL SYLLABLE KYAEG
CECA;HANGUL SYLLABLE KYAEGG
CECB;HANGUL SYLLABLE KYAEGS
CECC;HANGUL SYLLABLE KYAEN
CECD;HANGUL SYLLABLE KYAENJ
CECE;HANGUL SYLLABLE KYAENH
CECF;HANGUL SYLLABLE KYAED
CED0;HANGUL SYLLABLE KYAEL
CED1;HANGUL SYLLABLE KYAELG
CED2;HANGUL SYLLABLE KYAELM
CED3;HANGUL SYLLABLE KYAELB
CED4;HANGUL SYLLABLE KYAELS
CED5;HANGUL SYLLABLE KYAELT
CED6;HANGUL SYLLABLE KYAELP
CED7;HANGUL SYLLABLE KYAELH
CED8;HANGUL SYLLABLE KYAEM
CED9;HANGUL SYLLABLE KYAEB
CEDA;HANGUL SYLLABLE KYAEBS
CEDB;HANGUL SYLLABLE KYAES
CEDC;HANGUL SYLLABLE KYAESS
CEDD;HANGUL SYLLABLE KYAENG
CEDE;HANGUL SYLLABLE KYAEJ
CEDF;HANGUL SYLLABLE KYAEC
CEE0;HANGUL SYLLABLE KYAEK
CEE1;HANGUL SYLLABLE KYAET
CEE2;HANGUL SYLLABLE KYAEP
CEE3;HANGUL SYLLABLE KYAEH
CEE4;HANGUL SYLLABLE KEO
CEE5;HANGUL SYLLABLE KEOG
CEE6;HANGUL SYLLABLE KEOGG
CEE7;HANGUL SYLLABLE KEOGS
CEE8;HANGUL SYLLABLE KEON
CEE9;HANGUL SYLLABLE KEONJ
CEEA;HANGUL SYLLABLE KEONH
CEEB;HANGUL SYLLABLE KEOD
CEEC;HANGUL SYLLABLE KEOL
CEED;HANGUL SYLLABLE KEOLG
CEEE;HANGUL SYLLABLE KEOLM
CEEF;HANGUL SYLLABLE KEOLB
CEF0;HANGUL SYLLABLE KEOLS
CEF1;HANGUL SYLLABLE KEOLT
CEF2;HANGUL SYLLABLE KEOLP
CEF3;HANGUL SYLLABLE KEOLH
CEF4;HANGUL SYLLABLE KEOM
CEF5;HANGUL SYLLABLE KEOB
CEF6;HANGUL SYLLABLE KEOBS
CEF7;HANGUL SYLLABLE KEOS
CEF8;HANGUL SYLLABLE KEOSS
CEF9;HANGUL SYLLABLE KEONG
CEFA;HANGUL SYLLABLE KEOJ
CEFB;HANGUL SYLLABLE KEOC
CEFC;HANGUL SYLLABLE KEOK
CEFD;HANGUL SYLLABLE KEOT
CEFE;HANGUL SYLLABLE KEOP
CEFF;HANGUL SYLLABLE KEOH
CF00;HANGUL SYLLABLE KE
CF01;HANGUL SYLLABLE KEG
CF02;HANGUL SYLLABLE KEGG
CF03;HANGUL SYLLABLE KEGS
CF04;HANGUL SYLLABLE KEN
CF05;HANGUL SYLLABLE KENJ
CF06;HANGUL SYLLABLE KENH
CF07;HANGUL SYLLABLE KED
CF08;HANGUL SYLLABLE KEL
CF09;HANGUL SYLLABLE KELG
CF0A;HANGUL SYLLABLE KELM
CF0B;HANGUL SYLLABLE KELB
CF0C;HANGUL SYLLABLE KELS
CF0D;HANGUL SYLLABLE KELT
CF0E;HANGUL SYLLABLE KELP
CF0F;HANGUL SYLLABLE KELH
CF10;HANGUL SYLLABLE KEM
CF11;HANGUL SYLLABLE KEB
CF12;HANGUL SYLLABLE KEBS
CF13;HANGUL SYLLABLE KES
CF14;HANGUL SYLLABLE KESS
CF15;HANGUL SYLLABLE KENG
CF16;HANGUL SYLLABLE KEJ
CF17;HANGUL SYLLABLE KEC
CF18;HANGUL SYLLABLE KEK
CF19;HANGUL SYLLABLE KET
CF1A;HANGUL SYLLABLE KEP
CF1B;HANGUL SYLLABLE KEH
CF1C;HANGUL SYLLABLE KYEO
CF1D;HANGUL SYLLABLE KYEOG
CF1E;HANGUL SYLLABLE KYEOGG
CF1F;HANGUL SYLLABLE KYEOGS
CF20;HANGUL SYLLABLE KYEON
CF21;HANGUL SYLLABLE KYEONJ
CF22;HANGUL SYLLABLE KYEONH
CF23;HANGUL SYLLABLE KYEOD
CF24;HANGUL SYLLABLE KYEOL
CF25;HANGUL SYLLABLE KYEOLG
CF26;HANGUL SYLLABLE KYEOLM
CF27;HANGUL SYLLABLE KYEOLB
CF28;HANGUL SYLLABLE KYEOLS
CF29;HANGUL SYLLABLE KYEOLT
CF2A;HANGUL SYLLABLE KYEOLP
CF2B;HANGUL SYLLABLE KYEOLH
CF2C;HANGUL SYLLABLE KYEOM
CF2D;HANGUL SYLLABLE KYEOB
CF2E;HANGUL SYLLABLE KYEOBS
CF2F;HANGUL SYLLABLE KYEOS
CF30;HANGUL SYLLABLE KYEOSS
CF31;HANGUL SYLLABLE KYEONG
CF32;HANGUL SYLLABLE KYEOJ
CF33;HANGUL SYLLABLE KYEOC
CF34;HANGUL SYLLABLE KYEOK
CF35;HANGUL SYLLABLE KYEOT
CF36;HANGUL SYLLABLE KYEOP
CF37;HANGUL SYLLABLE KYEOH
CF38;HANGUL SYLLABLE KYE
CF39;HANGUL SYLLABLE KYEG
CF3A;HANGUL SYLLABLE KYEGG
CF3B;HANGUL SYLLABLE KYEGS
CF3C;HANGUL SYLLABLE KYEN
CF3D;HANGUL SYLLABLE KYENJ
CF3E;HANGUL SYLLABLE KYENH
CF3F;HANGUL SYLLABLE KYED
CF40;HANGUL SYLLABLE KYEL
CF41;HANGUL SYLLABLE KYELG
CF42;HANGUL SYLLABLE KYELM
CF43;HANGUL SYLLABLE KYELB
CF44;HANGUL SYLLABLE KYELS
CF45;HANGUL SYLLABLE KYELT
CF46;HANGUL SYLLABLE KYELP
CF47;HANGUL SYLLABLE KYELH
CF48;HANGUL SYLLABLE KYEM
CF49;HANGUL SYLLABLE KYEB
CF4A;HANGUL SYLLABLE KYEBS
CF4B;HANGUL SYLLABLE KYES
CF4C;HANGUL SYLLABLE KYESS
CF4D;HANGUL SYLLABLE KYENG
CF4E;HANGUL SYLLABLE KYEJ
CF4F;HANGUL SYLLABLE KYEC
CF50;HANGUL SYLLABLE KYEK
CF51;HANGUL SYLLABLE KYET
CF52;HANGUL SYLLABLE KYEP
CF53;HANGUL SYLLABLE KYEH
CF54;HANGUL SYLLABLE KO
CF55;HANGUL SYLLABLE KOG
CF56;HANGUL SYLLABLE KOGG
CF57;HANGUL SYLLABLE KOGS
CF58;HANGUL SYLLABLE KON
CF59;HANGUL SYLLABLE KONJ
CF5A;HANGUL SYLLABLE KONH
CF5B;HANGUL SYLLABLE KOD
CF5C;HANGUL SYLLABLE KOL
CF5D;HANGUL SYLLABLE KOLG
CF5E;HANGUL SYLLABLE KOLM
CF5F;HANGUL SYLLABLE KOLB
CF60;HANGUL SYLLABLE KOLS
CF61;HANGUL SYLLABLE KOLT
CF62;HANGUL SYLLABLE KOLP
CF63;HANGUL SYLLABLE KOLH
CF64;HANGUL SYLLABLE KOM
CF65;HANGUL SYLLABLE KOB
CF66;HANGUL SYLLABLE KOBS
CF67;HANGUL SYLLABLE KOS
CF68;HANGUL SYLLABLE KOSS
CF69;HANGUL SYLLABLE KONG
CF6A;HANGUL SYLLABLE KOJ
CF6B;HANGUL SYLLABLE KOC
CF6C;HANGUL SYLLABLE KOK
CF6D;HANGUL SYLLABLE KOT
CF6E;HANGUL SYLLABLE KOP
CF6F;HANGUL SYLLABLE KOH
CF70;HANGUL SYLLABLE KWA
CF71;HANGUL SYLLABLE KWAG
CF72;HANGUL SYLLABLE KWAGG
CF73;HANGUL SYLLABLE KWAGS
CF74;HANGUL SYLLABLE KWAN
CF75;HANGUL SYLLABLE KWANJ
CF76;HANGUL SYLLABLE KWANH
CF77;HANGUL SYLLABLE KWAD
CF78;HANGUL SYLLABLE KWAL
CF79;HANGUL SYLLABLE KWALG
CF7A;HANGUL SYLLABLE KWALM
CF7B;HANGUL SYLLABLE KWALB
CF7C;HANGUL SYLLABLE KWALS
CF7D;HANGUL SYLLABLE KWALT
CF7E;HANGUL SYLLABLE KWALP
CF7F;HANGUL SYLLABLE KWALH
CF80;HANGUL SYLLABLE KWAM
CF81;HANGUL SYLLABLE KWAB
CF82;HANGUL SYLLABLE KWABS
CF83;HANGUL SYLLABLE KWAS
CF84;HANGUL SYLLABLE KWASS
CF85;HANGUL SYLLABLE KWANG
CF86;HANGUL SYLLABLE KWAJ
CF87;HANGUL SYLLABLE KWAC
CF88;HANGUL SYLLABLE KWAK
CF89;HANGUL SYLLABLE KWAT
CF8A;HANGUL SYLLABLE KWAP
CF8B;HANGUL SYLLABLE KWAH
CF8C;HANGUL SYLLABLE KWAE
CF8D;HANGUL SYLLABLE KWAEG
CF8E;HANGUL SYLLABLE KWAEGG
CF8F;HANGUL SYLLABLE KWAEGS
CF90;HANGUL SYLLABLE KWAEN
CF91;HANGUL SYLLABLE KWAENJ
CF92;HANGUL SYLLABLE KWAENH
CF93;HANGUL SYLLABLE KWAED
CF94;HANGUL SYLLABLE KWAEL
CF95;HANGUL SYLLABLE KWAELG
CF96;HANGUL SYLLABLE KWAELM
CF97;HANGUL SYLLABLE KWAELB
CF98;HANGUL SYLLABLE KWAELS
CF99;HANGUL SYLLABLE KWAELT
CF9A;HANGUL SYLLABLE KWAELP
CF9B;HANGUL SYLLABLE KWAELH
CF9C;HANGUL SYLLABLE KWAEM
CF9D;HANGUL SYLLABLE KWAEB
CF9E;HANGUL SYLLABLE KWAEBS
CF9F;HANGUL SYLLABLE KWAES
CFA0;HANGUL SYLLABLE KWAESS
CFA1;HANGUL SYLLABLE KWAENG
CFA2;HANGUL SYLLABLE KWAEJ
CFA3;HANGUL SYLLABLE KWAEC
CFA4;HANGUL SYLLABLE KWAEK
CFA5;HANGUL SYLLABLE KWAET
CFA6;HANGUL SYLLABLE KWAEP
CFA7;HANGUL SYLLABLE KWAEH
CFA8;HANGUL SYLLABLE KOE
CFA9;HANGUL SYLLABLE KOEG
CFAA;HANGUL SYLLABLE KOEGG
CFAB;HANGUL SYLLABLE KOEGS
CFAC;HANGUL SYLLABLE KOEN
CFAD;HANGUL SYLLABLE KOENJ
CFAE;HANGUL SYLLABLE KOENH
CFAF;HANGUL SYLLABLE KOED
CFB0;HANGUL SYLLABLE KOEL
CFB1;HANGUL SYLLABLE KOELG
CFB2;HANGUL SYLLABLE KOELM
CFB3;HANGUL SYLLABLE KOELB
CFB4;HANGUL SYLLABLE KOELS
CFB5;HANGUL SYLLABLE KOELT
CFB6;HANGUL SYLLABLE KOELP
CFB7;HANGUL SYLLABLE KOELH
CFB8;HANGUL SYLLABLE KOEM
CFB9;HANGUL SYLLABLE KOEB
CFBA;HANGUL SYLLABLE KOEBS
CFBB;HANGUL SYLLABLE KOES
CFBC;HANGUL SYLLABLE KOESS
CFBD;HANGUL SYLLABLE KOENG
CFBE;HANGUL SYLLABLE KOEJ
CFBF;HANGUL SYLLABLE KOEC
CFC0;HANGUL SYLLABLE KOEK
CFC1;HANGUL SYLLABLE KOET
CFC2;HANGUL SYLLABLE KOEP
CFC3;HANGUL SYLLABLE KOEH
CFC4;HANGUL SYLLABLE KYO
CFC5;HANGUL SYLLABLE KYOG
CFC6;HANGUL SYLLABLE KYOGG
CFC7;HANGUL SYLLABLE KYOGS
CFC8;HANGUL SYLLABLE KYON
CFC9;HANGUL SYLLABLE KYONJ
CFCA;HANGUL SYLLABLE KYONH
CFCB;HANGUL SYLLABLE KYOD
CFCC;HANGUL SYLLABLE KYOL
CFCD;HANGUL SYLLABLE KYOLG
CFCE;HANGUL SYLLABLE KYOLM
CFCF;HANGUL SYLLABLE KYOLB
CFD0;HANGUL SYLLABLE KYOLS
CFD1;HANGUL SYLLABLE KYOLT
CFD2;HANGUL SYLLABLE KYOLP
CFD3;HANGUL SYLLABLE KYOLH
CFD4;HANGUL SYLLABLE KYOM
CFD5;HANGUL SYLLABLE KYOB
CFD6;HANGUL SYLLABLE KYOBS
CFD7;HANGUL SYLLABLE KYOS
CFD8;HANGUL SYLLABLE KYOSS
CFD9;HANGUL SYLLABLE KYONG
CFDA;HANGUL SYLLABLE KYOJ
CFDB;HANGUL SYLLABLE KYOC
CFDC;HANGUL SYLLABLE KYOK
CFDD;HANGUL SYLLABLE KYOT
CFDE;HANGUL SYLLABLE KYOP
CFDF;HANGUL SYLLABLE KYOH
CFE0;HANGUL SYLLABLE KU
CFE1;HANGUL SYLLABLE KUG
CFE2;HANGUL SYLLABLE KUGG
CFE3;HANGUL SYLLABLE KUGS
CFE4;HANGUL SYLLABLE KUN
CFE5;HANGUL SYLLABLE KUNJ
CFE6;HANGUL SYLLABLE KUNH
CFE7;HANGUL SYLLABLE KUD
CFE8;HANGUL SYLLABLE KUL
CFE9;HANGUL SYLLABLE KULG
CFEA;HANGUL SYLLABLE KULM
CFEB;HANGUL SYLLABLE KULB
CFEC;HANGUL SYLLABLE KULS
CFED;HANGUL SYLLABLE KULT
CFEE;HANGUL SYLLABLE KULP
CFEF;HANGUL SYLLABLE KULH
CFF0;HANGUL SYLLABLE KUM
CFF1;HANGUL SYLLABLE KUB
CFF2;HANGUL SYLLABLE KUBS
CFF3;HANGUL SYLLABLE KUS
CFF4;HANGUL SYLLABLE KUSS
CFF5;HANGUL SYLLABLE KUNG
CFF6;HANGUL SYLLABLE KUJ
CFF7;HANGUL SYLLABLE KUC
CFF8;HANGUL SYLLABLE KUK
CFF9;HANGUL SYLLABLE KUT
CFFA;HANGUL SYLLABLE KUP
CFFB;HANGUL SYLLABLE KUH
CFFC;HANGUL SYLLABLE KWEO
CFFD;HANGUL SYLLABLE KWEOG
CFFE;HANGUL SYLLABLE KWEOGG
CFFF;HANGUL SYLLABLE KWEOGS
D000;HANGUL SYLLABLE KWEON
D001;HANGUL SYLLABLE KWEONJ
D002;HANGUL SYLLABLE KWEONH
D003;HANGUL SYLLABLE KWEOD
D004;HANGUL SYLLABLE KWEOL
D005;HANGUL SYLLABLE KWEOLG
D006;HANGUL SYLLABLE KWEOLM
D007;HANGUL SYLLABLE KWEOLB
D008;HANGUL SYLLABLE KWEOLS
D009;HANGUL SYLLABLE KWEOLT
D00A;HANGUL SYLLABLE KWEOLP
D00B;HANGUL SYLLABLE KWEOLH
D00C;HANGUL SYLLABLE KWEOM
D00D;HANGUL SYLLABLE KWEOB
D00E;HANGUL SYLLABLE KWEOBS
D00F;HANGUL SYLLABLE KWEOS
D010;HANGUL SYLLABLE KWEOSS
D011;HANGUL SYLLABLE KWEONG
D012;HANGUL SYLLABLE KWEOJ
D013;HANGUL SYLLABLE KWEOC
D014;HANGUL SYLLABLE KWEOK
D015;HANGUL SYLLABLE KWEOT
D016;HANGUL SYLLABLE KWEOP
D017;HANGUL SYLLABLE KWEOH
D018;HANGUL SYLLABLE KWE
D019;HANGUL SYLLABLE KWEG
D01A;HANGUL SYLLABLE KWEGG
D01B;HANGUL SYLLABLE KWEGS
D01C;HANGUL SYLLABLE KWEN
D01D;HANGUL SYLLABLE KWENJ
D01E;HANGUL SYLLABLE KWENH
D01F;HANGUL SYLLABLE KWED
D020;HANGUL SYLLABLE KWEL
D021;HANGUL SYLLABLE KWELG
D022;HANGUL SYLLABLE KWELM
D023;HANGUL SYLLABLE KWELB
D024;HANGUL SYLLABLE KWELS
D025;HANGUL SYLLABLE KWELT
D026;HANGUL SYLLABLE KWELP
D027;HANGUL SYLLABLE KWELH
D028;HANGUL SYLLABLE KWEM
D029;HANGUL SYLLABLE KWEB
D02A;HANGUL SYLLABLE KWEBS
D02B;HANGUL SYLLABLE KWES
D02C;HANGUL SYLLABLE KWESS
D02D;HANGUL SYLLABLE KWENG
D02E;HANGUL SYLLABLE KWEJ
D02F;HANGUL SYLLABLE KWEC
D030;HANGUL SYLLABLE KWEK
D031;HANGUL SYLLABLE KWET
D032;HANGUL SYLLABLE KWEP
D033;HANGUL SYLLABLE KWEH
D034;HANGUL SYLLABLE KWI
D035;HANGUL SYLLABLE KWIG
D036;HANGUL SYLLABLE KWIGG
D037;HANGUL SYLLABLE KWIGS
D038;HANGUL SYLLABLE KWIN
D039;HANGUL SYLLABLE KWINJ
D03A;HANGUL SYLLABLE KWINH
D03B;HANGUL SYLLABLE KWID
D03C;HANGUL SYLLABLE KWIL
D03D;HANGUL SYLLABLE KWILG
D03E;HANGUL SYLLABLE KWILM
D03F;HANGUL SYLLABLE KWILB
D040;HANGUL SYLLABLE KWILS
D041;HANGUL SYLLABLE KWILT
D042;HANGUL SYLLABLE KWILP
D043;HANGUL SYLLABLE KWILH
D044;HANGUL SYLLABLE KWIM
D045;HANGUL SYLLABLE KWIB
D046;HANGUL SYLLABLE KWIBS
D047;HANGUL SYLLABLE KWIS
D048;HANGUL SYLLABLE KWISS
D049;HANGUL SYLLABLE KWING
D04A;HANGUL SYLLABLE KWIJ
D04B;HANGUL SYLLABLE KWIC
D04C;HANGUL SYLLABLE KWIK
D04D;HANGUL SYLLABLE KWIT
D04E;HANGUL SYLLABLE KWIP
D04F;HANGUL SYLLABLE KWIH
D050;HANGUL SYLLABLE KYU
D051;HANGUL SYLLABLE KYUG
D052;HANGUL SYLLABLE KYUGG
D053;HANGUL SYLLABLE KYUGS
D054;HANGUL SYLLABLE KYUN
D055;HANGUL SYLLABLE KYUNJ
D056;HANGUL SYLLABLE KYUNH
D057;HANGUL SYLLABLE KYUD
D058;HANGUL SYLLABLE KYUL
D059;HANGUL SYLLABLE KYULG
D05A;HANGUL SYLLABLE KYULM
D05B;HANGUL SYLLABLE KYULB
D05C;HANGUL SYLLABLE KYULS
D05D;HANGUL SYLLABLE KYULT
D05E;HANGUL SYLLABLE KYULP
D05F;HANGUL SYLLABLE KYULH
D060;HANGUL SYLLABLE KYUM
D061;HANGUL SYLLABLE KYUB
D062;HANGUL SYLLABLE KYUBS
D063;HANGUL SYLLABLE KYUS
D064;HANGUL SYLLABLE KYUSS
D065;HANGUL SYLLABLE KYUNG
D066;HANGUL SYLLABLE KYUJ
D067;HANGUL SYLLABLE KYUC
D068;HANGUL SYLLABLE KYUK
D069;HANGUL SYLLABLE KYUT
D06A;HANGUL SYLLABLE KYUP
D06B;HANGUL SYLLABLE KYUH
D06C;HANGUL SYLLABLE KEU
D06D;HANGUL SYLLABLE KEUG
D06E;HANGUL SYLLABLE KEUGG
D06F;HANGUL SYLLABLE KEUGS
D070;HANGUL SYLLABLE KEUN
D071;HANGUL SYLLABLE KEUNJ
D072;HANGUL SYLLABLE KEUNH
D073;HANGUL SYLLABLE KEUD
D074;HANGUL SYLLABLE KEUL
D075;HANGUL SYLLABLE KEULG
D076;HANGUL SYLLABLE KEULM
D077;HANGUL SYLLABLE KEULB
D078;HANGUL SYLLABLE KEULS
D079;HANGUL SYLLABLE KEULT
D07A;HANGUL SYLLABLE KEULP
D07B;HANGUL SYLLABLE KEULH
D07C;HANGUL SYLLABLE KEUM
D07D;HANGUL SYLLABLE KEUB
D07E;HANGUL SYLLABLE KEUBS
D07F;HANGUL SYLLABLE KEUS
D080;HANGUL SYLLABLE KEUSS
D081;HANGUL SYLLABLE KEUNG
D082;HANGUL SYLLABLE KEUJ
D083;HANGUL SYLLABLE KEUC
D084;HANGUL SYLLABLE KEUK
D085;HANGUL SYLLABLE KEUT
D086;HANGUL SYLLABLE KEUP
D087;HANGUL SYLLABLE KEUH
D088;HANGUL SYLLABLE KYI
D089;HANGUL SYLLABLE KYIG
D08A;HANGUL SYLLABLE KYIGG
D08B;HANGUL SYLLABLE KYIGS
D08C;HANGUL SYLLABLE KYIN
D08D;HANGUL SYLLABLE KYINJ
D08E;HANGUL SYLLABLE KYINH
D08F;HANGUL SYLLABLE KYID
D090;HANGUL SYLLABLE KYIL
D091;HANGUL SYLLABLE KYILG
D092;HANGUL SYLLABLE KYILM
D093;HANGUL SYLLABLE KYILB
D094;HANGUL SYLLABLE KYILS
D095;HANGUL SYLLABLE KYILT
D096;HANGUL SYLLABLE KYILP
D097;HANGUL SYLLABLE KYILH
D098;HANGUL SYLLABLE KYIM
D099;HANGUL SYLLABLE KYIB
D09A;HANGUL SYLLABLE KYIBS
D09B;HANGUL SYLLABLE KYIS
D09C;HANGUL SYLLABLE KYISS
D09D;HANGUL SYLLABLE KYING
D09E;HANGUL SYLLABLE KYIJ
D09F;HANGUL SYLLABLE KYIC
D0A0;HANGUL SYLLABLE KYIK
D0A1;HANGUL SYLLABLE KYIT
D0A2;HANGUL SYLLABLE KYIP
D0A3;HANGUL SYLLABLE KYIH
D0A4;HANGUL SYLLABLE KI
D0A5;HANGUL SYLLABLE KIG
D0A6;HANGUL SYLLABLE KIGG
D0A7;HANGUL SYLLABLE KIGS
D0A8;HANGUL SYLLABLE KIN
D0A9;HANGUL SYLLABLE KINJ
D0AA;HANGUL SYLLABLE KINH
D0AB;HANGUL SYLLABLE KID
D0AC;HANGUL SYLLABLE KIL
D0AD;HANGUL SYLLABLE KILG
D0AE;HANGUL SYLLABLE KILM
D0AF;HANGUL SYLLABLE KILB
D0B0;HANGUL SYLLABLE KILS
D0B1;HANGUL SYLLABLE KILT
D0B2;HANGUL SYLLABLE KILP
D0B3;HANGUL SYLLABLE KILH
D0B4;HANGUL SYLLABLE KIM
D0B5;HANGUL SYLLABLE KIB
D0B6;HANGUL SYLLABLE KIBS
D0B7;HANGUL SYLLABLE KIS
D0B8;HANGUL SYLLABLE KISS
D0B9;HANGUL SYLLABLE KING
D0BA;HANGUL SYLLABLE KIJ
D0BB;HANGUL SYLLABLE KIC
D0BC;HANGUL SYLLABLE KIK
D0BD;HANGUL SYLLABLE KIT
D0BE;HANGUL SYLLABLE KIP
D0BF;HANGUL SYLLABLE KIH
D0C0;HANGUL SYLLABLE TA
D0C1;HANGUL SYLLABLE TAG
D0C2;HANGUL SYLLABLE TAGG
D0C3;HANGUL SYLLABLE TAGS
D0C4;HANGUL SYLLABLE TAN
D0C5;HANGUL SYLLABLE TANJ
D0C6;HANGUL SYLLABLE TANH
D0C7;HANGUL SYLLABLE TAD
D0C8;HANGUL SYLLABLE TAL
D0C9;HANGUL SYLLABLE TALG
D0CA;HANGUL SYLLABLE TALM
D0CB;HANGUL SYLLABLE TALB
D0CC;HANGUL SYLLABLE TALS
D0CD;HANGUL SYLLABLE TALT
D0CE;HANGUL SYLLABLE TALP
D0CF;HANGUL SYLLABLE TALH
D0D0;HANGUL SYLLABLE TAM
D0D1;HANGUL SYLLABLE TAB
D0D2;HANGUL SYLLABLE TABS
D0D3;HANGUL SYLLABLE TAS
D0D4;HANGUL SYLLABLE TASS
D0D5;HANGUL SYLLABLE TANG
D0D6;HANGUL SYLLABLE TAJ
D0D7;HANGUL SYLLABLE TAC
D0D8;HANGUL SYLLABLE TAK
D0D9;HANGUL SYLLABLE TAT
D0DA;HANGUL SYLLABLE TAP
D0DB;HANGUL SYLLABLE TAH
D0DC;HANGUL SYLLABLE TAE
D0DD;HANGUL SYLLABLE TAEG
D0DE;HANGUL SYLLABLE TAEGG
D0DF;HANGUL SYLLABLE TAEGS
D0E0;HANGUL SYLLABLE TAEN
D0E1;HANGUL SYLLABLE TAENJ
D0E2;HANGUL SYLLABLE TAENH
D0E3;HANGUL SYLLABLE TAED
D0E4;HANGUL SYLLABLE TAEL
D0E5;HANGUL SYLLABLE TAELG
D0E6;HANGUL SYLLABLE TAELM
D0E7;HANGUL SYLLABLE TAELB
D0E8;HANGUL SYLLABLE TAELS
D0E9;HANGUL SYLLABLE TAELT
D0EA;HANGUL SYLLABLE TAELP
D0EB;HANGUL SYLLABLE TAELH
D0EC;HANGUL SYLLABLE TAEM
D0ED;HANGUL SYLLABLE TAEB
D0EE;HANGUL SYLLABLE TAEBS
D0EF;HANGUL SYLLABLE TAES
D0F0;HANGUL SYLLABLE TAESS
D0F1;HANGUL SYLLABLE TAENG
D0F2;HANGUL SYLLABLE TAEJ
D0F3;HANGUL SYLLABLE TAEC
D0F4;HANGUL SYLLABLE TAEK
D0F5;HANGUL SYLLABLE TAET
D0F6;HANGUL SYLLABLE TAEP
D0F7;HANGUL SYLLABLE TAEH
D0F8;HANGUL SYLLABLE TYA
D0F9;HANGUL SYLLABLE TYAG
D0FA;HANGUL SYLLABLE TYAGG
D0FB;HANGUL SYLLABLE TYAGS
D0FC;HANGUL SYLLABLE TYAN
D0FD;HANGUL SYLLABLE TYANJ
D0FE;HANGUL SYLLABLE TYANH
D0FF;HANGUL SYLLABLE TYAD
D100;HANGUL SYLLABLE TYAL
D101;HANGUL SYLLABLE TYALG
D102;HANGUL SYLLABLE TYALM
D103;HANGUL SYLLABLE TYALB
D104;HANGUL SYLLABLE TYALS
D105;HANGUL SYLLABLE TYALT
D106;HANGUL SYLLABLE TYALP
D107;HANGUL SYLLABLE TYALH
D108;HANGUL SYLLABLE TYAM
D109;HANGUL SYLLABLE TYAB
D10A;HANGUL SYLLABLE TYABS
D10B;HANGUL SYLLABLE TYAS
D10C;HANGUL SYLLABLE TYASS
D10D;HANGUL SYLLABLE TYANG
D10E;HANGUL SYLLABLE TYAJ
D10F;HANGUL SYLLABLE TYAC
D110;HANGUL SYLLABLE TYAK
D111;HANGUL SYLLABLE TYAT
D112;HANGUL SYLLABLE TYAP
D113;HANGUL SYLLABLE TYAH
D114;HANGUL SYLLABLE TYAE
D115;HANGUL SYLLABLE TYAEG
D116;HANGUL SYLLABLE TYAEGG
D117;HANGUL SYLLABLE TYAEGS
D118;HANGUL SYLLABLE TYAEN
D119;HANGUL SYLLABLE TYAENJ
D11A;HANGUL SYLLABLE TYAENH
D11B;HANGUL SYLLABLE TYAED
D11C;HANGUL SYLLABLE TYAEL
D11D;HANGUL SYLLABLE TYAELG
D11E;HANGUL SYLLABLE TYAELM
D11F;HANGUL SYLLABLE TYAELB
D120;HANGUL SYLLABLE TYAELS
D121;HANGUL SYLLABLE TYAELT
D122;HANGUL SYLLABLE TYAELP
D123;HANGUL SYLLABLE TYAELH
D124;HANGUL SYLLABLE TYAEM
D125;HANGUL SYLLABLE TYAEB
D126;HANGUL SYLLABLE TYAEBS
D127;HANGUL SYLLABLE TYAES
D128;HANGUL SYLLABLE TYAESS
D129;HANGUL SYLLABLE TYAENG
D12A;HANGUL SYLLABLE TYAEJ
D12B;HANGUL SYLLABLE TYAEC
D12C;HANGUL SYLLABLE TYAEK
D12D;HANGUL SYLLABLE TYAET
D12E;HANGUL SYLLABLE TYAEP
D12F;HANGUL SYLLABLE TYAEH
D130;HANGUL SYLLABLE TEO
D131;HANGUL SYLLABLE TEOG
D132;HANGUL SYLLABLE TEOGG
D133;HANGUL SYLLABLE TEOGS
D134;HANGUL SYLLABLE TEON
D135;HANGUL SYLLABLE TEONJ
D136;HANGUL SYLLABLE TEONH
D137;HANGUL SYLLABLE TEOD
D138;HANGUL SYLLABLE TEOL
D139;HANGUL SYLLABLE TEOLG
D13A;HANGUL SYLLABLE TEOLM
D13B;HANGUL SYLLABLE TEOLB
D13C;HANGUL SYLLABLE TEOLS
D13D;HANGUL SYLLABLE TEOLT
D13E;HANGUL SYLLABLE TEOLP
D13F;HANGUL SYLLABLE TEOLH
D140;HANGUL SYLLABLE TEOM
D141;HANGUL SYLLABLE TEOB
D142;HANGUL SYLLABLE TEOBS
D143;HANGUL SYLLABLE TEOS
D144;HANGUL SYLLABLE TEOSS
D145;HANGUL SYLLABLE TEONG
D146;HANGUL SYLLABLE TEOJ
D147;HANGUL SYLLABLE TEOC
D148;HANGUL SYLLABLE TEOK
D149;HANGUL SYLLABLE TEOT
D14A;HANGUL SYLLABLE TEOP
D14B;HANGUL SYLLABLE TEOH
D14C;HANGUL SYLLABLE TE
D14D;HANGUL SYLLABLE TEG
D14E;HANGUL SYLLABLE TEGG
D14F;HANGUL SYLLABLE TEGS
D150;HANGUL SYLLABLE TEN
D151;HANGUL SYLLABLE TENJ
D152;HANGUL SYLLABLE TENH
D153;HANGUL SYLLABLE TED
D154;HANGUL SYLLABLE TEL
D155;HANGUL SYLLABLE TELG
D156;HANGUL SYLLABLE TELM
D157;HANGUL SYLLABLE TELB
D158;HANGUL SYLLABLE TELS
D159;HANGUL SYLLABLE TELT
D15A;HANGUL SYLLABLE TELP
D15B;HANGUL SYLLABLE TELH
D15C;HANGUL SYLLABLE TEM
D15D;HANGUL SYLLABLE TEB
D15E;HANGUL SYLLABLE TEBS
D15F;HANGUL SYLLABLE TES
D160;HANGUL SYLLABLE TESS
D161;HANGUL SYLLABLE TENG
D162;HANGUL SYLLABLE TEJ
D163;HANGUL SYLLABLE TEC
D164;HANGUL SYLLABLE TEK
D165;HANGUL SYLLABLE TET
D166;HANGUL SYLLABLE TEP
D167;HANGUL SYLLABLE TEH
D168;HANGUL SYLLABLE TYEO
D169;HANGUL SYLLABLE TYEOG
D16A;HANGUL SYLLABLE TYEOGG
D16B;HANGUL SYLLABLE TYEOGS
D16C;HANGUL SYLLABLE TYEON
D16D;HANGUL SYLLABLE TYEONJ
D16E;HANGUL SYLLABLE TYEONH
D16F;HANGUL SYLLABLE TYEOD
D170;HANGUL SYLLABLE TYEOL
D171;HANGUL SYLLABLE TYEOLG
D172;HANGUL SYLLABLE TYEOLM
D173;HANGUL SYLLABLE TYEOLB
D174;HANGUL SYLLABLE TYEOLS
D175;HANGUL SYLLABLE TYEOLT
D176;HANGUL SYLLABLE TYEOLP
D177;HANGUL SYLLABLE TYEOLH
D178;HANGUL SYLLABLE TYEOM
D179;HANGUL SYLLABLE TYEOB
D17A;HANGUL SYLLABLE TYEOBS
D17B;HANGUL SYLLABLE TYEOS
D17C;HANGUL SYLLABLE TYEOSS
D17D;HANGUL SYLLABLE TYEONG
D17E;HANGUL SYLLABLE TYEOJ
D17F;HANGUL SYLLABLE TYEOC
D180;HANGUL SYLLABLE TYEOK
D181;HANGUL SYLLABLE TYEOT
D182;HANGUL SYLLABLE TYEOP
D183;HANGUL SYLLABLE TYEOH
D184;HANGUL SYLLABLE TYE
D185;HANGUL SYLLABLE TYEG
D186;HANGUL SYLLABLE TYEGG
D187;HANGUL SYLLABLE TYEGS
D188;HANGUL SYLLABLE TYEN
D189;HANGUL SYLLABLE TYENJ
D18A;HANGUL SYLLABLE TYENH
D18B;HANGUL SYLLABLE TYED
D18C;HANGUL SYLLABLE TYEL
D18D;HANGUL SYLLABLE TYELG
D18E;HANGUL SYLLABLE TYELM
D18F;HANGUL SYLLABLE TYELB
D190;HANGUL SYLLABLE TYELS
D191;HANGUL SYLLABLE TYELT
D192;HANGUL SYLLABLE TYELP
D193;HANGUL SYLLABLE TYELH
D194;HANGUL SYLLABLE TYEM
D195;HANGUL SYLLABLE TYEB
D196;HANGUL SYLLABLE TYEBS
D197;HANGUL SYLLABLE TYES
D198;HANGUL SYLLABLE TYESS
D199;HANGUL SYLLABLE TYENG
D19A;HANGUL SYLLABLE TYEJ
D19B;HANGUL SYLLABLE TYEC
D19C;HANGUL SYLLABLE TYEK
D19D;HANGUL SYLLABLE TYET
D19E;HANGUL SYLLABLE TYEP
D19F;HANGUL SYLLABLE TYEH
D1A0;HANGUL SYLLABLE TO
D1A1;HANGUL SYLLABLE TOG
D1A2;HANGUL SYLLABLE TOGG
D1A3;HANGUL SYLLABLE TOGS
D1A4;HANGUL SYLLABLE TON
D1A5;HANGUL SYLLABLE TONJ
D1A6;HANGUL SYLLABLE TONH
D1A7;HANGUL SYLLABLE TOD
D1A8;HANGUL SYLLABLE TOL
D1A9;HANGUL SYLLABLE TOLG
D1AA;HANGUL SYLLABLE TOLM
D1AB;HANGUL SYLLABLE TOLB
D1AC;HANGUL SYLLABLE TOLS
D1AD;HANGUL SYLLABLE TOLT
D1AE;HANGUL SYLLABLE TOLP
D1AF;HANGUL SYLLABLE TOLH
D1B0;HANGUL SYLLABLE TOM
D1B1;HANGUL SYLLABLE TOB
D1B2;HANGUL SYLLABLE TOBS
D1B3;HANGUL SYLLABLE TOS
D1B4;HANGUL SYLLABLE TOSS
D1B5;HANGUL SYLLABLE TONG
D1B6;HANGUL SYLLABLE TOJ
D1B7;HANGUL SYLLABLE TOC
D1B8;HANGUL SYLLABLE TOK
D1B9;HANGUL SYLLABLE TOT
D1BA;HANGUL SYLLABLE TOP
D1BB;HANGUL SYLLABLE TOH
D1BC;HANGUL SYLLABLE TWA
D1BD;HANGUL SYLLABLE TWAG
D1BE;HANGUL SYLLABLE TWAGG
D1BF;HANGUL SYLLABLE TWAGS
D1C0;HANGUL SYLLABLE TWAN
D1C1;HANGUL SYLLABLE TWANJ
D1C2;HANGUL SYLLABLE TWANH
D1C3;HANGUL SYLLABLE TWAD
D1C4;HANGUL SYLLABLE TWAL
D1C5;HANGUL SYLLABLE TWALG
D1C6;HANGUL SYLLABLE TWALM
D1C7;HANGUL SYLLABLE TWALB
D1C8;HANGUL SYLLABLE TWALS
D1C9;HANGUL SYLLABLE TWALT
D1CA;HANGUL SYLLABLE TWALP
D1CB;HANGUL SYLLABLE TWALH
D1CC;HANGUL SYLLABLE TWAM
D1CD;HANGUL SYLLABLE TWAB
D1CE;HANGUL SYLLABLE TWABS
D1CF;HANGUL SYLLABLE TWAS
D1D0;HANGUL SYLLABLE TWASS
D1D1;HANGUL SYLLABLE TWANG
D1D2;HANGUL SYLLABLE TWAJ
D1D3;HANGUL SYLLABLE TWAC
D1D4;HANGUL SYLLABLE TWAK
D1D5;HANGUL SYLLABLE TWAT
D1D6;HANGUL SYLLABLE TWAP
D1D7;HANGUL SYLLABLE TWAH
D1D8;HANGUL SYLLABLE TWAE
D1D9;HANGUL SYLLABLE TWAEG
D1DA;HANGUL SYLLABLE TWAEGG
D1DB;HANGUL SYLLABLE TWAEGS
D1DC;HANGUL SYLLABLE TWAEN
D1DD;HANGUL SYLLABLE TWAENJ
D1DE;HANGUL SYLLABLE TWAENH
D1DF;HANGUL SYLLABLE TWAED
D1E0;HANGUL SYLLABLE TWAEL
D1E1;HANGUL SYLLABLE TWAELG
D1E2;HANGUL SYLLABLE TWAELM
D1E3;HANGUL SYLLABLE TWAELB
D1E4;HANGUL SYLLABLE TWAELS
D1E5;HANGUL SYLLABLE TWAELT
D1E6;HANGUL SYLLABLE TWAELP
D1E7;HANGUL SYLLABLE TWAELH
D1E8;HANGUL SYLLABLE TWAEM
D1E9;HANGUL SYLLABLE TWAEB
D1EA;HANGUL SYLLABLE TWAEBS
D1EB;HANGUL SYLLABLE TWAES
D1EC;HANGUL SYLLABLE TWAESS
D1ED;HANGUL SYLLABLE TWAENG
D1EE;HANGUL SYLLABLE TWAEJ
D1EF;HANGUL SYLLABLE TWAEC
D1F0;HANGUL SYLLABLE TWAEK
D1F1;HANGUL SYLLABLE TWAET
D1F2;HANGUL SYLLABLE TWAEP
D1F3;HANGUL SYLLABLE TWAEH
D1F4;HANGUL SYLLABLE TOE
D1F5;HANGUL SYLLABLE TOEG
D1F6;HANGUL SYLLABLE TOEGG
D1F7;HANGUL SYLLABLE TOEGS
D1F8;HANGUL SYLLABLE TOEN
D1F9;HANGUL SYLLABLE TOENJ
D1FA;HANGUL SYLLABLE TOENH
D1FB;HANGUL SYLLABLE TOED
D1FC;HANGUL SYLLABLE TOEL
D1FD;HANGUL SYLLABLE TOELG
D1FE;HANGUL SYLLABLE TOELM
D1FF;HANGUL SYLLABLE TOELB
D200;HANGUL SYLLABLE TOELS
D201;HANGUL SYLLABLE TOELT
D202;HANGUL SYLLABLE TOELP
D203;HANGUL SYLLABLE TOELH
D204;HANGUL SYLLABLE TOEM
D205;HANGUL SYLLABLE TOEB
D206;HANGUL SYLLABLE TOEBS
D207;HANGUL SYLLABLE TOES
D208;HANGUL SYLLABLE TOESS
D209;HANGUL SYLLABLE TOENG
D20A;HANGUL SYLLABLE TOEJ
D20B;HANGUL SYLLABLE TOEC
D20C;HANGUL SYLLABLE TOEK
D20D;HANGUL SYLLABLE TOET
D20E;HANGUL SYLLABLE TOEP
D20F;HANGUL SYLLABLE TOEH
D210;HANGUL SYLLABLE TYO
D211;HANGUL SYLLABLE TYOG
D212;HANGUL SYLLABLE TYOGG
D213;HANGUL SYLLABLE TYOGS
D214;HANGUL SYLLABLE TYON
D215;HANGUL SYLLABLE TYONJ
D216;HANGUL SYLLABLE TYONH
D217;HANGUL SYLLABLE TYOD
D218;HANGUL SYLLABLE TYOL
D219;HANGUL SYLLABLE TYOLG
D21A;HANGUL SYLLABLE TYOLM
D21B;HANGUL SYLLABLE TYOLB
D21C;HANGUL SYLLABLE TYOLS
D21D;HANGUL SYLLABLE TYOLT
D21E;HANGUL SYLLABLE TYOLP
D21F;HANGUL SYLLABLE TYOLH
D220;HANGUL SYLLABLE TYOM
D221;HANGUL SYLLABLE TYOB
D222;HANGUL SYLLABLE TYOBS
D223;HANGUL SYLLABLE TYOS
D224;HANGUL SYLLABLE TYOSS
D225;HANGUL SYLLABLE TYONG
D226;HANGUL SYLLABLE TYOJ
D227;HANGUL SYLLABLE TYOC
D228;HANGUL SYLLABLE TYOK
D229;HANGUL SYLLABLE TYOT
D22A;HANGUL SYLLABLE TYOP
D22B;HANGUL SYLLABLE TYOH
D22C;HANGUL SYLLABLE TU
D22D;HANGUL SYLLABLE TUG
D22E;HANGUL SYLLABLE TUGG
D22F;HANGUL SYLLABLE TUGS
D230;HANGUL SYLLABLE TUN
D231;HANGUL SYLLABLE TUNJ
D232;HANGUL SYLLABLE TUNH
D233;HANGUL SYLLABLE TUD
D234;HANGUL SYLLABLE TUL
D235;HANGUL SYLLABLE TULG
D236;HANGUL SYLLABLE TULM
D237;HANGUL SYLLABLE TULB
D238;HANGUL SYLLABLE TULS
D239;HANGUL SYLLABLE TULT
D23A;HANGUL SYLLABLE TULP
D23B;HANGUL SYLLABLE TULH
D23C;HANGUL SYLLABLE TUM
D23D;HANGUL SYLLABLE TUB
D23E;HANGUL SYLLABLE TUBS
D23F;HANGUL SYLLABLE TUS
D240;HANGUL SYLLABLE TUSS
D241;HANGUL SYLLABLE TUNG
D242;HANGUL SYLLABLE TUJ
D243;HANGUL SYLLABLE TUC
D244;HANGUL SYLLABLE TUK
D245;HANGUL SYLLABLE TUT
D246;HANGUL SYLLABLE TUP
D247;HANGUL SYLLABLE TUH
D248;HANGUL SYLLABLE TWEO
D249;HANGUL SYLLABLE TWEOG
D24A;HANGUL SYLLABLE TWEOGG
D24B;HANGUL SYLLABLE TWEOGS
D24C;HANGUL SYLLABLE TWEON
D24D;HANGUL SYLLABLE TWEONJ
D24E;HANGUL SYLLABLE TWEONH
D24F;HANGUL SYLLABLE TWEOD
D250;HANGUL SYLLABLE TWEOL
D251;HANGUL SYLLABLE TWEOLG
D252;HANGUL SYLLABLE TWEOLM
D253;HANGUL SYLLABLE TWEOLB
D254;HANGUL SYLLABLE TWEOLS
D255;HANGUL SYLLABLE TWEOLT
D256;HANGUL SYLLABLE TWEOLP
D257;HANGUL SYLLABLE TWEOLH
D258;HANGUL SYLLABLE TWEOM
D259;HANGUL SYLLABLE TWEOB
D25A;HANGUL SYLLABLE TWEOBS
D25B;HANGUL SYLLABLE TWEOS
D25C;HANGUL SYLLABLE TWEOSS
D25D;HANGUL SYLLABLE TWEONG
D25E;HANGUL SYLLABLE TWEOJ
D25F;HANGUL SYLLABLE TWEOC
D260;HANGUL SYLLABLE TWEOK
D261;HANGUL SYLLABLE TWEOT
D262;HANGUL SYLLABLE TWEOP
D263;HANGUL SYLLABLE TWEOH
D264;HANGUL SYLLABLE TWE
D265;HANGUL SYLLABLE TWEG
D266;HANGUL SYLLABLE TWEGG
D267;HANGUL SYLLABLE TWEGS
D268;HANGUL SYLLABLE TWEN
D269;HANGUL SYLLABLE TWENJ
D26A;HANGUL SYLLABLE TWENH
D26B;HANGUL SYLLABLE TWED
D26C;HANGUL SYLLABLE TWEL
D26D;HANGUL SYLLABLE TWELG
D26E;HANGUL SYLLABLE TWELM
D26F;HANGUL SYLLABLE TWELB
D270;HANGUL SYLLABLE TWELS
D271;HANGUL SYLLABLE TWELT
D272;HANGUL SYLLABLE TWELP
D273;HANGUL SYLLABLE TWELH
D274;HANGUL SYLLABLE TWEM
D275;HANGUL SYLLABLE TWEB
D276;HANGUL SYLLABLE TWEBS
D277;HANGUL SYLLABLE TWES
D278;HANGUL SYLLABLE TWESS
D279;HANGUL SYLLABLE TWENG
D27A;HANGUL SYLLABLE TWEJ
D27B;HANGUL SYLLABLE TWEC
D27C;HANGUL SYLLABLE TWEK
D27D;HANGUL SYLLABLE TWET
D27E;HANGUL SYLLABLE TWEP
D27F;HANGUL SYLLABLE TWEH
D280;HANGUL SYLLABLE TWI
D281;HANGUL SYLLABLE TWIG
D282;HANGUL SYLLABLE TWIGG
D283;HANGUL SYLLABLE TWIGS
D284;HANGUL SYLLABLE TWIN
D285;HANGUL SYLLABLE TWINJ
D286;HANGUL SYLLABLE TWINH
D287;HANGUL SYLLABLE TWID
D288;HANGUL SYLLABLE TWIL
D289;HANGUL SYLLABLE TWILG
D28A;HANGUL SYLLABLE TWILM
D28B;HANGUL SYLLABLE TWILB
D28C;HANGUL SYLLABLE TWILS
D28D;HANGUL SYLLABLE TWILT
D28E;HANGUL SYLLABLE TWILP
D28F;HANGUL SYLLABLE TWILH
D290;HANGUL SYLLABLE TWIM
D291;HANGUL SYLLABLE TWIB
D292;HANGUL SYLLABLE TWIBS
D293;HANGUL SYLLABLE TWIS
D294;HANGUL SYLLABLE TWISS
D295;HANGUL SYLLABLE TWING
D296;HANGUL SYLLABLE TWIJ
D297;HANGUL SYLLABLE TWIC
D298;HANGUL SYLLABLE TWIK
D299;HANGUL SYLLABLE TWIT
D29A;HANGUL SYLLABLE TWIP
D29B;HANGUL SYLLABLE TWIH
D29C;HANGUL SYLLABLE TYU
D29D;HANGUL SYLLABLE TYUG
D29E;HANGUL SYLLABLE TYUGG
D29F;HANGUL SYLLABLE TYUGS
D2A0;HANGUL SYLLABLE TYUN
D2A1;HANGUL SYLLABLE TYUNJ
D2A2;HANGUL SYLLABLE TYUNH
D2A3;HANGUL SYLLABLE TYUD
D2A4;HANGUL SYLLABLE TYUL
D2A5;HANGUL SYLLABLE TYULG
D2A6;HANGUL SYLLABLE TYULM
D2A7;HANGUL SYLLABLE TYULB
D2A8;HANGUL SYLLABLE TYULS
D2A9;HANGUL SYLLABLE TYULT
D2AA;HANGUL SYLLABLE TYULP
D2AB;HANGUL SYLLABLE TYULH
D2AC;HANGUL SYLLABLE TYUM
D2AD;HANGUL SYLLABLE TYUB
D2AE;HANGUL SYLLABLE TYUBS
D2AF;HANGUL SYLLABLE TYUS
D2B0;HANGUL SYLLABLE TYUSS
D2B1;HANGUL SYLLABLE TYUNG
D2B2;HANGUL SYLLABLE TYUJ
D2B3;HANGUL SYLLABLE TYUC
D2B4;HANGUL SYLLABLE TYUK
D2B5;HANGUL SYLLABLE TYUT
D2B6;HANGUL SYLLABLE TYUP
D2B7;HANGUL SYLLABLE TYUH
D2B8;HANGUL SYLLABLE TEU
D2B9;HANGUL SYLLABLE TEUG
D2BA;HANGUL SYLLABLE TEUGG
D2BB;HANGUL SYLLABLE TEUGS
D2BC;HANGUL SYLLABLE TEUN
D2BD;HANGUL SYLLABLE TEUNJ
D2BE;HANGUL SYLLABLE TEUNH
D2BF;HANGUL SYLLABLE TEUD
D2C0;HANGUL SYLLABLE TEUL
D2C1;HANGUL SYLLABLE TEULG
D2C2;HANGUL SYLLABLE TEULM
D2C3;HANGUL SYLLABLE TEULB
D2C4;HANGUL SYLLABLE TEULS
D2C5;HANGUL SYLLABLE TEULT
D2C6;HANGUL SYLLABLE TEULP
D2C7;HANGUL SYLLABLE TEULH
D2C8;HANGUL SYLLABLE TEUM
D2C9;HANGUL SYLLABLE TEUB
D2CA;HANGUL SYLLABLE TEUBS
D2CB;HANGUL SYLLABLE TEUS
D2CC;HANGUL SYLLABLE TEUSS
D2CD;HANGUL SYLLABLE TEUNG
D2CE;HANGUL SYLLABLE TEUJ
D2CF;HANGUL SYLLABLE TEUC
D2D0;HANGUL SYLLABLE TEUK
D2D1;HANGUL SYLLABLE TEUT
D2D2;HANGUL SYLLABLE TEUP
D2D3;HANGUL SYLLABLE TEUH
D2D4;HANGUL SYLLABLE TYI
D2D5;HANGUL SYLLABLE TYIG
D2D6;HANGUL SYLLABLE TYIGG
D2D7;HANGUL SYLLABLE TYIGS
D2D8;HANGUL SYLLABLE TYIN
D2D9;HANGUL SYLLABLE TYINJ
D2DA;HANGUL SYLLABLE TYINH
D2DB;HANGUL SYLLABLE TYID
D2DC;HANGUL SYLLABLE TYIL
D2DD;HANGUL SYLLABLE TYILG
D2DE;HANGUL SYLLABLE TYILM
D2DF;HANGUL SYLLABLE TYILB
D2E0;HANGUL SYLLABLE TYILS
D2E1;HANGUL SYLLABLE TYILT
D2E2;HANGUL SYLLABLE TYILP
D2E3;HANGUL SYLLABLE TYILH
D2E4;HANGUL SYLLABLE TYIM
D2E5;HANGUL SYLLABLE TYIB
D2E6;HANGUL SYLLABLE TYIBS
D2E7;HANGUL SYLLABLE TYIS
D2E8;HANGUL SYLLABLE TYISS
D2E9;HANGUL SYLLABLE TYING
D2EA;HANGUL SYLLABLE TYIJ
D2EB;HANGUL SYLLABLE TYIC
D2EC;HANGUL SYLLABLE TYIK
D2ED;HANGUL SYLLABLE TYIT
D2EE;HANGUL SYLLABLE TYIP
D2EF;HANGUL SYLLABLE TYIH
D2F0;HANGUL SYLLABLE TI
D2F1;HANGUL SYLLABLE TIG
D2F2;HANGUL SYLLABLE TIGG
D2F3;HANGUL SYLLABLE TIGS
D2F4;HANGUL SYLLABLE TIN
D2F5;HANGUL SYLLABLE TINJ
D2F6;HANGUL SYLLABLE TINH
D2F7;HANGUL SYLLABLE TID
D2F8;HANGUL SYLLABLE TIL
D2F9;HANGUL SYLLABLE TILG
D2FA;HANGUL SYLLABLE TILM
D2FB;HANGUL SYLLABLE TILB
D2FC;HANGUL SYLLABLE TILS
D2FD;HANGUL SYLLABLE TILT
D2FE;HANGUL SYLLABLE TILP
D2FF;HANGUL SYLLABLE TILH
D300;HANGUL SYLLABLE TIM
D301;HANGUL SYLLABLE TIB
D302;HANGUL SYLLABLE TIBS
D303;HANGUL SYLLABLE TIS
D304;HANGUL SYLLABLE TISS
D305;HANGUL SYLLABLE TING
D306;HANGUL SYLLABLE TIJ
D307;HANGUL SYLLABLE TIC
D308;HANGUL SYLLABLE TIK
D309;HANGUL SYLLABLE TIT
D30A;HANGUL SYLLABLE TIP
D30B;HANGUL SYLLABLE TIH
D30C;HANGUL SYLLABLE PA
D30D;HANGUL SYLLABLE PAG
D30E;HANGUL SYLLABLE PAGG
D30F;HANGUL SYLLABLE PAGS
D310;HANGUL SYLLABLE PAN
D311;HANGUL SYLLABLE PANJ
D312;HANGUL SYLLABLE PANH
D313;HANGUL SYLLABLE PAD
D314;HANGUL SYLLABLE PAL
D315;HANGUL SYLLABLE PALG
D316;HANGUL SYLLABLE PALM
D317;HANGUL SYLLABLE PALB
D318;HANGUL SYLLABLE PALS
D319;HANGUL SYLLABLE PALT
D31A;HANGUL SYLLABLE PALP
D31B;HANGUL SYLLABLE PALH
D31C;HANGUL SYLLABLE PAM
D31D;HANGUL SYLLABLE PAB
D31E;HANGUL SYLLABLE PABS
D31F;HANGUL SYLLABLE PAS
D320;HANGUL SYLLABLE PASS
D321;HANGUL SYLLABLE PANG
D322;HANGUL SYLLABLE PAJ
D323;HANGUL SYLLABLE PAC
D324;HANGUL SYLLABLE PAK
D325;HANGUL SYLLABLE PAT
D326;HANGUL SYLLABLE PAP
D327;HANGUL SYLLABLE PAH
D328;HANGUL SYLLABLE PAE
D329;HANGUL SYLLABLE PAEG
D32A;HANGUL SYLLABLE PAEGG
D32B;HANGUL SYLLABLE PAEGS
D32C;HANGUL SYLLABLE PAEN
D32D;HANGUL SYLLABLE PAENJ
D32E;HANGUL SYLLABLE PAENH
D32F;HANGUL SYLLABLE PAED
D330;HANGUL SYLLABLE PAEL
D331;HANGUL SYLLABLE PAELG
D332;HANGUL SYLLABLE PAELM
D333;HANGUL SYLLABLE PAELB
D334;HANGUL SYLLABLE PAELS
D335;HANGUL SYLLABLE PAELT
D336;HANGUL SYLLABLE PAELP
D337;HANGUL SYLLABLE PAELH
D338;HANGUL SYLLABLE PAEM
D339;HANGUL SYLLABLE PAEB
D33A;HANGUL SYLLABLE PAEBS
D33B;HANGUL SYLLABLE PAES
D33C;HANGUL SYLLABLE PAESS
D33D;HANGUL SYLLABLE PAENG
D33E;HANGUL SYLLABLE PAEJ
D33F;HANGUL SYLLABLE PAEC
D340;HANGUL SYLLABLE PAEK
D341;HANGUL SYLLABLE PAET
D342;HANGUL SYLLABLE PAEP
D343;HANGUL SYLLABLE PAEH
D344;HANGUL SYLLABLE PYA
D345;HANGUL SYLLABLE PYAG
D346;HANGUL SYLLABLE PYAGG
D347;HANGUL SYLLABLE PYAGS
D348;HANGUL SYLLABLE PYAN
D349;HANGUL SYLLABLE PYANJ
D34A;HANGUL SYLLABLE PYANH
D34B;HANGUL SYLLABLE PYAD
D34C;HANGUL SYLLABLE PYAL
D34D;HANGUL SYLLABLE PYALG
D34E;HANGUL SYLLABLE PYALM
D34F;HANGUL SYLLABLE PYALB
D350;HANGUL SYLLABLE PYALS
D351;HANGUL SYLLABLE PYALT
D352;HANGUL SYLLABLE PYALP
D353;HANGUL SYLLABLE PYALH
D354;HANGUL SYLLABLE PYAM
D355;HANGUL SYLLABLE PYAB
D356;HANGUL SYLLABLE PYABS
D357;HANGUL SYLLABLE PYAS
D358;HANGUL SYLLABLE PYASS
D359;HANGUL SYLLABLE PYANG
D35A;HANGUL SYLLABLE PYAJ
D35B;HANGUL SYLLABLE PYAC
D35C;HANGUL SYLLABLE PYAK
D35D;HANGUL SYLLABLE PYAT
D35E;HANGUL SYLLABLE PYAP
D35F;HANGUL SYLLABLE PYAH
D360;HANGUL SYLLABLE PYAE
D361;HANGUL SYLLABLE PYAEG
D362;HANGUL SYLLABLE PYAEGG
D363;HANGUL SYLLABLE PYAEGS
D364;HANGUL SYLLABLE PYAEN
D365;HANGUL SYLLABLE PYAENJ
D366;HANGUL SYLLABLE PYAENH
D367;HANGUL SYLLABLE PYAED
D368;HANGUL SYLLABLE PYAEL
D369;HANGUL SYLLABLE PYAELG
D36A;HANGUL SYLLABLE PYAELM
D36B;HANGUL SYLLABLE PYAELB
D36C;HANGUL SYLLABLE PYAELS
D36D;HANGUL SYLLABLE PYAELT
D36E;HANGUL SYLLABLE PYAELP
D36F;HANGUL SYLLABLE PYAELH
D370;HANGUL SYLLABLE PYAEM
D371;HANGUL SYLLABLE PYAEB
D372;HANGUL SYLLABLE PYAEBS
D373;HANGUL SYLLABLE PYAES
D374;HANGUL SYLLABLE PYAESS
D375;HANGUL SYLLABLE PYAENG
D376;HANGUL SYLLABLE PYAEJ
D377;HANGUL SYLLABLE PYAEC
D378;HANGUL SYLLABLE PYAEK
D379;HANGUL SYLLABLE PYAET
D37A;HANGUL SYLLABLE PYAEP
D37B;HANGUL SYLLABLE PYAEH
D37C;HANGUL SYLLABLE PEO
D37D;HANGUL SYLLABLE PEOG
D37E;HANGUL SYLLABLE PEOGG
D37F;HANGUL SYLLABLE PEOGS
D380;HANGUL SYLLABLE PEON
D381;HANGUL SYLLABLE PEONJ
D382;HANGUL SYLLABLE PEONH
D383;HANGUL SYLLABLE PEOD
D384;HANGUL SYLLABLE PEOL
D385;HANGUL SYLLABLE PEOLG
D386;HANGUL SYLLABLE PEOLM
D387;HANGUL SYLLABLE PEOLB
D388;HANGUL SYLLABLE PEOLS
D389;HANGUL SYLLABLE PEOLT
D38A;HANGUL SYLLABLE PEOLP
D38B;HANGUL SYLLABLE PEOLH
D38C;HANGUL SYLLABLE PEOM
D38D;HANGUL SYLLABLE PEOB
D38E;HANGUL SYLLABLE PEOBS
D38F;HANGUL SYLLABLE PEOS
D390;HANGUL SYLLABLE PEOSS
D391;HANGUL SYLLABLE PEONG
D392;HANGUL SYLLABLE PEOJ
D393;HANGUL SYLLABLE PEOC
D394;HANGUL SYLLABLE PEOK
D395;HANGUL SYLLABLE PEOT
D396;HANGUL SYLLABLE PEOP
D397;HANGUL SYLLABLE PEOH
D398;HANGUL SYLLABLE PE
D399;HANGUL SYLLABLE PEG
D39A;HANGUL SYLLABLE PEGG
D39B;HANGUL SYLLABLE PEGS
D39C;HANGUL SYLLABLE PEN
D39D;HANGUL SYLLABLE PENJ
D39E;HANGUL SYLLABLE PENH
D39F;HANGUL SYLLABLE PED
D3A0;HANGUL SYLLABLE PEL
D3A1;HANGUL SYLLABLE PELG
D3A2;HANGUL SYLLABLE PELM
D3A3;HANGUL SYLLABLE PELB
D3A4;HANGUL SYLLABLE PELS
D3A5;HANGUL SYLLABLE PELT
D3A6;HANGUL SYLLABLE PELP
D3A7;HANGUL SYLLABLE PELH
D3A8;HANGUL SYLLABLE PEM
D3A9;HANGUL SYLLABLE PEB
D3AA;HANGUL SYLLABLE PEBS
D3AB;HANGUL SYLLABLE PES
D3AC;HANGUL SYLLABLE PESS
D3AD;HANGUL SYLLABLE PENG
D3AE;HANGUL SYLLABLE PEJ
D3AF;HANGUL SYLLABLE PEC
D3B0;HANGUL SYLLABLE PEK
D3B1;HANGUL SYLLABLE PET
D3B2;HANGUL SYLLABLE PEP
D3B3;HANGUL SYLLABLE PEH
D3B4;HANGUL SYLLABLE PYEO
D3B5;HANGUL SYLLABLE PYEOG
D3B6;HANGUL SYLLABLE PYEOGG
D3B7;HANGUL SYLLABLE PYEOGS
D3B8;HANGUL SYLLABLE PYEON
D3B9;HANGUL SYLLABLE PYEONJ
D3BA;HANGUL SYLLABLE PYEONH
D3BB;HANGUL SYLLABLE PYEOD
D3BC;HANGUL SYLLABLE PYEOL
D3BD;HANGUL SYLLABLE PYEOLG
D3BE;HANGUL SYLLABLE PYEOLM
D3BF;HANGUL SYLLABLE PYEOLB
D3C0;HANGUL SYLLABLE PYEOLS
D3C1;HANGUL SYLLABLE PYEOLT
D3C2;HANGUL SYLLABLE PYEOLP
D3C3;HANGUL SYLLABLE PYEOLH
D3C4;HANGUL SYLLABLE PYEOM
D3C5;HANGUL SYLLABLE PYEOB
D3C6;HANGUL SYLLABLE PYEOBS
D3C7;HANGUL SYLLABLE PYEOS
D3C8;HANGUL SYLLABLE PYEOSS
D3C9;HANGUL SYLLABLE PYEONG
D3CA;HANGUL SYLLABLE PYEOJ
D3CB;HANGUL SYLLABLE PYEOC
D3CC;HANGUL SYLLABLE PYEOK
D3CD;HANGUL SYLLABLE PYEOT
D3CE;HANGUL SYLLABLE PYEOP
D3CF;HANGUL SYLLABLE PYEOH
D3D0;HANGUL SYLLABLE PYE
D3D1;HANGUL SYLLABLE PYEG
D3D2;HANGUL SYLLABLE PYEGG
D3D3;HANGUL SYLLABLE PYEGS
D3D4;HANGUL SYLLABLE PYEN
D3D5;HANGUL SYLLABLE PYENJ
D3D6;HANGUL SYLLABLE PYENH
D3D7;HANGUL SYLLABLE PYED
D3D8;HANGUL SYLLABLE PYEL
D3D9;HANGUL SYLLABLE PYELG
D3DA;HANGUL SYLLABLE PYELM
D3DB;HANGUL SYLLABLE PYELB
D3DC;HANGUL SYLLABLE PYELS
D3DD;HANGUL SYLLABLE PYELT
D3DE;HANGUL SYLLABLE PYELP
D3DF;HANGUL SYLLABLE PYELH
D3E0;HANGUL SYLLABLE PYEM
D3E1;HANGUL SYLLABLE PYEB
D3E2;HANGUL SYLLABLE PYEBS
D3E3;HANGUL SYLLABLE PYES
D3E4;HANGUL SYLLABLE PYESS
D3E5;HANGUL SYLLABLE PYENG
D3E6;HANGUL SYLLABLE PYEJ
D3E7;HANGUL SYLLABLE PYEC
D3E8;HANGUL SYLLABLE PYEK
D3E9;HANGUL SYLLABLE PYET
D3EA;HANGUL SYLLABLE PYEP
D3EB;HANGUL SYLLABLE PYEH
D3EC;HANGUL SYLLABLE PO
D3ED;HANGUL SYLLABLE POG
D3EE;HANGUL SYLLABLE POGG
D3EF;HANGUL SYLLABLE POGS
D3F0;HANGUL SYLLABLE PON
D3F1;HANGUL SYLLABLE PONJ
D3F2;HANGUL SYLLABLE PONH
D3F3;HANGUL SYLLABLE POD
D3F4;HANGUL SYLLABLE POL
D3F5;HANGUL SYLLABLE POLG
D3F6;HANGUL SYLLABLE POLM
D3F7;HANGUL SYLLABLE POLB
D3F8;HANGUL SYLLABLE POLS
D3F9;HANGUL SYLLABLE POLT
D3FA;HANGUL SYLLABLE POLP
D3FB;HANGUL SYLLABLE POLH
D3FC;HANGUL SYLLABLE POM
D3FD;HANGUL SYLLABLE POB
D3FE;HANGUL SYLLABLE POBS
D3FF;HANGUL SYLLABLE POS
D400;HANGUL SYLLABLE POSS
D401;HANGUL SYLLABLE PONG
D402;HANGUL SYLLABLE POJ
D403;HANGUL SYLLABLE POC
D404;HANGUL SYLLABLE POK
D405;HANGUL SYLLABLE POT
D406;HANGUL SYLLABLE POP
D407;HANGUL SYLLABLE POH
D408;HANGUL SYLLABLE PWA
D409;HANGUL SYLLABLE PWAG
D40A;HANGUL SYLLABLE PWAGG
D40B;HANGUL SYLLABLE PWAGS
D40C;HANGUL SYLLABLE PWAN
D40D;HANGUL SYLLABLE PWANJ
D40E;HANGUL SYLLABLE PWANH
D40F;HANGUL SYLLABLE PWAD
D410;HANGUL SYLLABLE PWAL
D411;HANGUL SYLLABLE PWALG
D412;HANGUL SYLLABLE PWALM
D413;HANGUL SYLLABLE PWALB
D414;HANGUL SYLLABLE PWALS
D415;HANGUL SYLLABLE PWALT
D416;HANGUL SYLLABLE PWALP
D417;HANGUL SYLLABLE PWALH
D418;HANGUL SYLLABLE PWAM
D419;HANGUL SYLLABLE PWAB
D41A;HANGUL SYLLABLE PWABS
D41B;HANGUL SYLLABLE PWAS
D41C;HANGUL SYLLABLE PWASS
D41D;HANGUL SYLLABLE PWANG
D41E;HANGUL SYLLABLE PWAJ
D41F;HANGUL SYLLABLE PWAC
D420;HANGUL SYLLABLE PWAK
D421;HANGUL SYLLABLE PWAT
D422;HANGUL SYLLABLE PWAP
D423;HANGUL SYLLABLE PWAH
D424;HANGUL SYLLABLE PWAE
D425;HANGUL SYLLABLE PWAEG
D426;HANGUL SYLLABLE PWAEGG
D427;HANGUL SYLLABLE PWAEGS
D428;HANGUL SYLLABLE PWAEN
D429;HANGUL SYLLABLE PWAENJ
D42A;HANGUL SYLLABLE PWAENH
D42B;HANGUL SYLLABLE PWAED
D42C;HANGUL SYLLABLE PWAEL
D42D;HANGUL SYLLABLE PWAELG
D42E;HANGUL SYLLABLE PWAELM
D42F;HANGUL SYLLABLE PWAELB
D430;HANGUL SYLLABLE PWAELS
D431;HANGUL SYLLABLE PWAELT
D432;HANGUL SYLLABLE PWAELP
D433;HANGUL SYLLABLE PWAELH
D434;HANGUL SYLLABLE PWAEM
D435;HANGUL SYLLABLE PWAEB
D436;HANGUL SYLLABLE PWAEBS
D437;HANGUL SYLLABLE PWAES
D438;HANGUL SYLLABLE PWAESS
D439;HANGUL SYLLABLE PWAENG
D43A;HANGUL SYLLABLE PWAEJ
D43B;HANGUL SYLLABLE PWAEC
D43C;HANGUL SYLLABLE PWAEK
D43D;HANGUL SYLLABLE PWAET
D43E;HANGUL SYLLABLE PWAEP
D43F;HANGUL SYLLABLE PWAEH
D440;HANGUL SYLLABLE POE
D441;HANGUL SYLLABLE POEG
D442;HANGUL SYLLABLE POEGG
D443;HANGUL SYLLABLE POEGS
D444;HANGUL SYLLABLE POEN
D445;HANGUL SYLLABLE POENJ
D446;HANGUL SYLLABLE POENH
D447;HANGUL SYLLABLE POED
D448;HANGUL SYLLABLE POEL
D449;HANGUL SYLLABLE POELG
D44A;HANGUL SYLLABLE POELM
D44B;HANGUL SYLLABLE POELB
D44C;HANGUL SYLLABLE POELS
D44D;HANGUL SYLLABLE POELT
D44E;HANGUL SYLLABLE POELP
D44F;HANGUL SYLLABLE POELH
D450;HANGUL SYLLABLE POEM
D451;HANGUL SYLLABLE POEB
D452;HANGUL SYLLABLE POEBS
D453;HANGUL SYLLABLE POES
D454;HANGUL SYLLABLE POESS
D455;HANGUL SYLLABLE POENG
D456;HANGUL SYLLABLE POEJ
D457;HANGUL SYLLABLE POEC
D458;HANGUL SYLLABLE POEK
D459;HANGUL SYLLABLE POET
D45A;HANGUL SYLLABLE POEP
D45B;HANGUL SYLLABLE POEH
D45C;HANGUL SYLLABLE PYO
D45D;HANGUL SYLLABLE PYOG
D45E;HANGUL SYLLABLE PYOGG
D45F;HANGUL SYLLABLE PYOGS
D460;HANGUL SYLLABLE PYON
D461;HANGUL SYLLABLE PYONJ
D462;HANGUL SYLLABLE PYONH
D463;HANGUL SYLLABLE PYOD
D464;HANGUL SYLLABLE PYOL
D465;HANGUL SYLLABLE PYOLG
D466;HANGUL SYLLABLE PYOLM
D467;HANGUL SYLLABLE PYOLB
D468;HANGUL SYLLABLE PYOLS
D469;HANGUL SYLLABLE PYOLT
D46A;HANGUL SYLLABLE PYOLP
D46B;HANGUL SYLLABLE PYOLH
D46C;HANGUL SYLLABLE PYOM
D46D;HANGUL SYLLABLE PYOB
D46E;HANGUL SYLLABLE PYOBS
D46F;HANGUL SYLLABLE PYOS
D470;HANGUL SYLLABLE PYOSS
D471;HANGUL SYLLABLE PYONG
D472;HANGUL SYLLABLE PYOJ
D473;HANGUL SYLLABLE PYOC
D474;HANGUL SYLLABLE PYOK
D475;HANGUL SYLLABLE PYOT
D476;HANGUL SYLLABLE PYOP
D477;HANGUL SYLLABLE PYOH
D478;HANGUL SYLLABLE PU
D479;HANGUL SYLLABLE PUG
D47A;HANGUL SYLLABLE PUGG
D47B;HANGUL SYLLABLE PUGS
D47C;HANGUL SYLLABLE PUN
D47D;HANGUL SYLLABLE PUNJ
D47E;HANGUL SYLLABLE PUNH
D47F;HANGUL SYLLABLE PUD
D480;HANGUL SYLLABLE PUL
D481;HANGUL SYLLABLE PULG
D482;HANGUL SYLLABLE PULM
D483;HANGUL SYLLABLE PULB
D484;HANGUL SYLLABLE PULS
D485;HANGUL SYLLABLE PULT
D486;HANGUL SYLLABLE PULP
D487;HANGUL SYLLABLE PULH
D488;HANGUL SYLLABLE PUM
D489;HANGUL SYLLABLE PUB
D48A;HANGUL SYLLABLE PUBS
D48B;HANGUL SYLLABLE PUS
D48C;HANGUL SYLLABLE PUSS
D48D;HANGUL SYLLABLE PUNG
D48E;HANGUL SYLLABLE PUJ
D48F;HANGUL SYLLABLE PUC
D490;HANGUL SYLLABLE PUK
D491;HANGUL SYLLABLE PUT
D492;HANGUL SYLLABLE PUP
D493;HANGUL SYLLABLE PUH
D494;HANGUL SYLLABLE PWEO
D495;HANGUL SYLLABLE PWEOG
D496;HANGUL SYLLABLE PWEOGG
D497;HANGUL SYLLABLE PWEOGS
D498;HANGUL SYLLABLE PWEON
D499;HANGUL SYLLABLE PWEONJ
D49A;HANGUL SYLLABLE PWEONH
D49B;HANGUL SYLLABLE PWEOD
D49C;HANGUL SYLLABLE PWEOL
D49D;HANGUL SYLLABLE PWEOLG
D49E;HANGUL SYLLABLE PWEOLM
D49F;HANGUL SYLLABLE PWEOLB
D4A0;HANGUL SYLLABLE PWEOLS
D4A1;HANGUL SYLLABLE PWEOLT
D4A2;HANGUL SYLLABLE PWEOLP
D4A3;HANGUL SYLLABLE PWEOLH
D4A4;HANGUL SYLLABLE PWEOM
D4A5;HANGUL SYLLABLE PWEOB
D4A6;HANGUL SYLLABLE PWEOBS
D4A7;HANGUL SYLLABLE PWEOS
D4A8;HANGUL SYLLABLE PWEOSS
D4A9;HANGUL SYLLABLE PWEONG
D4AA;HANGUL SYLLABLE PWEOJ
D4AB;HANGUL SYLLABLE PWEOC
D4AC;HANGUL SYLLABLE PWEOK
D4AD;HANGUL SYLLABLE PWEOT
D4AE;HANGUL SYLLABLE PWEOP
D4AF;HANGUL SYLLABLE PWEOH
D4B0;HANGUL SYLLABLE PWE
D4B1;HANGUL SYLLABLE PWEG
D4B2;HANGUL SYLLABLE PWEGG
D4B3;HANGUL SYLLABLE PWEGS
D4B4;HANGUL SYLLABLE PWEN
D4B5;HANGUL SYLLABLE PWENJ
D4B6;HANGUL SYLLABLE PWENH
D4B7;HANGUL SYLLABLE PWED
D4B8;HANGUL SYLLABLE PWEL
D4B9;HANGUL SYLLABLE PWELG
D4BA;HANGUL SYLLABLE PWELM
D4BB;HANGUL SYLLABLE PWELB
D4BC;HANGUL SYLLABLE PWELS
D4BD;HANGUL SYLLABLE PWELT
D4BE;HANGUL SYLLABLE PWELP
D4BF;HANGUL SYLLABLE PWELH
D4C0;HANGUL SYLLABLE PWEM
D4C1;HANGUL SYLLABLE PWEB
D4C2;HANGUL SYLLABLE PWEBS
D4C3;HANGUL SYLLABLE PWES
D4C4;HANGUL SYLLABLE PWESS
D4C5;HANGUL SYLLABLE PWENG
D4C6;HANGUL SYLLABLE PWEJ
D4C7;HANGUL SYLLABLE PWEC
D4C8;HANGUL SYLLABLE PWEK
D4C9;HANGUL SYLLABLE PWET
D4CA;HANGUL SYLLABLE PWEP
D4CB;HANGUL SYLLABLE PWEH
D4CC;HANGUL SYLLABLE PWI
D4CD;HANGUL SYLLABLE PWIG
D4CE;HANGUL SYLLABLE PWIGG
D4CF;HANGUL SYLLABLE PWIGS
D4D0;HANGUL SYLLABLE PWIN
D4D1;HANGUL SYLLABLE PWINJ
D4D2;HANGUL SYLLABLE PWINH
D4D3;HANGUL SYLLABLE PWID
D4D4;HANGUL SYLLABLE PWIL
D4D5;HANGUL SYLLABLE PWILG
D4D6;HANGUL SYLLABLE PWILM
D4D7;HANGUL SYLLABLE PWILB
D4D8;HANGUL SYLLABLE PWILS
D4D9;HANGUL SYLLABLE PWILT
D4DA;HANGUL SYLLABLE PWILP
D4DB;HANGUL SYLLABLE PWILH
D4DC;HANGUL SYLLABLE PWIM
D4DD;HANGUL SYLLABLE PWIB
D4DE;HANGUL SYLLABLE PWIBS
D4DF;HANGUL SYLLABLE PWIS
D4E0;HANGUL SYLLABLE PWISS
D4E1;HANGUL SYLLABLE PWING
D4E2;HANGUL SYLLABLE PWIJ
D4E3;HANGUL SYLLABLE PWIC
D4E4;HANGUL SYLLABLE PWIK
D4E5;HANGUL SYLLABLE PWIT
D4E6;HANGUL SYLLABLE PWIP
D4E7;HANGUL SYLLABLE PWIH
D4E8;HANGUL SYLLABLE PYU
D4E9;HANGUL SYLLABLE PYUG
D4EA;HANGUL SYLLABLE PYUGG
D4EB;HANGUL SYLLABLE PYUGS
D4EC;HANGUL SYLLABLE PYUN
D4ED;HANGUL SYLLABLE PYUNJ
D4EE;HANGUL SYLLABLE PYUNH
D4EF;HANGUL SYLLABLE PYUD
D4F0;HANGUL SYLLABLE PYUL
D4F1;HANGUL SYLLABLE PYULG
D4F2;HANGUL SYLLABLE PYULM
D4F3;HANGUL SYLLABLE PYULB
D4F4;HANGUL SYLLABLE PYULS
D4F5;HANGUL SYLLABLE PYULT
D4F6;HANGUL SYLLABLE PYULP
D4F7;HANGUL SYLLABLE PYULH
D4F8;HANGUL SYLLABLE PYUM
D4F9;HANGUL SYLLABLE PYUB
D4FA;HANGUL SYLLABLE PYUBS
D4FB;HANGUL SYLLABLE PYUS
D4FC;HANGUL SYLLABLE PYUSS
D4FD;HANGUL SYLLABLE PYUNG
D4FE;HANGUL SYLLABLE PYUJ
D4FF;HANGUL SYLLABLE PYUC
D500;HANGUL SYLLABLE PYUK
D501;HANGUL SYLLABLE PYUT
D502;HANGUL SYLLABLE PYUP
D503;HANGUL SYLLABLE PYUH
D504;HANGUL SYLLABLE PEU
D505;HANGUL SYLLABLE PEUG
D506;HANGUL SYLLABLE PEUGG
D507;HANGUL SYLLABLE PEUGS
D508;HANGUL SYLLABLE PEUN
D509;HANGUL SYLLABLE PEUNJ
D50A;HANGUL SYLLABLE PEUNH
D50B;HANGUL SYLLABLE PEUD
D50C;HANGUL SYLLABLE PEUL
D50D;HANGUL SYLLABLE PEULG
D50E;HANGUL SYLLABLE PEULM
D50F;HANGUL SYLLABLE PEULB
D510;HANGUL SYLLABLE PEULS
D511;HANGUL SYLLABLE PEULT
D512;HANGUL SYLLABLE PEULP
D513;HANGUL SYLLABLE PEULH
D514;HANGUL SYLLABLE PEUM
D515;HANGUL SYLLABLE PEUB
D516;HANGUL SYLLABLE PEUBS
D517;HANGUL SYLLABLE PEUS
D518;HANGUL SYLLABLE PEUSS
D519;HANGUL SYLLABLE PEUNG
D51A;HANGUL SYLLABLE PEUJ
D51B;HANGUL SYLLABLE PEUC
D51C;HANGUL SYLLABLE PEUK
D51D;HANGUL SYLLABLE PEUT
D51E;HANGUL SYLLABLE PEUP
D51F;HANGUL SYLLABLE PEUH
D520;HANGUL SYLLABLE PYI
D521;HANGUL SYLLABLE PYIG
D522;HANGUL SYLLABLE PYIGG
D523;HANGUL SYLLABLE PYIGS
D524;HANGUL SYLLABLE PYIN
D525;HANGUL SYLLABLE PYINJ
D526;HANGUL SYLLABLE PYINH
D527;HANGUL SYLLABLE PYID
D528;HANGUL SYLLABLE PYIL
D529;HANGUL SYLLABLE PYILG
D52A;HANGUL SYLLABLE PYILM
D52B;HANGUL SYLLABLE PYILB
D52C;HANGUL SYLLABLE PYILS
D52D;HANGUL SYLLABLE PYILT
D52E;HANGUL SYLLABLE PYILP
D52F;HANGUL SYLLABLE PYILH
D530;HANGUL SYLLABLE PYIM
D531;HANGUL SYLLABLE PYIB
D532;HANGUL SYLLABLE PYIBS
D533;HANGUL SYLLABLE PYIS
D534;HANGUL SYLLABLE PYISS
D535;HANGUL SYLLABLE PYING
D536;HANGUL SYLLABLE PYIJ
D537;HANGUL SYLLABLE PYIC
D538;HANGUL SYLLABLE PYIK
D539;HANGUL SYLLABLE PYIT
D53A;HANGUL SYLLABLE PYIP
D53B;HANGUL SYLLABLE PYIH
D53C;HANGUL SYLLABLE PI
D53D;HANGUL SYLLABLE PIG
D53E;HANGUL SYLLABLE PIGG
D53F;HANGUL SYLLABLE PIGS
D540;HANGUL SYLLABLE PIN
D541;HANGUL SYLLABLE PINJ
D542;HANGUL SYLLABLE PINH
D543;HANGUL SYLLABLE PID
D544;HANGUL SYLLABLE PIL
D545;HANGUL SYLLABLE PILG
D546;HANGUL SYLLABLE PILM
D547;HANGUL SYLLABLE PILB
D548;HANGUL SYLLABLE PILS
D549;HANGUL SYLLABLE PILT
D54A;HANGUL SYLLABLE PILP
D54B;HANGUL SYLLABLE PILH
D54C;HANGUL SYLLABLE PIM
D54D;HANGUL SYLLABLE PIB
D54E;HANGUL SYLLABLE PIBS
D54F;HANGUL SYLLABLE PIS
D550;HANGUL SYLLABLE PISS
D551;HANGUL SYLLABLE PING
D552;HANGUL SYLLABLE PIJ
D553;HANGUL SYLLABLE PIC
D554;HANGUL SYLLABLE PIK
D555;HANGUL SYLLABLE PIT
D556;HANGUL SYLLABLE PIP
D557;HANGUL SYLLABLE PIH
D558;HANGUL SYLLABLE HA
D559;HANGUL SYLLABLE HAG
D55A;HANGUL SYLLABLE HAGG
D55B;HANGUL SYLLABLE HAGS
D55C;HANGUL SYLLABLE HAN
D55D;HANGUL SYLLABLE HANJ
D55E;HANGUL SYLLABLE HANH
D55F;HANGUL SYLLABLE HAD
D560;HANGUL SYLLABLE HAL
D561;HANGUL SYLLABLE HALG
D562;HANGUL SYLLABLE HALM
D563;HANGUL SYLLABLE HALB
D564;HANGUL SYLLABLE HALS
D565;HANGUL SYLLABLE HALT
D566;HANGUL SYLLABLE HALP
D567;HANGUL SYLLABLE HALH
D568;HANGUL SYLLABLE HAM
D569;HANGUL SYLLABLE HAB
D56A;HANGUL SYLLABLE HABS
D56B;HANGUL SYLLABLE HAS
D56C;HANGUL SYLLABLE HASS
D56D;HANGUL SYLLABLE HANG
D56E;HANGUL SYLLABLE HAJ
D56F;HANGUL SYLLABLE HAC
D570;HANGUL SYLLABLE HAK
D571;HANGUL SYLLABLE HAT
D572;HANGUL SYLLABLE HAP
D573;HANGUL SYLLABLE HAH
D574;HANGUL SYLLABLE HAE
D575;HANGUL SYLLABLE HAEG
D576;HANGUL SYLLABLE HAEGG
D577;HANGUL SYLLABLE HAEGS
D578;HANGUL SYLLABLE HAEN
D579;HANGUL SYLLABLE HAENJ
D57A;HANGUL SYLLABLE HAENH
D57B;HANGUL SYLLABLE HAED
D57C;HANGUL SYLLABLE HAEL
D57D;HANGUL SYLLABLE HAELG
D57E;HANGUL SYLLABLE HAELM
D57F;HANGUL SYLLABLE HAELB
D580;HANGUL SYLLABLE HAELS
D581;HANGUL SYLLABLE HAELT
D582;HANGUL SYLLABLE HAELP
D583;HANGUL SYLLABLE HAELH
D584;HANGUL SYLLABLE HAEM
D585;HANGUL SYLLABLE HAEB
D586;HANGUL SYLLABLE HAEBS
D587;HANGUL SYLLABLE HAES
D588;HANGUL SYLLABLE HAESS
D589;HANGUL SYLLABLE HAENG
D58A;HANGUL SYLLABLE HAEJ
D58B;HANGUL SYLLABLE HAEC
D58C;HANGUL SYLLABLE HAEK
D58D;HANGUL SYLLABLE HAET
D58E;HANGUL SYLLABLE HAEP
D58F;HANGUL SYLLABLE HAEH
D590;HANGUL SYLLABLE HYA
D591;HANGUL SYLLABLE HYAG
D592;HANGUL SYLLABLE HYAGG
D593;HANGUL SYLLABLE HYAGS
D594;HANGUL SYLLABLE HYAN
D595;HANGUL SYLLABLE HYANJ
D596;HANGUL SYLLABLE HYANH
D597;HANGUL SYLLABLE HYAD
D598;HANGUL SYLLABLE HYAL
D599;HANGUL SYLLABLE HYALG
D59A;HANGUL SYLLABLE HYALM
D59B;HANGUL SYLLABLE HYALB
D59C;HANGUL SYLLABLE HYALS
D59D;HANGUL SYLLABLE HYALT
D59E;HANGUL SYLLABLE HYALP
D59F;HANGUL SYLLABLE HYALH
D5A0;HANGUL SYLLABLE HYAM
D5A1;HANGUL SYLLABLE HYAB
D5A2;HANGUL SYLLABLE HYABS
D5A3;HANGUL SYLLABLE HYAS
D5A4;HANGUL SYLLABLE HYASS
D5A5;HANGUL SYLLABLE HYANG
D5A6;HANGUL SYLLABLE HYAJ
D5A7;HANGUL SYLLABLE HYAC
D5A8;HANGUL SYLLABLE HYAK
D5A9;HANGUL SYLLABLE HYAT
D5AA;HANGUL SYLLABLE HYAP
D5AB;HANGUL SYLLABLE HYAH
D5AC;HANGUL SYLLABLE HYAE
D5AD;HANGUL SYLLABLE HYAEG
D5AE;HANGUL SYLLABLE HYAEGG
D5AF;HANGUL SYLLABLE HYAEGS
D5B0;HANGUL SYLLABLE HYAEN
D5B1;HANGUL SYLLABLE HYAENJ
D5B2;HANGUL SYLLABLE HYAENH
D5B3;HANGUL SYLLABLE HYAED
D5B4;HANGUL SYLLABLE HYAEL
D5B5;HANGUL SYLLABLE HYAELG
D5B6;HANGUL SYLLABLE HYAELM
D5B7;HANGUL SYLLABLE HYAELB
D5B8;HANGUL SYLLABLE HYAELS
D5B9;HANGUL SYLLABLE HYAELT
D5BA;HANGUL SYLLABLE HYAELP
D5BB;HANGUL SYLLABLE HYAELH
D5BC;HANGUL SYLLABLE HYAEM
D5BD;HANGUL SYLLABLE HYAEB
D5BE;HANGUL SYLLABLE HYAEBS
D5BF;HANGUL SYLLABLE HYAES
D5C0;HANGUL SYLLABLE HYAESS
D5C1;HANGUL SYLLABLE HYAENG
D5C2;HANGUL SYLLABLE HYAEJ
D5C3;HANGUL SYLLABLE HYAEC
D5C4;HANGUL SYLLABLE HYAEK
D5C5;HANGUL SYLLABLE HYAET
D5C6;HANGUL SYLLABLE HYAEP
D5C7;HANGUL SYLLABLE HYAEH
D5C8;HANGUL SYLLABLE HEO
D5C9;HANGUL SYLLABLE HEOG
D5CA;HANGUL SYLLABLE HEOGG
D5CB;HANGUL SYLLABLE HEOGS
D5CC;HANGUL SYLLABLE HEON
D5CD;HANGUL SYLLABLE HEONJ
D5CE;HANGUL SYLLABLE HEONH
D5CF;HANGUL SYLLABLE HEOD
D5D0;HANGUL SYLLABLE HEOL
D5D1;HANGUL SYLLABLE HEOLG
D5D2;HANGUL SYLLABLE HEOLM
D5D3;HANGUL SYLLABLE HEOLB
D5D4;HANGUL SYLLABLE HEOLS
D5D5;HANGUL SYLLABLE HEOLT
D5D6;HANGUL SYLLABLE HEOLP
D5D7;HANGUL SYLLABLE HEOLH
D5D8;HANGUL SYLLABLE HEOM
D5D9;HANGUL SYLLABLE HEOB
D5DA;HANGUL SYLLABLE HEOBS
D5DB;HANGUL SYLLABLE HEOS
D5DC;HANGUL SYLLABLE HEOSS
D5DD;HANGUL SYLLABLE HEONG
D5DE;HANGUL SYLLABLE HEOJ
D5DF;HANGUL SYLLABLE HEOC
D5E0;HANGUL SYLLABLE HEOK
D5E1;HANGUL SYLLABLE HEOT
D5E2;HANGUL SYLLABLE HEOP
D5E3;HANGUL SYLLABLE HEOH
D5E4;HANGUL SYLLABLE HE
D5E5;HANGUL SYLLABLE HEG
D5E6;HANGUL SYLLABLE HEGG
D5E7;HANGUL SYLLABLE HEGS
D5E8;HANGUL SYLLABLE HEN
D5E9;HANGUL SYLLABLE HENJ
D5EA;HANGUL SYLLABLE HENH
D5EB;HANGUL SYLLABLE HED
D5EC;HANGUL SYLLABLE HEL
D5ED;HANGUL SYLLABLE HELG
D5EE;HANGUL SYLLABLE HELM
D5EF;HANGUL SYLLABLE HELB
D5F0;HANGUL SYLLABLE HELS
D5F1;HANGUL SYLLABLE HELT
D5F2;HANGUL SYLLABLE HELP
D5F3;HANGUL SYLLABLE HELH
D5F4;HANGUL SYLLABLE HEM
D5F5;HANGUL SYLLABLE HEB
D5F6;HANGUL SYLLABLE HEBS
D5F7;HANGUL SYLLABLE HES
D5F8;HANGUL SYLLABLE HESS
D5F9;HANGUL SYLLABLE HENG
D5FA;HANGUL SYLLABLE HEJ
D5FB;HANGUL SYLLABLE HEC
D5FC;HANGUL SYLLABLE HEK
D5FD;HANGUL SYLLABLE HET
D5FE;HANGUL SYLLABLE HEP
D5FF;HANGUL SYLLABLE HEH
D600;HANGUL SYLLABLE HYEO
D601;HANGUL SYLLABLE HYEOG
D602;HANGUL SYLLABLE HYEOGG
D603;HANGUL SYLLABLE HYEOGS
D604;HANGUL SYLLABLE HYEON
D605;HANGUL SYLLABLE HYEONJ
D606;HANGUL SYLLABLE HYEONH
D607;HANGUL SYLLABLE HYEOD
D608;HANGUL SYLLABLE HYEOL
D609;HANGUL SYLLABLE HYEOLG
D60A;HANGUL SYLLABLE HYEOLM
D60B;HANGUL SYLLABLE HYEOLB
D60C;HANGUL SYLLABLE HYEOLS
D60D;HANGUL SYLLABLE HYEOLT
D60E;HANGUL SYLLABLE HYEOLP
D60F;HANGUL SYLLABLE HYEOLH
D610;HANGUL SYLLABLE HYEOM
D611;HANGUL SYLLABLE HYEOB
D612;HANGUL SYLLABLE HYEOBS
D613;HANGUL SYLLABLE HYEOS
D614;HANGUL SYLLABLE HYEOSS
D615;HANGUL SYLLABLE HYEONG
D616;HANGUL SYLLABLE HYEOJ
D617;HANGUL SYLLABLE HYEOC
D618;HANGUL SYLLABLE HYEOK
D619;HANGUL SYLLABLE HYEOT
D61A;HANGUL SYLLABLE HYEOP
D61B;HANGUL SYLLABLE HYEOH
D61C;HANGUL SYLLABLE HYE
D61D;HANGUL SYLLABLE HYEG
D61E;HANGUL SYLLABLE HYEGG
D61F;HANGUL SYLLABLE HYEGS
D620;HANGUL SYLLABLE HYEN
D621;HANGUL SYLLABLE HYENJ
D622;HANGUL SYLLABLE HYENH
D623;HANGUL SYLLABLE HYED
D624;HANGUL SYLLABLE HYEL
D625;HANGUL SYLLABLE HYELG
D626;HANGUL SYLLABLE HYELM
D627;HANGUL SYLLABLE HYELB
D628;HANGUL SYLLABLE HYELS
D629;HANGUL SYLLABLE HYELT
D62A;HANGUL SYLLABLE HYELP
D62B;HANGUL SYLLABLE HYELH
D62C;HANGUL SYLLABLE HYEM
D62D;HANGUL SYLLABLE HYEB
D62E;HANGUL SYLLABLE HYEBS
D62F;HANGUL SYLLABLE HYES
D630;HANGUL SYLLABLE HYESS
D631;HANGUL SYLLABLE HYENG
D632;HANGUL SYLLABLE HYEJ
D633;HANGUL SYLLABLE HYEC
D634;HANGUL SYLLABLE HYEK
D635;HANGUL SYLLABLE HYET
D636;HANGUL SYLLABLE HYEP
D637;HANGUL SYLLABLE HYEH
D638;HANGUL SYLLABLE HO
D639;HANGUL SYLLABLE HOG
D63A;HANGUL SYLLABLE HOGG
D63B;HANGUL SYLLABLE HOGS
D63C;HANGUL SYLLABLE HON
D63D;HANGUL SYLLABLE HONJ
D63E;HANGUL SYLLABLE HONH
D63F;HANGUL SYLLABLE HOD
D640;HANGUL SYLLABLE HOL
D641;HANGUL SYLLABLE HOLG
D642;HANGUL SYLLABLE HOLM
D643;HANGUL SYLLABLE HOLB
D644;HANGUL SYLLABLE HOLS
D645;HANGUL SYLLABLE HOLT
D646;HANGUL SYLLABLE HOLP
D647;HANGUL SYLLABLE HOLH
D648;HANGUL SYLLABLE HOM
D649;HANGUL SYLLABLE HOB
D64A;HANGUL SYLLABLE HOBS
D64B;HANGUL SYLLABLE HOS
D64C;HANGUL SYLLABLE HOSS
D64D;HANGUL SYLLABLE HONG
D64E;HANGUL SYLLABLE HOJ
D64F;HANGUL SYLLABLE HOC
D650;HANGUL SYLLABLE HOK
D651;HANGUL SYLLABLE HOT
D652;HANGUL SYLLABLE HOP
D653;HANGUL SYLLABLE HOH
D654;HANGUL SYLLABLE HWA
D655;HANGUL SYLLABLE HWAG
D656;HANGUL SYLLABLE HWAGG
D657;HANGUL SYLLABLE HWAGS
D658;HANGUL SYLLABLE HWAN
D659;HANGUL SYLLABLE HWANJ
D65A;HANGUL SYLLABLE HWANH
D65B;HANGUL SYLLABLE HWAD
D65C;HANGUL SYLLABLE HWAL
D65D;HANGUL SYLLABLE HWALG
D65E;HANGUL SYLLABLE HWALM
D65F;HANGUL SYLLABLE HWALB
D660;HANGUL SYLLABLE HWALS
D661;HANGUL SYLLABLE HWALT
D662;HANGUL SYLLABLE HWALP
D663;HANGUL SYLLABLE HWALH
D664;HANGUL SYLLABLE HWAM
D665;HANGUL SYLLABLE HWAB
D666;HANGUL SYLLABLE HWABS
D667;HANGUL SYLLABLE HWAS
D668;HANGUL SYLLABLE HWASS
D669;HANGUL SYLLABLE HWANG
D66A;HANGUL SYLLABLE HWAJ
D66B;HANGUL SYLLABLE HWAC
D66C;HANGUL SYLLABLE HWAK
D66D;HANGUL SYLLABLE HWAT
D66E;HANGUL SYLLABLE HWAP
D66F;HANGUL SYLLABLE HWAH
D670;HANGUL SYLLABLE HWAE
D671;HANGUL SYLLABLE HWAEG
D672;HANGUL SYLLABLE HWAEGG
D673;HANGUL SYLLABLE HWAEGS
D674;HANGUL SYLLABLE HWAEN
D675;HANGUL SYLLABLE HWAENJ
D676;HANGUL SYLLABLE HWAENH
D677;HANGUL SYLLABLE HWAED
D678;HANGUL SYLLABLE HWAEL
D679;HANGUL SYLLABLE HWAELG
D67A;HANGUL SYLLABLE HWAELM
D67B;HANGUL SYLLABLE HWAELB
D67C;HANGUL SYLLABLE HWAELS
D67D;HANGUL SYLLABLE HWAELT
D67E;HANGUL SYLLABLE HWAELP
D67F;HANGUL SYLLABLE HWAELH
D680;HANGUL SYLLABLE HWAEM
D681;HANGUL SYLLABLE HWAEB
D682;HANGUL SYLLABLE HWAEBS
D683;HANGUL SYLLABLE HWAES
D684;HANGUL SYLLABLE HWAESS
D685;HANGUL SYLLABLE HWAENG
D686;HANGUL SYLLABLE HWAEJ
D687;HANGUL SYLLABLE HWAEC
D688;HANGUL SYLLABLE HWAEK
D689;HANGUL SYLLABLE HWAET
D68A;HANGUL SYLLABLE HWAEP
D68B;HANGUL SYLLABLE HWAEH
D68C;HANGUL SYLLABLE HOE
D68D;HANGUL SYLLABLE HOEG
D68E;HANGUL SYLLABLE HOEGG
D68F;HANGUL SYLLABLE HOEGS
D690;HANGUL SYLLABLE HOEN
D691;HANGUL SYLLABLE HOENJ
D692;HANGUL SYLLABLE HOENH
D693;HANGUL SYLLABLE HOED
D694;HANGUL SYLLABLE HOEL
D695;HANGUL SYLLABLE HOELG
D696;HANGUL SYLLABLE HOELM
D697;HANGUL SYLLABLE HOELB
D698;HANGUL SYLLABLE HOELS
D699;HANGUL SYLLABLE HOELT
D69A;HANGUL SYLLABLE HOELP
D69B;HANGUL SYLLABLE HOELH
D69C;HANGUL SYLLABLE HOEM
D69D;HANGUL SYLLABLE HOEB
D69E;HANGUL SYLLABLE HOEBS
D69F;HANGUL SYLLABLE HOES
D6A0;HANGUL SYLLABLE HOESS
D6A1;HANGUL SYLLABLE HOENG
D6A2;HANGUL SYLLABLE HOEJ
D6A3;HANGUL SYLLABLE HOEC
D6A4;HANGUL SYLLABLE HOEK
D6A5;HANGUL SYLLABLE HOET
D6A6;HANGUL SYLLABLE HOEP
D6A7;HANGUL SYLLABLE HOEH
D6A8;HANGUL SYLLABLE HYO
D6A9;HANGUL SYLLABLE HYOG
D6AA;HANGUL SYLLABLE HYOGG
D6AB;HANGUL SYLLABLE HYOGS
D6AC;HANGUL SYLLABLE HYON
D6AD;HANGUL SYLLABLE HYONJ
D6AE;HANGUL SYLLABLE HYONH
D6AF;HANGUL SYLLABLE HYOD
D6B0;HANGUL SYLLABLE HYOL
D6B1;HANGUL SYLLABLE HYOLG
D6B2;HANGUL SYLLABLE HYOLM
D6B3;HANGUL SYLLABLE HYOLB
D6B4;HANGUL SYLLABLE HYOLS
D6B5;HANGUL SYLLABLE HYOLT
D6B6;HANGUL SYLLABLE HYOLP
D6B7;HANGUL SYLLABLE HYOLH
D6B8;HANGUL SYLLABLE HYOM
D6B9;HANGUL SYLLABLE HYOB
D6BA;HANGUL SYLLABLE HYOBS
D6BB;HANGUL SYLLABLE HYOS
D6BC;HANGUL SYLLABLE HYOSS
D6BD;HANGUL SYLLABLE HYONG
D6BE;HANGUL SYLLABLE HYOJ
D6BF;HANGUL SYLLABLE HYOC
D6C0;HANGUL SYLLABLE HYOK
D6C1;HANGUL SYLLABLE HYOT
D6C2;HANGUL SYLLABLE HYOP
D6C3;HANGUL SYLLABLE HYOH
D6C4;HANGUL SYLLABLE HU
D6C5;HANGUL SYLLABLE HUG
D6C6;HANGUL SYLLABLE HUGG
D6C7;HANGUL SYLLABLE HUGS
D6C8;HANGUL SYLLABLE HUN
D6C9;HANGUL SYLLABLE HUNJ
D6CA;HANGUL SYLLABLE HUNH
D6CB;HANGUL SYLLABLE HUD
D6CC;HANGUL SYLLABLE HUL
D6CD;HANGUL SYLLABLE HULG
D6CE;HANGUL SYLLABLE HULM
D6CF;HANGUL SYLLABLE HULB
D6D0;HANGUL SYLLABLE HULS
D6D1;HANGUL SYLLABLE HULT
D6D2;HANGUL SYLLABLE HULP
D6D3;HANGUL SYLLABLE HULH
D6D4;HANGUL SYLLABLE HUM
D6D5;HANGUL SYLLABLE HUB
D6D6;HANGUL SYLLABLE HUBS
D6D7;HANGUL SYLLABLE HUS
D6D8;HANGUL SYLLABLE HUSS
D6D9;HANGUL SYLLABLE HUNG
D6DA;HANGUL SYLLABLE HUJ
D6DB;HANGUL SYLLABLE HUC
D6DC;HANGUL SYLLABLE HUK
D6DD;HANGUL SYLLABLE HUT
D6DE;HANGUL SYLLABLE HUP
D6DF;HANGUL SYLLABLE HUH
D6E0;HANGUL SYLLABLE HWEO
D6E1;HANGUL SYLLABLE HWEOG
D6E2;HANGUL SYLLABLE HWEOGG
D6E3;HANGUL SYLLABLE HWEOGS
D6E4;HANGUL SYLLABLE HWEON
D6E5;HANGUL SYLLABLE HWEONJ
D6E6;HANGUL SYLLABLE HWEONH
D6E7;HANGUL SYLLABLE HWEOD
D6E8;HANGUL SYLLABLE HWEOL
D6E9;HANGUL SYLLABLE HWEOLG
D6EA;HANGUL SYLLABLE HWEOLM
D6EB;HANGUL SYLLABLE HWEOLB
D6EC;HANGUL SYLLABLE HWEOLS
D6ED;HANGUL SYLLABLE HWEOLT
D6EE;HANGUL SYLLABLE HWEOLP
D6EF;HANGUL SYLLABLE HWEOLH
D6F0;HANGUL SYLLABLE HWEOM
D6F1;HANGUL SYLLABLE HWEOB
D6F2;HANGUL SYLLABLE HWEOBS
D6F3;HANGUL SYLLABLE HWEOS
D6F4;HANGUL SYLLABLE HWEOSS
D6F5;HANGUL SYLLABLE HWEONG
D6F6;HANGUL SYLLABLE HWEOJ
D6F7;HANGUL SYLLABLE HWEOC
D6F8;HANGUL SYLLABLE HWEOK
D6F9;HANGUL SYLLABLE HWEOT
D6FA;HANGUL SYLLABLE HWEOP
D6FB;HANGUL SYLLABLE HWEOH
D6FC;HANGUL SYLLABLE HWE
D6FD;HANGUL SYLLABLE HWEG
D6FE;HANGUL SYLLABLE HWEGG
D6FF;HANGUL SYLLABLE HWEGS
D700;HANGUL SYLLABLE HWEN
D701;HANGUL SYLLABLE HWENJ
D702;HANGUL SYLLABLE HWENH
D703;HANGUL SYLLABLE HWED
D704;HANGUL SYLLABLE HWEL
D705;HANGUL SYLLABLE HWELG
D706;HANGUL SYLLABLE HWELM
D707;HANGUL SYLLABLE HWELB
D708;HANGUL SYLLABLE HWELS
D709;HANGUL SYLLABLE HWELT
D70A;HANGUL SYLLABLE HWELP
D70B;HANGUL SYLLABLE HWELH
D70C;HANGUL SYLLABLE HWEM
D70D;HANGUL SYLLABLE HWEB
D70E;HANGUL SYLLABLE HWEBS
D70F;HANGUL SYLLABLE HWES
D710;HANGUL SYLLABLE HWESS
D711;HANGUL SYLLABLE HWENG
D712;HANGUL SYLLABLE HWEJ
D713;HANGUL SYLLABLE HWEC
D714;HANGUL SYLLABLE HWEK
D715;HANGUL SYLLABLE HWET
D716;HANGUL SYLLABLE HWEP
D717;HANGUL SYLLABLE HWEH
D718;HANGUL SYLLABLE HWI
D719;HANGUL SYLLABLE HWIG
D71A;HANGUL SYLLABLE HWIGG
D71B;HANGUL SYLLABLE HWIGS
D71C;HANGUL SYLLABLE HWIN
D71D;HANGUL SYLLABLE HWINJ
D71E;HANGUL SYLLABLE HWINH
D71F;HANGUL SYLLABLE HWID
D720;HANGUL SYLLABLE HWIL
D721;HANGUL SYLLABLE HWILG
D722;HANGUL SYLLABLE HWILM
D723;HANGUL SYLLABLE HWILB
D724;HANGUL SYLLABLE HWILS
D725;HANGUL SYLLABLE HWILT
D726;HANGUL SYLLABLE HWILP
D727;HANGUL SYLLABLE HWILH
D728;HANGUL SYLLABLE HWIM
D729;HANGUL SYLLABLE HWIB
D72A;HANGUL SYLLABLE HWIBS
D72B;HANGUL SYLLABLE HWIS
D72C;HANGUL SYLLABLE HWISS
D72D;HANGUL SYLLABLE HWING
D72E;HANGUL SYLLABLE HWIJ
D72F;HANGUL SYLLABLE HWIC
D730;HANGUL SYLLABLE HWIK
D731;HANGUL SYLLABLE HWIT
D732;HANGUL SYLLABLE HWIP
D733;HANGUL SYLLABLE HWIH
D734;HANGUL SYLLABLE HYU
D735;HANGUL SYLLABLE HYUG
D736;HANGUL SYLLABLE HYUGG
D737;HANGUL SYLLABLE HYUGS
D738;HANGUL SYLLABLE HYUN
D739;HANGUL SYLLABLE HYUNJ
D73A;HANGUL SYLLABLE HYUNH
D73B;HANGUL SYLLABLE HYUD
D73C;HANGUL SYLLABLE HYUL
D73D;HANGUL SYLLABLE HYULG
D73E;HANGUL SYLLABLE HYULM
D73F;HANGUL SYLLABLE HYULB
D740;HANGUL SYLLABLE HYULS
D741;HANGUL SYLLABLE HYULT
D742;HANGUL SYLLABLE HYULP
D743;HANGUL SYLLABLE HYULH
D744;HANGUL SYLLABLE HYUM
D745;HANGUL SYLLABLE HYUB
D746;HANGUL SYLLABLE HYUBS
D747;HANGUL SYLLABLE HYUS
D748;HANGUL SYLLABLE HYUSS
D749;HANGUL SYLLABLE HYUNG
D74A;HANGUL SYLLABLE HYUJ
D74B;HANGUL SYLLABLE HYUC
D74C;HANGUL SYLLABLE HYUK
D74D;HANGUL SYLLABLE HYUT
D74E;HANGUL SYLLABLE HYUP
D74F;HANGUL SYLLABLE HYUH
D750;HANGUL SYLLABLE HEU
D751;HANGUL SYLLABLE HEUG
D752;HANGUL SYLLABLE HEUGG
D753;HANGUL SYLLABLE HEUGS
D754;HANGUL SYLLABLE HEUN
D755;HANGUL SYLLABLE HEUNJ
D756;HANGUL SYLLABLE HEUNH
D757;HANGUL SYLLABLE HEUD
D758;HANGUL SYLLABLE HEUL
D759;HANGUL SYLLABLE HEULG
D75A;HANGUL SYLLABLE HEULM
D75B;HANGUL SYLLABLE HEULB
D75C;HANGUL SYLLABLE HEULS
D75D;HANGUL SYLLABLE HEULT
D75E;HANGUL SYLLABLE HEULP
D75F;HANGUL SYLLABLE HEULH
D760;HANGUL SYLLABLE HEUM
D761;HANGUL SYLLABLE HEUB
D762;HANGUL SYLLABLE HEUBS
D763;HANGUL SYLLABLE HEUS
D764;HANGUL SYLLABLE HEUSS
D765;HANGUL SYLLABLE HEUNG
D766;HANGUL SYLLABLE HEUJ
D767;HANGUL SYLLABLE HEUC
D768;HANGUL SYLLABLE HEUK
D769;HANGUL SYLLABLE HEUT
D76A;HANGUL SYLLABLE HEUP
D76B;HANGUL SYLLABLE HEUH
D76C;HANGUL SYLLABLE HYI
D76D;HANGUL SYLLABLE HYIG
D76E;HANGUL SYLLABLE HYIGG
D76F;HANGUL SYLLABLE HYIGS
D770;HANGUL SYLLABLE HYIN
D771;HANGUL SYLLABLE HYINJ
D772;HANGUL SYLLABLE HYINH
D773;HANGUL SYLLABLE HYID
D774;HANGUL SYLLABLE HYIL
D775;HANGUL SYLLABLE HYILG
D776;HANGUL SYLLABLE HYILM
D777;HANGUL SYLLABLE HYILB
D778;HANGUL SYLLABLE HYILS
D779;HANGUL SYLLABLE HYILT
D77A;HANGUL SYLLABLE HYILP
D77B;HANGUL SYLLABLE HYILH
D77C;HANGUL SYLLABLE HYIM
D77D;HANGUL SYLLABLE HYIB
D77E;HANGUL SYLLABLE HYIBS
D77F;HANGUL SYLLABLE HYIS
D780;HANGUL SYLLABLE HYISS
D781;HANGUL SYLLABLE HYING
D782;HANGUL SYLLABLE HYIJ
D783;HANGUL SYLLABLE HYIC
D784;HANGUL SYLLABLE HYIK
D785;HANGUL SYLLABLE HYIT
D786;HANGUL SYLLABLE HYIP
D787;HANGUL SYLLABLE HYIH
D788;HANGUL SYLLABLE HI
D789;HANGUL SYLLABLE HIG
D78A;HANGUL SYLLABLE HIGG
D78B;HANGUL SYLLABLE HIGS
D78C;HANGUL SYLLABLE HIN
D78D;HANGUL SYLLABLE HINJ
D78E;HANGUL SYLLABLE HINH
D78F;HANGUL SYLLABLE HID
D790;HANGUL SYLLABLE HIL
D791;HANGUL SYLLABLE HILG
D792;HANGUL SYLLABLE HILM
D793;HANGUL SYLLABLE HILB
D794;HANGUL SYLLABLE HILS
D795;HANGUL SYLLABLE HILT
D796;HANGUL SYLLABLE HILP
D797;HANGUL SYLLABLE HILH
D798;HANGUL SYLLABLE HIM
D799;HANGUL SYLLABLE HIB
D79A;HANGUL SYLLABLE HIBS
D79B;HANGUL SYLLABLE HIS
D79C;HANGUL SYLLABLE HISS
D79D;HANGUL SYLLABLE HING
D79E;HANGUL SYLLABLE HIJ
D79F;HANGUL SYLLABLE HIC
D7A0;HANGUL SYLLABLE HIK
D7A1;HANGUL SYLLABLE HIT
D7A2;HANGUL SYLLABLE HIP
D7A3;HANGUL SYLLABLE HIH
