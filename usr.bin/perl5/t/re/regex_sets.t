#!./perl

# This tests (?[...]).  XXX These are just basic tests, as full ones would be
# best done with an infrastructure change to allow getting out the inversion
# list of the constructed set and then comparing it character by character
# with the expected result.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib','.','../ext/re' );
    require './charset_tools.pl';
    require './loc_tools.pl';
}

skip_all_without_unicode_tables();

use strict;
use warnings;

$| = 1;

use utf8;

like("a", qr/(?[ [a]      # This is a comment
                    ])/, 'Can ignore a comment');
like("a", qr/(?[ [a]      # [[:notaclass:]]
                    ])/, 'A comment isn\'t parsed');
unlike(uni_to_native("\x85"), qr/(?[ \t ])/, 'NEL is white space');
like(uni_to_native("\x85"), qr/(?[ \t + \ ])/, 'can escape NEL to match');
like(uni_to_native("\x85"), qr/(?[ [\] ])/, '... including within nested []');
like("\t", qr/(?[ \t + \ ])/, 'can do basic union');
like("\cK", qr/(?[ \s ])/, '\s matches \cK');
unlike("\cK", qr/(?[ \s - \cK ])/, 'can do basic subtraction');
like(" ", qr/(?[ \s - \cK ])/, 'can do basic subtraction');
like(":", qr/(?[ [:] ])/, '[:] is not a posix class');
unlike("\t", qr/(?[ ! \t ])/, 'can do basic complement');
like("\t", qr/(?[ ! [ ^ \t ] ])/, 'can do basic complement');
unlike("\r", qr/(?[ \t ])/, '\r doesn\'t match \t ');
like("\r", qr/(?[ ! \t ])/, 'can do basic complement');
like("0", qr/(?[ [:word:] & [:digit:] ])/, 'can do basic intersection');
unlike("A", qr/(?[ [:word:] & [:digit:] ])/, 'can do basic intersection');
like("0", qr/(?[[:word:]&[:digit:]])/, 'spaces around internal [] aren\'t required');

like("a", qr/(?[ [a] | [b] ])/, '| means union');
like("b", qr/(?[ [a] | [b] ])/, '| means union');
unlike("c", qr/(?[ [a] | [b] ])/, '| means union');

like("a", qr/(?[ [ab] ^ [bc] ])/, 'basic symmetric difference works');
unlike("b", qr/(?[ [ab] ^ [bc] ])/, 'basic symmetric difference works');
like("c", qr/(?[ [ab] ^ [bc] ])/, 'basic symmetric difference works');

like("2", qr/(?[ ( ( \pN & ( [a] + [2] ) ) ) ])/, 'Nesting parens and grouping');
unlike("a", qr/(?[ ( ( \pN & ( [a] + [2] ) ) ) ])/, 'Nesting parens and grouping');

unlike("\x{17f}", qr/(?[ [k] + \p{Blk=ASCII} ])/i, '/i doesn\'t affect \p{}');
like("\N{KELVIN SIGN}", qr/(?[ [k] + \p{Blk=ASCII} ])/i, '/i does affect literals');

my $thai_or_lao = qr/(?[ \p{Thai} + \p{Lao} ])/;
my $thai_or_lao_digit = qr/(?[ \p{Digit} & $thai_or_lao ])/;
like("\N{THAI DIGIT ZERO}", $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
unlike(chr(ord("\N{THAI DIGIT ZERO}") - 1), $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
like("\N{THAI DIGIT NINE}", $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
unlike(chr(ord("\N{THAI DIGIT NINE}") + 1), $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
like("\N{LAO DIGIT ZERO}", $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
unlike(chr(ord("\N{LAO DIGIT ZERO}") - 1), $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
like("\N{LAO DIGIT NINE}", $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');
unlike(chr(ord("\N{LAO DIGIT NINE}") + 1), $thai_or_lao_digit, 'embedded qr/(?[ ])/ works');

my $ascii_word = qr/(?[ \w ])/a;
my $ascii_digits_plus_all_of_arabic = qr/(?[ \p{Arabic} + \p{Digit} & $ascii_word ])/;
like("9", $ascii_digits_plus_all_of_arabic, "/a, then interpolating and intersection works for ASCII in the set");
unlike("A", $ascii_digits_plus_all_of_arabic, "/a, then interpolating and intersection works for ASCII not in the set");
unlike("\N{BENGALI DIGIT ZERO}", $ascii_digits_plus_all_of_arabic, "/a, then interpolating and intersection works for non-ASCII not in either set");
unlike("\N{BENGALI LETTER A}", $ascii_digits_plus_all_of_arabic, "/a, then interpolating and intersection works for non-ASCII in one set");
like("\N{ARABIC LETTER HAMZA}", $ascii_digits_plus_all_of_arabic, "intersection has higher precedence than union");
like("\N{EXTENDED ARABIC-INDIC DIGIT ZERO}", $ascii_digits_plus_all_of_arabic, "intersection has higher precedence than union");

like("\r", qr/(?[ \p{lb=cr} ])/, '\r matches \p{lb=cr}');
unlike("\r", qr/(?[ ! \p{lb=cr} ])/, '\r doesnt match ! \p{lb=cr}');
like("\r", qr/(?[ ! ! \p{lb=cr} ])/, 'Two ! ! are the original');
unlike("\r", qr/(?[ ! ! ! \p{lb=cr} ])/, 'Three ! ! ! are the complement');
# left associatve

my $kelvin = qr/(?[ \N{KELVIN SIGN} ])/;
my $fold = qr/(?[ $kelvin ])/i;
like("\N{KELVIN SIGN}", $kelvin, '"\N{KELVIN SIGN}" matches compiled qr/(?[ \N{KELVIN SIGN} ])/');
unlike("K", $fold, "/i on outer (?[ ]) doesn't leak to interpolated one");
unlike("k", $fold, "/i on outer (?[ ]) doesn't leak to interpolated one");

my $kelvin_fold = qr/(?[ \N{KELVIN SIGN} ])/i;
my $still_fold = qr/(?[ $kelvin_fold ])/;
like("K", $still_fold, "/i on interpolated (?[ ]) is retained in outer without /i");
like("k", $still_fold, "/i on interpolated (?[ ]) is retained in outer without /i");

eval 'my $x = qr/(?[ [a] ])/; qr/(?[ $x ])/';
is($@, "", 'qr/(?[ [a] ])/ can be interpolated');

like("B", qr/(?[ [B] | ! ( [^B] ) ])/, "[perl #125892]");

like("a", qr/(?[ (?#comment) [a]])/, "Can have (?#comments)");

if (! is_miniperl() && locales_enabled('LC_CTYPE')) {
    my $utf8_locale = find_utf8_ctype_locale;
    SKIP: {
        skip("No LC_ALL on this platform", 8) unless locales_enabled('LC_ALL');
        skip("No utf8 locale available on this platform", 8) unless $utf8_locale;

        setlocale(&POSIX::LC_ALL, "C");
        use locale;

        $kelvin_fold = qr/(?[ \N{KELVIN SIGN} ])/i;
        my $single_char_class = qr/(?[ \: ])/;

        setlocale(&POSIX::LC_ALL, $utf8_locale);

        like("\N{KELVIN SIGN}", $kelvin_fold,
             '(?[ \N{KELVIN SIGN} ]) matches itself under /i in UTF8-locale');
        like("K", $kelvin_fold,
                '(?[ \N{KELVIN SIGN} ]) matches "K" under /i in UTF8-locale');
        like("k", $kelvin_fold,
                '(?[ \N{KELVIN SIGN} ]) matches "k" under /i in UTF8-locale');
        like(":", $single_char_class,
             '(?[ : ]) matches itself in UTF8-locale (a single character class)');

        setlocale(&POSIX::LC_ALL, "C");

        # These should generate warnings (the above 4 shouldn't), but like()
        # suppresses them, so the warnings tests are in t/lib/warnings/regexec
        $^W = 0;   # Suppress the warnings that occur when run by hand with
                   # the -w option
        like("\N{KELVIN SIGN}", $kelvin_fold,
             '(?[ \N{KELVIN SIGN} ]) matches itself under /i in C locale');
        like("K", $kelvin_fold,
                '(?[ \N{KELVIN SIGN} ]) matches "K" under /i in C locale');
        like("k", $kelvin_fold,
                '(?[ \N{KELVIN SIGN} ]) matches "k" under /i in C locale');
        like(":", $single_char_class,
             '(?[ : ]) matches itself in C locale (a single character class)');
    }
}

# Tests that no warnings given for valid Unicode digit range.
my $arabic_digits = qr/(?[ [ ٠ - ٩ ] ])/;
for my $char ("٠", "٥", "٩") {
    use charnames ();
    my @got = capture_warnings(sub {
                like("٠", $arabic_digits, "Matches "
                                                . charnames::viacode(ord $char));
            });
    is (@got, 0, "... without warnings");
}

# RT #126181: \cX behaves strangely inside (?[])
{
	no warnings qw(syntax regexp);

	eval { $_ = '/(?[(\c]) /'; qr/$_/ };
	like($@, qr/^Syntax error/, '/(?[(\c]) / should not panic');
	eval { $_ = '(?[\c#]' . "\n])"; qr/$_/ };
	like($@, qr/^Unexpected/, '/(?[(\c]) / should not panic');
	eval { $_ = '(?[(\c])'; qr/$_/ };
	like($@, qr/^Syntax error/, '/(?[(\c])/ should be a syntax error');
	eval { $_ = '(?[(\c]) ]\b'; qr/$_/ };
	like($@, qr/^Unexpected/, '/(?[(\c]) ]\b/ should be a syntax error');
	eval { $_ = '(?[\c[]](])'; qr/$_/ };
	like($@, qr/^Unexpected/, '/(?[\c[]](])/ should be a syntax error');
	like("\c#", qr/(?[\c#])/, '\c# should match itself');
	like("\c[", qr/(?[\c[])/, '\c[ should match itself');
	like("\c\ ", qr/(?[\c\])/, '\c\ should match itself');
	like("\c]", qr/(?[\c]])/, '\c] should match itself');
}

# RT #126481 !! with syntax error panics
{
    fresh_perl_like('qr/(?[ ! ! (\w])/',
                    qr/^Unmatched \(/, {},
                    'qr/(?[ ! ! (\w])/ doesnt panic');

    # The following didn't panic before, but easy to add this here with a
    # paren between the !!
    fresh_perl_like('qr/(?[ ! ( ! (\w)])/',
                    qr/^Unmatched \(/, {},
                    'qr/qr/(?[ ! ( ! (\w)])/');
}

{   # RT #129122
    my $pat = '(?[ ( [ABC] - [B] ) + ( [abc] - [b] ) + [def] ])';
    like("A", qr/$pat/, "'A' matches /$pat/");
    unlike("B", qr/$pat/, "'B' doesn't match /$pat/");
    like("C", qr/$pat/, "'C' matches /$pat/");
    unlike("D", qr/$pat/, "'D' doesn't match /$pat/");
    like("a", qr/$pat/, "'a' matches /$pat/");
    unlike("b", qr/$pat/, "'b' doesn't match /$pat/");
    like("c", qr/$pat/, "'c' matches /$pat/");
    like("d", qr/$pat/, "'d' matches /$pat/");
    like("e", qr/$pat/, "'e' matches /$pat/");
    like("f", qr/$pat/, "'f' matches /$pat/");
    unlike("g", qr/$pat/, "'g' doesn't match /$pat/");
}

{   # [perl #129322 ]  This crashed perl, so keep after the ones that don't
    my $pat = '(?[[!]&[0]^[!]&[0]+[a]])';
    like("a", qr/$pat/, "/$pat/ compiles and matches 'a'");
}

{   # [perl #132167]
    fresh_perl_is(
        'print "c" =~ qr/(?[ ( \p{Uppercase} ) + (\p{Lowercase} - ([a] + [b]))  ])/;',
        1, {},
        'qr/(?[ ( \p{Uppercase} ) + (\p{Lowercase} - ([a] + [b]))  ]) compiles and properly matches');
    fresh_perl_is(
        'print "b" =~ qr/(?[ ( \p{Uppercase} ) + (\p{Lowercase} - ([a] + [b]))  ])/;',
        "", {},
        'qr/(?[ ( \p{Uppercase} ) + (\p{Lowercase} - ([a] + [b]))  ]) compiles and properly matches');
}

{   # [perl #133889]    Caused assertion failure
    fresh_perl_like(
        'qr/(?[\P{Is0}])/', qr/\QUnknown user-defined property name "Is0"/, {}, "[perl #133889]");
}

{
    my $s = qr/(?x:(?[ [ x ] ]))/;
    like("x", qr/(?[ $s ])/ , "Modifier flags in interpolated set don't"
                            . " disrupt");
}

{   # GH #16779
    like("x", qr/(?[ (?^x:(?[ [x] ])) ])/ ,
         "Can use '^' flag in a nested call");
    like("x", qr/(?[ (?x-imns:(?[ [x] ])) ])/ ,
         "Can use various flags in a nested call");
}

done_testing();

1;
