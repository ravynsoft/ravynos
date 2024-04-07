# Test the /a, /d, etc regex modifiers
# For comprehensive tests, set $ENV{PERL_DEBUG_FULL_TEST} to some true value

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib', '../dist/if');
    require './loc_tools.pl';
}

use strict;
use warnings;
no warnings 'locale';   # Some /l tests use above-latin1 chars to make sure
                        # they work, even though they warn.
use Config;

plan('no_plan');

# Each case is a valid element of its hash key.  Choose, where available, at
# least one of each type: ASCII-range, non-ASCII range Latin-1, and above
# Latin1 range code point.
my %testcases = (
    '\w' => [ 0x16B ],
    '\d' => [ ord("0"), ord("1"), ord("2"), ord("3"), ord("4"), ord("5"),
              ord("6"), ord("7"), ord("8"), ord("9"),
              0x0662,
            ],
    '[:blank:]' => [ ord("\t"), ord(" "),
                     0x1680
                   ],
    '\s' => [ ord("\t"), ord("\n"), ord("\cK"), ord("\f"), ord("\r"),
              ord(" "),
              utf8::unicode_to_native(0x85),
              utf8::unicode_to_native(0xA0),
              0x2029,
            ],
    '[:graph:]' => [ 0x02C7 ],
    '[:lower:]' => [ ord("a"), ord("b"), ord("c"), ord("d"), ord("e"),
                     ord("f"), ord("g"), ord("h"), ord("i"), ord("j"),
                     ord("k"), ord("l"), ord("m"), ord("n"), ord("o"),
                     ord("p"), ord("q"), ord("r"), ord("s"), ord("t"),
                     ord("u"), ord("v"), ord("w"), ord("x"), ord("y"),
                     ord("z"),
                     0x0127 ],
    '[:punct:]' => [ ord('`'), ord('^'), ord('~'), ord('<'), ord('='),
                     ord('>'), ord('|'), ord('-'), ord(','), ord(';'),
                     ord(':'), ord('!'), ord('?'), ord('/'), ord('.'),
                     ord('"'), ord('('), ord(')'), ord('['), ord(']'),
                     ord('{'), ord('}'), ord('@'), ord('$'), ord('*'),
                     ord('\\'), ord('&'), ord('#'), ord('%'), ord('+'),
                     ord("'"),
                     0x055C
                   ],
    '[:upper:]' => [ ord("A"), ord("B"), ord("C"), ord("D"), ord("E"),
                     ord("F"), ord("G"), ord("H"), ord("I"), ord("J"),
                     ord("K"), ord("L"), ord("M"), ord("N"), ord("O"),
                     ord("P"), ord("Q"), ord("R"), ord("S"), ord("T"),
                     ord("U"), ord("V"), ord("W"), ord("X"), ord("Y"),
                     ord("Z"),
                     0x0126
                   ],
    '[:xdigit:]' => [ ord("0"), ord("1"), ord("2"), ord("3"), ord("4"),
                      ord("5"), ord("6"), ord("7"), ord("8"), ord("9"),
                      ord("A"), ord("B"), ord("C"), ord("D"), ord("E"),
                      ord("F"), ord("a"), ord("b"), ord("c"), ord("d"),
                      ord("e"), ord("f"),
                      0xFF15,
                    ],
);

if ($ENV{PERL_DEBUG_FULL_TEST}) {
    push @{$testcases{'[:cntrl:]'}}, utf8::unicode_to_native($_)
                                               for (0x00 .. 0x1F, 0x7F .. 0x9F);
    push @{$testcases{'[:blank:]'}}, utf8::unicode_to_native(0xA0);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xA1);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native($_)
                                                            for (0xA2 .. 0xA6);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xA7);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xA8);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xA9);
    push @{$testcases{'[:lower:]'}}, utf8::unicode_to_native(0xAA);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xAB);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native($_)
                                                            for (0xAC .. 0xB4);
    push @{$testcases{'[:lower:]'}}, utf8::unicode_to_native(0xB5);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xB6);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xB7);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xB8);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xB9);
    push @{$testcases{'[:lower:]'}}, utf8::unicode_to_native(0xBA);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xBB);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xBC);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xBD);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xBE);
    push @{$testcases{'[:punct:]'}}, utf8::unicode_to_native(0xBF);
    push @{$testcases{'[:upper:]'}}, utf8::unicode_to_native($_)
                                                            for (0xC0 .. 0xD6);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xD7);
    push @{$testcases{'[:upper:]'}}, utf8::unicode_to_native($_)
                                                            for (0xD8 .. 0xDE);
    push @{$testcases{'[:lower:]'}}, utf8::unicode_to_native($_)
                                                            for (0xDF .. 0xF6);
    push @{$testcases{'[:graph:]'}}, utf8::unicode_to_native(0xF7);
    push @{$testcases{'[:lower:]'}}, utf8::unicode_to_native($_)
                                                            for (0xF8 .. 0xF8);

    push @{$testcases{'[:alpha:]'}}, @{$testcases{'[:lower:]'}},
                                 @{$testcases{'[:upper:]'}};
    push @{$testcases{'[:alnum:]'}}, @{$testcases{'[:alpha:]'}},
                                 @{$testcases{'\d'}};
    push @{$testcases{'\w'}}, @{$testcases{'[:alnum:]'}}, ord("_");
    push @{$testcases{'[:print:]'}}, @{$testcases{'[:graph:]'}},
                                ord(" "),
                                utf8::unicode_to_native(0xA0);
}

@{$testcases{'[:digit:]'}} = @{$testcases{'\d'}};
@{$testcases{'[:space:]'}} = @{$testcases{'\s'}};
@{$testcases{'[:word:]'}}  = @{$testcases{'\w'}};

#use Data::Dumper;
#$Data::Dumper::Sortkeys = 1;
#print STDERR Dumper \%testcases;

my $utf8_locale;

my @charsets = qw(a d u aa);
my $locales_ok = locales_enabled([ 'LC_CTYPE', 'LC_ALL' ]);
if (! is_miniperl() && $locales_ok) {
    require POSIX;
    my $current_locale = POSIX::setlocale( &POSIX::LC_ALL, "C") // "";
    if ($current_locale eq 'C') {

        # test for d_setlocale is repeated here because this one is compile
        # time, and the one above is run time
        use if $Config{d_setlocale}, 'locale';

        # Some implementations don't have the 128-255 range characters all
        # mean nothing under the C locale (an example being VMS).  This is
        # legal, but since we don't know what the right answers should be,
        # skip the locale tests in that situation.
        for my $i (128 .. 255) {
            goto skip_adding_C_locale
                              if chr(utf8::unicode_to_native($i)) =~ /[[:print:]]/;
        }
        push @charsets, 'l';

    skip_adding_C_locale:

        # Use a pseudo-modifier 'L' to indicate to use /l with a UTF-8 locale
        $utf8_locale = find_utf8_ctype_locale();
        push @charsets, 'L' if defined $utf8_locale;
    }
}

# For each possible character set...
foreach my $charset (@charsets) {
    my $locale;
    my $charset_mod = lc $charset;
    my $charset_display;
    if ($charset_mod eq 'l') {
        $locale = POSIX::setlocale(&POSIX::LC_ALL, ($charset eq 'l')
                                            ? "C"
                                            : $utf8_locale
                           );
        die "Couldn't change locale" unless $locale;
        $charset_display = $charset_mod . " ($locale)";
    }
    else {
        $charset_display = $charset_mod;
    }

    # And in utf8 or not
    foreach my $upgrade ("", 'utf8::upgrade($a); ') {

        # reverse gets the, \w, \s, \d first.
        for my $class (reverse sort keys %testcases) {

            # The complement of \w is \W; of [:posix:] is [:^posix:]
            my $complement = $class;
            if ($complement !~ s/ ( \[: ) /$1^/x) {
                $complement = uc($class);
            }

            # For each test case
            foreach my $ord (@{$testcases{$class}}) {
                my $char = chr($ord);
                $char = ($char eq '$') ? '\$' : display($char);

                # > 255 already implies upgraded.  Skip the ones that don't
                # have an explicit upgrade.  This shows more clearly in the
                # output which tests are in utf8, or not.
                next if $ord > 255 && ! $upgrade;

                my $reason = "";    # Explanation output with each test
                my $neg_reason = "";
                my $match = 1;      # Calculated whether test regex should
                                    # match or not

                # Everything always matches in ASCII, or under /u, or under /l
                # with a UTF-8 locale
                if (utf8::native_to_unicode($ord) < 128
                    || $charset eq 'u'
                    || $charset eq 'L')
                {
                    $reason = "\"$char\" is a $class under /$charset_display";
                    $neg_reason = "\"$char\" is not a $complement under /$charset_display";
                }
                elsif ($charset eq "a" || $charset eq "aa") {
                    $match = 0;
                    $reason = "\"$char\" is non-ASCII, which can't be a $class under /$charset_display";
                    $neg_reason = "\"$char\" is non-ASCII, which is a $complement under /$charset_display";
                }
                elsif ($ord > 255) {
                    $reason = "\"$char\" is a $class under /$charset_display";
                    $neg_reason = "\"$char\" is not a $complement under /$charset_display";
                }
                elsif ($charset eq 'l') {

                    # We are using the C locale, which is essentially ASCII,
                    # but under utf8, the above-latin1 chars are treated as
                    # Unicode)
                    $reason = "\"$char\" is not a $class in the C locale under /$charset_mod";
                    $neg_reason = "\"$char\" is a $complement in the C locale under /$charset_mod";
                    $match = 0;
                }
                elsif ($upgrade) {
                    $reason = "\"$char\" is a $class in utf8 under /$charset_display";
                    $neg_reason = "\"$char\" is not a $complement in utf8 under /$charset_display";
                }
                else {
                    $reason = "\"$char\" is above-ASCII latin1, which requires utf8 to be a $class under /$charset_display";
                    $neg_reason = "\"$char\" is above-ASCII latin1, which is a $complement under /$charset_display (unless in utf8)";
                    $match = 0;
                }
                $reason = "; $reason" if $reason;
                $neg_reason = "; $neg_reason" if $neg_reason;

                my $op;
                my $neg_op;
                if ($match) {
                    $op = '=~';
                    $neg_op = '!~';
                }
                else {
                    $op = '!~';
                    $neg_op = '=~';
                }

                # In [...] or not
                foreach my $bracketed (0, 1) {
                    my $lb = "";
                    my $rb = "";
                    if ($bracketed) {

                        # Adds an extra char to the character class to make sure
                        # that the class doesn't get optimized away.  (Make
                        # sure to not use the character being tested.)
                        my $extra = ($char eq "_") ? ":" : "_";
                        $lb = ($bracketed) ? "[$extra" : "";
                        $rb = ($bracketed) ? ']' : "";
                    }
                    else {  # [:posix:] must be inside outer [ ]
                        next if $class =~ /\[/;
                    }

                    my $length = 10;    # For regexec.c regrepeat() cases by
                                        # matching more than one item
                    # Test both class and its complement, and with one or more
                    # than one item to match.
                    foreach my $eval (
                        qq[my \$a = "$char"; $upgrade\$a $op qr/ (?$charset_mod: $lb$class$rb ) /x],
                        qq[my \$a = "$char" x $length; $upgrade\$a $op qr/ (?$charset_mod: $lb$class$rb\{$length} ) /x],
                    ) {
                        ok (eval $eval, $eval . $reason);
                    }
                    foreach my $eval (
                        qq[my \$a = "$char"; $upgrade\$a $neg_op qr/ (?$charset_mod: $lb$complement$rb ) /x],
                        qq[my \$a = "$char" x $length; $upgrade\$a $neg_op qr/ (?$charset_mod: $lb$complement$rb\{$length} ) /x],
                    ) {
                        ok (eval $eval, $eval . $neg_reason);
                    }
                }

                next if $class ne '\w';

                # Test \b, \B at beginning and end of string
                foreach my $eval (
                    qq[my \$a = "$char"; $upgrade\$a $op qr/ (?$charset_mod: ^ \\b . ) /x],
                    qq[my \$a = "$char"; $upgrade\$a $op qr/ (?$charset_mod: . \\b \$) /x],
                ) {
                    ok (eval $eval, $eval . $reason);
                }
                foreach my $eval (
                    qq[my \$a = "$char"; $upgrade\$a $neg_op qr/(?$charset_mod: ^ \\B . ) /x],
                    qq[my \$a = "$char"; $upgrade\$a $neg_op qr/(?$charset_mod: . \\B \$ ) /x],
                ) {
                    ok (eval $eval, $eval . $neg_reason);
                }

                # Test \b, \B adjacent to a non-word char, both before it and
                # after.  We test with ASCII, Latin1 and Unicode non-word chars
                foreach my $space_ord (@{$testcases{'\s'}}) {

                    # This is an anomalous character, so skip.
                    next if $space_ord == ord("\n");

                    # Useless to try to test non-utf8 when the ord itself
                    # forces utf8
                    next if $space_ord > 255 && ! $upgrade;

                    my $space = display(chr $space_ord);

                    foreach my $eval (
                        qq[my \$a = "$space$char"; $upgrade\$a $op qr/ (?$charset_mod: . \\b . ) /x],
                        qq[my \$a = "$char$space"; $upgrade\$a $op qr/ (?$charset_mod: . \\b . ) /x],
                    ) {
                        ok (eval $eval, $eval . $reason . "; \"$space\" is not a \\w");
                    }
                    foreach my $eval (
                        qq[my \$a = "$space$char"; $upgrade\$a $neg_op qr/ (?$charset_mod: . \\B . ) /x],
                        qq[my \$a = "$char$space"; $upgrade\$a $neg_op qr/ (?$charset_mod: . \\B . ) /x],
                    ) {
                        ok (eval $eval, $eval . $neg_reason . "; \"$space\" is not a \\w");
                    }
                }

                # Test \b, \B in the middle of two nominally word chars, but
                # one or both may be considered non-word depending on range
                # and charset.
                foreach my $other_ord (@{$testcases{'\w'}}) {
                    next if $other_ord > 255 && ! $upgrade;
                    my $other = display(chr $other_ord);

                    # Determine if the other char is a word char in current
                    # circumstances
                    my $other_is_word = 1;
                    my $other_reason = "\"$other\" is a $class under /$charset_display";
                    my $other_neg_reason = "\"$other\" is not a $complement under /$charset_display";
                    if (utf8::native_to_unicode($other_ord) > 127
                        && $charset ne 'u' && $charset ne 'L'
                        && (($charset eq "a" || $charset eq "aa")
                            || ($other_ord < 256 && ($charset eq 'l' || ! $upgrade))))
                    {
                        $other_is_word = 0;
                        $other_reason = "\"$other\" is not a $class under /$charset_display";
                        $other_neg_reason = "\"$other\" is a $complement under /$charset_display";
                    }
                    my $both_reason = $reason;
                    $both_reason .= "; $other_reason" if $other_ord != $ord;
                    my $both_neg_reason = $neg_reason;
                    $both_neg_reason .= "; $other_neg_reason" if $other_ord != $ord;

                    # If both are the same wordness, then \b will fail; \B
                    # succeed
                    if ($match == $other_is_word) {
                        $op = '!~';
                        $neg_op = '=~';
                    }
                    else {
                        $op = '=~';
                        $neg_op = '!~';
                    }

                    foreach my $eval (
                        qq[my \$a = "$other$char"; $upgrade\$a $op qr/ (?$charset_mod: $other \\b $char ) /x],
                        qq[my \$a = "$char$other"; $upgrade\$a $op qr/ (?$charset_mod: $char \\b $other ) /x],
                    ) {
                        ok (eval $eval, $eval . $both_reason);
                    }
                    foreach my $eval (
                        qq[my \$a = "$other$char"; $upgrade\$a $neg_op qr/ (?$charset_mod: $other \\B $char ) /x],
                        qq[my \$a = "$char$other"; $upgrade\$a $neg_op qr/ (?$charset_mod: $char \\B $other ) /x],
                    ) {
                        ok (eval $eval, $eval . $both_neg_reason);
                    }

                    next if $other_ord == $ord;

                    # These start with the \b or \B.  They are included, based
                    # on source code analysis, to force the testing of the FBC
                    # (find_by_class) portions of regexec.c.
                    foreach my $eval (
                        qq[my \$a = "$other$char"; $upgrade\$a $op qr/ (?$charset_mod: \\b $char ) /x],
                        qq[my \$a = "$char$other"; $upgrade\$a $op qr/ (?$charset_mod: \\b $other ) /x],
                    ) {
                        ok (eval $eval, $eval . $both_reason);
                    }
                    foreach my $eval (
                        qq[my \$a = "$other$char"; $upgrade\$a $neg_op qr/ (?$charset_mod: \\B $char ) /x],
                        qq[my \$a = "$char$other"; $upgrade\$a $neg_op qr/ (?$charset_mod: \\B $other ) /x],
                    ) {
                        ok (eval $eval, $eval . $both_neg_reason);
                    }
                }
            } # End of each test case in a class
        } # End of \w, \s, ...
    } # End of utf8 upgraded or not
}

plan(curr_test() - 1);
