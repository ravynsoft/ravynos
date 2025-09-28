#!./perl
#
# This is a home for regular expression tests that do not fit into
# the format supported by re/regexp.t.  If you want to add a test
# that does fit that format, add it to re/re_tests, not here.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib .');
    require './charset_tools.pl';
    skip_all_if_miniperl("miniperl can't load Tie::Hash::NamedCapture, need for %+ and %-");
}

use strict;
use warnings;
use 5.010;
our ($REGMARK, $REGERROR);

sub run_tests;

$| = 1;

run_tests() unless caller;

#
# Tests start here.
#
sub run_tests {

    {
        # Japhy -- added 03/03/2001
        () = (my $str = "abc") =~ /(...)/;
        $str = "def";
        is($1, "abc", 'Changing subject does not modify $1');
    }

  SKIP:
    {
        # The trick is that in EBCDIC the explicit numeric range should
        # match (as also in non-EBCDIC) but the explicit alphabetic range
        # should not match.
        like "\x8e", qr/[\x89-\x91]/, '"\x8e" =~ /[\x89-\x91]/';
        like "\xce", qr/[\xc9-\xd1]/, '"\xce" =~ /[\xc9-\xd1]/';
        like "\xd0", qr/[\xc9-\xd1]/, '"\xd0" =~ /[\xc9-\xd1]/';

        skip "Not an EBCDIC platform", 2 unless ord ('i') == 0x89 &&
                                                ord ('J') == 0xd1;

        # In most places these tests would succeed since \x8e does not
        # in most character sets match 'i' or 'j' nor would \xce match
        # 'I' or 'J', but strictly speaking these tests are here for
        # the good of EBCDIC, so let's test these only there.
        unlike("\x8e", qr/[i-j]/, '"\x8e" !~ /[i-j]/');
        unlike("\xce", qr/[I-J]/, '"\xce" !~ /[I-J]/');
        unlike("\xd0", qr/[I-J]/, '"\xd0" !~ /[I-J]/');
    }

    {
        like "\x{ab}", qr/\x{ab}/,   '"\x{ab}"   =~ /\x{ab}/  ';
        like "\x{abcd}", qr/\x{abcd}/, '"\x{abcd}" =~ /\x{abcd}/';
    }

    {
        my $message = 'bug id 20001008.001 (#4407)';

        my $strasse = "stra" . uni_to_native("\337") . "e";
        my @x = ("$strasse 138", "$strasse 138");
        for (@x) {
            ok(s/(\d+)\s*([\w\-]+)/$1 . uc $2/e, $message);
            ok(my ($latin) = /^(.+)(?:\s+\d)/, $message);
            is($latin, $strasse, $message);
	    ok($latin =~ s/$strasse/straße/, $message);
            #
            # Previous code follows, but outcommented - there were no tests.
            #
            # $latin =~ s/stra\337e/straße/; # \303\237 after the 2nd a
            # use utf8; # needed for the raw UTF-8
            # $latin =~ s!(s)tr(?:aß|s+e)!$1tr.!; # \303\237 after the a
        }
    }

    {
        my $message = 'Test \x escapes';
        ok("ba\xd4c" =~ /([a\xd4]+)/ && $1 eq "a\xd4", $message);
        ok("ba\xd4c" =~ /([a\xd4]+)/ && $1 eq "a\x{d4}", $message);
        ok("ba\x{d4}c" =~ /([a\xd4]+)/ && $1 eq "a\x{d4}", $message);
        ok("ba\x{d4}c" =~ /([a\xd4]+)/ && $1 eq "a\xd4", $message);
        ok("ba\xd4c" =~ /([a\x{d4}]+)/ && $1 eq "a\xd4", $message);
        ok("ba\xd4c" =~ /([a\x{d4}]+)/ && $1 eq "a\x{d4}", $message);
        ok("ba\x{d4}c" =~ /([a\x{d4}]+)/ && $1 eq "a\x{d4}", $message);
        ok("ba\x{d4}c" =~ /([a\x{d4}]+)/ && $1 eq "a\xd4", $message);
    }

    {
        my $message = 'Match code points > 255';
        $_ = "abc\x{100}\x{200}\x{300}\x{380}\x{400}defg";
        ok(/(.\x{300})./, $message);
        ok($` eq "abc\x{100}"            && length ($`) == 4, $message);
        ok($& eq "\x{200}\x{300}\x{380}" && length ($&) == 3, $message);
        ok($' eq "\x{400}defg"           && length ($') == 5, $message);
        ok($1 eq "\x{200}\x{300}"        && length ($1) == 2, $message);
    }

    {
        my $x = "\x{10FFFD}";
        $x =~ s/(.)/$1/g;
        ok ord($x) == 0x10FFFD && length($x) == 1, "From Robin Houston";
    }

    {
        my %d = (
            "7f" => [0, 0, 0],
            "80" => [1, 1, 0],
            "ff" => [1, 1, 0],
           "100" => [0, 1, 1],
        );

        while (my ($code, $match) = each %d) {
            my $message = "Properties of \\x$code";
            my $char = eval qq ["\\x{$code}"];

            is(0 + ($char =~ /[\x80-\xff]/),    $$match[0], $message);
            is(0 + ($char =~ /[\x80-\x{100}]/), $$match[1], $message);
            is(0 + ($char =~ /[\x{100}]/),      $$match[2], $message);
        }
    }

    {
        # From Japhy
	foreach (qw(c g o)) {
	    warning_like(sub {'' =~ "(?$_)"},    qr/^Useless \(\?$_\)/);
	    warning_like(sub {'' =~ "(?-$_)"},   qr/^Useless \(\?-$_\)/);
	}

        # Now test multi-error regexes
	foreach (['(?g-o)', qr/^Useless \(\?g\)/, qr/^Useless \(\?-o\)/],
		 ['(?g-c)', qr/^Useless \(\?g\)/, qr/^Useless \(\?-c\)/],
		 # (?c) means (?g) error won't be thrown
		 ['(?o-cg)', qr/^Useless \(\?o\)/, qr/^Useless \(\?-c\)/],
		 ['(?ogc)', qr/^Useless \(\?o\)/, qr/^Useless \(\?g\)/,
		  qr/^Useless \(\?c\)/],
		) {
	    my ($re, @warnings) = @$_;
	    warnings_like(sub {eval "qr/$re/"}, \@warnings, "qr/$re/ warns");
	}
    }

    {
        my $message = "/x tests";
        $_ = "foo";
        foreach my $pat (<<"        --", <<"        --") {
          /f
           o\r
           o
           \$
          /x
        --
          /f
           o
           o
           \$\r
          /x
        --
	    is(eval $pat, 1, $message);
	    is($@, '', $message);
	}
    }

    {
        my $message = "/o feature";
        sub test_o {$_ [0] =~ /$_[1]/o; return $1}
        is(test_o ('abc', '(.)..'), 'a', $message);
        is(test_o ('abc', '..(.)'), 'a', $message);
    }

    {
        # Test basic $^N usage outside of a regex
        my $message = '$^N usage outside of a regex';
        my $x = "abcdef";
        ok(($x =~ /cde/                  and !defined $^N), $message);
        ok(($x =~ /(cde)/                and $^N eq "cde"), $message);
        ok(($x =~ /(c)(d)(e)/            and $^N eq   "e"), $message);
        ok(($x =~ /(c(d)e)/              and $^N eq "cde"), $message);
        ok(($x =~ /(foo)|(c(d)e)/        and $^N eq "cde"), $message);
        ok(($x =~ /(c(d)e)|(foo)/        and $^N eq "cde"), $message);
        ok(($x =~ /(c(d)e)|(abc)/        and $^N eq "abc"), $message);
        ok(($x =~ /(c(d)e)|(abc)x/       and $^N eq "cde"), $message);
        ok(($x =~ /(c(d)e)(abc)?/        and $^N eq "cde"), $message);
        ok(($x =~ /(?:c(d)e)/            and $^N eq   "d"), $message);
        ok(($x =~ /(?:c(d)e)(?:f)/       and $^N eq   "d"), $message);
        ok(($x =~ /(?:([abc])|([def]))*/ and $^N eq   "f"), $message);
        ok(($x =~ /(?:([ace])|([bdf]))*/ and $^N eq   "f"), $message);
        ok(($x =~ /(([ace])|([bd]))*/    and $^N eq   "e"), $message);
       {ok(($x =~ /(([ace])|([bdf]))*/   and $^N eq   "f"), $message);}
        ## Test to see if $^N is automatically localized -- it should now
        ## have the value set in the previous test.
        is($^N, "e", '$^N is automatically localized');

        # Now test inside (?{ ... })
        $message = '$^N usage inside (?{ ... })';
        our ($y, $z);
        ok(($x =~ /a([abc])(?{$y=$^N})c/                    and $y eq  "b"), $message);
        ok(($x =~ /a([abc]+)(?{$y=$^N})d/                   and $y eq  "bc"), $message);
        ok(($x =~ /a([abcdefg]+)(?{$y=$^N})d/               and $y eq  "bc"), $message);
        ok(($x =~ /(a([abcdefg]+)(?{$y=$^N})d)(?{$z=$^N})e/ and $y eq  "bc"
                                                            and $z eq "abcd"), $message);
        ok(($x =~ /(a([abcdefg]+)(?{$y=$^N})de)(?{$z=$^N})/ and $y eq  "bc"
                                                            and $z eq "abcde"), $message);

    }

  SKIP:
    {
        ## Should probably put in tests for all the POSIX stuff,
        ## but not sure how to guarantee a specific locale......

        my $message = 'Test [[:cntrl:]]';
        my $AllBytes = join "" => map {chr} 0 .. 255;
        (my $x = $AllBytes) =~ s/[[:cntrl:]]//g;
        $x = join "", sort { $a cmp $b }
                      map { chr utf8::native_to_unicode(ord $_) } split "", $x;
        is($x, join("", map {chr} 0x20 .. 0x7E, 0x80 .. 0xFF), $message);

        ($x = $AllBytes) =~ s/[^[:cntrl:]]//g;
        $x = join "", sort { $a cmp $b }
                       map { chr utf8::native_to_unicode(ord $_) } split "", $x;
        is($x, (join "", map {chr} 0x00 .. 0x1F, 0x7F), $message);
    }

    {
        # With /s modifier UTF8 chars were interpreted as bytes
        my $message = "UTF-8 chars aren't bytes";
        my $a = "Hello \x{263A} World";
        my @a = ($a =~ /./gs);
        is($#a, 12, $message);
    }

    {
        no warnings 'digit';
        # Check that \x## works. 5.6.1 and 5.005_03 fail some of these.
        my $x;
        $x = "\x4e" . "E";
        like ($x, qr/^\x4EE$/, "Check only 2 bytes of hex are matched.");

        $x = "\x4e" . "i";
        like ($x, qr/^\x4Ei$/, "Check that invalid hex digit stops it (2)");

        $x = "\x4" . "j";
        like ($x, qr/^\x4j$/,  "Check that invalid hex digit stops it (1)");

        $x = "\x0" . "k";
        like ($x, qr/^\xk$/,   "Check that invalid hex digit stops it (0)");

        $x = "\x0" . "x";
        like ($x, qr/^\xx$/, "\\xx isn't to be treated as \\0");

        $x = "\x0" . "xa";
        like ($x, qr/^\xxa$/, "\\xxa isn't to be treated as \\xa");

        $x = "\x9" . "_b";
        like ($x, qr/^\x9_b$/, "\\x9_b isn't to be treated as \\x9b");

        # and now again in [] ranges

        $x = "\x4e" . "E";
        like ($x, qr/^[\x4EE]{2}$/, "Check only 2 bytes of hex are matched.");

        $x = "\x4e" . "i";
        like ($x, qr/^[\x4Ei]{2}$/, "Check that invalid hex digit stops it (2)");

        $x = "\x4" . "j";
        like ($x, qr/^[\x4j]{2}$/,  "Check that invalid hex digit stops it (1)");

        $x = "\x0" . "k";
        like ($x, qr/^[\xk]{2}$/,   "Check that invalid hex digit stops it (0)");

        $x = "\x0" . "x";
        like ($x, qr/^[\xx]{2}$/, "\\xx isn't to be treated as \\0");

        $x = "\x0" . "xa";
        like ($x, qr/^[\xxa]{3}$/, "\\xxa isn't to be treated as \\xa");

        $x = "\x9" . "_b";
        like ($x, qr/^[\x9_b]{3}$/, "\\x9_b isn't to be treated as \\x9b");

        # Check that \x{##} works. 5.6.1 fails quite a few of these.

        $x = "\x9b";
        like ($x, qr/^\x{9_b}$/, "\\x{9_b} is to be treated as \\x9b");

        $x = "\x9b" . "y";
        like ($x, qr/^\x{9_b}y$/, "\\x{9_b} is to be treated as \\x9b (again)");

        $x = "\x9b" . "y";
        like ($x, qr/^\x{9b_}y$/, "\\x{9b_} is to be treated as \\x9b");

        $x = "\x9b" . "y";
        like ($x, qr/^\x{9_bq}y$/, "\\x{9_bc} is to be treated as \\x9b");

        $x = "\x0" . "y";
        like ($x, qr/^\x{x9b}y$/, "\\x{x9b} is to be treated as \\x0");

        $x = "\x0" . "y";
        like ($x, qr/^\x{0x9b}y$/, "\\x{0x9b} is to be treated as \\x0");

        $x = "\x9b" . "y";
        like ($x, qr/^\x{09b}y$/, "\\x{09b} is to be treated as \\x9b");

        $x = "\x9b";
        like ($x, qr/^[\x{9_b}]$/, "\\x{9_b} is to be treated as \\x9b");

        $x = "\x9b" . "y";
        like ($x, qr/^[\x{9_b}y]{2}$/,
                                 "\\x{9_b} is to be treated as \\x9b (again)");

        $x = "\x9b" . "y";
        like ($x, qr/^[\x{9b_}y]{2}$/, "\\x{9b_} is to be treated as \\x9b");

        $x = "\x9b" . "y";
        like ($x, qr/^[\x{9_bq}y]{2}$/, "\\x{9_bc} is to be treated as \\x9b");

        $x = "\x0" . "y";
        like ($x, qr/^[\x{x9b}y]{2}$/, "\\x{x9b} is to be treated as \\x0");

        $x = "\x0" . "y";
        like ($x, qr/^[\x{0x9b}y]{2}$/, "\\x{0x9b} is to be treated as \\x0");

        $x = "\x9b" . "y";
        like ($x, qr/^[\x{09b}y]{2}$/, "\\x{09b} is to be treated as \\x9b");

    }

    {
        # High bit bug -- japhy
        my $x = "ab\200d";
        like $x, qr/.*?\200/, "High bit fine";
    }

    {
        # The basic character classes and Unicode
        like "\x{0100}", qr/\w/, 'LATIN CAPITAL LETTER A WITH MACRON in /\w/';
        like "\x{0660}", qr/\d/, 'ARABIC-INDIC DIGIT ZERO in /\d/';
        like "\x{1680}", qr/\s/, 'OGHAM SPACE MARK in /\s/';
    }

    {
        my $message = "Folding matches and Unicode";
        like("a\x{100}", qr/A/i, $message);
        like("A\x{100}", qr/a/i, $message);
        like("a\x{100}", qr/a/i, $message);
        like("A\x{100}", qr/A/i, $message);
        like("\x{101}a", qr/\x{100}/i, $message);
        like("\x{100}a", qr/\x{100}/i, $message);
        like("\x{101}a", qr/\x{101}/i, $message);
        like("\x{100}a", qr/\x{101}/i, $message);
        like("a\x{100}", qr/A\x{100}/i, $message);
        like("A\x{100}", qr/a\x{100}/i, $message);
        like("a\x{100}", qr/a\x{100}/i, $message);
        like("A\x{100}", qr/A\x{100}/i, $message);
        like("a\x{100}", qr/[A]/i, $message);
        like("A\x{100}", qr/[a]/i, $message);
        like("a\x{100}", qr/[a]/i, $message);
        like("A\x{100}", qr/[A]/i, $message);
        like("\x{101}a", qr/[\x{100}]/i, $message);
        like("\x{100}a", qr/[\x{100}]/i, $message);
        like("\x{101}a", qr/[\x{101}]/i, $message);
        like("\x{100}a", qr/[\x{101}]/i, $message);
    }

    {
        use charnames ':full';
        my $message = "Folding 'LATIN LETTER A WITH GRAVE'";

        my $lower = "\N{LATIN SMALL LETTER A WITH GRAVE}";
        my $UPPER = "\N{LATIN CAPITAL LETTER A WITH GRAVE}";

        like($lower, qr/$UPPER/i, $message);
        like($UPPER, qr/$lower/i, $message);
        like($lower, qr/[$UPPER]/i, $message);
        like($UPPER, qr/[$lower]/i, $message);

        $message = "Folding 'GREEK LETTER ALPHA WITH VRACHY'";

        $lower = "\N{GREEK CAPITAL LETTER ALPHA WITH VRACHY}";
        $UPPER = "\N{GREEK SMALL LETTER ALPHA WITH VRACHY}";

        like($lower, qr/$UPPER/i, $message);
        like($UPPER, qr/$lower/i, $message);
        like($lower, qr/[$UPPER]/i, $message);
        like($UPPER, qr/[$lower]/i, $message);

        $message = "Folding 'LATIN LETTER Y WITH DIAERESIS'";

        $lower = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}";
        $UPPER = "\N{LATIN CAPITAL LETTER Y WITH DIAERESIS}";

        like($lower, qr/$UPPER/i, $message);
        like($UPPER, qr/$lower/i, $message);
        like($lower, qr/[$UPPER]/i, $message);
        like($UPPER, qr/[$lower]/i, $message);
    }

    {
        use charnames ':full';
        my $message = "GREEK CAPITAL LETTER SIGMA vs " .
                         "COMBINING GREEK PERISPOMENI";

        my $SIGMA = "\N{GREEK CAPITAL LETTER SIGMA}";
        my $char  = "\N{COMBINING GREEK PERISPOMENI}";

        warning_is(sub {unlike("_:$char:_", qr/_:$SIGMA:_/i, $message)}, undef,
		   'Did not warn [change a5961de5f4215b5c]');
    }

    {
        my $message = '\X';
        use charnames ':full';

        ok("a!"                          =~ /^(\X)!/ && $1 eq "a", $message);
        ok("\xDF!"                       =~ /^(\X)!/ && $1 eq "\xDF", $message);
        ok("\x{100}!"                    =~ /^(\X)!/ && $1 eq "\x{100}", $message);
        ok("\x{100}\x{300}!"             =~ /^(\X)!/ && $1 eq "\x{100}\x{300}", $message);
        ok("\N{LATIN CAPITAL LETTER E}!" =~ /^(\X)!/ &&
               $1 eq "\N{LATIN CAPITAL LETTER E}", $message);
        ok("\N{LATIN CAPITAL LETTER E}\N{COMBINING GRAVE ACCENT}!"
                                         =~ /^(\X)!/ &&
               $1 eq "\N{LATIN CAPITAL LETTER E}\N{COMBINING GRAVE ACCENT}", $message);

    }

    {
        my $message = "Final Sigma";

        my $SIGMA = "\x{03A3}"; # CAPITAL
        my $Sigma = "\x{03C2}"; # SMALL FINAL
        my $sigma = "\x{03C3}"; # SMALL

        like($SIGMA, qr/$SIGMA/i, $message);
        like($SIGMA, qr/$Sigma/i, $message);
        like($SIGMA, qr/$sigma/i, $message);

        like($Sigma, qr/$SIGMA/i, $message);
        like($Sigma, qr/$Sigma/i, $message);
        like($Sigma, qr/$sigma/i, $message);

        like($sigma, qr/$SIGMA/i, $message);
        like($sigma, qr/$Sigma/i, $message);
        like($sigma, qr/$sigma/i, $message);

        like($SIGMA, qr/[$SIGMA]/i, $message);
        like($SIGMA, qr/[$Sigma]/i, $message);
        like($SIGMA, qr/[$sigma]/i, $message);

        like($Sigma, qr/[$SIGMA]/i, $message);
        like($Sigma, qr/[$Sigma]/i, $message);
        like($Sigma, qr/[$sigma]/i, $message);

        like($sigma, qr/[$SIGMA]/i, $message);
        like($sigma, qr/[$Sigma]/i, $message);
        like($sigma, qr/[$sigma]/i, $message);

        $message = "More final Sigma";

        my $S3 = "$SIGMA$Sigma$sigma";

        ok(":$S3:" =~ /:(($SIGMA)+):/i   && $1 eq $S3 && $2 eq $sigma, $message);
        ok(":$S3:" =~ /:(($Sigma)+):/i   && $1 eq $S3 && $2 eq $sigma, $message);
        ok(":$S3:" =~ /:(($sigma)+):/i   && $1 eq $S3 && $2 eq $sigma, $message);

        ok(":$S3:" =~ /:(([$SIGMA])+):/i && $1 eq $S3 && $2 eq $sigma, $message);
        ok(":$S3:" =~ /:(([$Sigma])+):/i && $1 eq $S3 && $2 eq $sigma, $message);
        ok(":$S3:" =~ /:(([$sigma])+):/i && $1 eq $S3 && $2 eq $sigma, $message);
    }

    {
        use charnames ':full';
        my $message = "Parlez-Vous " .
                         "Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais?";

        ok("Fran\N{LATIN SMALL LETTER C}ais" =~ /Fran.ais/ &&
            $& eq "Francais", $message);
        ok("Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais" =~ /Fran.ais/ &&
            $& eq "Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais", $message);
        ok("Fran\N{LATIN SMALL LETTER C}ais" =~ /Fran\Xais/ &&
            $& eq "Francais", $message);
        ok("Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais" =~ /Fran\Xais/  &&
            $& eq "Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais", $message);
        ok("Franc\N{COMBINING CEDILLA}ais" =~ /Fran\Xais/ &&
            $& eq "Franc\N{COMBINING CEDILLA}ais", $message);
        ok("Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais" =~
           /Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais/  &&
            $& eq "Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais", $message);
        ok("Franc\N{COMBINING CEDILLA}ais" =~ /Franc\N{COMBINING CEDILLA}ais/ &&
            $& eq "Franc\N{COMBINING CEDILLA}ais", $message);

        my @f = (
            ["Fran\N{LATIN SMALL LETTER C}ais",                    "Francais"],
            ["Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais",
                               "Fran\N{LATIN SMALL LETTER C WITH CEDILLA}ais"],
            ["Franc\N{COMBINING CEDILLA}ais", "Franc\N{COMBINING CEDILLA}ais"],
        );
        foreach my $entry (@f) {
            my ($subject, $match) = @$entry;
            ok($subject =~ /Fran(?:c\N{COMBINING CEDILLA}?|
                    \N{LATIN SMALL LETTER C WITH CEDILLA})ais/x &&
               $& eq $match, $message);
        }
    }

    {
        my $message = "Lingering (and useless) UTF8 flag doesn't mess up /i";
        my $pat = "ABcde";
        my $str = "abcDE\x{100}";
        chop $str;
        like($str, qr/$pat/i, $message);

        $pat = "ABcde\x{100}";
        $str = "abcDE";
        chop $pat;
        like($str, qr/$pat/i, $message);

        $pat = "ABcde\x{100}";
        $str = "abcDE\x{100}";
        chop $pat;
        chop $str;
        like($str, qr/$pat/i, $message);
    }

    {
        use charnames ':full';
        my $message = "LATIN SMALL LETTER SHARP S " .
                         "(\N{LATIN SMALL LETTER SHARP S})";

        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/\N{LATIN SMALL LETTER SHARP S}/, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'\N{LATIN SMALL LETTER SHARP S}', $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/\N{LATIN SMALL LETTER SHARP S}/i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'\N{LATIN SMALL LETTER SHARP S}'i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/[\N{LATIN SMALL LETTER SHARP S}]/, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'[\N{LATIN SMALL LETTER SHARP S}]', $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/[\N{LATIN SMALL LETTER SHARP S}]/i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'[\N{LATIN SMALL LETTER SHARP S}]'i, $message);

        like("ss", qr /\N{LATIN SMALL LETTER SHARP S}/i, $message);
        like("ss", qr '\N{LATIN SMALL LETTER SHARP S}'i, $message);
        like("SS", qr /\N{LATIN SMALL LETTER SHARP S}/i, $message);
        like("SS", qr '\N{LATIN SMALL LETTER SHARP S}'i, $message);
        like("ss", qr/[\N{LATIN SMALL LETTER SHARP S}]/i, $message);
        like("ss", qr'[\N{LATIN SMALL LETTER SHARP S}]'i, $message);
        like("SS", qr/[\N{LATIN SMALL LETTER SHARP S}]/i, $message);
        like("SS", qr'[\N{LATIN SMALL LETTER SHARP S}]'i, $message);

        like("\N{LATIN SMALL LETTER SHARP S}", qr/ss/i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}", qr/SS/i, $message);

         $message = "Unoptimized named sequence in class";
        like("ss", qr/[\N{LATIN SMALL LETTER SHARP S}x]/i, $message);
        like("ss", qr'[\N{LATIN SMALL LETTER SHARP S}x]'i, $message);
        like("SS", qr/[\N{LATIN SMALL LETTER SHARP S}x]/i, $message);
        like("SS", qr'[\N{LATIN SMALL LETTER SHARP S}x]'i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/[\N{LATIN SMALL LETTER SHARP S}x]/, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'[\N{LATIN SMALL LETTER SHARP S}x]', $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr/[\N{LATIN SMALL LETTER SHARP S}x]/i, $message);
        like("\N{LATIN SMALL LETTER SHARP S}",
	     qr'[\N{LATIN SMALL LETTER SHARP S}x]'i, $message);
    }

    {
        # More whitespace: U+0085, U+2028, U+2029\n";

        # U+0085, U+00A0 need to be forced to be Unicode, the \x{100} does that.
        like "<\x{100}" . uni_to_native("\x{0085}") . ">", qr/<\x{100}\s>/, '\x{0085} in \s';
        like        "<" . uni_to_native("\x{0085}") . ">", qr/<\v>/, '\x{0085} in \v';
        like "<\x{100}" . uni_to_native("\x{00A0}") . ">", qr/<\x{100}\s>/, '\x{00A0} in \s';
        like        "<" . uni_to_native("\x{00A0}") . ">", qr/<\h>/, '\x{00A0} in \h';
        my @h = map {sprintf "%05x" => $_} 0x01680, 0x02000 .. 0x0200A,
                                           0x0202F, 0x0205F, 0x03000;
        my @v = map {sprintf "%05x" => $_} 0x02028, 0x02029;

        my @H = map {sprintf "%05x" => $_} 0x01361,   0x0200B, 0x02408, 0x02420,
                                           0x0303F,   0xE0020, 0x180E;
        my @V = map {sprintf "%05x" => $_} 0x0008A .. 0x0008D, 0x00348, 0x10100,
                                           0xE005F,   0xE007C, 0x180E;

        for my $hex (@h) {
            my $str = eval qq ["<\\x{$hex}>"];
            like $str, qr/<\s>/, "\\x{$hex} in \\s";
            like $str, qr/<\h>/, "\\x{$hex} in \\h";
            unlike $str, qr/<\v>/, "\\x{$hex} not in \\v";
        }

        for my $hex (@v) {
            my $str = eval qq ["<\\x{$hex}>"];
            like $str, qr/<\s>/, "\\x{$hex} in \\s";
            like $str, qr/<\v>/, "\\x{$hex} in \\v";
            unlike $str, qr/<\h>/, "\\x{$hex} not in \\h";
        }

        for my $hex (@H) {
            my $str = eval qq ["<\\x{$hex}>"];
            like $str, qr/<\S>/, "\\x{$hex} in \\S";
            like $str, qr/<\H>/, "\\x{$hex} in \\H";
        }

        for my $hex (@V) {
            my $str = eval qq ["<\\x{$hex}>"];
            like $str, qr/<\S>/, "\\x{$hex} in \\S";
            like $str, qr/<\V>/, "\\x{$hex} in \\V";
        }
    }

    {
        # . with /s should work on characters, as opposed to bytes
        my $message = ". with /s works on characters, not bytes";

        my $s = "\x{e4}\x{100}";
        # This is not expected to match: the point is that
        # neither should we get "Malformed UTF-8" warnings.
        warning_is(sub {$s =~ /\G(.+?)\n/gcs}, undef,
		   "No 'Malformed UTF-8' warning");

        my @c;
        push @c => $1 while $s =~ /\G(.)/gs;

        local $" = "";
        is("@c", $s, $message);

        # Test only chars < 256
        my $t1 = "Q003\n\n\x{e4}\x{f6}\n\nQ004\n\n\x{e7}";
        my $r1 = "";
        while ($t1 =~ / \G ( .+? ) \n\s+ ( .+? ) ( $ | \n\s+ ) /xgcs) {
        $r1 .= $1 . $2;
        }

        my $t2 = $t1 . "\x{100}"; # Repeat with a larger char
        my $r2 = "";
        while ($t2 =~ / \G ( .+? ) \n\s+ ( .+? ) ( $ | \n\s+ ) /xgcs) {
        $r2 .= $1 . $2;
        }
        $r2 =~ s/\x{100}//;

        is($r1, $r2, $message);
    }

    {
        my $message = "Unicode lookbehind";
        like("A\x{100}B",        qr/(?<=A.)B/, $message);
        like("A\x{200}\x{300}B", qr/(?<=A..)B/, $message);
        like("\x{400}AB",       qr/(?<=\x{400}.)B/, $message);
        like("\x{500}\x{600}B", qr/(?<=\x{500}.)B/, $message);

        # Original code also contained:
        # ok "\x{500\x{600}}B"  =~ /(?<=\x{500}.)B/;
        # but that looks like a typo.
    }

    {
        my $message = 'UTF-8 hash keys and /$/';
        # http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters
        #                                         /2002-01/msg01327.html

        my $u = "a\x{100}";
        my $v = substr ($u, 0, 1);
        my $w = substr ($u, 1, 1);
        my %u = ($u => $u, $v => $v, $w => $w);
        for (keys %u) {
            my $m1 =            /^\w*$/ ? 1 : 0;
            my $m2 = $u {$_} =~ /^\w*$/ ? 1 : 0;
            is($m1, $m2, $message);
        }
    }

    {
        my $message = "No SEGV in s/// and UTF-8";
        my $s = "s#\x{100}" x 4;
        ok($s =~ s/[^\w]/ /g, $message);
        if ( 1 or $ENV{PERL_TEST_LEGACY_POSIX_CC} ) {
            is($s, "s \x{100}" x 4, $message);
        }
        else {
            is($s, "s  " x 4, $message);
        }
    }

    {
        my $message = "UTF-8 bug (maybe already known?)";
        my $u = "foo";
        $u =~ s/./\x{100}/g;
        is($u, "\x{100}\x{100}\x{100}", $message);

        $u = "foobar";
        $u =~ s/[ao]/\x{100}/g;
        is($u, "f\x{100}\x{100}b\x{100}r", $message);

        $u =~ s/\x{100}/e/g;
        is($u, "feeber", $message);
    }

    {
        my $message = "UTF-8 bug with s///";
        # check utf8/non-utf8 mixtures
        # try to force all float/anchored check combinations

        my $c = "\x{100}";
        my $subst;
        for my $re ("xx.*$c", "x.*$c$c", "$c.*xx", "$c$c.*x",
                    "xx.*(?=$c)", "(?=$c).*xx",) {
            unlike("xxx", qr/$re/, $message);
            ok(+($subst = "xxx") !~ s/$re//, $message);
        }
        for my $re ("xx.*$c*", "$c*.*xx") {
            like("xxx", qr/$re/, $message);
            ok(+($subst = "xxx") =~ s/$re//, $message);
            is($subst, "", $message);
        }
        for my $re ("xxy*", "y*xx") {
            like("xx$c", qr/$re/, $message);
            ok(+($subst = "xx$c") =~ s/$re//, $message);
            is($subst, $c, $message);
            unlike("xy$c", qr/$re/, $message);
            ok(+($subst = "xy$c") !~ s/$re//, $message);
        }
        for my $re ("xy$c*z", "x$c*yz") {
            like("xyz", qr/$re/, $message);
            ok(+($subst = "xyz") =~ s/$re//, $message);
            is($subst, "", $message);
        }
    }

    {
        # The second half of RT #114808
        warning_is(sub {'aa' =~ /.+\x{100}/}, undef,
                   'utf8-only floating substr, non-utf8 target, no warning');
    }

    {
        my $message = "qr /.../x";
        my $R = qr / A B C # D E/x;
        ok("ABCDE" =~    $R   && $& eq "ABC", $message);
        ok("ABCDE" =~   /$R/  && $& eq "ABC", $message);
        ok("ABCDE" =~  m/$R/  && $& eq "ABC", $message);
        ok("ABCDE" =~  /($R)/ && $1 eq "ABC", $message);
        ok("ABCDE" =~ m/($R)/ && $1 eq "ABC", $message);
    }

    {
        local $\;
        $_ = 'aaaaaaaaaa';
        utf8::upgrade($_); chop $_; $\="\n";
        ok /[^\s]+/, 'm/[^\s]/ utf8';
        ok /[^\d]+/, 'm/[^\d]/ utf8';
        ok +($a = $_, $_ =~ s/[^\s]+/./g), 's/[^\s]/ utf8';
        ok +($a = $_, $a =~ s/[^\d]+/./g), 's/[^\s]/ utf8';
    }

    {
        # Subject: Odd regexp behavior
        # From: Markus Kuhn <Markus.Kuhn@cl.cam.ac.uk>
        # Date: Wed, 26 Feb 2003 16:53:12 +0000
        # Message-Id: <E18o4nw-0008Ly-00@wisbech.cl.cam.ac.uk>
        # To: perl-unicode@perl.org

        my $message = 'Markus Kuhn 2003-02-26';

        my $x = "\x{2019}\nk";
        ok($x =~ s/(\S)\n(\S)/$1 $2/sg, $message);
        is($x, "\x{2019} k", $message);

        $x = "b\nk";
        ok($x =~ s/(\S)\n(\S)/$1 $2/sg, $message);
        is($x, "b k", $message);

        like("\x{2019}", qr/\S/, $message);
    }

    {
        like "\x{100}\n", qr/\x{100}\n$/, "UTF-8 length cache and fbm_compile";
    }

    {
        package Str;
        use overload q /""/ => sub {${$_ [0]};};
        sub new {my ($c, $v) = @_; bless \$v, $c;}

        package main;
        $_ = Str -> new ("a\x{100}/\x{100}b");
        ok join (":", /\b(.)\x{100}/g) eq "a:/", "re_intuit_start and PL_bostr";
    }

    {
        my $re = qq /^([^X]*)X/;
        utf8::upgrade ($re);
        like "\x{100}X", qr/$re/, "S_cl_and ANYOF_UNICODE & ANYOF_INVERTED";
        my $loc_re = qq /(?l:^([^X]*)X)/;
        utf8::upgrade ($loc_re);
        no warnings 'locale';
        like "\x{100}X", qr/$loc_re/, "locale, S_cl_and ANYOF_UNICODE & ANYOF_INVERTED";
    }

    {
        like "123\x{100}", qr/^.*1.*23\x{100}$/,
           'UTF-8 + multiple floating substr';
    }

    {
        my $message = '<20030808193656.5109.1@llama.ni-s.u-net.com>';

        # LATIN SMALL/CAPITAL LETTER A WITH MACRON
        like("  \x{101}", qr/\x{100}/i, $message);

        # LATIN SMALL/CAPITAL LETTER A WITH RING BELOW
        like("  \x{1E01}", qr/\x{1E00}/i, $message);

        # DESERET SMALL/CAPITAL LETTER LONG I
        like("  \x{10428}", qr/\x{10400}/i, $message);

        # LATIN SMALL/CAPITAL LETTER A WITH RING BELOW + 'X'
        like("  \x{1E01}x", qr/\x{1E00}X/i, $message);
    }

    {
        for (120 .. 130, 240 .. 260) {
            my $head = 'x' x $_;
            my $message = q [Don't misparse \x{...} in regexp ] .
                             q [near EXACT char count limit];
            for my $tail ('\x{0061}', '\x{1234}', '\x61') {
                eval qq{like("$head$tail", qr/$head$tail/, \$message)};
		is($@, '', $message);
            }
            $message = q [Don't misparse \N{...} in regexp ] .
                             q [near EXACT char count limit];
            for my $tail ('\N{SNOWFLAKE}') {
                eval qq {use charnames ':full';
                         like("$head$tail", qr/$head$tail/, \$message)};
                eval qq {use charnames ':full';
                         like("$head$tail", qr'$head$tail', \$message)};
		is($@, '', $message);
            }
        }
    }

    {   # TRIE related
        our @got = ();
        "words" =~ /(word|word|word)(?{push @got, $1})s$/;
        is(@got, 1, "TRIE optimisation");

        @got = ();
        "words" =~ /(word|word|word)(?{push @got,$1})s$/i;
        is(@got, 1,"TRIEF optimisation");

        my @nums = map {int rand 1000} 1 .. 100;
        my $re = "(" . (join "|", @nums) . ")";
        $re = qr/\b$re\b/;

        foreach (@nums) {
            like $_, qr/$re/, "Trie nums";
        }

        $_ = join " ", @nums;
        @got = ();
        push @got, $1 while /$re/g;

        my %count;
        $count {$_} ++ for @got;
        my $ok = 1;
        for (@nums) {
            $ok = 0 if --$count {$_} < 0;
        }
        ok $ok, "Trie min count matches";
    }

    {
        # TRIE related
        # LATIN SMALL/CAPITAL LETTER A WITH MACRON
        ok "foba  \x{101}foo" =~ qr/(foo|\x{100}foo|bar)/i &&
           $1 eq "\x{101}foo",
           "TRIEF + LATIN SMALL/CAPITAL LETTER A WITH MACRON";

        # LATIN SMALL/CAPITAL LETTER A WITH RING BELOW
        ok "foba  \x{1E01}foo" =~ qr/(foo|\x{1E00}foo|bar)/i &&
           $1 eq "\x{1E01}foo",
           "TRIEF + LATIN SMALL/CAPITAL LETTER A WITH RING BELOW";

        # DESERET SMALL/CAPITAL LETTER LONG I
        ok "foba  \x{10428}foo" =~ qr/(foo|\x{10400}foo|bar)/i &&
           $1 eq "\x{10428}foo",
           "TRIEF + DESERET SMALL/CAPITAL LETTER LONG I";

        # LATIN SMALL/CAPITAL LETTER A WITH RING BELOW + 'X'
        ok "foba  \x{1E01}xfoo" =~ qr/(foo|\x{1E00}Xfoo|bar)/i &&
           $1 eq "\x{1E01}xfoo",
           "TRIEF + LATIN SMALL/CAPITAL LETTER A WITH RING BELOW + 'X'";

        use charnames ':full';

        my $s = "\N{LATIN SMALL LETTER SHARP S}";
        ok "foba  ba$s" =~ qr/(foo|Ba$s|bar)/i &&  $1 eq "ba$s",
           "TRIEF + LATIN SMALL LETTER SHARP S =~ ss";
        ok "foba  ba$s" =~ qr/(Ba$s|foo|bar)/i &&  $1 eq "ba$s",
           "TRIEF + LATIN SMALL LETTER SHARP S =~ ss";
        ok "foba  ba$s" =~ qr/(foo|bar|Ba$s)/i &&  $1 eq "ba$s",
           "TRIEF + LATIN SMALL LETTER SHARP S =~ ss";

        ok "foba  ba$s" =~ qr/(foo|Bass|bar)/i &&  $1 eq "ba$s",
           "TRIEF + LATIN SMALL LETTER SHARP S =~ ss";

        ok "foba  ba$s" =~ qr/(foo|BaSS|bar)/i &&  $1 eq "ba$s",
           "TRIEF + LATIN SMALL LETTER SHARP S =~ SS";

        ok "foba  ba${s}pxySS$s$s" =~ qr/(b(?:a${s}t|a${s}f|a${s}p)[xy]+$s*)/i
            &&  $1 eq "ba${s}pxySS$s$s",
           "COMMON PREFIX TRIEF + LATIN SMALL LETTER SHARP S";
    }

    {
	BEGIN {
	    unshift @INC, 'lib';
	}
        use Cname;  # Our custom charname plugin, currently found in
                    # t/lib/Cname.pm

        like 'fooB', qr/\N{foo}[\N{B}\N{b}]/, "Passthrough charname";
        my $name = "foo\xDF";
        my $result = eval "'A${name}B'  =~ /^A\\N{$name}B\$/";
        ok !$@ && $result,  "Passthrough charname of non-ASCII, Latin1";
        eval "qr/\\p{name=foo}/";
        like($@, qr/Can't find Unicode property definition "name=foo"/,
                '\p{name=} doesn\'t see a cumstom charnames translator');
        #
        # Why doesn't must_warn work here?
        #
        my $w;
        local $SIG {__WARN__} = sub {$w .= "@_"};
        $result = eval 'q(WARN) =~ /^[\N{WARN}]$/';
        ok !$@ && $result && ! $w,  '\N{} returning multi-char works';

        undef $w;
        eval q [unlike "\0", qr/[\N{EMPTY-STR}XY]/,
                   "Zerolength charname in charclass doesn't match \\\\0"];
        ok $w && $w =~ /Ignoring zero length/,
                 'Ignoring zero length \N{} in character class warning';
        undef $w;
        eval q [like 'xy', qr/x[\N{EMPTY-STR} y]/x,
                    'Empty string charname in [] is ignored; finds a following character'];
        ok $w && $w =~ /Ignoring zero length/,
                 'Ignoring zero length \N{} in character class warning';
        undef $w;
        eval q [like 'x ', qr/x[\N{EMPTY-STR} y]/,
                    'Empty string charname in [] is ignored; finds a following blank under /x'];
        like $w, qr/Ignoring zero length/,
                 'Ignoring zero length \N{} in character class warning';

        # EVIL keeps track of its calls, and appends a new character each
        # time: A AB ABC ABCD ...
        ok 'AB'  =~ /(\N{EVIL})/ && $1 eq 'A', 'Charname caching $1';
        like 'ABC', qr/(\N{EVIL})/,              'Charname caching $1';
        ok 'ABCD'  =~ m'(\N{EVIL})' && $1 eq 'ABC', 'Charname caching $1';
        ok 'ABCDE'  =~ m'(\N{EVIL})',          'Charname caching $1';
        like 'xy',  qr/x\N{EMPTY-STR}y/,
                    'Empty string charname produces NOTHING node';
        ok 'xy'  =~ 'x\N{EMPTY-STR}y',
                    'Empty string charname produces NOTHING node';
        like '', qr/\N{EMPTY-STR}/,
                    'Empty string charname produces NOTHING node';
        like "\N{LONG-STR}", qr/^\N{LONG-STR}$/, 'Verify that long string works';
        like "\N{LONG-STR}", qr/^\N{LONG-STR}$/i, 'Verify under folding that long string works';

        # perlhacktips points out that these work on both ASCII and EBCDIC
        like "\xfc", qr/\N{EMPTY-STR}\xdc/i, 'Empty \N{} should change /d to /u';
        like "\xfc", qr'\N{EMPTY-STR}\xdc'i, 'Empty \N{} should change /d to /u';

        eval '/(?[[\N{EMPTY-STR}]])/';
        like $@, qr/Zero length \\N\{\}/, 'Verify zero-length return from \N{} correctly fails';
        ok "\N{LONG-STR}" =~ /^\N{LONG-STR}$/, 'Verify that long string works';
        ok "\N{LONG-STR}" =~ '^\N{LONG-STR}$', 'Verify that long string works';
        ok "\N{LONG-STR}" =~ /^\N{LONG-STR}$/i, 'Verify under folding that long string works';
        ok "\N{LONG-STR}" =~ m'^\N{LONG-STR}$'i, 'Verify under folding that long string works';

        undef $w;
        {
            () = eval q ["\N{TOO  MANY SPACES}"];
            like ($@, qr/charnames alias definitions may not contain a sequence of multiple spaces/, "Multiple spaces in a row in a charnames alias is fatal");
            eval q [use utf8; () = "\N{TOO  MANY SPACES}"];
            like ($@, qr/charnames alias definitions may not contain a sequence of multiple spaces/,  "... same under utf8");
        }

        undef $w;
        my $Cedilla_Latin1 = "GAR"
                           . uni_to_native("\xC7")
                           . "ON";
        my $Cedilla_utf8 = $Cedilla_Latin1;
        utf8::upgrade($Cedilla_utf8);
        eval qq[is("\\N{$Cedilla_Latin1}", "$Cedilla_Latin1", "A cedilla in character name works")];
        undef $w;
            {
            use feature 'unicode_eval';
            eval qq[use utf8; is("\\N{$Cedilla_utf8}", "$Cedilla_utf8", "... same under 'use utf8': they work")];
        }

        undef $w;
        my $NBSP_Latin1 = "NBSP"
                        . uni_to_native("\xA0")
                        . "SEPARATED"
                        . uni_to_native("\xA0")
                        . "SPACE";
        my $NBSP_utf8 = $NBSP_Latin1;
        utf8::upgrade($NBSP_utf8);
        () = eval qq[is("\\N{$NBSP_Latin1}", "$NBSP_Latin1"];
        like ($@, qr/Invalid character in \\N\{...}/, "A NO-BREAK SPACE in a charnames alias is fatal");
        undef $w;
            {
            use feature 'unicode_eval';
            eval qq[use utf8; is("\\N{$NBSP_utf8}"];
            like ($@, qr/Invalid character in \\N\{...}/, "A NO-BREAK SPACE in a charnames alias is fatal");
        }

        {
            BEGIN { no strict; *CnameTest:: = *{"_charnames\0A::" } }
            package CnameTest { sub translator { pop } }
            BEGIN { $^H{charnames} = \&CnameTest::translator }
            undef $w;
            () = eval q ["\N{TOO  MANY SPACES}"];
            like ($@, qr/charnames alias definitions may not contain a sequence of multiple spaces/,
                 'translators in _charnames\0* packages get validated');
        }

        # If remove the limitation in regcomp code these should work
        # differently
        undef $w;
        eval q [like "\N{TOO-LONG-STR}" =~ /^\N{TOO-LONG-STR}$/, 'Verify that what once was too long a string works'];
        eval 'q() =~ /\N{4F}/';
        ok $@ && $@ =~ /Invalid character/, 'Verify that leading digit in name gives error';
        eval 'q() =~ /\N{COM,MA}/';
        ok $@ && $@ =~ /Invalid character/, 'Verify that comma in name gives error';
        $name = "A" . uni_to_native("\x{D7}") . "O";
        eval "q(W) =~ /\\N{$name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that latin1 symbol in name gives error';
        my $utf8_name = "7 CITIES OF GOLD";
        utf8::upgrade($utf8_name);
        eval "use utf8; q(W) =~ /\\N{$utf8_name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that leading digit in utf8 name gives error';
        $utf8_name = "SHARP #";
        utf8::upgrade($utf8_name);
        eval "use utf8; q(W) =~ /\\N{$utf8_name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that ASCII symbol in utf8 name gives error';
        $utf8_name = "A HOUSE " . uni_to_native("\xF7") . " AGAINST ITSELF";
        utf8::upgrade($utf8_name);
        eval "use utf8; q(W) =~ /\\N{$utf8_name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that latin1 symbol in utf8 name gives error';
        $utf8_name = "\x{664} HORSEMEN}";
        eval "use utf8; q(W) =~ /\\N{$utf8_name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that leading above Latin1 digit in utf8 name gives error';
        $utf8_name = "A \x{1F4A9} WOULD SMELL AS SWEET}";
        eval "use utf8; q(W) =~ /\\N{$utf8_name}/";
        ok $@ && $@ =~ /Invalid character/, 'Verify that above Latin1 symbol in utf8 name gives error';

        undef $w;
        $name = "A" . uni_to_native("\x{D1}") . "O";
        eval "q(W) =~ /\\N{$name}/";
        ok ! $w, 'Verify that latin1 letter in name doesnt give warning';

        # This tests the code path that restarts the parse when the recursive
        # call to S_reg() from within S_grok_bslash_N() discovers that the
        # pattern needs to be recalculated as UTF-8.  use eval to avoid
        # needing literal Unicode in this source file:
        my $r = eval "qr/\\N{\x{100}\x{100}}/";
        isnt $r, undef, "Generated regex for multi-char UTF-8 charname"
	    or diag($@);
        like "\x{100}\x{100}", $r, "which matches";
    }

    {
        use charnames ':full';

        unlike 'aabc', qr/a\N{PLUS SIGN}b/, '/a\N{PLUS SIGN}b/ against aabc';
        like 'a+bc', qr/a\N{PLUS SIGN}b/, '/a\N{PLUS SIGN}b/ against a+bc';

        like ' A B', qr/\N{SPACE}\N{U+0041}\N{SPACE}\N{U+0042}/,
            'Intermixed named and unicode escapes';
        like "\N{SPACE}\N{U+0041}\N{SPACE}\N{U+0042}",
             qr/\N{SPACE}\N{U+0041}\N{SPACE}\N{U+0042}/,
            'Intermixed named and unicode escapes';
        like "\N{SPACE}\N{U+0041}\N{SPACE}\N{U+0042}",
            qr/[\N{SPACE}\N{U+0041}][\N{SPACE}\N{U+0042}]/,
            'Intermixed named and unicode escapes';
        like "\0", qr/^\N{NULL}$/, 'Verify that \N{NULL} works; is not confused with an error';
    }

    {
        our $brackets;
        $brackets = qr{
            {  (?> [^{}]+ | (??{ $brackets }) )* }
        }x;

        unlike "{b{c}d", qr/^((??{ $brackets }))/, "Bracket mismatch";

        SKIP: {
            our @stack = ();
            my @expect = qw(
                stuff1
                stuff2
                <stuff1>and<stuff2>
                right
                <right>
                <<right>>
                <<<right>>>
                <<stuff1>and<stuff2>><<<<right>>>>
            );

            local $_ = '<<<stuff1>and<stuff2>><<<<right>>>>>';
            ok /^(<((?:(?>[^<>]+)|(?1))*)>(?{push @stack, $2 }))$/,
                "Recursion matches";
            is(@stack, @expect, "Right amount of matches")
                 or skip "Won't test individual results as count isn't equal",
                          0 + @expect;
            my $idx = 0;
            foreach my $expect (@expect) {
                is($stack [$idx], $expect,
		   "Expecting '$expect' at stack pos #$idx");
                $idx ++;
            }
        }
    }

    {
        my $s = '123453456';
        $s =~ s/(?<digits>\d+)\k<digits>/$+{digits}/;
        ok $s eq '123456', 'Named capture (angle brackets) s///';
        $s = '123453456';
        $s =~ s/(?'digits'\d+)\k'digits'/$+{digits}/;
        ok $s eq '123456', 'Named capture (single quotes) s///';
    }

    {
        my @ary = (
            pack('U', utf8::unicode_to_native(0x00F1)), # n-tilde
            '_'.pack('U', utf8::unicode_to_native(0x00F1)), # _ + n-tilde
            'c'.pack('U', 0x0327),        # c + cedilla
            pack('U*', utf8::unicode_to_native(0x00F1), 0x0327),# n-tilde + cedilla
            pack('U', 0x0391),            # ALPHA
            pack('U', 0x0391).'2',        # ALPHA + 2
            pack('U', 0x0391).'_',        # ALPHA + _
        );

        for my $uni (@ary) {
            my ($r1, $c1, $r2, $c2) = eval qq {
                use utf8;
                scalar ("..foo foo.." =~ /(?'${uni}'foo) \\k'${uni}'/),
                        \$+{${uni}},
                scalar ("..bar bar.." =~ /(?<${uni}>bar) \\k<${uni}>/),
                        \$+{${uni}};
            };
            ok $r1,                         "Named capture UTF (?'')";
            ok defined $c1 && $c1 eq 'foo', "Named capture UTF \%+";
            ok $r2,                         "Named capture UTF (?<>)";
            ok defined $c2 && $c2 eq 'bar', "Named capture UTF \%+";
        }
    }

    {
        my $s = 'foo bar baz';
        my @res;
        if ('1234' =~ /(?<A>1)(?<B>2)(?<A>3)(?<B>4)/) {
            foreach my $name (sort keys(%-)) {
                my $ary = $- {$name};
                foreach my $idx (0 .. $#$ary) {
                    push @res, "$name:$idx:$ary->[$idx]";
                }
            }
        }
        my @expect = qw (A:0:1 A:1:3 B:0:2 B:1:4);
        is("@res", "@expect", "Check %-");
        eval'
            no warnings "uninitialized";
            print for $- {this_key_doesnt_exist};
        ';
        ok !$@,'lvalue $- {...} should not throw an exception';
    }

    {
        # \c\ followed by _
        unlike "x\c_y", qr/x\c\_y/,    '\_ in a pattern';
        like "x\c\_y", qr/x\c\_y/,    '\_ in a pattern';

        # \c\ followed by other characters
        for my $c ("z", "\0", "!", chr(254), chr(256)) {
            my $targ = "a" . uni_to_native("\034") . "$c";
            my $reg  = "a\\c\\$c";
            ok eval ("qq/$targ/ =~ /$reg/"), "\\c\\ in pattern";
        }
    }

    {   # Test the (*PRUNE) pattern
        our $count = 0;
        'aaab' =~ /a+b?(?{$count++})(*FAIL)/;
        is($count, 9, "Expect 9 for no (*PRUNE)");
        $count = 0;
        'aaab' =~ /a+b?(*PRUNE)(?{$count++})(*FAIL)/;
        is($count, 3, "Expect 3 with (*PRUNE)");
        local $_ = 'aaab';
        $count = 0;
        1 while /.(*PRUNE)(?{$count++})(*FAIL)/g;
        is($count, 4, "/.(*PRUNE)/");
        $count = 0;
        'aaab' =~ /a+b?(??{'(*PRUNE)'})(?{$count++})(*FAIL)/;
        is($count, 3, "Expect 3 with (*PRUNE)");
        local $_ = 'aaab';
        $count = 0;
        1 while /.(??{'(*PRUNE)'})(?{$count++})(*FAIL)/g;
        is($count, 4, "/.(*PRUNE)/");
    }

    {   # Test the (*SKIP) pattern
        our $count = 0;
        'aaab' =~ /a+b?(*SKIP)(?{$count++})(*FAIL)/;
        is($count, 1, "Expect 1 with (*SKIP)");
        local $_ = 'aaab';
        $count = 0;
        1 while /.(*SKIP)(?{$count++})(*FAIL)/g;
        is($count, 4, "/.(*SKIP)/");
        $_ = 'aaabaaab';
        $count = 0;
        our @res = ();
        1 while /(a+b?)(*SKIP)(?{$count++; push @res,$1})(*FAIL)/g;
        is($count, 2, "Expect 2 with (*SKIP)");
        is("@res", "aaab aaab", "Adjacent (*SKIP) works as expected");
    }

    {   # Test the (*SKIP) pattern
        our $count = 0;
        'aaab' =~ /a+b?(*MARK:foo)(*SKIP)(?{$count++})(*FAIL)/;
        is($count, 1, "Expect 1 with (*SKIP)");
        local $_ = 'aaab';
        $count = 0;
        1 while /.(*MARK:foo)(*SKIP)(?{$count++})(*FAIL)/g;
        is($count, 4, "/.(*SKIP)/");
        $_ = 'aaabaaab';
        $count = 0;
        our @res = ();
        1 while /(a+b?)(*MARK:foo)(*SKIP)(?{$count++; push @res,$1})(*FAIL)/g;
        is($count, 2, "Expect 2 with (*SKIP)");
        is("@res", "aaab aaab", "Adjacent (*SKIP) works as expected");
    }

    {   # Test the (*SKIP) pattern
        our $count = 0;
        'aaab' =~ /a*(*MARK:a)b?(*MARK:b)(*SKIP:a)(?{$count++})(*FAIL)/;
        is($count, 3, "Expect 3 with *MARK:a)b?(*MARK:b)(*SKIP:a)");
        local $_ = 'aaabaaab';
        $count = 0;
        our @res = ();
        1 while
        /(a*(*MARK:a)b?)(*MARK:x)(*SKIP:a)(?{$count++; push @res,$1})(*FAIL)/g;
        is($count, 5, "Expect 5 with (*MARK:a)b?)(*MARK:x)(*SKIP:a)");
        is("@res", "aaab b aaab b ",
	   "Adjacent (*MARK:a)b?)(*MARK:x)(*SKIP:a) works as expected");
    }

    {   # Test the (*COMMIT) pattern
        our $count = 0;
        'aaabaaab' =~ /a+b?(*COMMIT)(?{$count++})(*FAIL)/;
        is($count, 1, "Expect 1 with (*COMMIT)");
        local $_ = 'aaab';
        $count = 0;
        1 while /.(*COMMIT)(?{$count++})(*FAIL)/g;
        is($count, 1, "/.(*COMMIT)/");
        $_ = 'aaabaaab';
        $count = 0;
        our @res = ();
        1 while /(a+b?)(*COMMIT)(?{$count++; push @res,$1})(*FAIL)/g;
        is($count, 1, "Expect 1 with (*COMMIT)");
        is("@res", "aaab", "Adjacent (*COMMIT) works as expected");

	unlike("1\n2a\n", qr/^\d+(*COMMIT)\w+/m, "COMMIT and anchors");
    }

    {
        # Test named commits and the $REGERROR var
        local $REGERROR;
        for my $name ('', ':foo') {
            for my $pat ("(*PRUNE$name)",
                         ($name ? "(*MARK$name)" : "") . "(*SKIP$name)",
                         "(*COMMIT$name)") {
                for my $suffix ('(*FAIL)', '') {
                    'aaaab' =~ /a+b$pat$suffix/;
                    is($REGERROR,
                         ($suffix ? ($name ? 'foo' : "1") : ""),
                        "Test $pat and \$REGERROR $suffix");
                }
            }
        }
    }

    {
        # Test named commits and the $REGERROR var
        package Fnorble;
        our $REGERROR;
        local $REGERROR;
        for my $name ('', ':foo') {
            for my $pat ("(*PRUNE$name)",
                         ($name ? "(*MARK$name)" : "") . "(*SKIP$name)",
                         "(*COMMIT$name)") {
                for my $suffix ('(*FAIL)','') {
                    'aaaab' =~ /a+b$pat$suffix/;
		    ::is($REGERROR,
                         ($suffix ? ($name ? 'foo' : "1") : ""),
			 "Test $pat and \$REGERROR $suffix");
                }
            }
        }
    }

    {
        # Test named commits and the $REGERROR var
	my $message = '$REGERROR';
        local $REGERROR;
        for my $word (qw (bar baz bop)) {
            $REGERROR = "";
            "aaaaa$word" =~
              /a+(?:bar(*COMMIT:bar)|baz(*COMMIT:baz)|bop(*COMMIT:bop))(*FAIL)/;
            is($REGERROR, $word, $message);
        }
    }

    {
        #Mindnumbingly simple test of (*THEN)
        for ("ABC","BAX") {
            ok /A (*THEN) X | B (*THEN) C/x, "Simple (*THEN) test";
        }
    }

    {
        my $message = "Relative Recursion";
        my $parens = qr/(\((?:[^()]++|(?-1))*+\))/;
        local $_ = 'foo((2*3)+4-3) + bar(2*(3+4)-1*(2-3))';
        my ($all, $one, $two) = ('', '', '');
        ok(m/foo $parens \s* \+ \s* bar $parens/x, $message);
        is($1, '((2*3)+4-3)', $message);
        is($2, '(2*(3+4)-1*(2-3))', $message);
        is($&, 'foo((2*3)+4-3) + bar(2*(3+4)-1*(2-3))', $message);
        is($&, $_, $message);
    }

    {
        my $spaces="      ";
        local $_ = join 'bar', $spaces, $spaces;
        our $count = 0;
        s/(?>\s+bar)(?{$count++})//g;
        is($_, $spaces, "SUSPEND final string");
        is($count, 1, "Optimiser should have prevented more than one match");
    }

    {
        # From Message-ID: <877ixs6oa6.fsf@k75.linux.bogus>
        my $dow_name = "nada";
        my $parser = "(\$dow_name) = \$time_string =~ /(D\x{e9}\\ " .
                     "C\x{e9}adaoin|D\x{e9}\\ Sathairn|\\w+|\x{100})/";
        my $time_string = "D\x{e9} C\x{e9}adaoin";
        eval $parser;
        ok !$@, "Test Eval worked";
        is($dow_name, $time_string, "UTF-8 trie common prefix extraction");
    }

    {
        my $v;
        ($v = 'bar') =~ /(\w+)/g;
        $v = 'foo';
        is("$1", 'bar',
	   '$1 is safe after /g - may fail due to specialized config in pp_hot.c');
    }

    {
        my $message = "http://nntp.perl.org/group/perl.perl5.porters/118663";
        my $qr_barR1 = qr/(bar)\g-1/;
        like("foobarbarxyz", $qr_barR1, $message);
        like("foobarbarxyz", qr/foo${qr_barR1}xyz/, $message);
        like("foobarbarxyz", qr/(foo)${qr_barR1}xyz/, $message);
        like("foobarbarxyz", qr/(foo)(bar)\g{-1}xyz/, $message);
        like("foobarbarxyz", qr/(foo${qr_barR1})xyz/, $message);
        like("foobarbarxyz", qr/(foo(bar)\g{-1})xyz/, $message);
    }

    {
        my $message = '$REGMARK';
        our @r = ();
        local $REGMARK;
        local $REGERROR;
        like('foofoo', qr/foo (*MARK:foo) (?{push @r,$REGMARK}) /x, $message);
        is("@r","foo", $message);
        is($REGMARK, "foo", $message);
        unlike('foofoo', qr/foo (*MARK:foo) (*FAIL) /x, $message);
        is($REGMARK, '', $message);
        is($REGERROR, 'foo', $message);
    }

    {
        my $message = '\K test';
        my $x;
        $x = "abc.def.ghi.jkl";
        $x =~ s/.*\K\..*//;
        is($x, "abc.def.ghi", $message);

        $x = "one two three four";
        $x =~ s/o+ \Kthree//g;
        is($x, "one two  four", $message);

        $x = "abcde";
        $x =~ s/(.)\K/$1/g;
        is($x, "aabbccddee", $message);
    }

    {
        sub kt {
            return '4' if $_[0] eq '09028623';
        }
        # Nested EVAL using PL_curpm (via $1 or friends)
        my $re;
        our $grabit = qr/ ([0-6][0-9]{7}) (??{ kt $1 }) [890] /x;
        $re = qr/^ ( (??{ $grabit }) ) $ /x;
        my @res = '0902862349' =~ $re;
        is(join ("-", @res), "0902862349",
	   'PL_curpm is set properly on nested eval');

        our $qr = qr/ (o) (??{ $1 }) /x;
        ok 'boob'=~/( b (??{ $qr }) b )/x && 1, "PL_curpm, nested eval";
    }

    {
        use charnames ":full";
        like "\N{ROMAN NUMERAL ONE}", qr/\p{Alphabetic}/, "I =~ Alphabetic";
        like "\N{ROMAN NUMERAL ONE}", qr/\p{Uppercase}/,  "I =~ Uppercase";
        unlike "\N{ROMAN NUMERAL ONE}", qr/\p{Lowercase}/,  "I !~ Lowercase";
        like "\N{ROMAN NUMERAL ONE}", qr/\p{IDStart}/,    "I =~ ID_Start";
        like "\N{ROMAN NUMERAL ONE}", qr/\p{IDContinue}/, "I =~ ID_Continue";
        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Alphabetic}/, "i =~ Alphabetic";
        unlike "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Uppercase}/,  "i !~ Uppercase";
        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Uppercase}/i,  "i =~ Uppercase under /i";
        unlike "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Titlecase}/,  "i !~ Titlecase";
        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Titlecase}/i,  "i =~ Titlecase under /i";
        like "\N{ROMAN NUMERAL ONE}", qr/\p{Lowercase}/i,  "I =~ Lowercase under /i";

        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{Lowercase}/,  "i =~ Lowercase";
        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{IDStart}/,    "i =~ ID_Start";
        like "\N{SMALL ROMAN NUMERAL ONE}", qr/\p{IDContinue}/, "i =~ ID_Continue"
    }

    {   # More checking that /i works on the few properties that it makes a
        # difference.  Uppercase, Lowercase, and Titlecase were done in the
        # block above
        like "A", qr/\p{PosixUpper}/,  "A =~ PosixUpper";
        like "A", qr/\p{PosixUpper}/i,  "A =~ PosixUpper under /i";
        unlike "A", qr/\p{PosixLower}/,  "A !~ PosixLower";
        like "A", qr/\p{PosixLower}/i,  "A =~ PosixLower under /i";
        unlike "a", qr/\p{PosixUpper}/,  "a !~ PosixUpper";
        like "a", qr/\p{PosixUpper}/i,  "a =~ PosixUpper under /i";
        like "a", qr/\p{PosixLower}/,  "a =~ PosixLower";
        like "a", qr/\p{PosixLower}/i,  "a =~ PosixLower under /i";

        like uni_to_native("\xC0"), qr/\p{XPosixUpper}/,  "\\xC0 =~ XPosixUpper";
        like uni_to_native("\xC0"), qr/\p{XPosixUpper}/i,  "\\xC0 =~ XPosixUpper under /i";
        unlike uni_to_native("\xC0"), qr/\p{XPosixLower}/,  "\\xC0 !~ XPosixLower";
        like uni_to_native("\xC0"), qr/\p{XPosixLower}/i,  "\\xC0 =~ XPosixLower under /i";
        unlike uni_to_native("\xE0"), qr/\p{XPosixUpper}/,  "\\xE0 !~ XPosixUpper";
        like uni_to_native("\xE0"), qr/\p{XPosixUpper}/i,  "\\xE0 =~ XPosixUpper under /i";
        like uni_to_native("\xE0"), qr/\p{XPosixLower}/,  "\\xE0 =~ XPosixLower";
        like uni_to_native("\xE0"), qr/\p{XPosixLower}/i,  "\\xE0 =~ XPosixLower under /i";

        like uni_to_native("\xC0"), qr/\p{UppercaseLetter}/,  "\\xC0 =~ UppercaseLetter";
        like uni_to_native("\xC0"), qr/\p{UppercaseLetter}/i,  "\\xC0 =~ UppercaseLetter under /i";
        unlike uni_to_native("\xC0"), qr/\p{LowercaseLetter}/,  "\\xC0 !~ LowercaseLetter";
        like uni_to_native("\xC0"), qr/\p{LowercaseLetter}/i,  "\\xC0 =~ LowercaseLetter under /i";
        unlike uni_to_native("\xC0"), qr/\p{TitlecaseLetter}/,  "\\xC0 !~ TitlecaseLetter";
        like uni_to_native("\xC0"), qr/\p{TitlecaseLetter}/i,  "\\xC0 =~ TitlecaseLetter under /i";
        unlike uni_to_native("\xE0"), qr/\p{UppercaseLetter}/,  "\\xE0 !~ UppercaseLetter";
        like uni_to_native("\xE0"), qr/\p{UppercaseLetter}/i,  "\\xE0 =~ UppercaseLetter under /i";
        like uni_to_native("\xE0"), qr/\p{LowercaseLetter}/,  "\\xE0 =~ LowercaseLetter";
        like uni_to_native("\xE0"), qr/\p{LowercaseLetter}/i,  "\\xE0 =~ LowercaseLetter under /i";
        unlike uni_to_native("\xE0"), qr/\p{TitlecaseLetter}/,  "\\xE0 !~ TitlecaseLetter";
        like uni_to_native("\xE0"), qr/\p{TitlecaseLetter}/i,  "\\xE0 =~ TitlecaseLetter under /i";
        unlike "\x{1C5}", qr/\p{UppercaseLetter}/,  "\\x{1C5} !~ UppercaseLetter";
        like "\x{1C5}", qr/\p{UppercaseLetter}/i,  "\\x{1C5} =~ UppercaseLetter under /i";
        unlike "\x{1C5}", qr/\p{LowercaseLetter}/,  "\\x{1C5} !~ LowercaseLetter";
        like "\x{1C5}", qr/\p{LowercaseLetter}/i,  "\\x{1C5} =~ LowercaseLetter under /i";
        like "\x{1C5}", qr/\p{TitlecaseLetter}/,  "\\x{1C5} =~ TitlecaseLetter";
        like "\x{1C5}", qr/\p{TitlecaseLetter}/i,  "\\x{1C5} =~ TitlecaseLetter under /i";
    }

    {
        # requirement of Unicode Technical Standard #18, 1.7 Code Points
        # cf. http://www.unicode.org/reports/tr18/#Supplementary_Characters
        for my $u (0x7FF, 0x800, 0xFFFF, 0x10000) {
            no warnings 'utf8'; # oops
            my $c = chr $u;
            my $x = sprintf '%04X', $u;
            like "A${c}B", qr/A[\0-\x{10000}]B/, "Unicode range - $x";
        }
    }

    {
        no warnings 'uninitialized';
        my $res = "";

        if ('1' =~ /(?|(?<digit>1)|(?<digit>2))/) {
            $res = "@{$- {digit}}";
        }
        is($res, "1 ",
	   "Check that repeated named captures in branch reset (?|...) work as expected");
        if ('2' =~ /(?|(?<digit>1)|(?<digit>2))/) {
            $res = "@{$- {digit}}";
        }
        is($res, " 2",
	   "Check that repeated named captures in branch reset (?|...) work as expected");

        $res = "";
        if ('11' =~ /(?|(?<digit>1)|(?<digit>2))(?&digit)/) {
            $res = "@{$- {digit}}";
        }
        is($res, "1 ",
	   "Check that (?&..) to a buffer inside a (?|...) goes to the leftmost");
    }

    {
        use warnings;
        my $message = "ASCII pattern that really is UTF-8";
        my @w;
        local $SIG {__WARN__} = sub {push @w, "@_"};
        my $c = qq (\x{DF});
        like($c, qr/${c}|\x{100}/, $message);
        is("@w", '', $message);
    }

    {
        my $message = "Corruption of match results of qr// across scopes";
        my $qr = qr/(fo+)(ba+r)/;
        'foobar' =~ /$qr/;
        is("$1$2", "foobar", $message);
        {
            'foooooobaaaaar' =~ /$qr/;
            is("$1$2", 'foooooobaaaaar', $message);
        }
        is("$1$2", "foobar", $message);
    }

    {
        my $message = "HORIZWS";
        local $_ = "\t \r\n \n \t".chr(11)."\n";
        s/\H/H/g;
        s/\h/h/g;
        is($_, "hhHHhHhhHH", $message);
        $_ = "\t \r\n \n \t" . chr (11) . "\n";
        utf8::upgrade ($_);
        s/\H/H/g;
        s/\h/h/g;
        is($_, "hhHHhHhhHH", $message);
    }

    {
        # Various whitespace special patterns
        my @h = map {chr utf8::unicode_to_native($_) }
                             0x09,   0x20,   0xa0,   0x1680, 0x2000,
                             0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006,
                             0x2007, 0x2008, 0x2009, 0x200a, 0x202f, 0x205f,
                             0x3000;
        my @v = map {chr utf8::unicode_to_native($_) }
                             0x0a,   0x0b,   0x0c,   0x0d,   0x85, 0x2028,
                             0x2029;
        my @lb = (uni_to_native("\x0D\x0A"),
                             map {chr utf8::unicode_to_native($_) }
                                  0x0A .. 0x0D, 0x85, 0x2028, 0x2029);
        foreach my $t ([\@h,  qr/\h/, qr/\h+/],
                       [\@v,  qr/\v/, qr/\v+/],
                       [\@lb, qr/\R/, qr/\R+/],) {
            my $ary = shift @$t;
            foreach my $pat (@$t) {
                foreach my $str (@$ary) {
                    my $temp_str = $str;
                    $temp_str = display($temp_str);
                    ok $str =~ /($pat)/, $temp_str . " =~ /($pat)";
                    my $temp_1 = $1;
                    is($1, $str, "\$1='" . display($temp_1) . "' eq '" . $temp_str . "' after ($pat)");
                    utf8::upgrade ($str);
                    ok $str =~ /($pat)/, "Upgraded " . $temp_str . " =~ /($pat)/";
                    is($1, $str, "\$1='" . display($temp_1) . "' eq '" . $temp_str . "'(upgraded) after ($pat)");
                }
            }
        }
    }

    {
        # Check that \\xDF match properly in its various forms
        # Test that \xDF matches properly. this is pretty hacky stuff,
        # but its actually needed. The malarky with '-' is to prevent
        # compilation caching from playing any role in the test.
        my @df = (chr utf8::unicode_to_native(0xDF), '-', chr utf8::unicode_to_native(0xDF));
        utf8::upgrade ($df [2]);
        my @strs = ('ss', 'sS', 'Ss', 'SS', chr utf8::unicode_to_native(0xDF));
        my @ss = map {("$_", "$_")} @strs;
        utf8::upgrade ($ss [$_ * 2 + 1]) for 0 .. $#strs;

        for my $ssi (0 .. $#ss) {
            for my $dfi (0 .. $#df) {
                my $pat = $df [$dfi];
                my $str = $ss [$ssi];
                my $utf_df = ($dfi > 1) ? 'utf8' : '';
                my $utf_ss = ($ssi % 2) ? 'utf8' : '';
                my $sstr;   # We hard-code the ebcdic value below to avoid
                            # perturbing the test
                ($sstr = $str) =~ s/\xDF/\\xDF/ if $::IS_ASCII;
                ($sstr = $str) =~ s/\x59/\\x59/ if $::IS_EBCDIC;

                if ($utf_df || $utf_ss || length ($ss [$ssi]) == 1) {
                    my $ret = $str =~ /$pat/i;
                    next if $pat eq '-';
                    if ($::IS_ASCII) {
                        ok $ret, "\"$sstr\" =~ /\\xDF/i " .
                             "(str is @{[$utf_ss||'latin']}, pat is " .
                             "@{[$utf_df||'latin']})";
                    }
                    else {
                        ok $ret, "\"$sstr\" =~ /\\x59/i " .
                             "(str is @{[$utf_ss||'latin']}, pat is " .
                             "@{[$utf_df||'latin']})";
                    }
                }
                else {
                    my $ret = $str !~ /$pat/i;
                    next if $pat eq '-';
                    if ($::IS_EBCDIC) {
                        ok $ret, "\"$sstr\" !~ /\\x59/i " .
                             "(str is @{[$utf_ss||'latin']}, pat is " .
                             "@{[$utf_df||'latin']})";
                    }
                    else {
                        ok $ret, "\"$sstr\" !~ /\\xDF/i " .
                             "(str is @{[$utf_ss||'latin']}, pat is " .
                             "@{[$utf_df||'latin']})";
                    }
                }
            }
        }
    }

    {
        my $message = "BBC(Bleadperl Breaks CPAN) Today: String::Multibyte";
        my $re  = qr/(?:[\x00-\xFF]{4})/;
        my $hyp = "\0\0\0-";
        my $esc = "\0\0\0\\";

        my $str = "$esc$hyp$hyp$esc$esc";
        my @a = ($str =~ /\G(?:\Q$esc$esc\E|\Q$esc$hyp\E|$re)/g);

        is(@a,3, $message);
        local $" = "=";
        is("@a","$esc$hyp=$hyp=$esc$esc", $message);
    }

    {
        # Test for keys in %+ and %-
        my $message = 'Test keys in %+ and %-';
        no warnings 'uninitialized';
        local $_ = "abcdef";
        /(?<foo>a)|(?<foo>b)/;
        is((join ",", sort keys %+), "foo", $message);
        is((join ",", sort keys %-), "foo", $message);
        is((join ",", sort values %+), "a", $message);
        is((join ",", sort map "@$_", values %-), "a ", $message);
        /(?<bar>a)(?<bar>b)(?<quux>.)/;
        is((join ",", sort keys %+), "bar,quux", $message);
        is((join ",", sort keys %-), "bar,quux", $message);
        is((join ",", sort values %+), "a,c", $message); # leftmost
        is((join ",", sort map "@$_", values %-), "a b,c", $message);
        /(?<un>a)(?<deux>c)?/; # second buffer won't capture
        is((join ",", sort keys %+), "un", $message);
        is((join ",", sort keys %-), "deux,un", $message);
        is((join ",", sort values %+), "a", $message);
        is((join ",", sort map "@$_", values %-), ",a", $message);
    }

    {
        # length() on captures, the numbered ones end up in Perl_magic_len
        local $_ = "aoeu " . uni_to_native("\xe6") . "var ook";
        /^ \w+ \s (?<eek>\S+)/x;

        is(length $`,      0, q[length $`]);
        is(length $',      4, q[length $']);
        is(length $&,      9, q[length $&]);
        is(length $1,      4, q[length $1]);
        is(length $+{eek}, 4, q[length $+{eek} == length $1]);
    }

    {
        my $ok = -1;

        $ok = exists ($-{x}) ? 1 : 0 if 'bar' =~ /(?<x>foo)|bar/;
        is($ok, 1, '$-{x} exists after "bar"=~/(?<x>foo)|bar/');
        is(scalar (%+), 0, 'scalar %+ == 0 after "bar"=~/(?<x>foo)|bar/');
        is(scalar (%-), 1, 'scalar %- == 1 after "bar"=~/(?<x>foo)|bar/');

        $ok = -1;
        $ok = exists ($+{x}) ? 1 : 0 if 'bar' =~ /(?<x>foo)|bar/;
        is($ok, 0, '$+{x} not exists after "bar"=~/(?<x>foo)|bar/');
        is(scalar (%+), 0, 'scalar %+ == 0 after "bar"=~/(?<x>foo)|bar/');
        is(scalar (%-), 1, 'scalar %- == 1 after "bar"=~/(?<x>foo)|bar/');

        $ok = -1;
        $ok = exists ($-{x}) ? 1 : 0 if 'foo' =~ /(?<x>foo)|bar/;
        is($ok, 1, '$-{x} exists after "foo"=~/(?<x>foo)|bar/');
        is(scalar (%+), 1, 'scalar %+ == 1 after "foo"=~/(?<x>foo)|bar/');
        is(scalar (%-), 1, 'scalar %- == 1 after "foo"=~/(?<x>foo)|bar/');

        $ok = -1;
        $ok = exists ($+{x}) ? 1 : 0 if 'foo'=~/(?<x>foo)|bar/;
        is($ok, 1, '$+{x} exists after "foo"=~/(?<x>foo)|bar/');
    }

    {
        local $_;
        ($_ = 'abc') =~ /(abc)/g;
        $_ = '123';
        is("$1", 'abc', "/g leads to unsafe match vars: $1");

        fresh_perl_is(<<'EOP', ">abc<\n", {}, 'mention $&');
$&;
my $x; 
($x='abc')=~/(abc)/g; 
$x='123'; 
print ">$1<\n";
EOP

        fresh_perl_is(<<'EOP', ">abc<\n", {}, 'no mention of $&');
my $x; 
($x='abc')=~/(abc)/g; 
$x='123'; 
print ">$1<\n";
EOP
    }

    {
        # Message-ID: <20070818091501.7eff4831@r2d2>
        my $str = "";
        for (0 .. 5) {
            my @x;
            $str .= "@x"; # this should ALWAYS be the empty string
            'a' =~ /(a|)/;
            push @x, 1;
        }
        is(length $str, 0, "Trie scope error, string should be empty");
        $str = "";
        my @foo = ('a') x 5;
        for (@foo) {
            my @bar;
            $str .= "@bar";
            s/a|/push @bar, 1/e;
        }
        is(length $str, 0, "Trie scope error, string should be empty");
    }

    {
# more TRIE/AHOCORASICK problems with mixed utf8 / latin-1 and case folding
    for my $ord (160 .. 255) {
        my $chr = utf8::unicode_to_native($ord);
        my $chr_byte = chr($chr);
        my $chr_utf8 = chr($chr); utf8::upgrade($chr_utf8);
        my $rx = qr{$chr_byte|X}i;
        like($chr_utf8, $rx, "utf8/latin, codepoint $chr");
    }
    }

    {
        our $a = 3; "" =~ /(??{ $a })/;
        our $b = $a;
        is($b, $a, "Copy of scalar used for postponed subexpression");
    }

    {
        our @ctl_n = ();
        our @plus = ();
        our $nested_tags;
        $nested_tags = qr{
            <
               (\w+)
               (?{
                       push @ctl_n,$^N;
                       push @plus,$+;
               })
            >
            (??{$nested_tags})*
            </\s* \w+ \s*>
        }x;

        my $match = '<bla><blubb></blubb></bla>' =~ m/^$nested_tags$/;
        ok $match, 'nested construct matches';
        is("@ctl_n", "bla blubb", '$^N inside of (?{}) works as expected');
        is("@plus",  "bla blubb", '$+  inside of (?{}) works as expected');
    }

    SKIP: {
        # XXX: This set of tests is essentially broken, POSIX character classes
        # should not have differing definitions under Unicode.
        # There are property names for that.
        skip "Tests assume ASCII", 4 unless $::IS_ASCII;

        my @notIsPunct = grep {/[[:punct:]]/ and not /\p{IsPunct}/}
                                map {chr} 0x20 .. 0x7f;
        is(join ('', @notIsPunct), '$+<=>^`|~',
	   '[:punct:] disagrees with IsPunct on Symbols');

        my @isPrint = grep {not /[[:print:]]/ and /\p{IsPrint}/}
                            map {chr} 0 .. 0x1f, 0x7f .. 0x9f;
        is(join ('', @isPrint), "",
	   'IsPrint agrees with [:print:] on control characters');

        my @isPunct = grep {/[[:punct:]]/ != /\p{IsPunct}/}
                            map {chr} 0x80 .. 0xff;
        is(join ('', @isPunct), "\xa1\xa7\xab\xb6\xb7\xbb\xbf",    # ¡ « · » ¿
	   'IsPunct disagrees with [:punct:] outside ASCII');

        my @isPunctLatin1 = eval q {
            grep {/[[:punct:]]/u != /\p{IsPunct}/} map {chr} 0x80 .. 0xff;
        };
        skip "Eval failed ($@)", 1 if $@;
        skip "PERL_LEGACY_UNICODE_CHARCLASS_MAPPINGS set to 0", 1
              if !$ENV{PERL_TEST_LEGACY_POSIX_CC};
        is(join ('', @isPunctLatin1), '',
	   'IsPunct agrees with [:punct:] with explicit Latin1');
    }

    {
	# Tests for [#perl 71942]
        our $count_a;
        our $count_b;

        my $c = 0;
        for my $re (
#            [
#                should match?,
#                input string,
#                re 1,
#                re 2,
#                expected values of count_a and count_b,
#            ]
            [
                0,
                "xababz",
                qr/a+(?{$count_a++})b?(*COMMIT)(*FAIL)/,
                qr/a+(?{$count_b++})b?(*COMMIT)z/,
                1,
            ],
            [
                0,
                "xababz",
                qr/a+(?{$count_a++})b?(*COMMIT)\s*(*FAIL)/,
                qr/a+(?{$count_b++})b?(*COMMIT)\s*z/,
                1,
            ],
            [
                0,
                "xababz",
                qr/a+(?{$count_a++})(?:b|)?(*COMMIT)(*FAIL)/,
                qr/a+(?{$count_b++})(?:b|)?(*COMMIT)z/,
                1,
            ],
            [
                0,
                "xababz",
                qr/a+(?{$count_a++})b{0,6}(*COMMIT)(*FAIL)/,
                qr/a+(?{$count_b++})b{0,6}(*COMMIT)z/,
                1,
            ],
            [
                0,
                "xabcabcz",
                qr/a+(?{$count_a++})(bc){0,6}(*COMMIT)(*FAIL)/,
                qr/a+(?{$count_b++})(bc){0,6}(*COMMIT)z/,
                1,
            ],
            [
                0,
                "xabcabcz",
                qr/a+(?{$count_a++})(bc*){0,6}(*COMMIT)(*FAIL)/,
                qr/a+(?{$count_b++})(bc*){0,6}(*COMMIT)z/,
                1,
            ],


            [
                0,
                "aaaabtz",
                qr/a+(?{$count_a++})b?(*PRUNE)(*FAIL)/,
                qr/a+(?{$count_b++})b?(*PRUNE)z/,
                4,
            ],
            [
                0,
                "aaaabtz",
                qr/a+(?{$count_a++})b?(*PRUNE)\s*(*FAIL)/,
                qr/a+(?{$count_b++})b?(*PRUNE)\s*z/,
                4,
            ],
            [
                0,
                "aaaabtz",
                qr/a+(?{$count_a++})(?:b|)(*PRUNE)(*FAIL)/,
                qr/a+(?{$count_b++})(?:b|)(*PRUNE)z/,
                4,
            ],
            [
                0,
                "aaaabtz",
                qr/a+(?{$count_a++})b{0,6}(*PRUNE)(*FAIL)/,
                qr/a+(?{$count_b++})b{0,6}(*PRUNE)z/,
                4,
            ],
            [
                0,
                "aaaabctz",
                qr/a+(?{$count_a++})(bc){0,6}(*PRUNE)(*FAIL)/,
                qr/a+(?{$count_b++})(bc){0,6}(*PRUNE)z/,
                4,
            ],
            [
                0,
                "aaaabctz",
                qr/a+(?{$count_a++})(bc*){0,6}(*PRUNE)(*FAIL)/,
                qr/a+(?{$count_b++})(bc*){0,6}(*PRUNE)z/,
                4,
            ],

            [
                0,
                "aaabaaab",
                qr/a+(?{$count_a++;})b?(*SKIP)(*FAIL)/,
                qr/a+(?{$count_b++;})b?(*SKIP)z/,
                2,
            ],
            [
                0,
                "aaabaaab",
                qr/a+(?{$count_a++;})b?(*SKIP)\s*(*FAIL)/,
                qr/a+(?{$count_b++;})b?(*SKIP)\s*z/,
                2,
            ],
            [
                0,
                "aaabaaab",
                qr/a+(?{$count_a++;})(?:b|)(*SKIP)(*FAIL)/,
                qr/a+(?{$count_b++;})(?:b|)(*SKIP)z/,
                2,
            ],
            [
                0,
                "aaabaaab",
                qr/a+(?{$count_a++;})b{0,6}(*SKIP)(*FAIL)/,
                qr/a+(?{$count_b++;})b{0,6}(*SKIP)z/,
                2,
            ],
            [
                0,
                "aaabcaaabc",
                qr/a+(?{$count_a++;})(bc){0,6}(*SKIP)(*FAIL)/,
                qr/a+(?{$count_b++;})(bc){0,6}(*SKIP)z/,
                2,
            ],
            [
                0,
                "aaabcaaabc",
                qr/a+(?{$count_a++;})(bc*){0,6}(*SKIP)(*FAIL)/,
                qr/a+(?{$count_b++;})(bc*){0,6}(*SKIP)z/,
                2,
            ],


            [
                0,
                "aaddbdaabyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? b?  (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? b?  (*SKIP:T1) z \s* c \1 /x,
                4,
            ],
            [
                0,
                "aaddbdaabyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? b?  (*SKIP:T1) \s* (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? b?  (*SKIP:T1) \s* z \s* c \1 /x,
                4,
            ],
            [
                0,
                "aaddbdaabyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? (?:b|)  (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? (?:b|)  (*SKIP:T1) z \s* c \1 /x,
                4,
            ],
            [
                0,
                "aaddbdaabyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? b{0,6}  (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? b{0,6}  (*SKIP:T1) z \s* c \1 /x,
                4,
            ],
            [
                0,
                "aaddbcdaabcyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? (bc){0,6}  (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? (bc){0,6}  (*SKIP:T1) z \s* c \1 /x,
                4,
            ],
            [
                0,
                "aaddbcdaabcyzc",
                qr/a (?{$count_a++;}) (*MARK:T1) (a*) .*? (bc*){0,6}  (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;}) (*MARK:T1) (a*) .*? (bc*){0,6}  (*SKIP:T1) z \s* c \1 /x,
                4,
            ],


            [
                0,
                "aaaaddbdaabyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? b?   (*MARK:T1) (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? b?   (*MARK:T1) (*SKIP:T1) z \s* c \1 /x,
                2,
            ],
            [
                0,
                "aaaaddbdaabyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? b?   (*MARK:T1) (*SKIP:T1) \s* (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? b?   (*MARK:T1) (*SKIP:T1) \s* z \s* c \1 /x,
                2,
            ],
            [
                0,
                "aaaaddbdaabyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? (?:b|)   (*MARK:T1) (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? (?:b|)   (*MARK:T1) (*SKIP:T1) z \s* c \1 /x,
                2,
            ],
            [
                0,
                "aaaaddbdaabyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? b{0,6}   (*MARK:T1) (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? b{0,6}   (*MARK:T1) (*SKIP:T1) z \s* c \1 /x,
                2,
            ],
            [
                0,
                "aaaaddbcdaabcyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? (bc){0,6}   (*MARK:T1) (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? (bc){0,6}   (*MARK:T1) (*SKIP:T1) z \s* c \1 /x,
                2,
            ],
            [
                0,
                "aaaaddbcdaabcyzc",
                qr/a (?{$count_a++;})  (a?) (*MARK:T1) (a*) .*? (bc*){0,6}   (*MARK:T1) (*SKIP:T1) (*FAIL) \s* c \1 /x,
                qr/a (?{$count_b++;})  (a?) (*MARK:T1) (a*) .*? (bc*){0,6}   (*MARK:T1) (*SKIP:T1) z \s* c \1 /x,
                2,
            ],


            [
                0,
                "AbcdCBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) C? (*THEN)  | A D) (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) C? (*THEN)  | A D) z/x,
                1,
            ],
            [
                0,
                "AbcdCBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) C? (*THEN)  | A D) \s* (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) C? (*THEN)  | A D) \s* z/x,
                1,
            ],
            [
                0,
                "AbcdCBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) (?:C|) (*THEN)  | A D) (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) (?:C|) (*THEN)  | A D) z/x,
                1,
            ],
            [
                0,
                "AbcdCBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) C{0,6} (*THEN)  | A D) (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) C{0,6} (*THEN)  | A D) z/x,
                1,
            ],
            [
                0,
                "AbcdCEBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) (CE){0,6} (*THEN)  | A D) (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) (CE){0,6} (*THEN)  | A D) z/x,
                1,
            ],
            [
                0,
                "AbcdCBefgBhiBqz",
                qr/(A (.*)  (?{ $count_a++ }) (CE*){0,6} (*THEN)  | A D) (*FAIL)/x,
                qr/(A (.*)  (?{ $count_b++ }) (CE*){0,6} (*THEN)  | A D) z/x,
                1,
            ],
        ) {
            $c++;
            $count_a = 0;
            $count_b = 0;

            my $match_a = ($re->[1] =~ $re->[2]) || 0;
            my $match_b = ($re->[1] =~ $re->[3]) || 0;

            is($match_a, $re->[0], "match a " . ($re->[0] ? "succeeded" : "failed") . " ($c)");
            is($match_b, $re->[0], "match b " . ($re->[0] ? "succeeded" : "failed") . " ($c)");
            is($count_a, $re->[4], "count a ($c)");
            is($count_b, $re->[4], "count b ($c)");
        }
    }

    {   # Bleadperl v5.13.8-292-gf56b639 breaks NEZUMI/Unicode-LineBreak-1.011
        # \xdf in lookbehind failed to compile as is multi-char fold
        my $message = "Lookbehind with \\xdf matchable compiles";
        my $r = eval 'qr{
            (?u: (?<=^url:) |
                 (?<=[/]) (?=[^/]) |
                 (?<=[^-.]) (?=[-~.,_?\#%=&]) |
                 (?<=[=&]) (?=.)
            )}iox';
	is($@, '', $message);
	object_ok($r, 'Regexp', $message);
    }

    # RT #82610
    like 'foo/file.fob', qr,^(?=[^\.])[^/]*/(?=[^\.])[^/]*\.fo[^/]$,;

    {   # This was failing unless an explicit /d was added
        my $E0 = uni_to_native("\xE0");
        utf8::upgrade($E0);
        my $p = qr/[_$E0]/i;
        like(uni_to_native("\xC0"), qr/$p/, "Verify \"\\xC0\" =~ /[\\xE0_]/i; pattern in utf8");
    }

    like "x", qr/\A(?>(?:(?:)A|B|C?x))\z/,
        "Check TRIE does not overwrite EXACT following NOTHING at start - RT #111842";

    {
        my $single = "z";
        my $upper = "\x{390}";  # Fold is 3 chars.
        my $multi = CORE::fc($upper);

        my $failed = 0;

        # Try forcing a node to be split, with a multi-char fold at the
        # boundary
        for my $repeat (1 .. 300) {
            my $string = $single x $repeat;
            my $lhs = $string . $upper;
            if ($lhs !~ m/$string$multi/i) {
                $failed = $repeat;
                last;
            }
        }
        ok(! $failed, "Matched multi-char fold across EXACTFish node boundaries; if failed, was at count $failed");

        $failed = 0;
        for my $repeat (1 .. 300) {
            my $string = $single x $repeat;
            my $lhs = $string . "\N{LATIN SMALL LIGATURE FFI}";
            if ($lhs !~ m/${string}ff\N{LATIN SMALL LETTER I}/i) {
                $failed = $repeat;
                last;
            }
        }
        ok(! $failed, "Matched multi-char fold across EXACTFish node boundaries; if failed, was at count $failed");

        $failed = 0;
        for my $repeat (1 .. 300) {
            my $string = $single x $repeat;
            my $lhs = $string . "\N{LATIN SMALL LIGATURE FFL}";
            if ($lhs !~ m/${string}ff\N{U+6c}/i) {
                $failed = $repeat;
                last;
            }
        }
        ok(! $failed, "Matched multi-char fold across EXACTFish node boundaries; if failed, was at count $failed");

        # This tests that under /d matching that an 'ss' split across two
        # parts of a node doesn't end up turning into something that matches
        # \xDF unless it is in utf8.
        $failed = 0;
        $single = 'a';  # Is non-terminal multi-char fold char
        for my $repeat (1 .. 300) {
            my $string = $single x $repeat;
            my $lhs = "$string\N{LATIN SMALL LETTER SHARP S}";
            utf8::downgrade($lhs);
            $string .= "s";
            if ($lhs =~ m/${string}s/di) {
                $failed = $repeat;
                last;
            }
        }
        ok(! $failed, "Matched multi-char fold 'ss' across EXACTF node boundaries; if failed, was at count $failed");

        for my $non_finals ("t", "ft", "ift", "sift") {
            my $base_pat = $non_finals . "enKalt";   # (The tail is taken from
                                                     # the trouble ticket, is
                                                     # arbitrary)
            for my $utf8 ("non-UTF-8", "UTF-8") {

                # Try at different lengths to be sure to get a node boundary
                for my $repeat (120 .. 270) {   # [perl #133756]
                    my $head = ("b" x $repeat) . "\xDC";
                    my $pat = $base_pat;
                    utf8::upgrade($pat) if $utf8 eq "UTF-8";
                    $pat     = $head . $pat;
                    my $text = $head . $base_pat;

                    if ($text !~ /$pat/i) {
                        $failed = $repeat;
                        last;
                    }
                }

                ok(! $failed, "A non-final fold character "
                            . (length($non_finals) - 1)
                            . " characters from the end of an EXACTFish"
                            . " $utf8 pattern works; if failed, was at count $failed");
            }
        }
    }

    {
        fresh_perl_is('print eval "\"\x{101}\" =~ /[[:lower:]]/", "\n"; print eval "\"\x{100}\" =~ /[[:lower:]]/i", "\n";',
                      "1\n1",   # Both re's should match
                      {},
                      "get [:lower:] swash in first eval; test under /i in second");
    }

    {
        fresh_perl_is(<<'EOF',
                my $s = "\x{41c}";
                $s =~ /(.*)/ or die;
                $ls = lc $1;
                print $ls eq lc $s ? "good\n" : "bad: [$ls]\n";
EOF
            "good\n",
            {},
            "swash triggered by lc() doesn't corrupt \$1"
        );
    }

    {
        #' RT #119075
        no warnings 'regexp';   # Silence "has useless greediness modifier"
        local $@;
        eval { /a{0}?/; };
        ok(! $@,
            "PCRE regression test: No 'Quantifier follows nothing in regex' warning");

    }

    {
        unlike("\xB5", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        like("\xB6", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        unlike("\xB7", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        like("\xB5", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");
        unlike("\xB6", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");
        like("\xB7", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");

        unlike("_\xB5", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        like("_\xB6", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        unlike("_\xB7", qr/^_?\p{IsMyRuntimeProperty}\z/, "yadayada");
        like("_\xB5", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");
        unlike("_\xB6", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");
        like("_\xB7", qr/^_?\P{IsMyRuntimeProperty}\z/, "yadayada");
    }

    # These are defined later, so won't be known at regex compile time above
    sub IsMyRuntimeProperty {
        return "B6\n";
    }

    sub IsntMyRuntimeProperty {
        return "!B6\n";
    }

    {   # [perl 121777]
        my $regex;
        { package Some;
            # define a Unicode propertyIs_q
            sub Is_q
            {
                sprintf '%x', ord 'q'
            }
            $regex = qr/\p{Is_q}/;

            # If we uncomment the following line, prior to the patch that
            # fixed this, everything would work because we would have expanded
            # the property by the time the regex in the 'like' below got
            # compiled.
            #'q' =~ $regex;
        }

        like('q', $regex, 'User-defined property matches outside package');

        package Some {
            main::like('abcq', qr/abc$regex/, 'Run-time compiled in-package user-defined property matches');
        }
    }

    {   # From Lingua::Stem::UniNE; no ticket filed but related to #121778
        use utf8;
        my $word = 'рабта';
        $word =~ s{ (?:
                          ия  # definite articles for nouns:
                        | ът  # ∙ masculine
                        | та  # ∙ feminine
                        | то  # ∙ neutral
                        | те  # ∙ plural
                    ) $ }{}x;
        is($word, 'раб', "Handles UTF8 trie correctly");
    }

    { # [perl #122460]
        my $a = "rdvark";
        $a =~ /(?{})(?=[A-Za-z0-9_])a*?/g;
        is (pos $a, 0, "optimizer correctly thinks (?=...) is 0-length");
    }

    {   # [perl #123417] multi-char \N{...} tripping roundly
        use Cname;
        my $qr = qr$(\N{foo})$;
        "afoot" =~ eval "qr/$qr/";
        is "$1" || $@, "foo", 'multichar \N{...} stringified and retoked';
    }

    is (scalar split(/\b{sb}/, "Don't think twice.  It's all right."),
        2, '\b{wb} splits sentences correctly');

    ok "my/dir/audio_07.mp3" =~
     qr/(.*)\/(.*)\/(.*)\.(?<=(?=(?:\.(?!\d+\b)\w{1,4}$)$)\.)(.*)$()/,
     "[perl #133948]";


    # !!! NOTE!  Keep the following tests last -- they may crash perl

    print "# Tests that follow may crash perl\n";
    {
        eval '/\k/';
        like $@, qr/\QSequence \k... not terminated in regex;\E/,
           'Lone \k not allowed';
    }

    {
        my $message = "Substitution with lookahead (possible segv)";
        $_ = "ns1ns1ns1";
        s/ns(?=\d)/ns_/g;
        is($_, "ns_1ns_1ns_1", $message);
        $_ = "ns1";
        s/ns(?=\d)/ns_/;
        is($_, "ns_1", $message);
        $_ = "123";
        s/(?=\d+)|(?<=\d)/!Bang!/g;
        is($_, "!Bang!1!Bang!2!Bang!3!Bang!", $message);
    }

    { 
        # Earlier versions of Perl said this was fatal.
        my $message = "U+0FFFF shouldn't crash the regex engine";
        no warnings 'utf8';
        my $a = eval "chr(65535)";
        use warnings;
        my $warning_message;
        local $SIG{__WARN__} = sub { $warning_message = $_[0] };
        eval $a =~ /[a-z]/;
        ok(1, $message);  # If it didn't crash, it worked.
    }

    {   # Was looping
        watchdog(10);
        like("\x{00DF}", qr/[\x{1E9E}_]*/i, "\"\\x{00DF}\" =~ /[\\x{1E9E}_]*/i was looping");
        watchdog(0);
    }

    {   # Bug #90536, caused failed assertion
        unlike("s\N{U+DF}", qr/^\x{00DF}/i, "\"s\\N{U+DF}\", qr/^\\x{00DF}/i");
    }

    # User-defined Unicode properties to match above-Unicode code points
    sub Is_31_Bit_Super { return "110000\t7FFFFFFF\n" }
    sub Is_Portable_Super { return '!utf8::Any' }   # Matches beyond 32 bits

    {   # Assertion was failing on on 64-bit platforms; just didn't work on 32.
        no warnings qw(non_unicode portable);
        use Config;

        # We use 'ok' instead of 'like' because the warnings are lexically
        # scoped, and want to turn them off, so have to do the match in this
        # scope.
        if ($Config{uvsize} < 8) {
            like(chr(0x7FFF_FFFE), qr/\p{Is_31_Bit_Super}/,
                            "chr(0x7FFF_FFFE) can match a Unicode property");
            like(chr(0x7FFF_FFFF), qr/\p{Is_31_Bit_Super}/,
                            "chr(0x7FFF_FFFF) can match a Unicode property");
            my $p = qr/^[\x{7FFF_FFFF}]$/;
            like(chr(0x7FFF_FFFF), qr/$p/,
                    "chr(0x7FFF_FFFF) can match itself in a [class]");
            like(chr(0x7FFF_FFFF), qr/$p/, # Tests any caching
                    "chr(0x7FFF_FFFF) can match itself in a [class] subsequently");
        }
        else {
            no warnings 'overflow';
            like(chr(0x7FFF_FFFF_FFFF_FFFE), qr/\p{Is_Portable_Super}/,
                    "chr(0x7FFF_FFFF_FFFF_FFFE) can match a Unicode property");
            like(chr(0x7FFF_FFFF_FFFF_FFFF), qr/^\p{Is_Portable_Super}$/,
                    "chr(0x7FFF_FFFF_FFFF_FFFF) can match a Unicode property");

            my $p = eval 'qr/^\x{7FFF_FFFF_FFFF_FFFF}$/';
            like(chr(0x7FFF_FFFF_FFFF_FFFF), qr/$p/,
                    "chr(0x7FFF_FFFF_FFFF_FFFF) can match itself in a [class]");
            like(chr(0x7FFF_FFFF_FFFF_FFFF), qr/$p/, # Tests any caching
                    "chr(0x7FFF_FFFF_FFFF_FFFF) can match itself in a [class] subsequently");

            # This test is because something was declared as 32 bits, but
            # should have been cast to 64; only a problem where
            # sizeof(STRLEN) != sizeof(UV)
            unlike(chr(0x7FFF_FFFF_FFFF_FFFE), qr/\p{Is_31_Bit_Super}/,
                   "chr(0x7FFF_FFFF_FFFF_FFFE) shouldn't match a range ending in 0x7FFF_FFFF");
        }
    }

    { # [perl #112530], the code below caused a panic
        sub InFoo { "a\tb\n9\ta\n" }
        like(chr(0xA), qr/\p{InFoo}/,
                            "Overlapping ranges in user-defined properties");
    }

    { # [perl #125990], the final 2 tests below each caused a panic.
        # The \0's are not necessary; it could be a printable character
        # instead, but were in the ticket, so using them.
        my $sharp_s = chr utf8::unicode_to_native(0xdf);
        my $string        = ("\0" x 8)
                          . ($sharp_s x 3)
                          . ("\0" x 42)
                          .  "ý";
        my $folded_string = ("\0" x 8)
                          . ("ss" x 3)
                          . ("\0" x 42)
                          .  "ý";
        utf8::downgrade($string);
        utf8::downgrade($folded_string);

        use Cname;
        like($string, qr/$string/i, "LATIN SMALL SHARP S matches itself under /id");
        unlike($folded_string, qr/$string/i, "LATIN SMALL SHARP S doesn't match 'ss' under /di");
        like($folded_string, qr/\N{EMPTY-STR}$string/i, "\\N{} earlier than LATIN SMALL SHARP S transforms /di into /ui, matches 'ss'");
        like($folded_string, qr/$string\N{EMPTY-STR}/i, "\\N{} after LATIN SMALL SHARP S transforms /di into /ui, matches 'ss'");
    }

    {   # [perl #126606 crashed the interpreter
        use Cname;
        like("sS", qr/\N{EMPTY-STR}Ss|/i, '\N{} with empty branch alternation works');
        like("sS", qr'\N{EMPTY-STR}Ss|'i, '\N{} with empty branch alternation works');
    }

    { # Regexp:Grammars was broken:
  # http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2013-06/msg01290.html
        fresh_perl_like('use warnings; "abc" =~ qr{(?&foo){0}abc(?<foo>)}',
                        qr/Quantifier unexpected on zero-length expression/,
                        {},
                        'No segfault on qr{(?&foo){0}abc(?<foo>)}');
    }

    SKIP:
    {   # [perl #125826] buffer overflow in TRIE_STORE_REVCHAR
        # (during compilation, so use a fresh perl)
        $Config{uvsize} == 8
	  or skip("need large code-points for this test", 1);

	fresh_perl_is('/\x{E000000000}|/ and print qq(ok\n)', "ok\n", {},
		      "buffer overflow in TRIE_STORE_REVCHAR");
    }

    {
        fresh_perl_like('use warnings; s                        qr/Switch \(\?\(condition\)\.\.\. not terminated/,
                        {},
                        'No segfault [perl #126886]');
    }

    {
        # [perl 130010]  Downstream application texinfo started to report panics
        # as of commit a5540cf.

        runperl( prog => 'A::xx(); package A; sub InFullwidth{ return qq|\n| } sub xx { split /[^\s\p{InFullwidth}]/, q|x| }' );
        ok(! $?, "User-defined pattern did not cause panic [perl 130010]");
    }

    {   # [perl #133999]    Previously assertion failure
	fresh_perl_like('0 =~ /\p{nv:(\B(*COMMIT)C+)}/',
                        qr/No Unicode property value wildcard matches/,
                        {},
                        "Assertion failure with *COMMIT and wildcard property");
    }

    {   # [perl #134029]    Previously assertion failure
        fresh_perl_like('qr/\p{upper:]}|\337(?|ss)|)(?0/',
                        qr/Unicode property wildcard not terminated/,
                        {},
                        "Assertion failure with single character wildcard");
    }

    {   # [perl #134034]    Previously assertion failure
        fresh_perl_is('use utf8; q!Ȧिम한글💣΢ყაოსაა!=~/(?li)\b{wb}\B(*COMMIT)0/;',
                      "", {}, "*COMMIT caused positioning beyond EOS");
    }

    {   # [GH #17486]    Previously assertion failure
        fresh_perl_is('0=~/(?iaa)ss\337(?0)|/',
                      "", {}, "EXACTFUP node isn't changed into something else");
    }

    {   # [GH #17593]
        fresh_perl_is('qr/((?+2147483647))/',
                      "Invalid reference to group in regex; marked by <--"
                    . " HERE in m/((?+2147483647) <-- HERE )/ at - line 1.",
                      {}, "integer overflow, undefined behavior in ASAN");
        fresh_perl_is('qr/((?-2147483647))/',
                      "Reference to nonexistent group in regex; marked by <--"
                    . " HERE in m/((?-2147483647) <-- HERE )/ at - line 1.",
                      {}, "Large negative relative capture group");
        fresh_perl_is('qr/((?+18446744073709551615))/',
                      "Invalid reference to group in regex; marked by <--"
                    . " HERE in m/((?+18446744073709551615 <-- HERE ))/ at -"
                    . " line 1.",
                      {}, "Too large relative group number");
        fresh_perl_is('qr/((?-18446744073709551615))/',
                      "Invalid reference to group in regex; marked by <--"
                    . " HERE in m/((?-18446744073709551615 <-- HERE ))/ at -"
                    . " line 1.",
                      {}, "Too large negative relative group number");
    }

    {   # GH #17734, ASAN use after free
        fresh_perl_like('no warnings "experimental::uniprop_wildcards";
                         my $re = q<[[\p{name=/[Y-]+Z/}]]>;
                         eval { "\N{BYZANTINE MUSICAL SYMBOL PSILI}"
                                =~ /$re/ }; print $@ if $@; print "Done\n";',
                         qr/Done/,
                         {}, "GH #17734");
    }

    {   # GH $17278 assertion fails
        fresh_perl_is('use locale;
                       my $A_grave = "\N{LATIN CAPITAL LETTER A WITH GRAVE}";
                       my $a_grave = "\N{LATIN SMALL LETTER A WITH GRAVE}";

                       my $z="q!$a_grave! =~ m!(?^i)[$A_grave]!";
                       print eval $z, "\n";',
                       1,
                       {}, "GH #17278");
    }
    
    for my $try ( 1 .. 10 ) {
        # GH $19350 assertion fails - run 10 times as this bug is a heisenbug
        # and does not always fail, but should fail at least once in 10 tries.
        fresh_perl_is('use re Debug=>"ALL";qr{(?{a})(?<b>\g{c}})',
                      <<'EOF_DEBUG_OUT',
Assembling pattern from 2 elements
Compiling REx "(?{a})(?<b>\g{c}"
Starting parse and generation
<(?{a})(?<b>>...|   1|  reg    
                |    |    brnc   
                |    |      piec   
                |    |        atom   
<?{a})(?<b>\>...|    |          reg    
<(?<b>\g{c}>    |   4|      piec   
                |    |        atom   
<?<b>\g{c}>     |    |          reg    
                |    |            Setting open paren #1 to 4
<\g{c}>         |   6|            brnc   
                |    |              piec   
                |    |                atom   
<>              |   9|            tail~ OPEN1 'b' (4) -> REFN
                |    |            Setting close paren #1 to 9
                |  11|          lsbr~ tying lastbr REFN <1> (6) to ender CLOSE1 'b' (9) offset 3
                |    |            tail~ REFN <1> (6) -> CLOSE
Unmatched ( in regex; marked by <-- HERE in m/(?{a})( <-- HERE ?<b>\g{c}/ at - line 1.
Freeing REx: "(?{a})(?<b>\g{c}"
EOF_DEBUG_OUT
                      {rtrim_result=>1},
                      "Github Issue #19350, assert fail in "
                          . "Debug => 'ALL' from malformed qr// (heisenbug try $try)");
    }
    {   # Related to GH $19350 but segfaults instead of asserts, and does so reliably, not randomly.
        # use re Debug => "PARSE" is similar to "ALL", but does not include the optimize info, so we
        # do not need to deal with normlazing memory addresses in the output.
        fresh_perl_is(
                      'use re Debug=>"PARSE";qr{(?<b>\g{c})(?<c>x)(?&b)}',
                      <<'EOF_DEBUG_OUT',
Assembling pattern from 1 elements
Compiling REx "(?<b>\g{c})(?<c>x)(?&b)"
Starting parse and generation
<(?<b>\g{c})>...|   1|  reg    
                |    |    brnc   
                |    |      piec   
                |    |        atom   
<?<b>\g{c})(>...|    |          reg    
<\g{c})(?<c>>...|   3|            brnc   
                |    |              piec   
                |    |                atom   
<)(?<c>x)(?&b)> |   6|            tail~ OPEN1 'b' (1) -> REFN
                |   8|          lsbr~ tying lastbr REFN <1> (3) to ender CLOSE1 'b' (6) offset 3
                |    |            tail~ REFN <1> (3) -> CLOSE
<(?<c>x)(?&b)>  |    |      piec   
                |    |        atom   
<?<c>x)(?&b)>   |    |          reg    
<x)(?&b)>       |  10|            brnc
                |    |              piec   
                |    |                atom   
<)(?&b)>        |  12|            tail~ OPEN2 'c' (8) -> EXACT
                |  14|          lsbr~ tying lastbr EXACT <x> (10) to ender CLOSE2 'c' (12) offset 2
                |    |            tail~ EXACT <x> (10) -> CLOSE
<(?&b)>         |    |      tail~ OPEN1 'b' (1)  
                |    |          ~ REFN <1> (3)
                |    |          ~ CLOSE1 'b' (6) -> OPEN
                |    |      piec   
                |    |        atom   
<?&b)>          |    |          reg    
<>              |  17|      tail~ OPEN2 'c' (8)
                |    |          ~ EXACT <x> (10)
                |    |          ~ CLOSE2 'c' (12) -> GOSUB
                |  18|  lsbr~ tying lastbr OPEN1 'b' (1) to ender END (17) offset 16
                |    |    tail~ OPEN1 'b' (1)  
                |    |        ~ REFN <1> (3)
                |    |        ~ CLOSE1 'b' (6)
                |    |        ~ OPEN2 'c' (8)
                |    |        ~ EXACT <x> (10)
                |    |        ~ CLOSE2 'c' (12)
                |    |        ~ GOSUB1[+0:14] 'b' (14) -> END
Need to redo parse
Freeing REx: "(?<b>\g{c})(?<c>x)(?&b)"
Starting parse and generation
<(?<b>\g{c})>...|   1|  reg    
                |    |    brnc   
                |    |      piec   
                |    |        atom   
<?<b>\g{c})(>...|    |          reg    
<\g{c})(?<c>>...|   3|            brnc   
                |    |              piec   
                |    |                atom   
<)(?<c>x)(?&b)> |   6|            tail~ OPEN1 'b' (1) -> REFN
                |   8|          lsbr~ tying lastbr REFN2 'c' <1> (3) to ender CLOSE1 'b' (6) offset 3
                |    |            tail~ REFN2 'c' <1> (3) -> CLOSE
<(?<c>x)(?&b)>  |    |      piec   
                |    |        atom   
<?<c>x)(?&b)>   |    |          reg    
<x)(?&b)>       |  10|            brnc
                |    |              piec   
                |    |                atom   
<)(?&b)>        |  12|            tail~ OPEN2 'c' (8) -> EXACT
                |  14|          lsbr~ tying lastbr EXACT <x> (10) to ender CLOSE2 'c' (12) offset 2
                |    |            tail~ EXACT <x> (10) -> CLOSE
<(?&b)>         |    |      tail~ OPEN1 'b' (1)  
                |    |          ~ REFN2 'c' <1> (3)
                |    |          ~ CLOSE1 'b' (6) -> OPEN
                |    |      piec   
                |    |        atom   
<?&b)>          |    |          reg    
<>              |  17|      tail~ OPEN2 'c' (8)
                |    |          ~ EXACT <x> (10)
                |    |          ~ CLOSE2 'c' (12) -> GOSUB
                |  18|  lsbr~ tying lastbr OPEN1 'b' (1) to ender END (17) offset 16
                |    |    tail~ OPEN1 'b' (1)  
                |    |        ~ REFN2 'c' <1> (3)
                |    |        ~ CLOSE1 'b' (6)
                |    |        ~ OPEN2 'c' (8)
                |    |        ~ EXACT <x> (10)
                |    |        ~ CLOSE2 'c' (12)
                |    |        ~ GOSUB1[+0:14] 'b' (14) -> END
Required size 17 nodes
first at 3
Freeing REx: "(?<b>\g{c})(?<c>x)(?&b)"
EOF_DEBUG_OUT
                      {rtrim_result=>1},
                      "Related to Github Issue #19350, forward \\g{x} pattern segv under use re Debug => 'PARSE'");
    }

    {   # perl-security#140, read/write past buffer end
        fresh_perl_like('qr/\p{utf8::perl x}/',
                        qr/Illegal user-defined property name "utf8::perl x" in regex/,
                        {}, "perl-security#140");
        fresh_perl_is('qr/\p{utf8::_perl_surrogate}/', "",
                        {}, "perl-security#140");
    }

    {   # GH 20009
        my $x = "awesome quotes";
        utf8::upgrade($x);
        $x =~ s/^[\x{0301}\x{030C}]+//;
    }


    # !!! NOTE that tests that aren't at all likely to crash perl should go
    # a ways above, above these last ones.  There's a comment there that, like
    # this comment, contains the word 'NOTE'

    done_testing();
} # End of sub run_tests

1;
